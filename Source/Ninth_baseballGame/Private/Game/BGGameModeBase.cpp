#include "Game/BGGameModeBase.h"

#include "Game/BGGameStateBase.h"
#include "Player/BGPlayerController.h"
#include "Player/BGPlayerState.h"
#include "EngineUtils.h"
#include "TimerManager.h"

void ABGGameModeBase::BeginPlay()
{
	Super::BeginPlay();
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
		if (IsValid(BGGameStateBase) == true && IsValid(BGPS) == true)
		{
			BGGameStateBase->MulticastRPCBroadcastLoginMessage(BGPS->PlayerNameString);
			
			if (AllPlayerControllers.Num() == 1)
			{
				CurrentTurnPlayerIndex = 0;
				BGGameStateBase->CurrentTurnPlayerName = BGPS->PlayerNameString;
			}
		}
		
		if (BGGameStateBase->bGameStarted  == true)
		{
			BGPlayerController->ClientRPCHideStartWidget();
		}
	}
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
		
		// 턴 내에 입력하지 않았으면 턴 종료 + 기회 차감
		EndTurn();
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

void ABGGameModeBase::StartTurn()
{
	bDidSubmitThisTurn = false;

	ABGPlayerController* CurrentTurnPC = GetCurrentTurnPlayerController();
	ABGGameStateBase* BGGameState = GetGameState<ABGGameStateBase>();

	if (IsValid(CurrentTurnPC) && IsValid(BGGameState))
	{
		ABGPlayerState* BGPS = CurrentTurnPC->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS))
		{
			BGGameState->CurrentTurnPlayerName = BGPS->PlayerNameString;
		}
	}

	StartRoundTimer();
}

void ABGGameModeBase::EndTurn()
{
	StopRoundTimer();
	
	ABGPlayerController* CurrentTurnPC = GetCurrentTurnPlayerController();
	if (IsValid(CurrentTurnPC) == false) return;

	ABGPlayerState* BGPS = CurrentTurnPC->GetPlayerState<ABGPlayerState>();
	if (IsValid(BGPS) == false) return;

	// 입력 안 했을 때
	if (bDidSubmitThisTurn == false)
	{
		IncreaseGuessCount(CurrentTurnPC);
		CurrentTurnPC->ClientRPCPrintChatMessageString(TEXT("입력하지 않아 기회가 차감되었습니다."));
	}

	// 모든 플레이어가 기회를 다 썼는지 확인
	bool bIsDraw = true;
	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		ABGPlayerState* OtherPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(OtherPS) == true)
		{
			if (OtherPS->CurrentGuessCount < OtherPS->MaxGuessCount)
			{
				bIsDraw = false;
				break;
			}
		}
	}

	// 무승부
	if (bIsDraw == true)
	{
		ShowResultToAllPlayers(FText::FromString(TEXT("Draw...")));
		return;
	}

	NextTurn();
}

void ABGGameModeBase::NextTurn()
{
	if (AllPlayerControllers.Num() == 0) return;

	// 다음 플레이어로 이동
	CurrentTurnPlayerIndex = (CurrentTurnPlayerIndex + 1) % AllPlayerControllers.Num();

	// 이미 기회를 다 쓴 플레이어는 건너뜀
	int32 LoopCount = 0;
	while (LoopCount < AllPlayerControllers.Num())
	{
		ABGPlayerController* NextPC = GetCurrentTurnPlayerController();
		if (IsValid(NextPC))
		{
			ABGPlayerState* NextPS = NextPC->GetPlayerState<ABGPlayerState>();
			if (IsValid(NextPS) && NextPS->CurrentGuessCount < NextPS->MaxGuessCount)
			{
				break;
			}
		}

		CurrentTurnPlayerIndex = (CurrentTurnPlayerIndex + 1) % AllPlayerControllers.Num();
		LoopCount++;
	}

	StartTurn();
}

ABGPlayerController* ABGGameModeBase::GetCurrentTurnPlayerController() const
{
	if (AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex))
	{
		return AllPlayerControllers[CurrentTurnPlayerIndex];
	}

	return nullptr;
}

