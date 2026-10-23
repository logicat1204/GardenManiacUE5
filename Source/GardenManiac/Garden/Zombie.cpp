#include "Zombie.h"
#include "Planta.h"
#include "GardenWorld.h"
#include "GardenPlayer.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AZombie::AZombie()
{
    PrimaryActorTick.bCanEverTick = true;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZombieMesh"));
    SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    Mesh->SetStaticMesh(Shape.Object);
    Mesh->SetRelativeScale3D(FVector(.65f, .65f, 1.8f));
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    // Lane actors are moved deterministically; they must not block one another.
    Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
}

void AZombie::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
    GardenVisual::Color(Mesh, IsA<AZombieBoss>() ? FLinearColor(.6f,.08f,.12f) :
        IsA<AZombieEspecial>() ? FLinearColor(.6f,.2f,.8f) : FLinearColor(.25f,.5f,.15f));
}

void AZombie::Tick(float Dt)
{
    Super::Tick(Dt);
    AGardenDirector* Director = AGardenDirector::Find(GetWorld());
    if (!Director || Director->bDefeated) return;
    if (IsGrabbingPlayer()) return;
    if (GrabbedPlayer.IsValid()) { GrabbedPlayer.Reset(); OnGrabEnded(); }
    for (TActorIterator<AGardenPlayer> Player(GetWorld()); Player; ++Player)
        if (TryGrabPlayer(*Player)) return;
    if (Route.IsEmpty()) return;
    HabilidadEspecial(Dt);
    AttackRemaining = FMath::Max(0.f, AttackRemaining - Dt);
    APlanta* Target = nullptr;
    float BestDistance = 145.f;
    for (TActorIterator<APlanta> It(GetWorld()); It; ++It)
    {
        if (It->Sector != Sector || It->IsActorBeingDestroyed()) continue;
        const FVector Offset = It->GetActorLocation() - GetActorLocation();
        if (FMath::Abs(Offset.Y) < 80.f && FMath::Abs(Offset.Z) < 130.f && Offset.X <= 35.f && Offset.X >= -BestDistance)
        {
            Target = *It;
            BestDistance = -Offset.X;
        }
    }
    if (Target)
    {
        if (AttackRemaining <= 0.f)
        {
            UGameplayStatics::ApplyDamage(Target, AttackDamage, nullptr, this, nullptr);
            AttackRemaining = FMath::Max(.1f, AttackInterval);
        }
        return;
    }
    if (!Route.IsValidIndex(Waypoint)) return;
    const FVector Destination = Route[Waypoint];
    SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(), Destination, Dt, FMath::Max(0.f, Speed)));
    if (FVector::DistSquared(GetActorLocation(), Destination) < 4.f)
    {
        ++Waypoint;
        if (Waypoint >= Route.Num())
        {
            Director->DamageHouse(AttackDamage * 3.f);
            Destroy();
        }
    }
}

float AZombie::TakeDamage(float Amount, FDamageEvent const& Event, AController* EventInstigator, AActor* Causer)
{
    if (Health <= 0.f) return 0.f;
    const float Applied = FMath::Min(Health, FMath::Max(0.f, Amount));
    Health -= Applied;
    if (Applied > 0.f && IsGrabbingPlayer() && (Cast<APlanta>(Causer) || Health <= 0.f))
    {
        GrabbedPlayer->ReleaseGrab();
        GrabbedPlayer.Reset();
        OnGrabEnded();
    }
    if (Health <= 0.f)
    {
        if (AGardenDirector* Director = AGardenDirector::Find(GetWorld())) Director->AddSun(Reward);
        Destroy();
    }
    return Applied;
}
bool AZombie::IsGrabbingPlayer() const
{
    return GrabbedPlayer.IsValid() && GrabbedPlayer->IsGrabbed() && GrabbedPlayer->GetGrabbingZombie() == this;
}
bool AZombie::TryGrabPlayer(AGardenPlayer* Player)
{
    if (!bCanGrab || Health <= 0.f || IsActorBeingDestroyed() || IsGrabbingPlayer() || !IsValid(Player)) return false;
    const FVector Offset = Player->GetActorLocation() - GetActorLocation();
    if (Offset.SizeSquared() > FMath::Square(FMath::Max(0.f,GrabRange)) || FMath::Abs(Offset.Z) > 90.f) return false;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ZombieGrab), false, this);
    GetWorld()->LineTraceSingleByChannel(Hit,GetActorLocation(),Player->GetActorLocation(),ECC_Visibility,Params);
    if (Hit.bBlockingHit && Hit.GetActor() != Player) return false;
    if (!Player->TryBeginGrab(this)) return false;
    GrabbedPlayer = Player;
    OnGrabStarted(Player);
    return true;
}
void AZombie::EndPlay(const EEndPlayReason::Type Reason)
{
    // Destroy() can invalidate the player's weak captor reference before EndPlay runs.
    if (GrabbedPlayer.IsValid() && (GrabbedPlayer->GetGrabbingZombie() == this || !IsValid(GrabbedPlayer->GetGrabbingZombie())))
        GrabbedPlayer->ReleaseGrab();
    Super::EndPlay(Reason);
}
void AZombie::HabilidadEspecial_Implementation(float DeltaSeconds) {}
AZombieEspecial::AZombieEspecial() { MaxHealth = 160.f; Speed = 105.f; Reward = 25; }
AZombieBoss::AZombieBoss()
{
    MaxHealth = 800.f; Speed = 40.f; AttackDamage = 45.f; Reward = 100;
    Mesh->SetRelativeScale3D(FVector(1.f, 1.f, 2.2f));
}
