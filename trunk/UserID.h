//////////////////////////////////////////////////
// Blabber [UserID.h]
//     Encapsulates user data for any account,
//     Jabber or otherwise.
//////////////////////////////////////////////////

#ifndef USER_ID_H
#define USER_ID_H

#include <string>

class UserID
{
	public:
	
		enum online_status  { UNKNOWN, UNACCEPTED, OFFLINE,	ONLINE, TRANSPORT_ONLINE, CONF_STATUS };
		enum affiliation	{ OWNER, ADMIN, MODERATOR, MEMBER, GUEST };
		enum user_type      { INVALID, JABBER, TRANSPORT, CONFERENCE };

          	             		UserID(std::string username);
          	             		UserID(const UserID &copied_userid);
		virtual             	~UserID();

				UserID          &operator=(const UserID &rhs);

		const	user_type		UserType() const;
		const	online_status	OnlineStatus() const;

		const	std::string		Handle() const;
		const	std::string		FriendlyName() const;
		const	std::string		SubscriptionStatus() const;
		const	std::string		ExactOnlineStatus() const;
		const	std::string		MoreExactOnlineStatus() const;


				bool			HaveSubscriptionTo() const;
				bool			IsUser() const;

		const	std::string		JabberHandle() const;         // xxx@yyy
		const	std::string		JabberCompleteHandle() const; // xxx@yyy/zzz
		const	std::string		JabberUsername() const;       // xxx
		const	std::string		JabberServer() const;         // yyy
		const	std::string		JabberResource() const;       // zzz

	const std::string    TransportID() const;
	const std::string    TransportUsername() const;
	const std::string    TransportPassword() const;

				void			StripJabberResource();        // xxx@yyy/zzz -> xxx@yyy
				std::string		WhyNotValidJabberHandle();

				void            SetUsertype(user_type usertype);
				void            SetOnlineStatus(online_status status);

				void            SetHandle(std::string handle);
				void            SetFriendlyName(std::string friendly_name);
				void            SetSubscriptionStatus(std::string status);
				void            SetExactOnlineStatus(std::string exact_status);
				void            SetMoreExactOnlineStatus(std::string exact_status);

				void            _ProcessHandle();

				online_status   _status;
				user_type       _user_type;
				
				std::string		_handle;
				std::string		_friendly_name;
				std::string		_service;
				std::string		_ask;
				std::string		_exact_status;
				std::string		_more_exact_status;
				std::string		_subscription_status;

				std::string		_jabber_username;
				std::string		_jabber_server;
				std::string		_jabber_resource;

	std::string          _transport_id;
	std::string          _transport_username;
	std::string          _transport_password;
};

#endif
