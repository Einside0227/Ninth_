#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BGGameModeBase.generated.h"

class ABGPlayerController;

UCLASS()
class NINTH_BASEBALLGAME_API ABGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	
	// 타이머
	void TickTimer();
	void StartRoundTimer();
	void StopRoundTimer();

	bool CanPlayNumberBaseball() const;
	
	// 턴 시스템
	void StartTurn();
	void EndTurn();
	void NextTurn();
	ABGPlayerController* GetCurrentTurnPlayerController() const;
	
	// 채팅(숫자 입력) 처리
	void PrintChatMessageString(ABGPlayerController* InChattingPlayerController, const FString& InChatMessageString);
	
	// 숫자 생성 / 입력 검사 / 결과 판정
	FString GenerateSecretNumber();
	bool IsGuessNumberString(const FString& InNumberString);
	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);
	
	void IncreaseGuessCount(ABGPlayerController* InChattingPlayerController);
	void ResetGame(ABGPlayerController* InStartingPlayerController);
	void JudgeGame(ABGPlayerController* InChattingPlayerController, int InStrikeCount);
	
	// 게임 시작
	void StartGameByPlayer(ABGPlayerController* InStartingPlayerController);
	
	// 결과창
	void ShowResultToAllPlayers(const FText& InResultText);
	
protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ABGPlayerController>> AllPlayerControllers;
	
	FTimerHandle RoundTimerHandle;
	
	// 현재 턴 플레이어 인덱스
	int32 CurrentTurnPlayerIndex = 0;

	// 이번 턴에 숫자를 입력했는지 여부
	bool bDidSubmitThisTurn = false;
	
	bool bIsWaitingRestart = false;
};
