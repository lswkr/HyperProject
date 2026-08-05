#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	TurningLeft		UMETA(DisplayName = "Turning Left"),
	TurningRight		UMETA(DisplayName = "Turning Right"),
	NotTurning UMETA(DisplayName = "Not Turning"),

	DefaultMAX		UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Unoccupied      UMETA(DisplayName = "Unoccupied"),
	Reloading		UMETA(DisplayName = "Reloading"),
	SwappingWeapons UMETA(DisplayName = "SwappingWeapons"),

	DefaultMAX		UMETA(DisplayName = "DefaultMAX")
};