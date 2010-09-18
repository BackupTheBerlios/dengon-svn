#include <Application.h>
#include <Roster.h>
#include <Notification.h>
#include <Resources.h>
#include <TranslationUtils.h>
#include "UserNotification.h"

BNotification * UserNotification::notification = NULL;

void UserNotification::Notify(const BString& title, const BString& msg)
{
	if(!msg.Length()) 
		return;
	UserNotification::CheckNotificationExists();
	UserNotification::notification->SetTitle(title);
	UserNotification::notification->SetContent(msg);
	be_roster->Notify(*UserNotification::notification, 5000000);	
}

void UserNotification::CheckNotificationExists()
{
	if(UserNotification::notification)
		return;
	UserNotification::notification = new BNotification(B_INFORMATION_NOTIFICATION);
	UserNotification::notification->SetApplication(be_app->Name());
	BResources * res = be_app->AppResources();
	if(res->HasResource('VICN', "BEOS:ICON"))
	{
		size_t size;		
		const void * data = res->LoadResource('VICN', "BEOS:ICON", &size);
		BMemoryIO * memio = new BMemoryIO(data, size);
		BBitmap * icon = BTranslationUtils::GetBitmap(memio);
		if(icon)
			UserNotification::notification->SetIcon(icon);
	}
}
