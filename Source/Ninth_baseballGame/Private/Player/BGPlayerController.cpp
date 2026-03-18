#include "Player/BGPlayerController.h"

#include "Ninth_baseballGame/Ninth_baseballGame.h"
#include "UI/BGChatInput.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Game/BGGameModeBase.h"
#include "Game/BGGameStateBase.h"
#include "Net/UnrealNetwork.h"

ABGPlayerController::ABGPlayerController()
{
	bReplicates = true;
}

void ABGPlayerController::BeginPlay() 
{
	Super::BeginPlay();
	
	if (IsLocalController() == false) return;
	
	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);
	
	bShowMouseCursor = true;

	if (IsValid(ChatInputWidgetClass) == true) 	
	{
		ChatInputWidgetInstance = CreateWidget<UBGChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true) 
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}
	
	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
	ABGGameStateBase* BGGameState = GetWorld()->GetGameState<ABGGameStateBase>();
	
	if (IsValid(StartWidgetClass) && (!IsValid(BGGameState) || BGGameState->bGameStarted == false))
	{
		StartWidgetInstance = CreateWidget<UUserWidget>(this, StartWidgetClass);
		if (IsValid(StartWidgetInstance))
		{
			StartWidgetInstance->AddToViewport(200);
		}
	}
	
	if (IsValid(ResultWidgetClass) == true)
	{
		ResultWidgetInstance = CreateWidget<UUserWidget>(this, ResultWidgetClass);
		if (IsValid(ResultWidgetInstance) == true)
		{
			ResultWidgetInstance->AddToViewport(100);
			ResultWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ABGPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, NotificationText);
}

void ABGPlayerController::SetChatMessageString(const FString& InChatMessageString) 
{
	ChatMessageString = InChatMessageString;
	
	if (IsLocalController() == true) 
	{
		ServerRPCPrintChatMessageString(InChatMessageString);
	}
}

void ABGPlayerController::PrintChatMessageString(const FString& InChatMessageString) 
{
	BGFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void ABGPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString) 
{
	PrintChatMessageString(InChatMessageString);
}

void ABGPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString) 
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true) 
	{
		ABGGameModeBase* BGGM = Cast<ABGGameModeBase>(GM);
		if (IsValid(BGGM) == true) 
		{
			BGGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void ABGPlayerController::ClientRPCHideStartWidget_Implementation()
{
	if (IsValid(StartWidgetInstance))
	{
		StartWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABGPlayerController::ServerRPCRequestGameStart_Implementation()
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM))
	{
		ABGGameModeBase* BGGM = Cast<ABGGameModeBase>(GM);
		if (IsValid(BGGM))
		{
			BGGM->StartGameByPlayer(this);
		}
	}
}

void ABGPlayerController::ClientRPCShowResultWidget_Implementation(const FText& InResultText)
{
	if (IsValid(ResultWidgetInstance) == false) return;

	ResultWidgetInstance->SetVisibility(ESlateVisibility::Visible);

	if (UTextBlock* ResultTextBlock = Cast<UTextBlock>(ResultWidgetInstance->GetWidgetFromName(TEXT("ResultText"))))
	{
		ResultTextBlock->SetText(InResultText);
	}
}

void ABGPlayerController::ClientRPCHideResultWidget_Implementation()
{
	if (IsValid(ResultWidgetInstance))
	{
		ResultWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABGPlayerController::ServerRPCRequestRestart_Implementation()
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM))
	{
		ABGGameModeBase* BGGM = Cast<ABGGameModeBase>(GM);
		if (IsValid(BGGM))
		{
			BGGM->ResetGame(this);
		}
	}
}
