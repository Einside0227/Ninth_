#include "Game/BGGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/BGPlayerController.h"

ABGGameStateBase::ABGGameStateBase()
{
	RemainingTime = 10;
	MaxTime = 10;
}

void ABGGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString) 
{
	if (HasAuthority() == false) 
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true) 
		{
			ABGPlayerController* BGPC = Cast<ABGPlayerController>(PC);
			if (IsValid(BGPC) == true) 
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				BGPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}

void ABGGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABGGameStateBase, RemainingTime);
	DOREPLIFETIME(ABGGameStateBase, CurrentTurnPlayerName);
	DOREPLIFETIME(ThisClass, bGameStarted);
}
