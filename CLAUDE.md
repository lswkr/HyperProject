# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 개요

Overwatch에서 모티브를 얻은 하이퍼 TPS 멀티플레이 게임 (Unreal Engine 5.5, C++ 단일 모듈 `HyperProject`).
상용 에셋은 저장소에 없으므로 코드만 존재한다. 대부분의 수치/클래스 연결(어빌리티 맵, 이펙트, 투사체 클래스 등)은 `EditDefaultsOnly`로 노출되어 BP/데이터 에셋에서 지정되며, C++만 보고는 어떤 BP가 어떤 값을 쓰는지 알 수 없다.

## 빌드 / 실행

- 타겟: `HyperProjectEditor`(에디터), `HyperProject`(게임), `HyperProjectServer`(데디케이티드 서버). 모두 `Source/*.Target.cs`.
- 빌드: `HyperProject.sln`(Visual Studio/Rider) 또는 UBT 직접 호출
  `"<UE_5.5>/Engine/Build/BatchFiles/Build.bat" HyperProjectEditor Win64 Development -Project="<repo>/HyperProject.uproject" -WaitMutex`
- 로컬 테스트 실행: `launchScripts/launchServer.bat`, `launchScripts/launchGame.bat` (`%UNREAL_EDITOR%` 환경변수 필요).
  - 서버는 커맨드라인 인자 `-SESSION_NAME`, `-SESSION_SEARCH_ID`, `-PORT`로 세션 정보를 받는다.
  - 클라는 `-TESTING_URL="127.0.0.1:7777"`로 직접 접속 테스트 가능.
  - 인자 파싱은 `UHPNetStatics`(`Network/HPNetStatics`)에 모여 있다.
- 테스트 코드는 없다.

## 아키텍처

모든 클래스는 `Public/<폴더>/X.h` ↔ `Private/<폴더>/X.cpp` 1:1 대응. 접두사는 `HP`.

### GAS (Gameplay Ability System) 중심 구조
- `AHPCharacterBase`가 `UHPAbilitySystemComponent`(ASC)와 `UHPAttributeSet`을 **캐릭터 자체에** 소유한다 (PlayerState 아님). Owner/Avatar 모두 캐릭터.
- 초기화 흐름:
  - 서버: `AHPPlayerController::OnPossess` → `AHPPlayerCharacter::ServerSideInit` → `UHPAbilitySystemComponent::ServerSideInit` (DataTable 기반 기본 스탯 → 초기 이펙트 → 어빌리티 부여 → `AbilitiesGivenDelegate`).
  - 클라: `AHPPlayerController::AcknowledgePossession` → `ClientSideInit` → `AHPHUD::InitOverlay`로 UI 바인딩.
  - 클라에서 어빌리티 부여 완료는 `OnRep_ActivateAbilities`에서 `AbilitiesGivenDelegate`로 통지된다.
- 어빌리티 입력은 `EHPAbilityInputID`(`AbilitySystem/HPGameplayAbilityTypes.h`)를 InputID로 사용. 캐릭터의 `GameplayAbilityInputActions` 맵이 Enhanced Input 액션 ↔ InputID를 연결한다.
- 무기 교체 시 `UHPAbilitySystemComponent::OnEquipWeapon`이 이전 무기 어빌리티를 제거하고 `FHPWeaponInfo::WeaponAbilityMap`의 어빌리티를 새로 부여, 탄약은 `SetByCaller.MaxBullet` GE로 설정한다. 즉 **공격 어빌리티는 캐릭터가 아니라 무기(`AHPWeaponBase`)에 붙는다.**
- 공통 스탯/패시브/사망 이펙트는 `UHPAbilitySystemDataAsset`, 캐릭터 선택 정보는 `UPDA_CharacterDefinition`(PrimaryAsset, `HPAssetManager`로 로드).
- 네이티브 게임플레이 태그는 `FHPGameplayTags`(`HPGameplayTags.h/.cpp`)에 싱글톤으로 등록. 새 태그는 여기 추가. 전투 상태(`ECombatState`)도 태그로 판정한다.
- 어빌리티 계층: `UHPGameplayAbility` → `UHPDamageAbility` → `HPGA_Fire*`. 사격 계열(`HPGA_Fire`, `HPGA_Fire_Aiming`, `HPGA_Fire_Projectile`, `HPGA_HybridFire`)은 공통 패턴을 따른다: 로컬에서 TargetData 생성 → `SendTargetDataToServer` → 서버 `OnServerReceiveTargetData`에서 GE 적용. 커스텀 TargetData `FGameplayAbilityTargetData_HPCustom`은 HitTime/StartPoint를 담아 서버 되감기에 쓰인다.
- VFX/SFX는 GameplayCue Notify(`FX/GCNB_*`, `HPGameplayCueNS_BulletTrail*`)로 처리.

