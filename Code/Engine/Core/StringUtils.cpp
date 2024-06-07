#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

Strings Split( std::string const& str, std::string const& delimiter, bool skip_empty/* = false*/, int maxOperationTime/* = 999999*/ )
{
	Strings elems;
	std::string::size_type pos = 0, prev = 0;
	while ((pos = str.find_first_of( delimiter, prev )) != std::string::npos && maxOperationTime)
	{
		if (pos > prev)
		{
			elems.emplace_back( str, prev, pos - prev );
		}
		else if (!skip_empty && pos == prev)
		{
			elems.emplace_back( "" );
		}
		prev = pos + 1;
		maxOperationTime--;
	}

	if (prev < str.size())
	{
		elems.emplace_back( str, prev, str.size() - prev );
	}
	return elems;
}

Strings Split( std::string const& str, char delimiter, bool skip_empty/* = false*/, int maxOperationTime/* = 999999*/ )
{
	return Split( str, std::string( 1, delimiter ), skip_empty, maxOperationTime );
}
