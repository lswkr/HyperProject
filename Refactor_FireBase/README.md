# FireBase 리팩터링안

기존 파일은 건드리지 않았다. 이 폴더는 `Source` 밖이라 UBT가 컴파일하지 않는다.

## 적용 방법
아래처럼 `_New`를 떼고 기존 파일 위치에 덮어쓰면 된다. (덮어쓰기 전에 커밋 권장)

| 이 폴더 | 원래 위치 |
|---|---|
| `HPGA_FireBase_New.h` | `Source/HyperProject/Public/AbilitySystem/Abilities/HPGA_FireBase.h` |
| `HPGA_FireBase_New.cpp` | `Source/HyperProject/Private/AbilitySystem/Abilities/HPGA_FireBase.cpp` |
| `HPGA_Fire_New.h` / `.cpp` | `.../HPGA_Fire.h` / `.cpp` |
| `HPGA_Fire_Projectile_New.h` / `.cpp` | `.../HPGA_Fire_Projectile.h` / `.cpp` |
| `HPGA_HybridFire_New.h` / `.cpp` | `.../HPGA_HybridFire.h` / `.cpp` |

## BP 값 유지 (중요)
- 이름이 같은 프로퍼티(FireMontage, FX 태그, 투사체 클래스, Primary/Secondary* 등)는 부모로 옮겨도 값이 유지된다.
- **Fire의 `DamageEffectClass` / `Damage`만 이름이 바뀐다** (`UHPDamageAbility` 상속을 끊고 `PrimaryEffectClass` / `PrimaryValue` 사용).
  `Config/DefaultEngine.ini`에 추가:

```ini
[CoreRedirects]
+PropertyRedirects=(OldName="/Script/HyperProject.HPGA_Fire.DamageEffectClass",NewName="/Script/HyperProject.HPGA_Fire.PrimaryEffectClass")
+PropertyRedirects=(OldName="/Script/HyperProject.HPGA_Fire.Damage",NewName="/Script/HyperProject.HPGA_Fire.PrimaryValue")
```
  리디렉터가 먹었는지 에디터에서 Fire 계열 GA BP를 열어 확인할 것. 적용 전 값 스크린샷 권장.
- `ReloadMontage`는 세 클래스 모두 미사용이라 제거. BP에 넣어둔 값은 무시된다.

## 구조
- `UHPGA_FireBase` (Abstract): Activate/End, 입력 해제, 발사 딜레이, 히트스캔/투사체 경로 전부, 서버 처리
- `UHPGA_Fire`: `FireOneShot → Fire_HitScan`
- `UHPGA_Fire_Projectile`: `FireOneShot → Fire_Projectile`, 서버 투사체 처리 뒤 궁 태그 + `EndAbility`
- `UHPGA_HybridFire`: `FireOneShot`에서 `IsAiming()` 분기, `StartFireDelay`에서 Primary/SecondaryFireInterval 선택

## 동작이 바뀌는 부분
1. **서버 수신은 Hybrid 방식 유지**: `_HitScan` / `_Projectile`을 둘 다 델리게이트에 등록하고 ApplicationTag(`Aiming` / `Normal`)로 거름.
   Fire / Fire_Projectile도 이 태그를 보내도록 바뀜 (기존엔 빈 태그). 태그 확인 뒤 TargetData 타입도 한 번 더 확인.
2. **`ConsumeClientReplicatedTargetData`를 투사체 경로에도 호출** (기존 Fire_Projectile / Hybrid 투사체는 누락).
   ⚠️ 아래 두 개는 Hybrid 기준으로 합치다 생긴 변경이라 원래 동작과 다름 (결정 필요):
   - Fire_Projectile의 SSR / Visual 투사체에도 팀원 무시가 적용됨 → `bIsForBoth`가 꺼진 아군 힐 투사체는 아군을 통과
   - Fire에서도 `LocalHead/BodyHitSoundCueTag`가 재생됨 (기존 Fire는 선언만 하고 안 씀)
3. **SSR 투사체를 Cost Commit 성공 후에 스폰** (기존엔 Commit 전에 스폰 → 탄약 없어도 발사 가능했음).
4. 투사체 경로 CombatInterface 조회 1회 → `MakeTargetData_Projectile(ProjectileSpawnPoint, EndLocation, Out)`.
   `Implements` 실패 시 `EndAbility` (기존엔 0벡터로 진행).
5. 히트스캔 로컬: **Commit 후 몽타주 재생** (기존엔 몽타주 먼저 → 탄약 없어도 몽타주 재생).
6. 몽타주 재생 속도를 `FireMontagePlayRate` / `AimingFireMontagePlayRate` 프로퍼티로. 각 자식 생성자에서 기존 값으로 초기화.
   - 예외: Hybrid 투사체 **서버** 몽타주가 1.0 → 2.0 (로컬과 통일).
7. `ApplyHitGameplayEffect`는 Hybrid 버전 기준 + Fire 쪽 기능 합침
   - 이펙트 Context에 HitResult 추가, Cue에 Instigator 설정 (Fire에만 있던 것)
   - 데미지/힐이 실제로 적용된 경우에만 Hit Cue / `Client_HitConfirm` (Hybrid는 적용 안 돼도 재생했음)
   - Local Head/Body 사운드 큐는 태그가 비어 있으면 생략
   - 디버그 로그(`Deal`, `HEAL`, `NanoBoosted`) 제거
8. SSR 투사체: Hybrid의 팀원 무시 + Fire_Projectile의 `IgnoreActorWhenMoving` 둘 다 적용.
   Visual 투사체 팀원 무시 설정이 Fire_Projectile에도 적용됨.
9. Hybrid의 `Fire_HitScan` / `Fire_Projectile` / `SendTargetDataToServer_HitScan` 안의 `IsAiming()` 체크 제거 (분기는 `FireOneShot`에서).

## 손대지 않은 것
- 비SSR 경로 (투사체 비SSR 스폰, 히트스캔 HitTime=0 시 서버 판정)
- `UHPCombatComponent::bUseServerSideRewind`가 `GetLifetimeReplicatedProps`에 없어 실제로 복제 안 되는 문제
- `UHPDamageAbility` 클래스 자체 (이제 아무도 상속 안 함, 필요 없으면 삭제)
- 서버 히트스캔: `DoesContainAiming`이 true인데 비조준 사격이면 서버 몽타주를 재생하지 않는 기존 로직 (그대로 유지)
- 컴파일 확인은 안 함
