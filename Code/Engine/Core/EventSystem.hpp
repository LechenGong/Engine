#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>

#include <any>
#include <functional>
#include <utility>

#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"

typedef NamedStrings EventArgs;
typedef void (*EventCallBackFunc)(EventArgs);

struct EventSubscription
{
	//EventCallBackFunc functionPtr;
	//void(*functionPtr)() = nullptr;
	void* functionPtr = nullptr;
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
	
	template<typename Ret, typename... Args>
	void SubscribeEventCallBackFunc( std::string const& eventName, Ret( *func )(Args...) )
	{
		m_subscriptionMutex.lock();
	
		for (EventSubscription& registeredFunction : m_subscriptionListByName[eventName]) 
		{
			if (registeredFunction.functionPtr == reinterpret_cast<void*>(func))
			{
				m_subscriptionMutex.unlock();
				return;
			}
		}

		for (EventSubscription& registeredFunctions : m_subscriptionListByName[eventName])
		{
			if (registeredFunctions.functionPtr == nullptr)
			{
				registeredFunctions.functionPtr = reinterpret_cast<void*>(*func);
				m_subscriptionMutex.unlock();
				return;
			}
		}

		m_subscriptionListByName[eventName].push_back( EventSubscription{ reinterpret_cast<void(*)(Args...)>(func), 0, "" } );

		m_subscriptionMutex.unlock();
	};

	void UnsubscribeEventCallbackFunc( std::string const& eventName, void(*functionPtr)() );
	//int  FireEvent( std::string const& eventName );
	//int	 FireEvent( std::string const& eventName, EventArgs& args );
	int	FireEvent( std::string args );
	int	FireEvent( Strings args );

	template<typename Arg>
	decltype(auto) ConvertArgument( Arg&& arg ) {
		if constexpr (std::is_same_v<std::decay_t<Arg>, std::string>) 
		{
			return TryConvert( arg );
		}
		else 
		{
			return std::forward<Arg>( arg );
		}
	}

	std::unordered_map<std::string, SubscriptionList>::iterator CaseInsensitiveFind(
		std::unordered_map<std::string, SubscriptionList>& subscriptionList, const std::string& key );

	template<typename... Args>
	int FireEventEX( std::string const& eventName, Args&&... args )
	{
		int counter = 0;
		m_subscriptionMutex.lock();
		auto it = CaseInsensitiveFind( m_subscriptionListByName, eventName );
		if (it != m_subscriptionListByName.end()) 
		{
			for (auto& func : it->second) {
				// Cast the function pointer back and invoke it
				auto typedFunc = reinterpret_cast<bool(*)(Args...)>(func.functionPtr);
				bool flag = typedFunc( std::forward<Args>( args )... );
				if (flag)
				{
					m_subscriptionMutex.unlock();
					return counter;
				}
				counter++;
			}
		}
		m_subscriptionMutex.unlock();
		return counter;
	}

	bool IsEventSubscribed( std::string const& eventName );

	Strings GetAllSubscribedName();

protected:
	EventSystemConfig m_config;
	std::unordered_map<std::string, SubscriptionList> m_subscriptionListByName;
	mutable std::recursive_mutex m_subscriptionMutex;
};
