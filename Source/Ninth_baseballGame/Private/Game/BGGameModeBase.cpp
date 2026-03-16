#include "Game/BGGameModeBase.h"

#include "Game/BGGameStateBase.h"
#include "Player/BGPlayerController.h"
#include "Player/BGPlayerState.h"
#include "EngineUtils.h"
#include "TimerManager.h"

void ABGGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);
	
	StartRoundTimer();
}

void ABGGameModeBase::TickTimer()
{
	ABGGameStateBase* BGGameState = GetGameState<ABGGameStateBase>();
	if (IsValid(BGGameState) == false) return;

	if (BGGameState->RemainingTime > 0)
	{
		BGGameState->RemainingTime--;
	}

	if (BGGameState->RemainingTime <= 0)
	{
		BGGameState->RemainingTime = 0;
		StopRoundTimer();
	}
}

void ABGGameModeBase::StartRoundTimer()
{
	ABGGameStateBase* BGGameState = GetGameState<ABGGameStateBase>();
	if (IsValid(BGGameState))
	{
		BGGameState->RemainingTime = BGGameState->MaxTime;
	}

	GetWorldTimerManager().SetTimer(
		RoundTimerHandle,
		this,
		&ABGGameModeBase::TickTimer,
		1.0f,
		true
	);
}

void ABGGameModeBase::StopRoundTimer()
{
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
}

bool ABGGameModeBase::CanPlayNumberBaseball() const
{
	const ABGGameStateBase* BGGameState = GetGameState<ABGGameStateBase>();
	if (IsValid(BGGameState) == false) return false;

	return BGGameState->RemainingTime > 0;
}

void ABGGameModeBase::PrintChatMessageString(ABGPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	// 시간 종료
	if (CanPlayNumberBaseball() == false)
	{
		if (IsValid(InChattingPlayerController))
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("시간이 종료되어 더 이상 입력할 수 없습니다."));
		}
		return;
	}
	// 기회 소진
	ABGPlayerState* BGPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
	if (IsValid(BGPS) == false) return;

	if (BGPS->CurrentGuessCount >= BGPS->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("더 이상 입력할 수 없습니다."));
		return;
	}
	
	FString GuessNumberString = InChatMessageString;
	
	if (IsGuessNumberString(GuessNumberString) == true) 
	{
		IncreaseGuessCount(InChattingPlayerController);
		
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		for (TActorIterator<ABGPlayerController> It(GetWorld()); It; ++It) 
		{
			ABGPlayerController* BGPlayerController = *It;
			if (IsValid(BGPlayerController) == true) 
			{
				FString CombinedMessageString =
					BGPS->GetPlayerInfoString()
					+ TEXT(" : ")
					+ GuessNumberString
					+ TEXT(" -> ")
					+ JudgeResultString;
				BGPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);

			}
		}
		
		int32 StrikeCount = 0;
		if (JudgeResultString != TEXT("OUT"))
		{
			StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
		}

		JudgeGame(InChattingPlayerController, StrikeCount);
	}
	else 
	{
		if (IsValid(InChattingPlayerController) == true)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("다시 입력하세요"));
		}
	}
}

void ABGGameModeBase::OnPostLogin(AController* NewPlayer) 
{
	Super::OnPostLogin(NewPlayer);
	
	ABGPlayerController* BGPlayerController = Cast<ABGPlayerController>(NewPlayer);
	if (IsValid(BGPlayerController) == true)
	{
		BGPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
		
		AllPlayerControllers.Add(BGPlayerController);
		
		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			BGPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ABGGameStateBase* BGGameStateBase =  GetGameState<ABGGameStateBase>();
		if (IsValid(BGGameStateBase) == true)
		{
			BGGameStateBase->MulticastRPCBroadcastLoginMessage(BGPS->PlayerNameString);
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
	for (int32 i = 0; i < 3; ++i) 
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ABGGameModeBase::IsGuessNumberString(const FString& InNumberString) 
{
	bool bCanPlay = false;

	do
	{
		// 3자리 숫자인가?
		if (InNumberString.Len() != 3) break;

		TSet<TCHAR> UniqueDigits;

		for (TCHAR C : InNumberString)
		{
			// 문자가 포함되지는 않았는가?
			if (FChar::IsDigit(C) == false || C == '0') break;

			UniqueDigits.Add(C);
		}

		// 중복되는 숫자가 있는가?
		if (UniqueDigits.Num() != 3) break;

		bCanPlay = true;

	} while (false);

	return bCanPlay;
}

FString ABGGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString) 
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i]) StrikeCount++;
		else 
		{
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

void ABGGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);
	
	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS) == true)
		{
			BGPS->CurrentGuessCount = 0;
		}
	}
	
	ABGGameStateBase* BGGameState = GetGameState<ABGGameStateBase>();
	if (IsValid(BGGameState))
	{
		BGGameState->RemainingTime = BGGameState->MaxTime;
	}

	StopRoundTimer();
	StartRoundTimer();
}

void ABGGameModeBase::JudgeGame(ABGPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ABGPlayerState* IBGPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
		for (const auto& BGPlayerController : AllPlayerControllers)
		{
			if (IsValid(IBGPS) == true) // 승리 판정
			{
				FString CombinedMessageString = IBGPS->PlayerNameString + TEXT(" has won the game.");
				BGPlayerController->NotificationText = FText::FromString(CombinedMessageString);
			}
		}
		
		ResetGame();
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& BGPlayerController : AllPlayerControllers)
		{
			ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
			if (IsValid(BGPS) == true)
			{
				if (BGPS->CurrentGuessCount < BGPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)  // 무승부 판정
		{
			for (const auto& BGPlayerController : AllPlayerControllers)
			{
				BGPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));
			}
			
			ResetGame();
		}
	}
}
