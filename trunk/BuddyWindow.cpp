//////////////////////////////////////////////////
// Blabber [BuddyWindow.cpp]
//////////////////////////////////////////////////

#include "BuddyWindow.h"
#include <cstdio>
#include <Application.h>
#include "Agent.h"
#include "AgentList.h"
#include "AppLocation.h"
#include "Settings.h"
#include "GenericFunctions.h"
#include "Messages.h"
#include "ModalAlertFactory.h"
#include "JRoster.h"
#include "TalkManager.h"
#include <strings.h>

BuddyWindow *BuddyWindow::_instance = NULL;

BuddyWindow *BuddyWindow::Instance()
{
	if (_instance == NULL) {
		float main_window_width  = 340;
		float main_window_height = 165;
		
		// create window frame position
		BRect frame(GenericFunctions::CenteredFrame(main_window_width, main_window_height));

		// create window singleton
		_instance = new BuddyWindow(frame, NULL);
	}
	
	return _instance;
}

BuddyWindow::~BuddyWindow() {
	_instance = NULL;
}

void
BuddyWindow::SetUser(UserID *user)
{
	userID = user;
	
	if (userID)
	{
		_realname->SetText(userID->FriendlyName().c_str());
		_handle->SetText(userID->JabberHandle().c_str());
	}
			
	_chat_services->SetEnabled(!userID);
	_handle->SetEnabled(!userID);
	
	if (userID && userID->OnlineStatus() == UserID::CONF_STATUS)
	 	_chat_services_selection->FindItem("Conference")->SetMarked(true);
	else
		_chat_services_selection->FindItem("User")->SetMarked(true);
	
	if (userID)
		_ok->SetLabel("Save");
	else
		_ok->SetLabel("Create");
}

void
BuddyWindow::ApplyChangesToUser()
{
	string username = _handle->Text();
	userID = new UserID(UserID(username));
	userID->SetFriendlyName(_realname->Text());
}

BuddyWindow::BuddyWindow(BRect frame, UserID *user)
	: BWindow(frame, "Contact", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{

	BRect rect;

	// encompassing view
	rect = Bounds();
	rect.OffsetTo(B_ORIGIN);
	
	_full_view = new BView(rect, NULL, B_FOLLOW_ALL, B_WILL_DRAW);
	_full_view->SetViewColor(216, 216, 216, 255);
	
	rect.OffsetTo(B_ORIGIN);
	rect.InsetBy(10.0, 12.0);
	
	// realname
	rect.bottom = rect.top + 18;
	_realname = new BTextControl(rect, "realname", "Username:", NULL, NULL, B_FOLLOW_ALL_SIDES);
	_realname->SetDivider(_realname->Divider() - 75);
	
	rect.OffsetBy(0.0, 23.0);
	_handle = new BTextControl(rect, "handle", "Jabber ID:", NULL, NULL, B_FOLLOW_ALL_SIDES);
	_handle->SetDivider(_handle->Divider() - 75);
	
	
	// chat service
	rect.OffsetBy(0.0, 23.0);
	_chat_services_selection = new BPopUpMenu("Simple");
	_chat_services = new BMenuField(rect, "chat_services", "Item Type: ", _chat_services_selection);	
	_chat_services->SetDivider(_chat_services->Divider() - 75);
	_chat_services_selection->AddItem(new BMenuItem("User", new BMessage(AGENT_MENU_CHANGED_TO_JABBER)));
	_chat_services_selection->AddItem(new BMenuItem("Conference", new BMessage(AGENT_MENU_CHANGED_TO_JABBER_CONFERENCE)));
	// ok button
	rect.OffsetBy(220.0, 55.0);
	rect.right = rect.left + 92;

	_ok = new BButton(rect, "ok", "", new BMessage(JAB_OK));
	_ok->MakeDefault(true);
	_ok->SetTarget(this);
	

	// add GUI components to BView
	_full_view->AddChild(_realname);
	_full_view->AddChild(_handle);
	_full_view->AddChild(_chat_services);
	_full_view->AddChild(_ok);
	AddChild(_full_view);

	// focus to start
	_realname->MakeFocus(true);
}

void BuddyWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case JAB_OK: {
			AddNewUser();
			break;
		}
		
		case JAB_CANCEL: {
			PostMessage(B_QUIT_REQUESTED);
			break;
		}

		case AGENT_MENU_CHANGED_TO_JABBER: {
			//_enter_note->SetText("Please enter a User JID (e.g., joe@xmpp.org).");
			_handle->SetLabel("JID:");
			_realname->SetLabel("Username:");
			break;
		}
		
		case AGENT_MENU_CHANGED_TO_JABBER_CONFERENCE: {
			//_enter_note->SetText("Please enter a Conference JID (e.g., haiku@jabber.jp).");
			_handle->SetLabel("MUC JID:");
			_realname->SetLabel("Conference:");
			break;
		}

		case AGENT_MENU_CHANGED_TO_YAHOO: {
			_enter_note->SetText("Please enter the user's Yahoo! IM screenname or Yahoo.com login (e.g., YahooMan14).");
			_handle->SetLabel("Yahoo ID:");
			break;
		}

		case AGENT_MENU_CHANGED_TO_ICQ: {
			_enter_note->SetText("Please enter the user's ICQ numeric ID (e.g., 99818234).");
			_handle->SetLabel("ICQ #:");
			break;
		}

		case AGENT_MENU_CHANGED_TO_AIM: {
			_enter_note->SetText("Please enter the user's AOL screenname or AIM screenname (e.g., BeOSLover213).");
			_handle->SetLabel("AOL Screen Name:");
			break;
		}

		case AGENT_MENU_CHANGED_TO_MSN: {
			_enter_note->SetText("Please enter the user's Hotmail or Passport username excluding the domain (e.g., ZaBlanc and not ZaBlanc@hotmail.com).");
			_handle->SetLabel("Passport Sign-In:");
			break;
		}
	}
}

