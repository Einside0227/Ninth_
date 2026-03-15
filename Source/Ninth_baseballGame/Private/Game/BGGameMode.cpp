#include "Game/BGGameMode.h"

#include "Game/BGGameStateBase.h"

void ABGGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
	
	ABGGameStateBase* CXGameStateBase =  GetGameState<ABGGameStateBase>();
	if (IsValid(CXGameStateBase) == true)
	{
		CXGameStateBase->MulticastRPCBroadcastLoginMessage(TEXT("XXXXXXX"));
	}
}
