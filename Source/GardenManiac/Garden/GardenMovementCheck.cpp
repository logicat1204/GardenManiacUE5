#if WITH_DEV_AUTOMATION_TESTS
#include "GardenPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "TimerManager.h"

// Run in a separate -game process with -ExecCmds=Garden.VerifyMovement. Exits with test status.
static FAutoConsoleCommandWithWorld MovementCheck(TEXT("Garden.VerifyMovement"),
    TEXT("Check real keyboard movement in the loaded garden and exit the test process."),
    FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
    {
        if (!World || !World->IsGameWorld()) return;
        FTimerHandle StartHandle;
        World->GetTimerManager().SetTimer(StartHandle, FTimerDelegate::CreateWeakLambda(World, [World]()
        {
            APlayerController* PC = World->GetFirstPlayerController();
            AGardenPlayer* Player = PC ? Cast<AGardenPlayer>(PC->GetPawn()) : nullptr;
            if (!Player) { UE_LOG(LogTemp, Error, TEXT("GardenMovement: missing possessed player")); FPlatformMisc::RequestExitWithStatus(false, 1); return; }
            const FVector Start = Player->GetActorLocation();
            PC->InputKey(FInputKeyEventArgs(nullptr, IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(), EKeys::W, IE_Pressed, 0u));
            FTimerHandle EndHandle;
            World->GetTimerManager().SetTimer(EndHandle, FTimerDelegate::CreateWeakLambda(PC, [PC, Player, Start]()
            {
                const FVector End = Player->GetActorLocation();
                UE_LOG(LogTemp, Display, TEXT("GardenMovement: start=%s end=%s W=%d ignore=%d mode=%d velocity=%s"),
                    *Start.ToString(), *End.ToString(), PC->IsInputKeyDown(EKeys::W), PC->IsMoveInputIgnored(),
                    static_cast<int32>(Player->GetCharacterMovement()->MovementMode), *Player->GetVelocity().ToString());
                FHitResult Hit;
                FCollisionQueryParams Params(SCENE_QUERY_STAT(GardenMovementCheck), false, Player);
                Player->GetWorld()->SweepSingleByChannel(Hit, End, End + FVector(100,0,0), FQuat::Identity,
                    ECC_Pawn, FCollisionShape::MakeCapsule(34.f,88.f), Params);
                UE_LOG(LogTemp, Display, TEXT("GardenMovement: componentActive=%d tick=%d acceleration=%s blocker=%s penetrating=%d depth=%.2f"),
                    Player->GetCharacterMovement()->IsActive(), Player->GetCharacterMovement()->IsComponentTickEnabled(),
                    *Player->GetCharacterMovement()->GetCurrentAcceleration().ToString(), *GetNameSafe(Hit.GetActor()), Hit.bStartPenetrating, Hit.PenetrationDepth);
                PC->InputKey(FInputKeyEventArgs(nullptr, IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(), EKeys::W, IE_Released, 0u));
                const bool Passed = End.X - Start.X > 100.f;
                UE_LOG(LogTemp, Display, TEXT("GardenMovement: %s"), Passed ? TEXT("PASS") : TEXT("FAIL"));
                FPlatformMisc::RequestExitWithStatus(false, Passed ? 0 : 1);
            }), .6f, false);
        }), 1.f, false);
    }));
#endif