void ABGGameModeBase::PrintChatMessageString(ABGPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	// 결과창 떠 있는 동안 입력 불가
	if (bIsWaitingRestart == true) return;
	
	// 시간이 끝나면 입력 불가
	if (CanPlayNumberBaseball() == false) return;
	
	// 현재 턴 플레이어만 입력 가능
	ABGPlayerController* CurrentTurnPC = GetCurrentTurnPlayerController();
	if (InChattingPlayerController != CurrentTurnPC)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("지금은 당신의 턴이 아닙니다."));
		return;
	}
	
	ABGPlayerState* BGPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
	if (IsValid(BGPS) == false) return;
	
	// 기회 소진 (턴이 지나가지만 예외용)
	if (BGPS->CurrentGuessCount >= BGPS->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("더 이상 입력할 수 없습니다."));
		return;
	}
	
	FString GuessNumberString = InChatMessageString;
	
	if (IsGuessNumberString(GuessNumberString) == true) 
	{
		bDidSubmitThisTurn = true;
		
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

		// 승리 판정
		if (StrikeCount == 3)
		{
			IncreaseGuessCount(InChattingPlayerController);
			JudgeGame(InChattingPlayerController, StrikeCount);
			return;
		}

		EndTurn();
	}
	else 
	{
		if (IsValid(InChattingPlayerController) == true)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("다시 입력하세요"));
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

void ABGGameModeBase::ResetGame(ABGPlayerController* InStartingPlayerController)
{
	StopRoundTimer();
	
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);

	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		ABGPlayerState* BGPS = BGPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(BGPS))
		{
			BGPS->CurrentGuessCount = 0;
		}

		if (IsValid(BGPlayerController))
		{
			BGPlayerController->ClientRPCHideResultWidget();
		}
	}

	ABGGameStateBase* BGGameState = GetGameState<ABGGameStateBase>();
	if (IsValid(BGGameState))
	{
		BGGameState->RemainingTime = BGGameState->MaxTime;
		BGGameState->bGameStarted = true;
		BGGameState->ForceNetUpdate();
	}
	
	bDidSubmitThisTurn = false;
	bIsWaitingRestart = false;
	
	// 리스타트 누른 플레이어를 시작 턴으로 설정
	if (IsValid(InStartingPlayerController))
	{
		int32 FoundIndex = AllPlayerControllers.IndexOfByKey(InStartingPlayerController);
		CurrentTurnPlayerIndex = (FoundIndex != INDEX_NONE) ? FoundIndex : 0;
	}
	else
	{
		CurrentTurnPlayerIndex = 0;
	}
	StartTurn();
}

void ABGGameModeBase::JudgeGame(ABGPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ABGPlayerState* WinnerPS = InChattingPlayerController->GetPlayerState<ABGPlayerState>();
		if (IsValid(WinnerPS))
		{
			FString ResultString = WinnerPS->PlayerNameString + TEXT(" Wins!");
			ShowResultToAllPlayers(FText::FromString(ResultString));
		}
	}
}

void ABGGameModeBase::StartGameByPlayer(ABGPlayerController* InStartingPlayerController)
{
	ABGGameStateBase* BGGameState = GetGameState<ABGGameStateBase>();
	if (IsValid(BGGameState) && BGGameState->bGameStarted == true) return;
	if (IsValid(InStartingPlayerController) == false) return;

	const int32 FoundIndex = AllPlayerControllers.Find(InStartingPlayerController);
	if (FoundIndex == INDEX_NONE) return;
	
	if (IsValid(BGGameState))
	{
		BGGameState->bGameStarted = true;
	}
	
	bIsWaitingRestart = false;
	bDidSubmitThisTurn = false;
	CurrentTurnPlayerIndex = FoundIndex;

	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp, Error, TEXT("%s"), *SecretNumberString);

	for (ABGPlayerController* BGPlayerController : AllPlayerControllers)
	{
		if (IsValid(BGPlayerController))
		{
			BGPlayerController->ClientRPCHideStartWidget();
		}
	}

	StartTurn();
}

void ABGGameModeBase::ShowResultToAllPlayers(const FText& InResultText)
{
	bIsWaitingRestart = true;
	StopRoundTimer();

	for (const auto& BGPlayerController : AllPlayerControllers)
	{
		if (IsValid(BGPlayerController))
		{
			BGPlayerController->ClientRPCShowResultWidget(InResultText);
		}
	}
}
