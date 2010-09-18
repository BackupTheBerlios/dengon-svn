#ifndef USERNOTIFICATION_H
#define USERNOTIFICATION_H

class BNotification;

class UserNotification
{
public:
	static void Notify(const BString& title, const BString& msg);
private:
	static void CheckNotificationExists();	
	static BNotification * notification;
};

#endif
