#pragma once

#include <string>
#include <any>
#include <unordered_map>

class Blackboard
{
public:
	template<typename T>
	void SetValue( std::string const& key, T value )
	{
		m_data[key] = value;
	}

	template<typename T>
	T GetValue( std::string const& key )
	{
		return m_data[key];
	}

private:
	std::unordered_map<std::string, std::any> m_data;
};

