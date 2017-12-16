#pragma once

bool is_cache_not_expired();

bool flush_cache();

enum CacheType
{
	cPosition = 0,
	cSize = 1,
	cEnabled = 2,
	cShow = 3,
};

/*
 * before call update_cache()
 * we need to check enable_cache and
 * disable_cache_position etc
 */
template<typename T>
void update_cache(/*int index, */PCSTR name, T value, CacheType cnt)
{
	assert(enable_cache);
	//static auto& base_cache_ref = (*global_cache_configs_ref);
	//static auto& base_main_ref = (*global_configs_pointer);
	//static auto& base_cache_ref = global_stat["cache"]["configs"];
	//static auto& base_main_ref = global_stat["configs"];
	extern int cache_config_cursor;
	extern nlohmann::json* global_cache_configs_ref;
	extern nlohmann::json* global_configs_pointer;
	auto& cache_ref = (*global_cache_configs_ref)[cache_config_cursor][name];
	//auto& main_ref= base_main_ref[cache_config_cursor][name];
	if (cache_ref != value)
	{
		cache_ref = value;
		is_cache_valid = false;
		if (0 == strcmp(name, "start_show"))
		{
			(*global_configs_pointer)[cache_config_cursor][name] = value;
		}
		//base_ref[cache_config_cursor]["valid"] |= (1 << cnt);
		//LOGMESSAGE(L"base_ref[cache_config_cursor]["valid"]: 0x%x\n", base_ref[cache_config_cursor]["valid"].get<int>());

		auto& valid_ref = (*global_cache_configs_ref)[cache_config_cursor]["valid"];
		int valid_value = valid_ref, valid_mask = 1 << cnt;
		if ((valid_value & valid_mask) == 0)
		{
			valid_ref = valid_value | valid_mask;
		}
		LOGMESSAGE(L"cache updated! %S\n", name);
		//flush_cache();
	}
}

void update_cache_enabled_start_show(bool enabled, bool start_show);
void update_cache_position_size(HWND hWnd);

inline bool check_cache_valid(int valid, CacheType cnt)
{
	assert(cnt >= 0 && cnt < 4);
	return valid & (1 << cnt);
}

template<typename T>
T get_cache(PCSTR name)
{
	assert(enable_cache);
	//static auto& base_ref = global["cache"]["configs"];
	extern int cache_config_cursor;
	return (*global_cache_configs_ref)[cache_config_cursor][name];
}
