#include <string>

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

EventSystem* g_eventSystem = nullptr;

EventSystem::EventSystem( EventSystemConfig const& config )
	: m_config( config )
{
}

EventSystem::~EventSystem()
{
}

void EventSystem::Startup()
{
}

void EventSystem::Shutdown()
{
}

void EventSystem::BeginFrame()
{
}

void EventSystem::EndFrame()
{
}

void EventSystem::SubscribeEventCallBackFunc( std::string const& eventName, void(*functionPtr)(), int numberOfArgs, std::string formatting )
{
	for (EventSubscription& subIndex : m_subscriptionListByName[eventName])
	{
		if (subIndex.functionPtr == nullptr)
		{
			subIndex.functionPtr = functionPtr;
			subIndex.numberOfArgs = numberOfArgs;
			subIndex.formatting = formatting;
			return;
		}
	}
	m_subscriptionListByName[eventName].push_back( EventSubscription{ functionPtr, numberOfArgs, formatting } );
}

void EventSystem::UnsubscribeEventCallbackFunc( std::string const& eventName, void(*functionPtr)() )
{
	for (EventSubscription& subIndex : m_subscriptionListByName[eventName])
	{
		if (subIndex.functionPtr == functionPtr)
		{
			subIndex.functionPtr = nullptr;
			return;
		}
	}
}

// int EventSystem::FireEvent( std::string const& eventName )
// {
// 	EventArgs args;
// 	return FireEvent( eventName, args );
// }

// int EventSystem::FireEvent( std::string const& eventName, EventArgs& args )
// {
// 	int counter = 0;
// 	for (EventSubscription subIndex : m_subscriptionListByName[eventName])
// 	{
// 		EventCallBackFunc func = subIndex.functionPtr;
// 		if (func != nullptr)
// 		{
// 			func( args );
// 			counter++;
// 		}
// 	}
// 	return counter;
// }

int EventSystem::FireEvent( std::string args )
{
	Strings strs;
	if (args[0] != '"')
		strs = Split( args, " " );
	else
		strs.push_back( args );
	return FireEvent( strs );
}

int EventSystem::FireEvent( Strings args )
{
	std::string eventName = args[0];
	if (m_subscriptionListByName.find( eventName ) == m_subscriptionListByName.end())
		return 0;

	int counter = 0;
	for (EventSubscription subIndex : m_subscriptionListByName[eventName])
	{
		if (subIndex.functionPtr == nullptr)
			continue;
		if (args.size() != subIndex.numberOfArgs)
		{
			g_devConsole->AddLine( g_devConsole->ERRORMSG, Stringf( "%s event requires %d arguments but received %d", eventName.c_str(), subIndex.numberOfArgs, args.size() ) );
			g_devConsole->AddLine( g_devConsole->ERRORMSG, Stringf( "correct format is: %s", subIndex.formatting.c_str() ) );
			continue;
		}

		bool flag = false;
		if (eventName == "pause")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "shutdown")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "control")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "help")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "history")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "clear")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "DRClear")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "DRToggle")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "funcinputdown")
		{
			flag = reinterpret_cast<bool(*)(unsigned int)>(subIndex.functionPtr)((unsigned int)std::stoul( args[1] ));
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "funcinputup")
		{
			flag = reinterpret_cast<bool(*)(unsigned int)>(subIndex.functionPtr)((unsigned int)std::stod( args[1] ));
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "killallenemies")
		{
			flag = reinterpret_cast<bool(*)()>(subIndex.functionPtr)();
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "litinput")
		{
			flag = reinterpret_cast<bool(*)(unsigned int)>(subIndex.functionPtr)((unsigned int)std::stoi( args[1] ));
			counter++;
			if (flag)
			{
				return flag;
			}
		}
		else if (eventName == "set")
		{
			flag = reinterpret_cast<bool(*)(char const*, float)>(subIndex.functionPtr)(args[1].c_str(), stof( args[2] ));
			counter++;
			if (flag)
			{
				return flag;
			}
		}
	}
	return counter;
}

bool EventSystem::IsEventSubscribed( std::string const& eventName )
{
	return m_subscriptionListByName.find( eventName ) != m_subscriptionListByName.end();
}

Strings EventSystem::GetAllSubscribedName()
{
	Strings names;
	for (auto &subscription : m_subscriptionListByName)
	{
		names.push_back( subscription.first );
	}
	return names;
}
