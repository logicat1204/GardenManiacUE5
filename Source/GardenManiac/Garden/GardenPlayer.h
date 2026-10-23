#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/HUD.h"
#include "GardenTypes.h"
#include "GardenPlayer.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class AZombie;

UCLASS(Blueprintable)
class GARDENMANIAC_API AGardenPlayer : public ACharacter
{
    GENERATED_BODY()
public:
    AGardenPlayer();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UFUNCTION(BlueprintCallable) void SelectSlot(int32 Index);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tools") TObjectPtr<UStaticMeshComponent> ShovelMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tools") TObjectPtr<UStaticMeshComponent> WateringCanMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tools") TObjectPtr<UPointLightComponent> ToolFillLight;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 SelectedSlot = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) EGardenTool Tool = EGardenTool::Semilla;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Feedback;
    AActor* GetAimedActor(FHitResult& Hit) const;
    float GetCooldown(int32 Index) const;
    UFUNCTION(BlueprintPure, Category="Grab") bool IsGrabbed() const { return bGrabbed; }
    UFUNCTION(BlueprintPure, Category="Grab") AZombie* GetGrabbingZombie() const;
    bool TryBeginGrab(AZombie* Zombie);
    void ReleaseGrab();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grab") float GrabSecondsRemaining = 0.f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grab") bool bKilledByZombie = false;
    UFUNCTION(BlueprintImplementableEvent, Category="Grab") void OnGrabStarted(AZombie* Zombie);
    UFUNCTION(BlueprintImplementableEvent, Category="Grab") void OnGrabReleased();
private:
    UPROPERTY() TWeakObjectPtr<AZombie> GrabbingZombie;
    bool bGrabbed = false;
    float RescueGraceRemaining = 0.f;
    void Use();
    void StartUse();
    void StopUse();
    void Shovel();
    void WateringCan();
    void NextSlot();
    void PreviousSlot();
    void Upgrade();
    void RestartAfterDefeat();
    bool bUsing = false;
    float UseRemaining = 0.f;
    float FeedbackRemaining = 0.f;
    TArray<float> SlotCooldowns;
    void Notify(const FString& Message);
    void UpdateToolVisuals(float DeltaSeconds);
    float ShovelSwingRemaining = 0.f;
    float WateringTilt = 0.f;
};

UCLASS()
class GARDENMANIAC_API AGardenHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
