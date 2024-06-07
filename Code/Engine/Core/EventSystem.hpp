#pragma once

#include <vector>
#include <map>
#include <string>

#include "Engine/Core/NamedStrings.hpp"

typedef NamedStrings EventArgs;
typedef void (*EventCallBackFunc)(EventArgs);

struct EventSubscription
{
	//EventCallBackFunc functionPtr;
	void(*functionPtr)() = nullptr;
	int numberOfArgs = 1;
	std::string formatting;
};

typedef std::vector<EventSubscription> SubscriptionList;

struct EventSystemConfig
{

};

class EventSystem
{
public:
	EventSystem( EventSystemConfig const& config );
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeEventCallBackFunc( std::string const& eventName, void(*functionPtr)(), int numberOfArgs, std::string formatting = "" );
	void UnsubscribeEventCallbackFunc( std::string const& eventName, void(*functionPtr)() );
	//int  FireEvent( std::string const& eventName );
	//int	 FireEvent( std::string const& eventName, EventArgs& args );
	int	FireEvent( std::string args );
	int	FireEvent( Strings args );
	bool IsEventSubscribed( std::string const& eventName );

	Strings GetAllSubscribedName();

protected:
	EventSystemConfig m_config;
	std::map<std::string, SubscriptionList> m_subscriptionListByName;
};
