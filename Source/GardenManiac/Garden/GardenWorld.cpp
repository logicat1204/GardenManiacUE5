#include "GardenWorld.h"
#include "Planta.h"
#include "Zombie.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

void GardenVisual::Color(UStaticMeshComponent* Mesh, const FLinearColor& Value)
{
    UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Mesh))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Value);
        Mesh->SetMaterial(0, Material);
    }
}

AGardenPlot::AGardenPlot()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plot"));
    SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Shape.Object);
    Mesh->SetRelativeScale3D(FVector(2.3f, 1.65f, .12f));
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

AGardenDirector::AGardenDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    NormalClass = AZombieNormal::StaticClass();
    SpecialClass = AZombieEspecial::StaticClass();
    BossClass = AZombieBoss::StaticClass();
}
AGardenDirector* AGardenDirector::Find(const UWorld* World)
{
    for (TActorIterator<AGardenDirector> It(World); It; ++It) return *It;
    return nullptr;
}
void AGardenDirector::BeginPlay()
{
    Super::BeginPlay();
    CreateSeeds();
    BuildPrototype();
}
void AGardenDirector::CreateSeeds()
{
    // Eight useful prototype presets; replace with authored UGardenSeed data assets later.
    const TCHAR* Names[] = {TEXT("Lanzabrote"), TEXT("Girasol"), TEXT("Nuez"), TEXT("Rafaga"),
        TEXT("Brote acuatico"), TEXT("Nuez acuatica"), TEXT("Brote pesado"), TEXT("Girasol mayor")};
    const int32 Costs[] = {50, 50, 75, 100, 75, 100, 125, 100};
    for (int32 I = 0; I < 8; ++I)
    {
        if (SeedLoadout.IsValidIndex(I) && SeedLoadout[I] && SeedLoadout[I]->PlantClass)
        {
            Seeds.Add(SeedLoadout[I]);
            continue;
        }
        UGardenSeed* Seed = NewObject<UGardenSeed>(this);
        Seed->DisplayName = FText::FromString(Names[I]);
        Seed->Cost = Costs[I];
        Seed->Cooldown = I == 6 ? 5.f : 2.f;
        Seed->bAquatic = I == 4 || I == 5;
        if (I == 3) { Seed->ActionIntervalMultiplier = .65f / 1.5f; Seed->DamageMultiplier = 18.f / 25.f; }
        if (I == 6) { Seed->ActionIntervalMultiplier = 2.f; Seed->DamageMultiplier = 85.f / 25.f; }
        if (I == 7) Seed->ProductionMultiplier = 2.f;
        Seed->PlantClass = I == 1 || I == 7 ? APlantaGirasol::StaticClass() :
            I == 2 || I == 5 ? APlantaDefensiva::StaticClass() : APlantaOfensiva::StaticClass();
        Seed->Color = I == 1 || I == 7 ? FLinearColor(.95f,.65f,.05f) :
            I == 2 || I == 5 ? FLinearColor(.4f,.2f,.07f) :
            Seed->bAquatic ? FLinearColor(.03f,.65f,.7f) : FLinearColor(.12f,.7f,.1f);
        Seeds.Add(Seed);
    }
}

