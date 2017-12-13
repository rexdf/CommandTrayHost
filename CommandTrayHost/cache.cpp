#include "stdafx.h"
#include "language.h"
#include "configure.h"
#include "utils.hpp"

extern nlohmann::json global_stat;
extern int number_of_configs;

extern bool enable_cache;
extern bool disable_cache_position;
extern bool disable_cache_size;
extern bool disable_cache_enabled;
extern bool disable_cache_show;
extern bool is_cache_valid;

bool is_cache_not_expired()
{
	PCWSTR json_filename = CONFIG_FILENAMEW;
	PCWSTR cache_filename = CACHE_FILENAMEW;
	if (TRUE != PathFileExists(cache_filename))
	{
		LOGMESSAGE(L"PathFileExists failed\n");
		return false;
	}
	HANDLE json_hFile = CreateFile(json_filename, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE cache_hFile = CreateFile(cache_filename, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

#define CLOSE_CREATEFILE(fp) { \
	if(fp)CloseHandle(fp); \
	fp=NULL; \
}

#define RETURN_AND_CLOSE_CREATEFILE(ret) { \
	CLOSE_CREATEFILE(json_hFile);\
	CLOSE_CREATEFILE(cache_hFile);\
	return ret; \
}
	if (!json_hFile || !cache_hFile)
	{
		LOGMESSAGE(L"CreateFile failed\n");
		RETURN_AND_CLOSE_CREATEFILE(false);
	}
	FILETIME json_write_timestamp, cache_write_timestamp;
	if (!GetFileTime(json_hFile, NULL, NULL, &json_write_timestamp) ||
		!GetFileTime(cache_hFile, NULL, NULL, &cache_write_timestamp))
	{
		LOGMESSAGE(L"GetFileTime failed\n");
		RETURN_AND_CLOSE_CREATEFILE(false);
	}
	if (CompareFileTime(&json_write_timestamp, &cache_write_timestamp) >= 0)
	{
		LOGMESSAGE(L"json_write_timestamp is later than cache_write_timestamp\n");
		RETURN_AND_CLOSE_CREATEFILE(false);
	}
	RETURN_AND_CLOSE_CREATEFILE(true);
}

bool flush_cache()
{
	assert(enable_cache);
	assert(false == is_cache_valid);
	LOGMESSAGE(L"Now flush cache\n");
	is_cache_valid = true;
	std::ofstream o(CACHE_FILENAMEA);
	if (o.good())
	{
#ifdef _DEBUG
		o << global_stat["cache"].dump(4);
#else
		o << global_stat["cache"];
#endif
		return true;
	}
	else
	{
		return false;
	}
}

