#include "stdafx.h"
#include "cache.h"
#include "language.h"
#include "configure.h"
#include "utils.hpp"

extern nlohmann::json global_stat;
extern nlohmann::json* global_cache_configs_ref;
extern nlohmann::json* global_configs_pointer;
extern int number_of_configs;

extern bool enable_cache;
extern bool conform_cache_expire;
extern bool disable_cache_position;
extern bool disable_cache_size;
extern bool disable_cache_enabled;
extern bool disable_cache_show;
extern bool is_cache_valid;

extern BOOL isZHCN, isENUS;

bool is_cache_not_expired()
{
	PCWSTR json_filename = CONFIG_FILENAMEW;
	PCWSTR cache_filename = CACHE_FILENAMEW;
	if (TRUE != PathFileExists(cache_filename))
	{
		if (global_stat != nullptr)
		{
			if (conform_cache_expire)
			{
				const int result = MessageBox(NULL,
					isZHCN ? L"缓存文件被删除了，是否要写入旧缓存！\n\n选择 是 则清空缓存"
					L"\n\n选择 否 则保留缓存数据，如果改动了" CONFIG_FILENAMEW L"同时删除了缓存，选 否 缓存可能会错位"
					:
					translate_w2w(L"You just Delete " CONFIG_FILENAMEW L"\n\nChoose Yes to clear"
						L" cache\n\nChoose No to keep expired cache.").c_str(),
					isZHCN ? L"是否要清空缓存？" : translate_w2w(L"Clear cache?").c_str(),
					MB_YESNO
				);
				if (IDNO == result)
				{
					return true;
				}
			}
		}
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
		bool return_val = false;
		if (conform_cache_expire)
		{
			//bool isZHCN = GetSystemDefaultLCID() == 2052 || GetACP() == 936;
			LOGMESSAGE(L"isZHCN:%d isENUS:%d\n", isZHCN, isENUS);
			const int result = MessageBox(NULL,
				isZHCN ? L"config.json被编辑过了，缓存可能已经失效！\n\n选择 是 则清空缓存"
				L"\n\n选择 否 则保留缓存数据"
				:
				translate_w2w(L"You just edit config.json!\n\nChoose Yes to clear"
					L" cache\n\nChoose No to keep expired cache.").c_str(),
				isZHCN ? L"是否要清空缓存？" : translate_w2w(L"Clear cache?").c_str(),
				MB_YESNO
			);
			if (IDNO == result)
			{
				return_val = true;
				if (global_stat == nullptr)
				{
					std::ofstream o_cache(CACHE_FILENAMEA, std::ios_base::app | std::ios_base::out);
					o_cache << std::endl;
				}
			}
		}
		RETURN_AND_CLOSE_CREATEFILE(return_val);
	}
	RETURN_AND_CLOSE_CREATEFILE(true);
}

void update_cache_enabled_start_show(bool enabled, bool start_show)
{
	if (enable_cache)
	{
		if (false == disable_cache_enabled)
		{
			update_cache("enabled", enabled, cEnabled);
		}
		if (false == disable_cache_show)
		{
			update_cache("start_show", start_show, cShow);
		}
	}
}

void update_cache_position_size(HWND hWnd)
{
	if (enable_cache && hWnd)
	{
		//if (false == disable_cache_show)
		//{
		//	update_cache("start_show", false, 3);
		//}
		if (false == disable_cache_position || false == disable_cache_size)
		{
			RECT rect = {};
			if (NULL != GetWindowRect(hWnd, &rect))
			{
				LOGMESSAGE(L"left:%d top:%d right:%d bottom:%d\n", rect.left, rect.top, rect.right, rect.bottom);
				if (get_cache<int>("left") != rect.left)
				{
					update_cache("left", rect.left, cPosition);
				}
				if (get_cache<int>("top") != rect.top)
				{
					update_cache("top", rect.top, cPosition);
				}

				/*if (false == disable_cache_position)
				{
				int left = get_cache<int>("left"), top = get_cache<int>("top");
				}
				*/

				if (false == disable_cache_size)
				{
					if (get_cache<int>("right") != rect.right)
					{
						update_cache("right", rect.right, cSize);
					}
					if (get_cache<int>("bottom") != rect.bottom)
					{
						update_cache("bottom", rect.bottom, cSize);
					}
				}
			}
		}
	}
}

bool flush_cache()
{
	assert(enable_cache);
	assert(false == is_cache_valid);
	LOGMESSAGE(L"Now flush cache\n");
	//return true;
	is_cache_valid = true;
	if (is_cache_not_expired())
	{
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
	}

	return false;
}

