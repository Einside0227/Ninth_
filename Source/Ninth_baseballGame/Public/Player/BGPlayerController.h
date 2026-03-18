
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BGPlayerController.generated.h"

class UBGChatInput;
class UUserWidget;

UCLASS()
class NINTH_BASEBALLGAME_API ABGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABGPlayerController();
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetChatMessageString(const FString& InChatMessageString);
	void PrintChatMessageString(const FString& InChatMessageString);
	
	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);
	
	// 게임 시작창 숨김
	UFUNCTION(Client, Reliable)
	void ClientRPCHideStartWidget();
	
	// Start 버튼 눌렀을 때 게임 시작
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRPCRequestGameStart();
	
	// 결과창 표시
	UFUNCTION(Client, Reliable)
	void ClientRPCShowResultWidget(const FText& InResultText);

	// 결과창 숨김
	UFUNCTION(Client, Reliable)
	void ClientRPCHideResultWidget();

	// Restart 버튼 눌렀을 때 서버에 재시작 요청
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRPCRequestRestart();

protected:
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UBGChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UBGChatInput> ChatInputWidgetInstance;

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;
	
	// 시작 위젯 블루프린트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TSubclassOf<UUserWidget> StartWidgetClass;

	UPROPERTY()
	UUserWidget* StartWidgetInstance;
	
	// 결과 위젯 블루프린트
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UUserWidget> ResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> ResultWidgetInstance;
	
	FString ChatMessageString;
	
public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;
};
