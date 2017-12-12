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

void initial_write_cache()
{
	assert(enable_cache);
	if (false == is_cache_valid || false == is_cache_not_expired())
	{
		//generate cache from global_stat
		global_stat["cache"] = R"({"configs":[]})"_json;
		auto& cache_config_ref = global_stat["cache"]["configs"];
		auto& global_config_ref = global_stat["configs"];
		PCSTR names_to_sync[] = {
			"alpha",
			"enabled",
			"start_show",
#ifdef _DEBUG
			"name",
#endif
		};
		for (int i = 0; i < number_of_configs; i++)
		{
			auto& global_config_i_ref = global_config_ref[i];
			cache_config_ref[i] = u8R"json({
				"x": 0,
				"y": 0,
				"width": 0,
				"height": 0,
				"alpha": 170,
				"enabled": false,
				"start_show": false
			})json"_json;
			auto& cache_config_i_ref = cache_config_ref[i];
			if (json_object_has_member(global_config_i_ref, "alpha"))
			{
				int alpha = global_config_i_ref["alpha"];
				cache_config_i_ref["alpha"] = alpha;
			}
			for (int j = 0; j < ARRAYSIZE(names_to_sync); j++)
			{
				PCSTR cur_sync = names_to_sync[j];
				if (json_object_has_member(global_config_i_ref, cur_sync))
				{
					cache_config_i_ref[cur_sync] = global_config_i_ref[cur_sync];
				}
			}
		}
		//}
		std::ofstream o(CACHE_FILENAMEA);
#ifdef _DEBUG
		o << global_stat["cache"].dump(4);
#else
		o << global_stat["cache"];
#endif
	}
}

void initial_read_cache()
{
	assert(enable_cache);
	if (true == is_cache_not_expired())
	{
		/* // done in rapidjson
		if (json_object_has_member(global_stat, "cache"))
		{
			MessageBox(NULL, L"You should never put \"cache\" in " CONFIG_FILENAME
				L"\n Cache is now disabled!",
				L"Cache error",
				MB_OK | MB_ICONWARNING
			);
			global_stat.erase(global_stat.find("cache"));
		}*/

		auto& cache_ref = global_stat["cache"];
		std::ifstream i(CACHE_FILENAMEA);
		i >> cache_ref;
		LOGMESSAGE(L"read cache:%s\n", utf8_to_wstring(global_stat["cache"].dump()).c_str());
		if (false == json_object_has_member(cache_ref, "configs") ||
			false == cache_ref["configs"].is_array() ||
			number_of_configs != cache_ref["configs"].size()
			)
		{
			MessageBox(NULL, L"You should not manually edit " CACHE_FILENAMEW,
				L"Cache error",
				MB_OK | MB_ICONERROR
			);
			enable_cache = false;
			global_stat.erase(global_stat.find("cache"));
			return;
		}
		PCSTR names_to_check[]={
			"x",
			"y",
			"width",

		};
	}
}
