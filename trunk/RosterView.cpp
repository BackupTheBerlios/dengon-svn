//////////////////////////////////////////////////
// Blabber [RosterView.cpp]
//////////////////////////////////////////////////

#include "RosterView.h"
#include <cstdio>
#include <MenuItem.h>
#include "Settings.h"
#include "Messages.h"
#include "ModalAlertFactory.h"
#include <strings.h>

RosterView::RosterView(BRect frame)
	: BOutlineListView(frame, NULL, B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL) {
}

RosterView::~RosterView() {
	delete _popup;

	// remember superitem statuses
	BlabberSettings::Instance()->SetTag("online-collapsed", !_online->IsExpanded());
	//BlabberSettings::Instance()->SetTag("unaccepted-collapsed", !_unaccepted->IsExpanded());
	BlabberSettings::Instance()->SetTag("unknown-collapsed", !_unknown->IsExpanded());
	BlabberSettings::Instance()->SetTag("offline-collapsed", !_offline->IsExpanded());
	//BlabberSettings::Instance()->SetTag("transports-collapsed", !_transports->IsExpanded());
	BlabberSettings::Instance()->SetTag("conferences-collapsed", !_conferences->IsExpanded());
	BlabberSettings::Instance()->WriteToFile();
	
	SetInvocationMessage(new BMessage(JAB_OPEN_CHAT_WITH_DOUBLE_CLICK));
}

static int _ListComparison(const BListItem *a, const BListItem *b) {
	if ( 
	((RosterItem *)a)->GetUserID()->FriendlyName() ==
	((RosterItem *)b)->GetUserID()->FriendlyName())
		return 0;
	else if ( 
	((RosterItem *)a)->GetUserID()->FriendlyName() >
	((RosterItem *)b)->GetUserID()->FriendlyName()) 
		return 1;
	else return -1;

//	return strcasecmp(str_a, str_b);
}   

