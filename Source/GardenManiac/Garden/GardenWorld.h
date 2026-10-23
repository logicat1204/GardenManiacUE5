#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GardenTypes.h"
#include "GardenWorld.generated.h"

class UStaticMeshComponent;
class APlanta;
class AZombie;

UCLASS(Blueprintable)
class GARDENMANIAC_API AGardenPlot : public AActor
{
    GENERATED_BODY()
public:
    AGardenPlot();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EGardenSector Sector = EGardenSector::PatioA;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bWater = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TWeakObjectPtr<APlanta> Plant;
};

UCLASS(Blueprintable)
class GARDENMANIAC_API AGardenDirector : public AActor
{
    GENERATED_BODY()
public:
    AGardenDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintPure) bool IsSectorUnlocked(EGardenSector Sector) const;
    UFUNCTION(BlueprintCallable) bool Spend(int32 Amount);
    UFUNCTION(BlueprintCallable) void AddSun(int32 Amount);
    UFUNCTION(BlueprintCallable) void DamageHouse(float Amount);
    UFUNCTION(BlueprintCallable) bool UpgradeGarden();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Sun = 250;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 Day = 1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 GardenLevel = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float HouseHealth = 500.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bDefeated = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bWaveActive = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float PreparationRemaining = 40.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Oleadas") float PreparationDuration = 35.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Oleadas") int32 PatioBDay = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Oleadas") int32 RoofDay = 5;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Oleadas") TSubclassOf<AZombie> NormalClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Oleadas") TSubclassOf<AZombie> SpecialClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Oleadas") TSubclassOf<AZombie> BossClass;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TArray<TObjectPtr<UGardenSeed>> Seeds;
    /** Optional eight authored seeds; invalid/missing entries use the prototype defaults. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Semillas", meta=(EditFixedOrder)) TArray<TObjectPtr<UGardenSeed>> SeedLoadout;
    UPROPERTY(VisibleAnywhere) TArray<TWeakObjectPtr<AZombie>> Zombies;
    FVector Origin = FVector(12000, 0, 0);
    static AGardenDirector* Find(const UWorld* World);
private:
    void BuildPrototype();
    void CreateSeeds();
    void SpawnZombie();
    int32 SpawnsRemaining = 0;
    int32 SpawnIndex = 0;
    float SpawnRemaining = 0.f;
    float PassiveRemaining = 0.f;
};

namespace GardenVisual
{
    void Color(UStaticMeshComponent* Mesh, const FLinearColor& Color);
}
