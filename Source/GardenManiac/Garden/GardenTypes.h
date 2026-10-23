#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GardenTypes.generated.h"

class APlanta;

UENUM(BlueprintType)
enum class EGardenSector : uint8 { PatioA, PatioB, Tejado };

UENUM(BlueprintType)
enum class EGardenTool : uint8 { Semilla, Pala, Regadera };

/** Seed data is independent from the planted actor. Create data assets for new species. */
UCLASS(BlueprintType)
class GARDENMANIAC_API UGardenSeed : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<APlanta> PlantClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0")) int32 Cost = 50;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0")) float Cooldown = 2.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bAquatic = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FLinearColor Color = FLinearColor::Green;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1")) float ActionIntervalMultiplier = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1")) float DamageMultiplier = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1")) float ProductionMultiplier = 1.f;
};