void RosterView::AttachedToWindow() {
	// superclass call	
	BOutlineListView::AttachedToWindow();

	// on double-click
	//SetInvocationMessage(new BMessage(JAB_OPEN_CHAT_WITH_DOUBLE_CLICK));

	// popup menu
	_popup = new BPopUpMenu(NULL, false, false);

		_chat_item         = new BMenuItem("Chat...", new BMessage(JAB_OPEN_CHAT));
		_chat_item->SetEnabled(false);
		//_message_item      = new BMenuItem("Send Message...", new BMessage(JAB_OPEN_MESSAGE));
		_change_user_item  = new BMenuItem("Edit", new BMessage(JAB_OPEN_EDIT_BUDDY_WINDOW));
		_change_user_item->SetEnabled(false);
		_remove_user_item  = new BMenuItem("Remove", new BMessage(JAB_REMOVE_BUDDY));
		_remove_user_item->SetEnabled(false);
		_user_info_item    = new BMenuItem("Userinfo", new BMessage(JAB_USER_INFO));
		_user_info_item->SetEnabled(false);
		_user_chatlog_item = new BMenuItem("Chatlog", new BMessage(JAB_SHOW_CHATLOG));
		_user_chatlog_item->SetEnabled(false);

		_presence          = new BMenu("Presence");
		_presence->SetEnabled(false);
			
			_subscribe_presence   = new BMenuItem("Subscribe", new BMessage(JAB_SUBSCRIBE_PRESENCE));
			_unsubscribe_presence = new BMenuItem("Unsubscribe", new BMessage(JAB_UNSUBSCRIBE_PRESENCE));
			_revoke_presence = new BMenuItem("Revoke", new BMessage(JAB_REVOKE_PRESENCE));
			_resend_presence = new BMenuItem("Resend", new BMessage(JAB_RESEND_PRESENCE));

			_presence->AddItem(_subscribe_presence);
			_presence->AddItem(_unsubscribe_presence);
			_presence->AddSeparatorItem();
			_presence->AddItem(_revoke_presence);
			_presence->AddItem(_resend_presence);

	_popup->AddItem(_chat_item);
	_popup->AddSeparatorItem();
	_popup->AddItem(_user_info_item);
	_popup->AddItem(_user_chatlog_item);
	_popup->AddSeparatorItem();
	_popup->AddItem(_change_user_item);
	_popup->AddItem(_remove_user_item);
	_popup->AddItem(_presence);

	// initialize menu
	UpdatePopUpMenu();

	// create top level lists
	AddItem(_online  = new RosterSuperitem("Online"));
	//AddItem(_unaccepted = new RosterSuperitem("Pending Presence"));
	AddItem(_offline = new RosterSuperitem("Offline"));
	AddItem(_unknown = new RosterSuperitem("No Presence"));
	AddItem(_conferences = new RosterSuperitem("Conferences"));
	//AddItem(_transports = new RosterSuperitem("Live Transports"));
	
	
	// make maps (BUGBUG better way to do two-way map?)
	_item_to_status_map[_offline] = UserID::OFFLINE;
	_item_to_status_map[_online]  = UserID::ONLINE;
	_item_to_status_map[_unknown] = UserID::UNKNOWN;
	//_item_to_status_map[_unaccepted] = UserID::UNACCEPTED;
	_item_to_status_map[_transports] = UserID::TRANSPORT;
	_item_to_status_map[_conferences] = UserID::CONF_STATUS;

	// ignore online...doesn't seem to work...?
	_offline->SetExpanded(!BlabberSettings::Instance()->Tag("offline-collapsed"));
	_unknown->SetExpanded(!BlabberSettings::Instance()->Tag("unknown-collapsed"));
	//_unaccepted->SetExpanded(!BlabberSettings::Instance()->Tag("unaccepted-collapsed"));
	//_transports->SetExpanded(!BlabberSettings::Instance()->Tag("transports-collapsed"));
	_conferences->SetExpanded(!BlabberSettings::Instance()->Tag("conferences-collapsed"));

	_status_to_item_map[UserID::OFFLINE] = _offline;
	_status_to_item_map[UserID::ONLINE]  = _online;
	_status_to_item_map[UserID::UNKNOWN] = _unknown;
	//_status_to_item_map[UserID::TRANSPORT] = _transports;
	//_status_to_item_map[UserID::UNACCEPTED] = _unaccepted;
	_status_to_item_map[UserID::CONF_STATUS] = _conferences;
	
	// BUGBUG events
	_presence->SetTargetForItems(Window());
	_popup->SetTargetForItems(Window());
}

RosterItem *RosterView::CurrentItemSelection() {
	int32 index = CurrentSelection();
	
	if (index >= 0) {
		return dynamic_cast<RosterItem *>(ItemAt(index));
	} else {
		return NULL;
	}
}

void RosterView::KeyDown(const char *bytes, int32 len) {
}

void RosterView::MouseDown(BPoint point) {
	// accept first click
	Window()->Activate(true);

	// get mouse info before it's too late!
	uint32 buttons = 0;
	GetMouse(&point, &buttons, true);

	// superclass stuff
	BOutlineListView::MouseDown(point);

	if (buttons & B_SECONDARY_MOUSE_BUTTON) {
		// update menu before presentation
		UpdatePopUpMenu();
		
		BPoint screen_point(point);
		ConvertToScreen(&screen_point);
		
		BRect r(screen_point.x - 4, screen_point.y - 20, screen_point.x + 24, screen_point.y + 4);
		_popup->Go(screen_point, true, true, r, false);
		//_popup->Go(screen_point, true, true, false);
	}
}

void RosterView::RemoveSelected()
{
	if (CurrentItemSelection())
	{
		// numeric and object based selections
		int32       selected = CurrentSelection();
		RosterItem *item     = CurrentItemSelection();
		
		if (item == NULL) {
			// not a roster item, won't remove
			return;
		}
		
		// remove item from view
		RemoveItem(CurrentSelection());

		// select next buddy for continuity
		if (ItemAt(selected))
			Select(selected);
		else if (ItemAt(selected - 1))
			Select(selected - 1);
	}
}

void RosterView::SelectionChanged() {
	// customize popup menu
	UpdatePopUpMenu();
	
	BOutlineListView::SelectionChanged();
}