void AGardenDirector::BuildPrototype()
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto Box = [&](FVector Position, FVector Size, FLinearColor Color)
    {
        AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(Origin + Position, FRotator::ZeroRotator);
        if (!Actor) return;
        auto* Mesh = Actor->GetStaticMeshComponent();
        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(Cube);
        Mesh->SetWorldScale3D(Size / 100.f);
        Mesh->SetCollisionProfileName(TEXT("BlockAll"));
        GardenVisual::Color(Mesh, Color);
    };
    auto Label = [&](const TCHAR* Text, FVector Position)
    {
        ATextRenderActor* Actor = GetWorld()->SpawnActor<ATextRenderActor>(Origin + Position, FRotator(0, 180, 0));
        if (!Actor) return;
        Actor->GetTextRender()->SetText(FText::FromString(Text));
        Actor->GetTextRender()->SetWorldSize(65.f);
        Actor->GetTextRender()->SetTextRenderColor(FColor::White);
    };
    const FLinearColor Grass(.12f,.28f,.08f), Wall(.65f,.57f,.4f), Roof(.35f,.16f,.09f);
    Box(FVector(550,1200,-45), FVector(4300,3300,80), Grass);
    Box(FVector(-750,1200,0), FVector(1500,3000,30), FLinearColor(.4f,.32f,.22f));
    // House walls, with two openings facing the gardens.
    Box(FVector(-1500,1200,300), FVector(35,3000,600), Wall);
    Box(FVector(-750,-300,300), FVector(1500,35,600), Wall);
    Box(FVector(-750,2700,300), FVector(1500,35,600), Wall);
    Box(FVector(0,-100,300), FVector(35,400,600), Wall);
    Box(FVector(0,1200,300), FVector(35,900,600), Wall);
    Box(FVector(0,2525,300), FVector(35,350,600), Wall);
    Box(FVector(0,400,525), FVector(35,600,150), Wall);
    Box(FVector(0,2000,525), FVector(35,700,150), Wall);
    // Accessible roof with a real opening above the interior staircase.
    Box(FVector(-600,800,600), FVector(1800,2200,35), Roof);
    Box(FVector(-600,2600,600), FVector(1800,200,35), Roof);
    Box(FVector(100,2200,600), FVector(400,600,35), Roof);
    Box(FVector(-215,2200,600), FVector(240,600,35), Roof);
    for (int32 I = 0; I < 20; ++I)
    {
        const float Height = (I + 1) * 30.f;
        Box(FVector(-1400 + I * 55,2200,Height / 2), FVector(55,320,Height), Wall);
    }
    // Safety parapets leave the stairs and zombie climbing edge accessible.
    Box(FVector(-1480,800,665), FVector(30,2200,100), Wall);
    Box(FVector(-600,-280,665), FVector(1800,30,100), Wall);
    Box(FVector(1300,2000,0), FVector(2400,240,20), FLinearColor(.02f,.3f,.65f));
    Label(TEXT("PATIO A - DIA 1"), FVector(2450,100,330));
    Label(TEXT("PATIO B - DIA 3"), FVector(2450,1700,330));
    Label(TEXT("TEJADO - DIA 5"), FVector(260,900,930));
    Label(TEXT("ESCALERA / TEJADO"), FVector(-250,2050,370));
    for (int32 S = 0; S < 3; ++S)
    {
        for (int32 Lane = 0; Lane < 3; ++Lane)
        {
            for (int32 Column = 0; Column < (S == 2 ? 4 : 6); ++Column)
            {
                const FVector Position(S == 2 ? -1200.f + Column * 300.f : 400.f + Column * 320.f,
                    S == 0 ? 200.f + Lane * 240.f : S == 1 ? 1760.f + Lane * 240.f : 900.f + Lane * 240.f,
                    S == 2 ? 627.f : 20.f);
                AGardenPlot* Plot = GetWorld()->SpawnActor<AGardenPlot>(Origin + Position, FRotator::ZeroRotator);
                if (!Plot) continue;
                Plot->Sector = static_cast<EGardenSector>(S);
                Plot->bWater = S == 1 && Lane == 1;
                GardenVisual::Color(Plot->Mesh, Plot->bWater ? FLinearColor(.05f,.45f,.8f) : FLinearColor(.24f,.42f,.1f));
            }
        }
    }
    ADirectionalLight* Sunlight = GetWorld()->SpawnActor<ADirectionalLight>(Origin + FVector(0,0,2500), FRotator(-55,-35,0));
    if (Sunlight) { Sunlight->SetMobility(EComponentMobility::Movable); Sunlight->GetLightComponent()->SetIntensity(3.f); }
    ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>();
    if (Sky) { Sky->GetLightComponent()->SetMobility(EComponentMobility::Movable); Sky->GetLightComponent()->SetIntensity(.7f); Sky->GetLightComponent()->RecaptureSky(); }
}

