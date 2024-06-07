#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

typedef std::vector<std::string> Strings;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

Strings Split( std::string const& str, std::string const& delimiter, bool skip_empty = false, int maxOperationTime = 999999 );
Strings Split( std::string const& str, char delimiter, bool skip_empty = false, int maxOperationTime = 999999 );