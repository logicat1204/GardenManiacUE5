#include "GardenPlayer.h"
#include "GardenWorld.h"
#include "Planta.h"
#include "Zombie.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"
#include "DrawDebugHelpers.h"

AGardenPlayer::AGardenPlayer()
{
    PrimaryActorTick.bCanEverTick = true;
    GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("GardenCamera"));
    Camera->SetupAttachment(GetCapsuleComponent());
    Camera->SetRelativeLocation(FVector(0,0,65));
    Camera->bUsePawnControlRotation = true;
    Camera->FieldOfView = 90.f;
    ShovelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShovelMesh"));
    WateringCanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WateringCanMesh"));
    for (UStaticMeshComponent* ToolMesh : {ShovelMesh.Get(), WateringCanMesh.Get()})
    {
        ToolMesh->SetupAttachment(Camera);
        ToolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ToolMesh->SetGenerateOverlapEvents(false);
        ToolMesh->SetOnlyOwnerSee(true);
        ToolMesh->SetCastShadow(false);
        ToolMesh->SetLightingChannels(true, true, false);
        ToolMesh->SetVisibility(false);
    }
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ShovelAsset(TEXT("/Game/GardenManiac/Tools/GardenTools/StaticMeshes/SM_GM_Shovel.SM_GM_Shovel"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CanAsset(TEXT("/Game/GardenManiac/Tools/GardenTools/StaticMeshes/SM_GM_WateringCan.SM_GM_WateringCan"));
    ShovelMesh->SetStaticMesh(ShovelAsset.Object);
    WateringCanMesh->SetStaticMesh(CanAsset.Object);
    // A soft first-person fill only lights channel 1 (the tools), keeping metals readable indoors.
    ToolFillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ToolFillLight"));
    ToolFillLight->SetupAttachment(Camera);
    ToolFillLight->SetRelativeLocation(FVector(5.f,-20.f,25.f));
    ToolFillLight->SetIntensityUnits(ELightUnits::Lumens);
    ToolFillLight->SetIntensity(10.f);
    ToolFillLight->SetAttenuationRadius(180.f);
    ToolFillLight->SetSourceRadius(15.f);
    ToolFillLight->SetCastShadows(false);
    ToolFillLight->SetLightingChannels(false,true,false);
    ToolFillLight->SetVisibility(false);
    GetCharacterMovement()->MaxWalkSpeed = 550.f;
    GetCharacterMovement()->MaxStepHeight = 40.f;
    GetCharacterMovement()->JumpZVelocity = 480.f;
    GetCharacterMovement()->AirControl = .3f;
    SlotCooldowns.Init(0.f, 8);
}
void AGardenPlayer::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AGardenPlayer::StartUse);
    Input->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AGardenPlayer::StopUse);
    Input->BindKey(EKeys::F, IE_Pressed, this, &AGardenPlayer::Shovel);
    Input->BindKey(EKeys::R, IE_Pressed, this, &AGardenPlayer::WateringCan);
    Input->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AGardenPlayer::NextSlot);
    Input->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AGardenPlayer::PreviousSlot);
    Input->BindKey(EKeys::E, IE_Pressed, this, &AGardenPlayer::Upgrade);
    Input->BindKey(EKeys::Enter, IE_Pressed, this, &AGardenPlayer::RestartAfterDefeat);
}
void AGardenPlayer::Tick(float Dt)
{
    Super::Tick(Dt);
    UpdateToolVisuals(Dt);
    for (float& Cooldown : SlotCooldowns) Cooldown = FMath::Max(0.f, Cooldown - Dt);
    UseRemaining = FMath::Max(0.f, UseRemaining - Dt);
    FeedbackRemaining -= Dt;
    if (FeedbackRemaining <= 0.f) Feedback.Empty();
    APlayerController* PC = Cast<APlayerController>(GetController());
    AGardenDirector* Director = AGardenDirector::Find(GetWorld());
    RescueGraceRemaining = FMath::Max(0.f, RescueGraceRemaining - Dt);
    if (Director && !Director->bDefeated && bGrabbed)
    {
        if (!GrabbingZombie.IsValid() || GrabbingZombie->IsActorBeingDestroyed() || GrabbingZombie->Health <= 0.f) ReleaseGrab();
        else
        {
            GrabSecondsRemaining = FMath::Max(0.f, GrabSecondsRemaining - Dt);
            if (GrabSecondsRemaining <= 0.f)
            {
                bKilledByZombie = true;
                Director->bDefeated = true;
                bUsing = false;
            }
        }
    }
    if (!PC || !Director || Director->bDefeated) return;
    float MX = 0.f, MY = 0.f;
    PC->GetInputMouseDelta(MX, MY);
    AddControllerYawInput(MX * .15f);
    AddControllerPitchInput(-MY * .15f);
    if (bGrabbed) return; // Looking is allowed, but movement, jumping and tools require rescue.
    const FRotator Yaw(0, GetControlRotation().Yaw, 0);
    AddMovementInput(Yaw.Vector(), (PC->IsInputKeyDown(EKeys::W) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::S) ? 1.f : 0.f));
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), (PC->IsInputKeyDown(EKeys::D) ? 1.f : 0.f) - (PC->IsInputKeyDown(EKeys::A) ? 1.f : 0.f));
    if (PC->WasInputKeyJustPressed(EKeys::SpaceBar)) Jump();
    if (PC->WasInputKeyJustReleased(EKeys::SpaceBar)) StopJumping();
    const FKey Keys[] = {EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight};
    for (int32 I = 0; I < 8; ++I) if (PC->WasInputKeyJustPressed(Keys[I])) SelectSlot(I);
    if (bUsing && Tool == EGardenTool::Regadera && UseRemaining <= 0.f) Use();
    if (GetActorLocation().Z < -500.f) SetActorLocation(Director->Origin + FVector(-900,400,130));
    FHitResult Hit;
    if (AGardenPlot* Plot = Cast<AGardenPlot>(GetAimedActor(Hit)))
    {
        const bool Valid = !Plot->Plant.IsValid() && Director->IsSectorUnlocked(Plot->Sector)
            && Director->Seeds.IsValidIndex(SelectedSlot) && Director->Seeds[SelectedSlot]->bAquatic == Plot->bWater;
        DrawDebugBox(GetWorld(), Plot->GetActorLocation() + FVector(0,0,8), FVector(114,82,8), Valid ? FColor::Green : FColor::Red);
    }
}
AZombie* AGardenPlayer::GetGrabbingZombie() const { return GrabbingZombie.Get(); }
bool AGardenPlayer::TryBeginGrab(AZombie* Zombie)
{
    const AGardenDirector* Director = AGardenDirector::Find(GetWorld());
    if (!IsValid(Zombie) || bGrabbed || bKilledByZombie || RescueGraceRemaining > 0.f || !Director || Director->bDefeated) return false;
    GrabbingZombie = Zombie;
    bGrabbed = true;
    GrabSecondsRemaining = 3.f;
    bUsing = false;
    ShovelSwingRemaining = 0.f;
    StopJumping();
    ConsumeMovementInputVector();
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    OnGrabStarted(Zombie);
    return true;
}
void AGardenPlayer::ReleaseGrab()
{
    if (!bGrabbed || bKilledByZombie) return;
    bGrabbed = false;
    GrabbingZombie.Reset();
    GrabSecondsRemaining = 0.f;
    RescueGraceRemaining = 2.f;
    GetCharacterMovement()->SetDefaultMovementMode();
    Notify(TEXT("Libre! Alejate del zombi y busca cobertura de tus plantas."));
    OnGrabReleased();
}
void AGardenPlayer::UpdateToolVisuals(float Dt)
{
    ShovelSwingRemaining = FMath::Max(0.f, ShovelSwingRemaining - Dt);
    ShovelMesh->SetVisibility(Tool == EGardenTool::Pala);
    WateringCanMesh->SetVisibility(Tool == EGardenTool::Regadera);
    ToolFillLight->SetVisibility(Tool != EGardenTool::Semilla);
    const float Swing = ShovelSwingRemaining > 0.f ? FMath::Sin((1.f - ShovelSwingRemaining / .4f) * PI) : 0.f;
    WateringTilt = FMath::FInterpTo(WateringTilt, Tool == EGardenTool::Regadera && bUsing ? 1.f : 0.f, Dt, 9.f);
    auto Place = [](UStaticMeshComponent* ToolMesh, FVector Center, FRotator Rotation, float Scale)
    {
        if (!ToolMesh->GetStaticMesh()) return;
        // Imported mesh origins may retain the studio placement; position by its actual bounds.
        const FVector PivotOffset = Rotation.RotateVector(ToolMesh->GetStaticMesh()->GetBounds().Origin * Scale);
        ToolMesh->SetRelativeTransform(FTransform(Rotation, Center - PivotOffset, FVector(Scale)));
    };
    Place(ShovelMesh, FVector(55.f + Swing * 13.f, 27.f - Swing * 9.f, -35.f), FRotator(-12.f - Swing * 48.f, 90.f, 180.f), .65f);
    Place(WateringCanMesh, FVector(66.f,24.f,-13.f), FRotator(-WateringTilt * 24.f,-10.f,0.f), .48f);
}
void AGardenPlayer::SelectSlot(int32 Index)
{
    if (bGrabbed || bKilledByZombie) return;
    SelectedSlot = FMath::Clamp(Index, 0, 7);
    Tool = EGardenTool::Semilla;
}
void AGardenPlayer::NextSlot() { SelectSlot((SelectedSlot + 1) % 8); }
void AGardenPlayer::PreviousSlot() { SelectSlot((SelectedSlot + 7) % 8); }
void AGardenPlayer::Shovel() { if (!bGrabbed && !bKilledByZombie) Tool = Tool == EGardenTool::Pala ? EGardenTool::Semilla : EGardenTool::Pala; }
void AGardenPlayer::WateringCan() { if (!bGrabbed && !bKilledByZombie) Tool = Tool == EGardenTool::Regadera ? EGardenTool::Semilla : EGardenTool::Regadera; }
void AGardenPlayer::StartUse() { if (bGrabbed || bKilledByZombie) return; bUsing = true; Use(); }
void AGardenPlayer::StopUse() { bUsing = false; }
void AGardenPlayer::Notify(const FString& Message) { Feedback = Message; FeedbackRemaining = 2.5f; }
float AGardenPlayer::GetCooldown(int32 Index) const { return SlotCooldowns.IsValidIndex(Index) ? SlotCooldowns[Index] : 0.f; }
AActor* AGardenPlayer::GetAimedActor(FHitResult& Hit) const
{
    const FVector Start = Camera->GetComponentLocation();
    const float Range = Tool == EGardenTool::Pala ? 240.f : 650.f;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(GardenUse), false, this);
    GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Camera->GetForwardVector() * Range, ECC_Visibility, Params);
    return Hit.GetActor();
}
void AGardenPlayer::Use()
{
    AGardenDirector* Director = AGardenDirector::Find(GetWorld());
    if (!Director || Director->bDefeated || bGrabbed || UseRemaining > 0.f) return;
    FHitResult Hit;
    AActor* Target = GetAimedActor(Hit);
    APlanta* Plant = Cast<APlanta>(Target);
    AGardenPlot* Plot = Cast<AGardenPlot>(Target);
    if (!Plant && Plot) Plant = Plot->Plant.Get();
    if (Tool == EGardenTool::Pala)
    {
        ShovelSwingRemaining = .4f;
        UseRemaining = .45f;
        if (Plant) { Plant->Destroy(); Notify(TEXT("Planta retirada (sin reembolso)")); }
        else if (AZombie* Zombie = Cast<AZombie>(Target))
        {
            UGameplayStatics::ApplyDamage(Zombie, 35.f, GetController(), this, nullptr);
            Notify(TEXT("Golpe de pala: 35"));
        }
        else Notify(TEXT("Acercate para usar la pala"));
        return;
    }
    if (Tool == EGardenTool::Regadera)
    {
        UseRemaining = .15f;
        if (Plant)
        {
            Plant->Regar(6.f);
            const FVector Spout = WateringCanMesh->GetSocketLocation(TEXT("WaterSpout"));
            DrawDebugLine(GetWorld(), Spout, Hit.ImpactPoint, FColor::Cyan, false, .16f, 0, 3.f);
        }
        return;
    }
    if (!Plot) { Notify(TEXT("Apunta a una casilla del jardin")); return; }
    if (!Director->IsSectorUnlocked(Plot->Sector)) { Notify(TEXT("Este sector se desbloquea en los proximos dias")); return; }
    if (Plot->Plant.IsValid()) { Notify(TEXT("Casilla ocupada")); return; }
    if (!Director->Seeds.IsValidIndex(SelectedSlot)) return;
    UGardenSeed* Seed = Director->Seeds[SelectedSlot];
    if (Seed->bAquatic != Plot->bWater) { Notify(Plot->bWater ? TEXT("Necesitas una semilla acuatica") : TEXT("Esta semilla necesita piscina")); return; }
    if (GetCooldown(SelectedSlot) > 0.f) { Notify(TEXT("Semilla recargando")); return; }
    if (!Seed->PlantClass || !Director->Spend(Seed->Cost)) { Notify(TEXT("No tienes suficiente sol")); return; }
    const FTransform Transform(FRotator::ZeroRotator, Plot->GetActorLocation() + FVector(0,0,65));
    Plant = GetWorld()->SpawnActorDeferred<APlanta>(Seed->PlantClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!Plant) { Director->AddSun(Seed->Cost); return; }
    Plant->Sector = Plot->Sector;
    Plant->Plot = Plot;
    Plant->ActionInterval *= FMath::Max(.1f, Seed->ActionIntervalMultiplier);
    if (APlantaOfensiva* Shooter = Cast<APlantaOfensiva>(Plant))
    {
        Shooter->AttackDamage *= FMath::Max(.1f, Seed->DamageMultiplier);
    }
    if (APlantaGirasol* Sunflower = Cast<APlantaGirasol>(Plant))
        Sunflower->Production = FMath::RoundToInt(Sunflower->Production * FMath::Max(.1f, Seed->ProductionMultiplier));
    UGameplayStatics::FinishSpawningActor(Plant, Transform);
    Plot->Plant = Plant;
    GardenVisual::Color(Plant->Mesh, Seed->Color);
    SlotCooldowns[SelectedSlot] = FMath::Max(0.f, Seed->Cooldown);
    Notify(TEXT("Plantada: ") + Seed->DisplayName.ToString());
}
void AGardenPlayer::Upgrade()
{
    if (bGrabbed || bKilledByZombie) return;
    if (AGardenDirector* Director = AGardenDirector::Find(GetWorld()))
        Notify(Director->UpgradeGarden() ? TEXT("Jardin mejorado: mas ingresos y casa reparada") : TEXT("Mejora no disponible: revisa el coste o nivel maximo"));
}
void AGardenPlayer::RestartAfterDefeat()
{
    if (AGardenDirector* Director = AGardenDirector::Find(GetWorld()))
        if (Director->bDefeated) UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AGardenHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;
    AGardenPlayer* Player = Cast<AGardenPlayer>(GetOwningPawn());
    AGardenDirector* Director = AGardenDirector::Find(GetWorld());
    if (!Player || !Director) return;
    const float W = Canvas->ClipX, H = Canvas->ClipY;
    DrawRect(FLinearColor(0.015f,.035f,.02f,.85f), 12, 12, W - 24, 105);
    DrawText(FString::Printf(TEXT("GARDEN MANIAC  |  Dia %d  |  Sol %d  |  Casa %.0f  |  Jardin %d/3"),
        Director->Day, Director->Sun, Director->HouseHealth, Director->GardenLevel), FColor::White, 25, 20, nullptr, 1.2f);
    DrawText(Director->bWaveActive ? FString::Printf(TEXT("HORDA EN CURSO - Zombis activos: %d"), Director->Zombies.Num()) :
        FString::Printf(TEXT("Preparacion: %.0f s"), Director->PreparationRemaining), FColor::Yellow, 25, 49);
    DrawText(FString::Printf(TEXT("WASD mover | Espacio saltar | 1-8 / Rueda semillas | F pala | R regadera | Clic usar | E mejorar (%d sol)"),
        150 * (Director->GardenLevel + 1)), FColor::White, 25, 75);
    DrawText(TEXT("Sectores: A dia 1 / B dia 3 / Tejado dia 5. Mantener clic para regar."), FColor::Silver, 25, 94);
    const TCHAR* ToolName = Player->Tool == EGardenTool::Pala ? TEXT("PALA") : Player->Tool == EGardenTool::Regadera ? TEXT("REGADERA") : TEXT("SEMILLA");
    DrawText(ToolName, FColor::Cyan, W / 2 - 35, H / 2 + 25);
    DrawLine(W/2 - 6,H/2,W/2 + 6,H/2,FLinearColor::White);
    DrawLine(W/2,H/2 - 6,W/2,H/2 + 6,FLinearColor::White);
    FHitResult Hit;
    APlanta* Plant = Cast<APlanta>(Player->GetAimedActor(Hit));
    if (Plant) DrawText(FString::Printf(TEXT("Vida %.0f/%.0f | Agua %.0f%% | Rendimiento %.0f%%"),
        Plant->Health, Plant->MaxHealth, Plant->Hydration, Plant->GetRendimiento()*100.f), FColor::Cyan, W/2 - 170,H/2 + 50);
    DrawText(Player->Feedback, FColor::Yellow, 25, H - 145);
    const float SlotWidth = (W - 32) / 8;
    for (int32 I = 0; I < 8 && Director->Seeds.IsValidIndex(I); ++I)
    {
        const float X = 16 + I * SlotWidth;
        const bool Selected = Player->SelectedSlot == I && Player->Tool == EGardenTool::Semilla;
        DrawRect(Selected ? FLinearColor(.18f,.42f,.12f,.95f) : FLinearColor(.03f,.06f,.04f,.9f), X,H-100,SlotWidth-4,85);
        DrawText(FString::Printf(TEXT("%d - %s"),I+1,*Director->Seeds[I]->DisplayName.ToString()),FColor::White,X+6,H-92,nullptr,.85f);
        DrawText(FString::Printf(TEXT("%d sol"),Director->Seeds[I]->Cost),FColor::Yellow,X+6,H-65);
        DrawText(Player->GetCooldown(I) > 0.f ? FString::Printf(TEXT("%.1f s"),Player->GetCooldown(I)) : TEXT("Lista"),FColor::Silver,X+6,H-42);
    }
    if (Player->IsGrabbed() && !Director->bDefeated)
    {
        DrawRect(FLinearColor(.3f,0,0,.85f),W*.2f,H*.22f,W*.6f,85.f);
        DrawText(FString::Printf(TEXT("TE HA AGARRADO UN ZOMBI - %.1f s"),Player->GrabSecondsRemaining),FColor::Yellow,W*.22f,H*.24f,nullptr,1.35f);
        DrawText(TEXT("Una planta debe alcanzar al zombi para liberarte."),FColor::White,W*.22f,H*.30f);
    }
    if (Director->bDefeated)
    {
        DrawRect(FLinearColor(0,0,0,.9f),W*.2f,H*.35f,W*.6f,120);
        DrawText(Player->bKilledByZombie ? TEXT("EL ZOMBI TE ATRAPO") : TEXT("LA CASA HA CAIDO"),FColor::Red,W*.3f,H*.38f,nullptr,2.f);
        DrawText(TEXT("Enter: reiniciar la partida"),FColor::White,W*.3f,H*.46f);
    }
}
