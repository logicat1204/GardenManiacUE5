#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GardenTypes.h"
#include "Zombie.generated.h"

class UStaticMeshComponent;
class AGardenPlayer;

/** Prototype follows an explicit lane route, including roof climbing. No NavMesh required. */
UCLASS(Abstract, Blueprintable)
class GARDENMANIAC_API AZombie : public AActor
{
    GENERATED_BODY()
public:
    AZombie();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UFUNCTION(BlueprintPure, Category="Grab") bool IsGrabbingPlayer() const;
    UFUNCTION(BlueprintCallable, Category="Grab") bool TryGrabPlayer(AGardenPlayer* Player);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grab") bool bCanGrab = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grab", meta=(ClampMin="0")) float GrabRange = 135.f;
    UFUNCTION(BlueprintImplementableEvent, Category="Grab") void OnGrabStarted(AGardenPlayer* Player);
    UFUNCTION(BlueprintImplementableEvent, Category="Grab") void OnGrabEnded();
    virtual float TakeDamage(float Amount, FDamageEvent const& Event, AController* EventInstigator, AActor* Causer) override;
    UFUNCTION(BlueprintNativeEvent, Category="Zombie") void HabilidadEspecial(float DeltaSeconds);
    virtual void HabilidadEspecial_Implementation(float DeltaSeconds);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie") float MaxHealth = 100.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Health = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie") float Speed = 65.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie") float AttackDamage = 15.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie", meta=(ClampMin="0.1")) float AttackInterval = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Zombie") int32 Reward = 15;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EGardenSector Sector = EGardenSector::PatioA;
    UPROPERTY(BlueprintReadWrite) TArray<FVector> Route;
private:
    UPROPERTY() TWeakObjectPtr<AGardenPlayer> GrabbedPlayer;
    int32 Waypoint = 0;
    float AttackRemaining = 0.f;
};

UCLASS(Blueprintable)
class GARDENMANIAC_API AZombieNormal : public AZombie
{
    GENERATED_BODY()
};

UCLASS(Blueprintable)
class GARDENMANIAC_API AZombieEspecial : public AZombie
{
    GENERATED_BODY()
public:
    AZombieEspecial();
};

UCLASS(Blueprintable)
class GARDENMANIAC_API AZombieBoss : public AZombie
{
    GENERATED_BODY()
public:
    AZombieBoss();
};
