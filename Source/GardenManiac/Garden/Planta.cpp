#include "Planta.h"
#include "GardenWorld.h"
#include "Zombie.h"
#include "GardenPlayer.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

APlanta::APlanta()
{
    PrimaryActorTick.bCanEverTick = true;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlantMesh"));
    SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Mesh->SetStaticMesh(Shape.Object);
    Mesh->SetRelativeScale3D(FVector(.85f, .85f, 1.1f));
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}

void APlanta::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
}

void APlanta::Tick(float Dt)
{
    Super::Tick(Dt);
    const AGardenDirector* Director = AGardenDirector::Find(GetWorld());
    if (Director && Director->bDefeated) return;
    Hydration = FMath::Clamp(Hydration - FMath::Max(0.f, WaterDrain) * Dt, 0.f, 100.f);
    ActionProgress += Dt * GetRendimiento();
    if (ActionProgress >= FMath::Max(.1f, ActionInterval))
    {
        ActionProgress = 0.f;
        Producir();
    }
}

float APlanta::TakeDamage(float Amount, FDamageEvent const& Event, AController* EventInstigator, AActor* Causer)
{
    const float Applied = FMath::Min(Health, FMath::Max(0.f, Amount));
    Health -= Applied;
    if (Health <= 0.f) Destroy();
    return Applied;
}

void APlanta::Regar(float Amount) { Hydration = FMath::Clamp(Hydration + FMath::Max(0.f, Amount), 0.f, 100.f); }
float APlanta::GetRendimiento() const { return Hydration <= 0.f ? 0.f : FMath::Lerp(.3f, 1.f, Hydration / 100.f); }
void APlanta::Producir_Implementation() {}

APlantaOfensiva::APlantaOfensiva() { ActionInterval = 1.5f; }
void APlantaOfensiva::Producir_Implementation() { Atacar(); }
void APlantaOfensiva::Atacar_Implementation()
{
    AZombie* Target = nullptr;
    float BestDistance = FMath::Square(AttackRange);
    bool bRescueTarget = false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PlantAttack), false, this);
    // Only attack traces ignore allies; planting, watering and shovel traces remain unchanged.
    for (TActorIterator<APlanta> Ally(GetWorld()); Ally; ++Ally) Params.AddIgnoredActor(*Ally);
    for (TActorIterator<AGardenPlayer> Player(GetWorld()); Player; ++Player) Params.AddIgnoredActor(*Player);
    for (TActorIterator<AZombie> It(GetWorld()); It; ++It)
    {
        if (It->Sector != Sector || It->IsActorBeingDestroyed()) continue;
        const FVector Offset = It->GetActorLocation() - GetActorLocation();
        // Plants shoot outward along their lane. Roof climbers are attackable after reaching the roof.
        if (FMath::Abs(Offset.Y) > 95.f || FMath::Abs(Offset.Z) > 150.f || Offset.X < -40.f) continue;
        const float Distance = Offset.SizeSquared();
        FHitResult Hit;
        GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), It->GetActorLocation(), ECC_Visibility, Params);
        const bool bNeedsRescue = It->IsGrabbingPlayer();
        if (Distance <= FMath::Square(AttackRange) && (!Hit.bBlockingHit || Hit.GetActor() == *It)
            && ((!Target) || (bNeedsRescue && !bRescueTarget) || (bNeedsRescue == bRescueTarget && Distance < BestDistance)))
        {
            Target = *It;
            BestDistance = Distance;
            bRescueTarget = bNeedsRescue;
        }
    }
    if (Target)
    {
        DrawDebugLine(GetWorld(), GetActorLocation(), Target->GetActorLocation(), FColor::Green, false, .18f, 0, 5.f);
        UGameplayStatics::ApplyDamage(Target, AttackDamage, nullptr, this, nullptr);
    }
}

APlantaDefensiva::APlantaDefensiva()
{
    MaxHealth = 450.f;
    Mesh->SetRelativeScale3D(FVector(1.1f, 1.1f, 1.4f));
}
void APlantaMiscelanea::EjecutarEspecial_Implementation() {}
void APlantaMiscelanea::Producir_Implementation() { EjecutarEspecial(); }
APlantaGirasol::APlantaGirasol() { ActionInterval = 8.f; MaxHealth = 80.f; }
void APlantaGirasol::EjecutarEspecial_Implementation()
{
    if (AGardenDirector* Director = AGardenDirector::Find(GetWorld())) Director->AddSun(Production);
}
