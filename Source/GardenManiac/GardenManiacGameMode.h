// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GardenManiacGameMode.generated.h"

class AGardenDirector;
class AGardenPlayer;

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AGardenManiacGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGardenManiacGameMode();

    virtual void StartPlay() override;

    /** Disable on a derived Blueprint to use the original template map without the garden. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Garden Prototype")
    bool bEnableGardenPrototype = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Garden Prototype")
    TSubclassOf<AGardenDirector> GardenDirectorClass;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Garden Prototype")
    TSubclassOf<AGardenPlayer> GardenPlayerClass;
};



