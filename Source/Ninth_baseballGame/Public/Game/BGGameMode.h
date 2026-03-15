#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BGGameMode.generated.h"

UCLASS()
class NINTH_BASEBALLGAME_API ABGGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void OnPostLogin(AController* NewPlayer) override;	
};
