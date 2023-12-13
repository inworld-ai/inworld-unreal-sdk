// Copyright 2023 Theai, Inc. (DBA Inworld) All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "InworldEditorNotification"

class FInworldEditorRestartRequiredNotification
{
public:
	void SetOnRestartApplicationCallback(FSimpleDelegate InRestartApplicationDelegate)
	{
		RestartApplicationDelegate = InRestartApplicationDelegate;
	}

	void OnRestartRequired()
	{
		TSharedPtr<SNotificationItem> NotificationPin = NotificationPtr.Pin();
		if (NotificationPin.IsValid() || !RestartApplicationDelegate.IsBound())
		{
			return;
		}

		FNotificationInfo Info(LOCTEXT("RestartRequiredTitle", "Restart required to apply new InworldAI changes."));

		// Add the buttons with text, tooltip and callback
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			LOCTEXT("RestartNow", "Restart Now"),
			LOCTEXT("RestartNowToolTip", "InworldAI editor requires reset to apply changes."),
			FSimpleDelegate::CreateRaw(this, &FInworldEditorRestartRequiredNotification::OnRestartClicked))
		);
		Info.ButtonDetails.Add(FNotificationButtonInfo(
			LOCTEXT("RestartLater", "Restart Later"),
			LOCTEXT("RestartLaterToolTip", "Dismiss this notificaton without restarting. New InworldAI changes will not be applied."),
			FSimpleDelegate::CreateRaw(this, &FInworldEditorRestartRequiredNotification::OnDismissClicked))
		);

		// We will be keeping track of this ourselves
		Info.bFireAndForget = false;

		// Set the width so that the notification doesn't resize as its text changes
		Info.WidthOverride = 300.0f;

		Info.bUseLargeFont = false;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = false;

		// Launch notification
		NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationPin = NotificationPtr.Pin();

		if (NotificationPin.IsValid())
		{
			NotificationPin->SetCompletionState(SNotificationItem::CS_Pending);
		}
	}

private:
	void OnRestartClicked()
	{
		TSharedPtr<SNotificationItem> NotificationPin = NotificationPtr.Pin();
		if (NotificationPin.IsValid())
		{
			NotificationPin->SetText(LOCTEXT("RestartingNow", "Restarting..."));
			NotificationPin->SetCompletionState(SNotificationItem::CS_Success);
			NotificationPin->ExpireAndFadeout();
			NotificationPtr.Reset();
		}

		RestartApplicationDelegate.ExecuteIfBound();
	}

	void OnDismissClicked()
	{
		TSharedPtr<SNotificationItem> NotificationPin = NotificationPtr.Pin();
		if (NotificationPin.IsValid())
		{
			NotificationPin->SetText(LOCTEXT("RestartDismissed", "Restart Dismissed..."));
			NotificationPin->SetCompletionState(SNotificationItem::CS_None);
			NotificationPin->ExpireAndFadeout();
			NotificationPtr.Reset();
		}
	}

	/** Used to reference to the active restart notification */
	TWeakPtr<SNotificationItem> NotificationPtr;

	/** Used to actually restart the application */
	FSimpleDelegate RestartApplicationDelegate;
};

#undef LOCTEXT_NAMESPACE
