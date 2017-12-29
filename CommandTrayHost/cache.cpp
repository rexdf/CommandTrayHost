#include "stdafx.h"
#include "CommandTrayHost.h"
#include "cache.h"
#include "language.h"
#include "utils.hpp"
#include "configure.h"

extern nlohmann::json global_stat;
extern nlohmann::json* global_cache_configs_pointer;
extern nlohmann::json* global_configs_pointer;
extern size_t number_of_configs;

extern bool enable_cache;
extern bool conform_cache_expire;
extern bool disable_cache_position;
extern bool disable_cache_size;
extern bool disable_cache_enabled;
extern bool disable_cache_show;
extern bool disable_cache_alpha;
extern bool is_cache_valid;
extern bool auto_hot_reloading_config;

extern bool reload_config_with_cache;

extern BOOL isZHCN, isENUS;
//extern CRITICAL_SECTION CriticalSection;
//extern bool enable_critialsection;

bool get_filetime(PCWSTR const filename, FILETIME& file_write_timestamp)
{
	if (TRUE != PathFileExists(filename))
	{
		msg_prompt(filename, L"File not exist");
		return false;
	}
	HANDLE json_hFile = CreateFile(filename, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (!json_hFile)return false;
	bool ret = true;
	if (!GetFileTime(json_hFile, NULL, NULL, &file_write_timestamp))ret = false;
	CloseHandle(json_hFile);
	return ret;
}

int is_cache_not_expired2()
{
	LOGMESSAGE(L"GetCurrentThreadId:%d\n", GetCurrentThreadId());

	//assert(enable_cache);

	PCWSTR json_filename = CONFIG_FILENAMEW;
	PCWSTR cache_filename = CACHE_FILENAMEW;

	if (TRUE != PathFileExists(json_filename))
	{
		/*extern HANDLE ghJob;
		extern HICON gHicon;
		if (NULL == init_global(ghJob, gHicon))
		{
			return true;
		}*/
		return 1;
	}
	if (TRUE != PathFileExists(cache_filename))
	{
		return 2;
	}

	FILETIME json_write_timestamp, cache_write_timestamp;
	if (!get_filetime(json_filename, json_write_timestamp) || !get_filetime(cache_filename, cache_write_timestamp))
	{
		return 3;
	}


	if (CompareFileTime(&json_write_timestamp, &cache_write_timestamp) >= 0)
	{
		LOGMESSAGE(L"json_write_timestamp is later than cache_write_timestamp\n");
		if (global_stat == nullptr)
		{
			// prompt when start CommandTrayHost
			int result;
			if (!auto_hot_reloading_config)
			{
				result = msg_prompt(//NULL,
					isZHCN ?
					L"config.json被编辑过了，缓存可能已经失效！\n\n选择 是 则清空缓存"
					L"\n\n选择 否 则保留缓存数据"
					:
					translate_w2w(L"You just edit config.json!\n\nChoose Yes to clear"
						L" cache\n\nChoose No to keep expired cache.").c_str(),
					isZHCN ? L"是否要清空缓存？" : translate_w2w(L"Clear cache?").c_str(),
					MB_YESNO
				);
			}
			else
			{
				result = IDNO;
			}
			reload_config_with_cache = IDNO == result;
			if (result == IDYES)
			{
				return 4;
			}
		}
	}
	reload_config_with_cache = true;
	return 0;
}

bool is_config_changed()
{
	static int atom_variable = 0;
	if (atom_variable)return false;
	atom_variable = 1;
	extern FILETIME config_last_timestamp;
	LOGMESSAGE(L"dwLowDateTime:%d dwHighDateTime:%d\n", config_last_timestamp.dwLowDateTime, config_last_timestamp.dwHighDateTime);
	FILETIME current_config_timestamp;
	if (get_filetime(CONFIG_FILENAMEW, current_config_timestamp))
	{
		if (CompareFileTime(&current_config_timestamp, &config_last_timestamp) != 0)
		{
			config_last_timestamp = current_config_timestamp;
			atom_variable = 0;
			return true;
		}
	}
	atom_variable = 0;
	return false;
}

bool reload_config()
{
	LOGMESSAGE(L"GetCurrentThreadId:%d\n", GetCurrentThreadId());
	if (global_stat == nullptr)
	{
		LOGMESSAGE(L"global_stat is nullptr\n");
		return false;
	}
	assert(conform_cache_expire);
	{
		int result;
		if (!auto_hot_reloading_config)
		{
			result = msg_prompt(//NULL,
				isZHCN ?
				L"config.json被编辑过了,缓存可能已经失效！"
				L"\n\n选择 是 重新加载配置，但是清空缓存(如果启用缓存)，关闭全部运行中的程序"
				L"\n\n选择 否 重新加载配置，尽最大努力保留缓存(如果启用缓存)，cmd path working_directory未修改的运行中的程序不会被关闭"
				L"\n\n选择 取消 下次启动CommandTrayHost才加载配置"
				:
				translate_w2w(L"You just edit config.json!\n\nChoose Yes to clear"
					L" cache\n\nChoose No to keep expired cache.\n\nChoose Cancel to do nothing").c_str(),
				isZHCN ? L"是否要清空缓存？" : translate_w2w(L"Clear cache?").c_str(),
				MB_YESNOCANCEL
			);
		}
		else
		{
			result = IDNO;
		}
		if (IDCANCEL == result)
		{
			return true;
		}


		reload_config_with_cache = IDNO == result;

		unregisterhotkey_killtimer_all();

		if (IDYES == result)
		{
			/* // we donot need to delete cache, because of reload_config_with_cache false
			if (enable_cache && NULL == DeleteFile(CACHE_FILENAMEW))
			{
				LOGMESSAGE(L"DeleteFile GetLastError:%d\n", GetLastError());
				msg_prompt(L"Delete " CACHE_FILENAMEW L" Failed!", L"Delete failed", MB_OK);
			}*/
			enable_cache = false;
			kill_all();
		}

		extern HANDLE ghJob;
		extern HICON gHicon;
		if (NULL == init_global(ghJob, gHicon))
		{
			return false;
		}
		start_all(ghJob);
		DeleteTrayIcon();
		ShowTrayIcon(CONFIG_FILENAMEW L" has been reloaded.", NIM_ADD);
	}
	return true;
}


/*
 * type:	-1 means cache_name invalid, return idx
 *			0 fatal error
 *			1 return from cache memory
 *			2 return from cache file
 *			3 return from config.json
 *			4 not found from above all, return idx
 */
rapidjson::SizeType get_cache_index(
	const rapidjson::Value& d_configs,
	const rapidjson::Value* d_cache_configs,
	const PCSTR cache_name,
	//PCSTR cache_item_name,
	const rapidjson::SizeType cache_size,
	const rapidjson::SizeType global_cache_size,
	const rapidjson::SizeType idx,
	int& type
)
{
	type = -1;
	// if cache name is empty return idx
	if (cache_name == NULL || cache_name[0] == NULL)
	{
		return idx;
	}
	type = 0;
	// Why read both from cache memory and file, 
	// because cache can be disable and enable during reloading
	// and cache file can be deleted during reloading
	// try to read from cache memory
	if (idx < global_cache_size)
	{
		type = 1;
		for (rapidjson::SizeType i = idx; i < global_cache_size; i++)
		{
			auto& global_cache_config_i_ref = (*global_cache_configs_pointer)[i];
			if (global_cache_config_i_ref["name"] == cache_name)
			{
				return i;
			}
		}
		if (idx)for (rapidjson::SizeType i = 0; i < idx; i++)
		{
			auto& global_cache_config_i_ref = (*global_cache_configs_pointer)[i];
			if (global_cache_config_i_ref["name"] == cache_name)
			{
				return i;
			}
		}
	}
	// try to read from cache file
	if (idx < cache_size)
	{
		type = 2;
		for (rapidjson::SizeType i = idx; i < cache_size; i++)
		{
			auto& d_cache_config_i_ref = (*d_cache_configs)[idx];
			//assert(d_cache_config_i_ref.HasMember("name"));
			if (d_cache_config_i_ref.HasMember("name") && d_cache_config_i_ref["name"] == cache_name)
			{
				return i;
			}
		}
		if (idx)for (rapidjson::SizeType i = 0; i < idx; i++)
		{
			auto& d_cache_config_i_ref = (*d_cache_configs)[idx];
			//assert(d_cache_config_i_ref.HasMember("name"));
			if (d_cache_config_i_ref.HasMember("name") && d_cache_config_i_ref["name"] == cache_name)
			{
				return i;
			}
		}
	}
	// try to get from just read config.json
	const rapidjson::SizeType _next_number_of_configs = d_configs.Size();
	if (idx < _next_number_of_configs)
	{
		type = 3;
		for (rapidjson::SizeType i = idx; i < _next_number_of_configs; i++)
		{
			auto& global_config_i_ref = d_configs[i];
			if (global_config_i_ref["name"] == cache_name)
			{
#ifdef _DEBUG
				msg_prompt(L"it works!", L"global_config_i_ref[\"name\"] == cache_name");
#endif
				return i;
			}
		}
		if (idx)for (rapidjson::SizeType i = 0; i < idx; i++)
		{
			auto& global_config_i_ref = d_configs[i];
			if (global_config_i_ref["name"] == cache_name)
			{
				return i;
			}
		}
	}
	type = 4;
	return idx;
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
		if (!disable_cache_alpha)
		{
			BYTE alpha;
			if (get_alpha(hWnd, alpha, true))
			{
				if (get_cache<int>("alpha") != alpha)
				{
					update_cache("alpha", alpha, cAlpha);
				}
			}
		}
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

bool flush_cache(/*bool is_exit*/)
{
	assert(enable_cache);
	assert(false == is_cache_valid);
	/*static int cache_write_cnt = 0;
	if (is_exit == false)
	{
		if (cache_write_cnt < 100)
		{
			cache_write_cnt++;
			return true;
		}
		else
		{
			cache_write_cnt = 0;
		}
	}
	else
	{
#ifdef _DEBUG
		std::ofstream o("cache_write.txt");
		o << "cache_write_cnt: " << cache_write_cnt << std::endl;
#endif
		cache_write_cnt = 0;
	}*/

	LOGMESSAGE(L"Now flush cache\n");
	//return true;
	is_cache_valid = true;
	//if (is_cache_not_expired(true))
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

