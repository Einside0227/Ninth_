#include "Game/BGGameModeBase.h"

#include "Game/BGGameStateBase.h"
#include "Player/BGPlayerController.h"
#include "Player/BGPlayerState.h"
#include "EngineUtils.h"

void ABGGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);
}

void ABGGameModeBase::PrintChatMessageString(ABGPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	if (IsGuessNumberString(GuessNumberString) == true) {
		IncreaseGuessCount(InChattingPlayerController);
		
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		for (TActorIterator<ABGPlayerController> It(GetWorld()); It; ++It) {
			ABGPlayerController* BGPlayerController = *It;
			if (IsValid(BGPlayerController) == true) {
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				BGPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}
	}
	else {
		for (TActorIterator<ABGPlayerController> It(GetWorld()); It; ++It) {
			ABGPlayerController* BGPlayerController = *It;
			if (IsValid(BGPlayerController) == true) {
				BGPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
	}
}

void ABGGameModeBase::OnPostLogin(AController* NewPlayer) 
{
	Super::OnPostLogin(NewPlayer);
	
	ABGPlayerController* BGPlayerController = Cast<ABGPlayerController>(NewPlayer);
	if (IsValid(BGPlayerController) == true)
	{
		AllPlayerControllers.Add(BGPlayerController);

		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			BGPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ABGGameStateBase* CXGameStateBase =  GetGameState<ABGGameStateBase>();
		if (IsValid(CXGameStateBase) == true)
		{
			CXGameStateBase->MulticastRPCBroadcastLoginMessage(BGPS->PlayerNameString);
		}
	}
}

FString ABGGameModeBase::GenerateSecretNumber() 
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i) Numbers.Add(i);

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });
	
	FString Result;
	for (int32 i = 0; i < 3; ++i) {
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ABGGameModeBase::IsGuessNumberString(const FString& InNumberString) 
{
	bool bCanPlay = false;

	do {

		if (InNumberString.Len() != 3) break;

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString) {
			if (FChar::IsDigit(C) == false || C == '0') {
				bIsUnique = false;
				break;
			}
			
			UniqueDigits.Add(C);
		}

		if (bIsUnique == false) break;

		bCanPlay = true;
		
	} while (false);	

	return bCanPlay;
}

FString ABGGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString) 
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i) 	{
		if (InSecretNumberString[i] == InGuessNumberString[i]) StrikeCount++;
		else {
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar)) BallCount++;
		}
	}

	if (StrikeCount == 0 && BallCount == 0) return TEXT("OUT");

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ABGGameModeBase::IncreaseGuessCount(ABGPlayerController* InChattingPlayerController)
{
	ABGPlayerState* BGPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
	if (IsValid(BGPS) == true)
	{
		BGPS->CurrentGuessCount++;
	}
}