### 랙 보상 (Server-Side Rewind)
- `ULagCompensationComponent`가 서버에서 캐릭터별 히트박스(`AHPPlayerCharacter`의 본별 `UBoxComponent`들, `HitCollisionBoxes`) 프레임 히스토리를 저장하고 `HitTime` 기준으로 되감아 판정한다. 히트스캔/투사체/폭발/생성형 투사체별로 별도 `*ServerApplyValidHit` Server RPC가 있다.
- SSR 사용 여부는 핑에 따라 동적으로 결정된다: `AHPPlayerController`가 주기적으로 핑을 체크해 `HighPingDelegate` → `UHPCombatComponent::ShouldUseServerSideRewind`. 어빌리티는 `ICombatInterface::Execute_IsUsingServerRewind`로 조회.
- 투사체는 두 경로가 있다: SSR 사용 시 로컬 클라가 `ServerSideRewindProjectileClass`(비복제)를 스폰해 히트를 판정 후 RPC로 서버 검증, 미사용 시 서버가 권위 투사체를 스폰. `AHPVisualProjectile`은 순수 연출용. `bReplicates`는 두 용도의 BP에서 각각 설정하므로 생성자에서 지정하지 않는다.
- 데미지/이펙트 적용은 서버 권한. 투사체 이펙트 대상은 `EEffectApplyTargetPolicy`와 `IGenericTeamAgentInterface` 팀 ID로 아군/적군을 구분한다 (`AHPProjectileBase`의 Enemy/Team EffectClass).

### 무기 / 투사체 / 스폰 액터 (`Weapons/`)
- `AHPProjectileBase` 하위: `HPLinearProjectile`, `HPBallisticProjectile`, `HPPointEffectProjectile`(단일 대상), `HPRadialEffectProjectile`(범위), `HPSpawningProjectile`(착탄 시 액터 생성).
- `AAbilitySpawnableActor` 하위: 지뢰(`_Mine`), 중력자탄(`SpawnableActor_GravitonSurge`).

### 게임 모드 / 매치 흐름 (`GameMode/`, `Controller/`, `Network/`)
- MainMenu(`MainMenuGameMode`, EOS 로그인/세션 검색) → Lobby(`LobbyGameMode`, `AHPGameState`가 팀 슬롯/캐릭터 선택 `FPlayerSelection` 복제) → 인게임(`AControlPointGameMode` 거점 점령 모드, 또는 `AHPGameMode`).
- 캐릭터 선택은 `AHPPlayerState`에 저장되고 `CopyProperties`로 Seamless Travel 시 인게임까지 유지된다. 팀 ID는 슬롯 번호에서 파생(`GetTeamIdBasedOnSlot`).
- `AControlPointGameMode`는 `EControlPointGameModeState` 상태머신 + 타이머로 진행하고, `AHPControlPoint`의 점령 완료 델리게이트를 받는다. 거점 상태는 `FHPControlPointData`로 `AControlPointGameState`를 통해 클라에 전달.
- `*Test*` 접미사 클래스(`ControlPointTestGameMode`, `MainMenuTestGameMode`, `HPGameTestInstance`, `MainMenuTestWidget`)는 EOS/코디네이터 없이 로컬 테스트용.
- 온라인: OnlineSubsystemEOS. `UHPGameInstance`가 세션 생성 요청을 HTTP로 코디네이터(`Coordinator/coordinator.py`, Flask `/Sessions`)에 보내면 코디네이터가 데디케이티드 서버 프로세스를 띄우고, 클라는 `SESSION_SEARCH_ID`로 생성된 세션을 찾아 접속한다. 배포 구성은 `ServerDeploy/`(docker-compose).

### UI (`UI/`)
- MVC 형태: `AHPHUD::InitOverlay` → `FWidgetControllerParams`(PC/ASC/AttributeSet/CombatComponent)로 `UHPWidgetController` 하위(`OverlayWidgetController`, `OverHeadWidgetController`) 생성 → `UHPUserWidget::SetWidgetController`. 위젯 컨트롤러가 ASC 어트리뷰트 변경 델리게이트에 바인딩(`BindCallbacksToDependencies`)하고 `BroadcastInitialValues`로 초기값을 보낸다.
- 쿨다운 UI는 `UWaitCooldownChange` 비동기 태스크 사용.

## 코드 컨벤션
- 주석은 한국어. 서버 전용 로직은 `HasAuthority()` 체크, 로컬 판정은 `IsLocallyControlled()` 체크를 함수 초입에 두는 패턴을 따른다.
- 헤더 섹션은 `/* Xxx Begin */ ... /* Xxx End */` 주석으로 구분한다.
