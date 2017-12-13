#pragma once

bool is_cache_not_expired();

bool flush_cache();

/*enum CacheType
{
	cEnabled,
	cShow,
	cPosition,
	cSize,
};*/

/*
 * before call update_cache()
 * we need to check enable_cache and
 * disable_cache_position etc
 */
template<typename T>
void update_cache(/*int index, */PCSTR name, T value, int cnt)
{
	assert(enable_cache);
	static auto& base_ref = global_stat["cache"]["configs"];
	extern int cache_config_cursor;
	auto& ref = base_ref[cache_config_cursor][name];
	if (ref != value)
	{
		ref = value;
		is_cache_valid = false;

		//base_ref[cache_config_cursor]["valid"] |= (1 << cnt);
		//LOGMESSAGE(L"base_ref[cache_config_cursor]["valid"]: 0x%x\n", base_ref[cache_config_cursor]["valid"].get<int>());

		auto& valid_ref = base_ref[cache_config_cursor]["valid"];
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

inline bool check_cache_valid(int valid, int cnt)
{
	assert(cnt >= 0 && cnt < 4);
	return valid & (1 << cnt);
}

template<typename T>
T get_cache(PCSTR name)
{
	assert(enable_cache);
	static auto& base_ref = global_stat["cache"]["configs"];
	extern int cache_config_cursor;
	return base_ref[cache_config_cursor][name];
}