bool AGardenDirector::IsSectorUnlocked(EGardenSector Sector) const
{
    return Sector == EGardenSector::PatioA || (Sector == EGardenSector::PatioB ? Day >= PatioBDay : Day >= RoofDay);
}
bool AGardenDirector::Spend(int32 Amount)
{
    if (Amount < 0 || Sun < Amount || bDefeated) return false;
    Sun -= Amount;
    return true;
}
void AGardenDirector::AddSun(int32 Amount) { if (!bDefeated) Sun += FMath::Max(0, Amount); }
void AGardenDirector::DamageHouse(float Amount)
{
    HouseHealth = FMath::Max(0.f, HouseHealth - FMath::Max(0.f, Amount));
    bDefeated = HouseHealth <= 0.f;
}
bool AGardenDirector::UpgradeGarden()
{
    if (GardenLevel >= 3 || !Spend(150 * (GardenLevel + 1))) return false;
    ++GardenLevel;
    HouseHealth = FMath::Min(500.f + GardenLevel * 100.f, HouseHealth + 150.f);
    return true;
}
void AGardenDirector::Tick(float Dt)
{
    Super::Tick(Dt);
    if (bDefeated) return;
    PassiveRemaining -= Dt;
    if (PassiveRemaining <= 0.f) { AddSun(10 + GardenLevel * 5); PassiveRemaining = 8.f; }
    Zombies.RemoveAll([](const TWeakObjectPtr<AZombie>& Zombie) { return !Zombie.IsValid() || Zombie->IsActorBeingDestroyed(); });
    if (!bWaveActive)
    {
        PreparationRemaining = FMath::Max(0.f, PreparationRemaining - Dt);
        if (PreparationRemaining <= 0.f)
        {
            bWaveActive = true;
            SpawnsRemaining = FMath::Min(30, 3 + Day * 2);
            SpawnIndex = 0;
            SpawnRemaining = 0.f;
        }
    }
    else
    {
        SpawnRemaining -= Dt;
        if (SpawnsRemaining > 0 && SpawnRemaining <= 0.f)
        {
            SpawnZombie();
            --SpawnsRemaining;
            ++SpawnIndex;
            SpawnRemaining = FMath::Max(1.2f, 4.f - Day * .2f);
        }
        if (SpawnsRemaining == 0 && Zombies.IsEmpty())
        {
            ++Day;
            bWaveActive = false;
            PreparationRemaining = PreparationDuration;
            AddSun(75);
        }
    }
}
void AGardenDirector::SpawnZombie()
{
    const int32 SectorCount = Day >= RoofDay ? 3 : Day >= PatioBDay ? 2 : 1;
    const int32 S = SpawnIndex % SectorCount;
    const int32 Lane = (SpawnIndex / SectorCount) % 3;
    const float Y = S == 0 ? 200.f + Lane * 240.f : S == 1 ? 1760.f + Lane * 240.f : 900.f + Lane * 240.f;
    const FVector Start = Origin + FVector(S == 2 ? 350.f : 2500.f, Y, 110.f);
    TSubclassOf<AZombie> Class = Day % 5 == 0 && SpawnIndex == 0 ? BossClass : Day >= 2 && SpawnIndex % 4 == 3 ? SpecialClass : NormalClass;
    if (!Class) Class = AZombieNormal::StaticClass();
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AZombie* Zombie = GetWorld()->SpawnActor<AZombie>(Class, Start, FRotator(0,180,0), Params);
    if (!Zombie) return;
    Zombie->Sector = static_cast<EGardenSector>(S);
    if (S == 2)
    {
        Zombie->Route.Add(Origin + FVector(350,Y,720));
        Zombie->Route.Add(Origin + FVector(-1400,Y,720));
    }
    else Zombie->Route.Add(Origin + FVector(80,Y,110));
    Zombies.Add(Zombie);
}
