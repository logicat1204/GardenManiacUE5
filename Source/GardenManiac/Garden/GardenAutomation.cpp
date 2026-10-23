#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GardenWorld.h"
#include "Planta.h"
#include "Zombie.h"
#include "GardenPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGardenSystemsTest, "GardenManiac.Prototype.Systems",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGardenSystemsTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("GardenSystemsTest"));
    if (!TestNotNull(TEXT("Test world"), World)) return false;
    FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
    Context.SetCurrentWorld(World);
    AGardenDirector* Director = World->SpawnActor<AGardenDirector>();
    Director->DispatchBeginPlay();
    TestEqual(TEXT("Exactly eight default seeds"), Director->Seeds.Num(), 8);
    TestFalse(TEXT("Negative purchases rejected"), Director->Spend(-10));
    TestFalse(TEXT("Overspending rejected"), Director->Spend(Director->Sun + 1));
    TestEqual(TEXT("Rejected purchases preserve balance"), Director->Sun, 250);
    TestFalse(TEXT("Pool starts locked"), Director->IsSectorUnlocked(EGardenSector::PatioB));
    Director->Day = 3;
    TestTrue(TEXT("Pool unlocks on day three"), Director->IsSectorUnlocked(EGardenSector::PatioB));
    TestFalse(TEXT("Roof still locked on day three"), Director->IsSectorUnlocked(EGardenSector::Tejado));
    Director->Day = 5;
    TestTrue(TEXT("Roof unlocks on day five"), Director->IsSectorUnlocked(EGardenSector::Tejado));

    const FVector TestOrigin(30000,0,100);
    APlantaGirasol* Sunflower = World->SpawnActor<APlantaGirasol>(TestOrigin, FRotator::ZeroRotator);
    Sunflower->DispatchBeginPlay();
    Sunflower->Hydration = 0.f;
    const int32 Before = Director->Sun;
    Sunflower->Tick(20.f);
    TestEqual(TEXT("Dry sunflower produces nothing"), Director->Sun, Before);
    Sunflower->Regar(500.f);
    TestEqual(TEXT("Water cannot exceed maximum"), Sunflower->Hydration, 100.f);
    Sunflower->WaterDrain = 0.f;
    Sunflower->Tick(8.f);
    TestEqual(TEXT("Watering restores production"), Director->Sun, Before + Sunflower->Production);

    APlantaOfensiva* Shooter = World->SpawnActor<APlantaOfensiva>(TestOrigin + FVector(0,500,0), FRotator::ZeroRotator);
    Shooter->DispatchBeginPlay();
    AZombieNormal* Zombie = World->SpawnActor<AZombieNormal>(TestOrigin + FVector(400,500,0), FRotator::ZeroRotator);
    Zombie->DispatchBeginPlay();
    Shooter->Atacar();
    TestEqual(TEXT("Offensive plant hits visible zombie in its lane"), Zombie->Health, 75.f);
    Zombie->Sector = EGardenSector::Tejado;
    Shooter->Atacar();
    TestEqual(TEXT("Plants do not shoot across sectors"), Zombie->Health, 75.f);
    Zombie->Sector = EGardenSector::PatioA;
    const int32 BeforeKill = Director->Sun;
    Zombie->TakeDamage(200.f, FDamageEvent(), nullptr, Shooter);
    TestEqual(TEXT("Killing zombie awards currency once"), Director->Sun, BeforeKill + Zombie->Reward);
    Zombie->TakeDamage(200.f, FDamageEvent(), nullptr, Shooter);
    TestEqual(TEXT("Repeated death cannot duplicate reward"), Director->Sun, BeforeKill + Zombie->Reward);

    TestTrue(TEXT("First garden upgrade affordable"), Director->UpgradeGarden());
    TestEqual(TEXT("Upgrade improves garden level"), Director->GardenLevel, 1);
    Director->DamageHouse(10000.f);
    TestTrue(TEXT("House destruction ends run"), Director->bDefeated);
    const int32 DefeatBalance = Director->Sun;
    Sunflower->Tick(20.f);
    Director->Tick(20.f);
    TestEqual(TEXT("Defeat stops all production"), Director->Sun, DefeatBalance);
    TestFalse(TEXT("Cannot spend after defeat"), Director->Spend(1));

    World->DestroyWorld(false);
    GEngine->DestroyWorldContext(World);
    return true;
}
#endif