void RosterView::LinkUser(const UserID *added_user)
{
	if (added_user->UserType() == UserID::CONFERENCE)
	{
		AddUnder(new RosterItem(added_user), _conferences);
	}
	else if (added_user->UserType() == UserID::JABBER)
	{
		if (added_user->SubscriptionStatus() == "none")
			AddUnder(new RosterItem(added_user), _unknown);
		else 
			AddUnder(new RosterItem(added_user), _offline);
	}
}

void RosterView::UnlinkUser(const UserID *removed_user) {
	// does user exist
	uint32 index = FindUser(removed_user);
	
	if (index >= 0) {
		RemoveItem(index);	
	}
}

int32 RosterView::FindUser(const UserID *compare_user) {
	// handle NULL argument
	if (compare_user == NULL) {
		return -1;
	}

	for (int i=0; i<FullListCountItems(); ++i) {
		// get item
		RosterItem *item = dynamic_cast<RosterItem *>(FullListItemAt(i));

		if (item == NULL || item->StalePointer()) {
			continue;
		}
				
		// compare against RosterView
		if (item->GetUserID() == compare_user) {
			return i;
		}
	}

	// no match
	return -1;
}

void
RosterView::UpdatePopUpMenu()
{
	char buffer[1024];

	RosterItem *item = CurrentItemSelection();

	if (item && !item->StalePointer()) {
		const UserID *user = item->GetUserID();

		// if an item is selected
		_chat_item->SetEnabled(true);
		_change_user_item->SetEnabled(true);
		_remove_user_item->SetEnabled(true);
		//_user_info_item->SetEnabled(true);
		_user_chatlog_item->SetEnabled(BlabberSettings::Instance()->Tag("autoopen-chatlog"));
		_presence->SetEnabled(true);

		if (user->HaveSubscriptionTo()) {
			_subscribe_presence->SetEnabled(false);
			_unsubscribe_presence->SetEnabled(true);
		} else {
			_subscribe_presence->SetEnabled(true);
			_unsubscribe_presence->SetEnabled(false);
		}
	} else {		
		// if not
		_chat_item->SetEnabled(false);
		_change_user_item->SetEnabled(false);
		_remove_user_item->SetEnabled(false);
		_user_info_item->SetEnabled(false);
		_user_chatlog_item->SetEnabled(false);
		_presence->SetEnabled(false);
	}
}

void RosterView::UpdateRoster()
{
	JRoster *roster = JRoster::Instance();

	roster->Lock();

	// add entries from JRoster that are not in RosterView
	for (JRoster::ConstRosterIter i = roster->BeginIterator(); i != roster->EndIterator(); ++i)
	{
		if (FindUser(*i) < 0)
		{
			// this entry does not exist in the RosterView
			LinkUser(*i);
		} 
	}

	RESET:
	
	// adjust online status of users
	for (int i = 0; i < FullListCountItems(); ++i)
	{
		RosterItem *item = dynamic_cast<RosterItem *>(FullListItemAt(i));
		
		// skip illegal entries
		if (item == NULL)
		{
			continue;
		}
		else
		{
			
			// process removals
			if (!roster->ExistingUserObject(item->GetUserID()) || !roster->FindUser(item->GetUserID()))
			{
				item->SetStalePointer(true);
				RemoveItem(item);
				fprintf(stderr, "RostetView::UpdateRoster process removomals");
				goto RESET;
			}
			
		
			// change of statuses
			if (item->GetUserID()->OnlineStatus() != _item_to_status_map[Superitem(item)])
			{
				UserID::online_status old_status = _item_to_status_map[Superitem(item)];
				RemoveItem(item);
				AddUnder(item, _status_to_item_map[item->GetUserID()->OnlineStatus()]);
				goto RESET;
			}			

			// clean it
			InvalidateItem(i);
		} 

	}
	
	SortItemsUnder(_online, true, _ListComparison);
	SortItemsUnder(_offline, true, _ListComparison);
	SortItemsUnder(_unknown, true, _ListComparison);
	SortItemsUnder(_conferences, true, _ListComparison);
	
	Invalidate();

	roster->Unlock();
}
