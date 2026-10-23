// Copyright Epic Games, Inc. All Rights Reserved.

#include "GardenManiacGameMode.h"
#include "Garden/GardenWorld.h"
#include "Garden/GardenPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

AGardenManiacGameMode::AGardenManiacGameMode()
{
    GardenDirectorClass = AGardenDirector::StaticClass();
    GardenPlayerClass = AGardenPlayer::StaticClass();
}

void AGardenManiacGameMode::StartPlay()
{
    Super::StartPlay();
    if (!bEnableGardenPrototype) return;
    if (!GardenDirectorClass || !GardenPlayerClass) return;
    AGardenDirector* Director = GetWorld()->SpawnActor<AGardenDirector>(GardenDirectorClass);
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!Director || !PC) return;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AGardenPlayer* Player = GetWorld()->SpawnActor<AGardenPlayer>(GardenPlayerClass, Director->Origin + FVector(-900,400,130), FRotator::ZeroRotator, Params);
    if (!Player) return;
    APawn* OldPawn = PC->GetPawn();
    PC->Possess(Player);
    if (OldPawn) OldPawn->Destroy();
    PC->SetControlRotation(FRotator::ZeroRotator);
    PC->ClientSetHUD(AGardenHUD::StaticClass());
    PC->SetInputMode(FInputModeGameOnly());
    PC->bShowMouseCursor = false;
}
