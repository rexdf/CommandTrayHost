#pragma once

//bool is_cache_not_expired(bool is_from_flush = false/*, bool is_from_other_thread = false*/);
int is_cache_not_expired2();

bool is_config_changed();
bool reload_config();
bool get_filetime(PCWSTR const filename, FILETIME& file_write_timestamp);

bool flush_cache(/*bool is_exist = false*/);

enum CacheType
{
	cPosition = 0,
	cSize = 1,
	cEnabled = 2,
	cShow = 3,
	cAlpha = 4,
};

/*
 * before call update_cache()
 * we need to check enable_cache and
 * disable_cache_position etc
 * make sure cache_config_cursor correct
 */
template<typename T>
void update_cache(/*int index, */PCSTR name, T value, const CacheType cnt)
{
	assert(enable_cache);
	//static auto& base_cache_ref = (*global_cache_configs_pointer);
	//static auto& base_main_ref = (*global_configs_pointer);
	//static auto& base_cache_ref = global_stat["cache"]["configs"];
	//static auto& base_main_ref = global_stat["configs"];
	extern int cache_config_cursor;
	extern nlohmann::json* global_cache_configs_pointer;
	//extern nlohmann::json* global_configs_pointer;
	auto& cache_ref = (*global_cache_configs_pointer)[cache_config_cursor][name];
	//auto& main_ref= base_main_ref[cache_config_cursor][name];
	if (cache_ref != value)
	{
		cache_ref = value;
		is_cache_valid = false;
		// why we need it? when restart app
		//auto& main_config_i_ref = (*global_configs_pointer)[cache_config_cursor];
		/*if (0 == strcmp(name, "start_show"))
		{
			main_config_i_ref[name] = value;
		}
		else if (0 == strcmp(name, "alpha") && json_object_has_member(main_config_i_ref, "alpha"))
		{
			main_config_i_ref[name] = value;
		}*/
		//base_ref[cache_config_cursor]["valid"] |= (1 << cnt);
		//LOGMESSAGE(L"base_ref[cache_config_cursor]["valid"]: 0x%x\n", base_ref[cache_config_cursor]["valid"].get<int>());

		auto& valid_ref = (*global_cache_configs_pointer)[cache_config_cursor]["valid"];
		const int valid_value = valid_ref, valid_mask = 1 << cnt;
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

inline bool check_cache_valid(const int valid, CacheType cnt)
{
	assert(cnt >= 0 && cnt < 5);
	return valid & (1 << cnt);
}

template<typename T>
T get_cache(PCSTR name)
{
	assert(enable_cache);
	//static auto& base_ref = global["cache"]["configs"];
	extern int cache_config_cursor;
	extern nlohmann::json* global_cache_configs_pointer;
	return (*global_cache_configs_pointer)[cache_config_cursor][name];
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
	const rapidjson::Value& d_configs, // d["configs"]
	const rapidjson::Value* d_cache_configs, // &(d_cache["configs"])
	const PCSTR cache_name,
	//PCSTR cache_item_name,
	const rapidjson::SizeType cache_size, // d_cache["configs"].Size()
	const rapidjson::SizeType global_cache_size, // global_stat["cache"]["configs"].size()
	const rapidjson::SizeType idx,
	int& type
);

template<typename Type>
Type get_cache_value(
	const rapidjson::Value* d_cache_configs,
	const rapidjson::SizeType cache_size,
	const rapidjson::SizeType idx,
	const size_t global_cache_size,
	PCSTR name,
	const int find_type,
	Type default_val
)
{
	//if (global_stat != nullptr && global_cache_configs_pointer != nullptr)
	{
		if (find_type == 1 && idx < global_cache_size)
		{
			assert(global_stat != nullptr && global_cache_configs_pointer != nullptr);
			auto& global_cache_config_i_ref = (*global_cache_configs_pointer)[idx];
			/*//#ifdef _DEBUG
			//			try_read_optional_json(global_cache_config_i_ref, default_val, name, __FUNCTION__);
			//#else
			try_read_optional_json(global_cache_config_i_ref, default_val, name);
			//#endif
			return default_val;*/
			return global_cache_config_i_ref.value(name, default_val);
		}
	}

	if (find_type == 2 && idx < cache_size)
	{
		auto& d_cache_config_i_ref = (*d_cache_configs)[idx];
		if (d_cache_config_i_ref.HasMember(name))
		{
			return d_cache_config_i_ref[name].Get<Type>();
		}
	}

	return default_val;
}

