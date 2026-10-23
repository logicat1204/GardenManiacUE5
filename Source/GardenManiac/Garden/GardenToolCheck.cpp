#if WITH_DEV_AUTOMATION_TESTS
#include "GardenPlayer.h"
#include "GardenWorld.h"
#include "Planta.h"
#include "Zombie.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "InputKeyEventArgs.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "TimerManager.h"

namespace GardenToolCheck
{
    void Key(APlayerController* PC, FKey Key, EInputEvent Event)
    {
        PC->InputKey(FInputKeyEventArgs(nullptr, IPlatformInputDeviceMapper::Get().GetDefaultInputDevice(), Key, Event, 0u));
    }
    void After(UWorld* World, float Delay, TFunction<void()> Action)
    {
        FTimerHandle Handle;
        World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(World, [Action]() { Action(); }), Delay, false);
    }
}
// Launch a separate -game -GardenAutomation process. Captures actual first-person views and tests tool input.
static FAutoConsoleCommandWithWorld CheckTools(TEXT("Garden.VerifyTools"), TEXT("Verify tool meshes, input, melee and watering in an isolated test run."),
    FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
    {
        using namespace GardenToolCheck;
        if (!World || World->WorldType != EWorldType::Game || !FParse::Param(FCommandLine::Get(), TEXT("GardenAutomation"))) return;
        After(World, 5.f, [World]()
        {
            APlayerController* PC = World->GetFirstPlayerController();
            AGardenPlayer* Player = PC ? Cast<AGardenPlayer>(PC->GetPawn()) : nullptr;
            if (!Player || !Player->ShovelMesh->GetStaticMesh() || !Player->WateringCanMesh->GetStaticMesh())
            { UE_LOG(LogTemp, Error, TEXT("GardenTools: missing player or meshes")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
            if (AGardenDirector* Director = AGardenDirector::Find(World)) Director->PreparationRemaining = 120.f;
            Key(PC,EKeys::F,IE_Pressed); Key(PC,EKeys::F,IE_Released);
            After(World,1.f,[]() { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/GardenShovel.png"),false,false); });
            After(World, 2.f, [World, PC, Player]()
            {
                const bool ShovelVisible = Player->Tool == EGardenTool::Pala && Player->ShovelMesh->IsVisible() && !Player->WateringCanMesh->IsVisible();
                const FVector Target = Player->Camera->GetComponentLocation() + Player->Camera->GetForwardVector()*160.f;
                AZombieNormal* Zombie = World->SpawnActor<AZombieNormal>(Target,FRotator::ZeroRotator);
                Key(PC,EKeys::LeftMouseButton,IE_Pressed);
                After(World,.15f,[World, PC, Player, Zombie, ShovelVisible]()
                {
                    Key(PC,EKeys::LeftMouseButton,IE_Released);
                    const bool ShovelWorks = ShovelVisible && Zombie && FMath::IsNearlyEqual(Zombie->Health,65.f);
                    if (Zombie) Zombie->Destroy();
                    Key(PC,EKeys::R,IE_Pressed); Key(PC,EKeys::R,IE_Released);
                    After(World,.8f,[]() { FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir()/TEXT("Screenshots/GardenWateringCan.png"),false,false); });
                    After(World,1.5f,[World,PC,Player,ShovelWorks]()
                    {
                        const bool CanVisible = Player->Tool == EGardenTool::Regadera && Player->WateringCanMesh->IsVisible() && !Player->ShovelMesh->IsVisible();
                        const FVector Target = Player->Camera->GetComponentLocation()+Player->Camera->GetForwardVector()*250.f;
                        APlantaGirasol* Plant = World->SpawnActor<APlantaGirasol>(Target,FRotator::ZeroRotator);
                        if (Plant) Plant->Hydration=0.f;
                        Key(PC,EKeys::LeftMouseButton,IE_Pressed);
                        After(World,.6f,[World,PC,Player,Plant,CanVisible,ShovelWorks]()
                        {
                            Key(PC,EKeys::LeftMouseButton,IE_Released);
                            const bool WaterWorks = CanVisible && Plant && Plant->Hydration > 10.f && Player->WateringCanMesh->DoesSocketExist(TEXT("WaterSpout"));
                            Player->SelectSlot(0);
                            After(World,1.f,[Player,ShovelWorks,WaterWorks]()
                            {
                                const bool Hidden = !Player->ShovelMesh->IsVisible() && !Player->WateringCanMesh->IsVisible();
                                UE_LOG(LogTemp,Display,TEXT("GardenTools: shovel=%d watering=%d unequip=%d %s"),ShovelWorks,WaterWorks,Hidden,
                                    ShovelWorks && WaterWorks && Hidden ? TEXT("PASS") : TEXT("FAIL"));
                                FPlatformMisc::RequestExitWithStatus(false,ShovelWorks && WaterWorks && Hidden ? 0 : 1);
                            });
                        });
                    });
                });
            });
        });
    }));
#endif
