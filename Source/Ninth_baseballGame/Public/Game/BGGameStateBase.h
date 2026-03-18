#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BGGameStateBase.generated.h"

UCLASS()
class NINTH_BASEBALLGAME_API ABGGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ABGGameStateBase();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBroadcastLoginMessage(const FString& InNameString = FString(TEXT("XXXXXXX")));
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 RemainingTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxTime;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	FString CurrentTurnPlayerName;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bGameStarted = false;
};
