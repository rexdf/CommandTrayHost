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
void update_cache(/*int index, */PCSTR name, T value)
{
	assert(enable_cache);
	static auto& base_ref = global_stat["cache"]["configs"];
	extern int cache_config_cursor;
	auto& ref = base_ref[cache_config_cursor][name];
	if (ref != value)
	{
		ref = value;
		is_cache_valid = false;
		//flush_cache();
	}
}

template<typename T>
T get_cache(PCSTR name)
{
	assert(enable_cache);
	static auto& base_ref = global_stat["cache"]["configs"];
	extern int cache_config_cursor;
	return base_ref[cache_config_cursor][name];
}
