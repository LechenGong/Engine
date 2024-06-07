#include "Engine/Core/FileUtil.hpp"

bool FileReadToBuffer( std::vector<uint8_t>& outBuffer, std::string const& fileName )
{
	FILE* loadedFile;
	errno_t err = fopen_s( &loadedFile, fileName.c_str(), "rb" );
	if (err != 0)
	{
		return false;
	}
	else
	{
		int seekErr = fseek( loadedFile, 0, SEEK_END );
		if (seekErr != 0)
		{
			fclose( loadedFile );
			return false;
		}
		else
		{
			int size = ftell( loadedFile );
			outBuffer.resize( size );
			fseek( loadedFile, 0, SEEK_SET );
			fread( outBuffer.data(), 1, (size_t)size, loadedFile );
			fclose( loadedFile );
		}
	}

	return true;
}

bool FileReadToString( std::string& outString, std::string const& fileName )
{
	std::vector<uint8_t> buffer;
	if (!FileReadToBuffer( buffer, fileName ))
	{
		return false;
	}
	buffer.push_back( '\0' );
	outString = (char*)buffer.data();
	return true;
}

bool FileWriteToBuffer( std::vector<uint8_t> const& buffer, std::string const& filePath )
{
	FILE* file = nullptr;
	errno_t result = fopen_s( &file, filePath.c_str(), "wb" );
	if (result != NULL || file == nullptr)
	{
		return false;
	}

	fwrite( buffer.data(), 1, buffer.size(), file );
	fclose( file );
	return true;
}
