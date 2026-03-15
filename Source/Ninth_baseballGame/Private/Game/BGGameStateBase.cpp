#include "Game/BGGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "Player/BGPlayerController.h"

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
