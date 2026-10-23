#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GardenTypes.h"
#include "Planta.generated.h"

class UStaticMeshComponent;
class AGardenPlot;

UCLASS(Abstract, Blueprintable)
class GARDENMANIAC_API APlanta : public AActor
{
    GENERATED_BODY()
public:
    APlanta();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float Amount, FDamageEvent const& Event, AController* EventInstigator, AActor* Causer) override;
    UFUNCTION(BlueprintCallable) void Regar(float Amount);
    UFUNCTION(BlueprintPure) float GetRendimiento() const;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planta", meta=(ClampMin="1")) float MaxHealth = 100.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Health = 100.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) float Hydration = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planta", meta=(ClampMin="0")) float WaterDrain = 1.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Planta", meta=(ClampMin="0.1")) float ActionInterval = 2.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EGardenSector Sector = EGardenSector::PatioA;
    UPROPERTY() TWeakObjectPtr<AGardenPlot> Plot;
protected:
    UFUNCTION(BlueprintNativeEvent, Category="Planta") void Producir();
    virtual void Producir_Implementation();
    float ActionProgress = 0.f;
};

UCLASS(Blueprintable)
class GARDENMANIAC_API APlantaOfensiva : public APlanta
{
    GENERATED_BODY()
public:
    APlantaOfensiva();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ataque") float AttackDamage = 25.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ataque") float AttackRange = 1700.f;
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ataque") void Atacar();
    virtual void Atacar_Implementation();
protected:
    virtual void Producir_Implementation() override;
};

UCLASS(Blueprintable)
class GARDENMANIAC_API APlantaDefensiva : public APlanta
{
    GENERATED_BODY()
public:
    APlantaDefensiva();
};

UCLASS(Abstract, Blueprintable)
class GARDENMANIAC_API APlantaMiscelanea : public APlanta
{
    GENERATED_BODY()
protected:
    UFUNCTION(BlueprintNativeEvent, Category="Especial") void EjecutarEspecial();
    virtual void EjecutarEspecial_Implementation();
    virtual void Producir_Implementation() override;
};

UCLASS(Blueprintable)
class GARDENMANIAC_API APlantaGirasol : public APlantaMiscelanea
{
    GENERATED_BODY()
public:
    APlantaGirasol();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Economia") int32 Production = 25;
protected:
    virtual void EjecutarEspecial_Implementation() override;
};
