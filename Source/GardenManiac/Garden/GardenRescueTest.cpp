#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GardenPlayer.h"
#include "GardenWorld.h"
#include "Planta.h"
#include "Zombie.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGardenRescueTest, "GardenManiac.Prototype.FriendlyFireAndRescue",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FGardenRescueTest::RunTest(const FString& Parameters)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false,TEXT("GardenRescueTest"));
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 // Initialize the game world's actor lifecycle so Destroy routes EndPlay as it does in Play.
 World->InitializeActorsForPlay(FURL());
 AGardenDirector* Director=World->SpawnActor<AGardenDirector>(); Director->DispatchBeginPlay();
 FVector Origin(30000,0,100);
 APlantaOfensiva* Shooter=World->SpawnActor<APlantaOfensiva>(Origin,FRotator::ZeroRotator); Shooter->DispatchBeginPlay();
 APlantaDefensiva* Ally=World->SpawnActor<APlantaDefensiva>(Origin+FVector(200,0,0),FRotator::ZeroRotator); Ally->DispatchBeginPlay();
 AZombieNormal* Zombie=World->SpawnActor<AZombieNormal>(Origin+FVector(500,0,0),FRotator::ZeroRotator); Zombie->DispatchBeginPlay();
 Shooter->Atacar();
 TestEqual(TEXT("Plant behind ally still damages zombie"),Zombie->Health,75.f);
 TestEqual(TEXT("Ally takes no friendly damage"),Ally->Health,Ally->MaxHealth);
 AStaticMeshActor* Wall=World->SpawnActor<AStaticMeshActor>(Origin+FVector(450,0,0),FRotator::ZeroRotator);
 Wall->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
 Wall->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 Wall->SetActorScale3D(FVector(.1f,2.f,3.f));
 Wall->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
 Shooter->Atacar();
 TestEqual(TEXT("Wall continues blocking plant shots"),Zombie->Health,75.f);
 AGardenPlayer* Player=World->SpawnActor<AGardenPlayer>(Origin+FVector(390,0,0),FRotator::ZeroRotator); Player->DispatchBeginPlay();
 TestFalse(TEXT("Zombie cannot grab through a wall"),Zombie->TryGrabPlayer(Player));
 Wall->Destroy();
 TestTrue(TEXT("Nearby zombie grabs player"),Zombie->TryGrabPlayer(Player));
 TestTrue(TEXT("Movement disabled during grab"),Player->GetCharacterMovement()->MovementMode==MOVE_None);
 TestEqual(TEXT("Rescue window starts at three seconds"),Player->GrabSecondsRemaining,3.f);
 Zombie->TakeDamage(1.f,FDamageEvent(),nullptr,Player);
 TestTrue(TEXT("Non-plant nonlethal damage does not rescue"),Player->IsGrabbed());
 Shooter->Atacar();
 TestFalse(TEXT("Plant shot through ally and player releases grab"),Player->IsGrabbed());
 TestEqual(TEXT("Nonlethal plant hit is enough for rescue"),Zombie->Health,49.f);
 TestTrue(TEXT("Movement restored after rescue"),Player->GetCharacterMovement()->MovementMode!=MOVE_None);
 TestFalse(TEXT("Immediate regrab prevented"),Zombie->TryGrabPlayer(Player));
 Player->Tick(2.1f);
 TestTrue(TEXT("Can grab after rescue grace"),Zombie->TryGrabPlayer(Player));
 Zombie->Destroy();
 TestFalse(TEXT("Removing grabbing zombie releases player"),Player->IsGrabbed());
 Player->Tick(2.1f);
 AZombieNormal* Last=World->SpawnActor<AZombieNormal>(Origin+FVector(500,0,0),FRotator::ZeroRotator); Last->DispatchBeginPlay();
 TestTrue(TEXT("Next zombie can grab"),Last->TryGrabPlayer(Player));
 Player->Tick(2.9f);
 TestFalse(TEXT("No defeat before deadline"),Director->bDefeated);
 Player->Tick(.11f);
 TestTrue(TEXT("Unrescued grab ends run after three seconds"),Director->bDefeated && Player->bKilledByZombie);
 World->DestroyWorld(false); GEngine->DestroyWorldContext(World);
 return true;
}
#endif