bool BuddyWindow::QuitRequested() {
	_instance = NULL;
	return true;
}

void BuddyWindow::AddNewUser()
{
	char buffer[4096];

	if (!strcmp(_realname->Text(), "")) {
		ModalAlertFactory::Alert("Please specify your buddy's real name.", "Oops!");
		_realname->MakeFocus(true);

		return;
	}
	
	if (!strcmp(_handle->Text(), "")) {
		sprintf(buffer, "Please specify %s's %s handle (or screenname).", _realname->Text(), (_chat_services_selection->FindMarked())->Label()); 
		ModalAlertFactory::Alert(buffer, "Oops!");
		_handle->MakeFocus(true);

		return;
	}
	
	// internally replace the username with a proper one if necessary (AOL, Yahoo!, etc...)
	Agent *agent;
	string username = _handle->Text();

	// if not Jabber
	if (strcasecmp(_handle->Label(), "Jabber ID:")) {
		username = GenericFunctions::CrushOutWhitespace(username);
	}

	// make a user to validate against	
	UserID validating_user(username);
	
	if (!strcasecmp(_handle->Label(), "Jabber ID:") && validating_user.WhyNotValidJabberHandle().size()) {
		sprintf(buffer, "%s is not a valid Jabber ID for the following reason:\n\n%s\n\nPlease correct it.", _handle->Text(), validating_user.WhyNotValidJabberHandle().c_str()); 
		ModalAlertFactory::Alert(buffer, "Hmm, better check that...");
		_handle->MakeFocus(true);
		
		return;
	}
	
	if (userID == NULL)
	{
	// make sure it's not a duplicate of one already existing (unless itself)
	JRoster::Instance()->Lock();
	if (JRoster::Instance()->FindUser(JRoster::COMPLETE_HANDLE, _handle->Text())) {
		sprintf(buffer, "%s already exists in your buddy list.  Please choose another so you won't get confused.", _handle->Text()); 
		ModalAlertFactory::Alert(buffer, "Good Idea!");
		_handle->MakeFocus(true);
		
		JRoster::Instance()->Unlock();
		return;
	}
	JRoster::Instance()->Unlock();
	}

	// create a new user
	UserID *new_user;
	
	if (userID) // edit existing
	{
		new_user = userID;
		userID = NULL;
	} else 
	{
		new_user = new UserID(UserID(username));

		if (!strcasecmp(_handle->Label(), "Jabber ID:"))
		{
			new_user->SetUsertype(UserID::JABBER);
		} else {
			new_user->SetUsertype(UserID::CONFERENCE);
			new_user->SetOnlineStatus(UserID::CONF_STATUS);
		}
	}
	
	new_user->SetFriendlyName(_realname->Text());
		
	TalkManager::Instance()->jabber->AddToRoster(new_user);
	
	// add this user to the roster
	JRoster::Instance()->Lock();
	JRoster::Instance()->AddNewUser(new_user);
	JRoster::Instance()->Unlock();

	// alert all RosterViews
	//JRoster::Instance()->RefreshRoster();
	
	// close window explicitly
	PostMessage(B_QUIT_REQUESTED);
}
