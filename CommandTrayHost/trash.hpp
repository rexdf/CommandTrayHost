//Used for abandoned code

/*
* return NULL: failed
* return others: numbers of configs
*/
int configure_reader(std::string& out)
{
	PCWSTR json_filename = L"config.json";
	if (TRUE != PathFileExists(json_filename))
	{
		if (!initial_configure())
		{
			return NULL;
		}
	}

	int64_t json_file_size = FileSize(json_filename);
	if (-1 == json_file_size)
	{
		return NULL;
	}
	LOGMESSAGE(L"config.json file size: %lld\n", json_file_size);
	if (json_file_size > 1024 * 1024 * 100)
	{
		json_file_size = 1024 * 1024 * 100;
		MessageBox(NULL, L"The file size of config.json is larger than 100MB!", L"WARNING", MB_OK | MB_ICONWARNING);
	}
	char* readBuffer = reinterpret_cast<char*>(malloc(static_cast<size_t>(json_file_size + 10)));
	if (NULL == readBuffer)
	{
		return NULL;
	}
	FILE* fp;
	errno_t err = _wfopen_s(&fp, json_filename, L"rb"); // 非 Windows 平台使用 "r"
	if (0 != err)
	{
		MessageBox(NULL, L"Open configure failed!", L"Error", MB_OK | MB_ICONERROR);
		free(readBuffer);
		return NULL;
	}

#define SAFE_RETURN_VAL_FREE_FCLOSE(buf_p,f_p,val) {free(buf_p);fclose(f_p); return val; }

	using namespace rapidjson;

	// FileReadStream bis(fp, readBuffer, sizeof(readBuffer)); //WARNING logical Error
	FileReadStream bis(fp, readBuffer, static_cast<size_t>(json_file_size + 5));
	AutoUTFInputStream<unsigned, FileReadStream> eis(bis);  // 用 eis 包装 bis
#ifdef _DEBUG
	const char* utf_type_name[] = {
		"kUTF8 = 0,      //!< UTF-8.",
		"kUTF16LE = 1,   //!< UTF-16 little endian.",
		"kUTF16BE = 2,   //!< UTF-16 big endian.",
		"kUTF32LE = 3,   //!< UTF-32 little endian.",
		"kUTF32BE = 4    //!< UTF-32 big endian."
	};
	LOGMESSAGE(L"config.json encoding is: %S HasBom:%d\n", utf_type_name[eis.GetType()], eis.HasBOM());
#endif
	Document d;         // Document 为 GenericDocument<UTF8<> > 
						//d.ParseStream<0, AutoUTF<unsigned> >(eis); // 把任何 UTF 编码的文件解析至内存中的 UTF-8

	Document::AllocatorType& allocator = d.GetAllocator(); // for change position/size double to in


														   /*Document d;
														   std::ifstream i("config.json");
														   if (i.bad())
														   {
														   return NULL;
														   }
														   IStreamWrapper isw(i);
														   LOGMESSAGE(L"configure_reader\n");
														   if (d.ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag>(isw).HasParseError())*/
	if (d.ParseStream<kParseCommentsFlag | kParseTrailingCommasFlag,
		AutoUTF<unsigned>>(eis).HasParseError())
	{
		LOGMESSAGE(L"\nError(offset %u): %S\n",
			(unsigned)d.GetErrorOffset(),
			GetParseError_En(d.GetParseError()));
		// ...
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}

	assert(d.IsObject());
	assert(!d.ObjectEmpty());
	if (!d.IsObject() || d.ObjectEmpty())
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}


	assert(d.HasMember("configs"));
	if (!d.HasMember("configs"))
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}

#ifdef _DEBUG
	static const char* kTypeNames[] =
	{ "Null", "False", "True", "Object", "Array", "String", "Number" };
#endif

	assert(d["configs"].IsArray());
	if (!(d["configs"].IsArray()))
	{
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
		/*free(readBuffer);
		fclose(fp);
		return NULL;*/
	}

	// type check for global optional items
	/*if (d.HasMember("require_admin") && !(d["require_admin"].IsBool()) ||
	d.HasMember("enable_groups") && !(d["enable_groups"].IsBool()) ||
	d.HasMember("groups_menu_symbol") && !(d["groups_menu_symbol"].IsString()) ||
	d.HasMember("icon") && !(d["icon"].IsString()) ||
	d.HasMember("lang") && !(d["lang"].IsString()) ||
	d.HasMember("left_click") && !(d["left_click"].IsArray()) ||
	d.HasMember("icon_size") && !(d["icon_size"].IsInt())
	)*/

	/*if (!rapidjson_check_exist_type<bool>(d, "require_admin") ||
	!rapidjson_check_exist_type<bool>(d, "enable_groups") ||
	!rapidjson_check_exist_type<JsonStr>(d, "groups_menu_symbol") ||
	!rapidjson_check_exist_type<JsonStr>(d, "icon") ||
	!rapidjson_check_exist_type<JsonStr>(d, "lang") ||
	!rapidjson_check_exist_type<JsonArray>(d, "left_click") ||
	!rapidjson_check_exist_type<int>(d, "icon_size")
	)*/
	if (!rapidjson_check_exist_type2(d, "require_admin", iBoolType) ||
		!rapidjson_check_exist_type2(d, "enable_groups", iBoolType) ||
		!rapidjson_check_exist_type2(d, "groups_menu_symbol", iStringType) ||
		!rapidjson_check_exist_type2(d, "icon", iStringType) ||
		!rapidjson_check_exist_type2(d, "lang", iStringType) ||
		!rapidjson_check_exist_type2(d, "left_click", iArrayType) ||
		!rapidjson_check_exist_type2(d, "icon_size", iIntType)
		)
	{
		MessageBox(NULL, L"One of global section require_admin(bool) icon(string) lang(string)"
			L" icon_size(number) has type error!",
			L"Type Error",
			MB_OK | MB_ICONERROR
		);
		SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
	}

	//remove empty string, except for groups_menu_symbol.
	{
		PCSTR str_names_to_remove[] = { "lang","icon" };
		for (int i = 0; i < ARRAYSIZE(str_names_to_remove); i++)
		{
			PCSTR str_pointer = str_names_to_remove[i];
			if (d.HasMember(str_pointer) && d[str_pointer] == "")
			{
				d.RemoveMember(str_pointer);
			}
		}
	}

	if (d.HasMember("enable_groups") &&
		(true == d["enable_groups"].GetBool()) &&
		d.HasMember("groups")
		)
	{
		enable_groups_menu = true;
	}
	else
	{
		enable_groups_menu = false;
	}
	LOGMESSAGE(L"enable_groups_menu:%d\n", enable_groups_menu);

	//cache options
	{
		extern bool enable_cache;
		extern bool disable_cache_position;
		extern bool disable_cache_size;
		extern bool disable_cache_enabled;
		extern bool disable_cache_show;
		PCSTR cache_options_strs[] = {
			"enable_cache",
			"disable_cache_position",
			"disable_cache_size",
			"disable_cache_enabled",
			"disable_cache_show"
		};
		bool* cache_options_global_pointer[] = {
			&enable_cache,
			&disable_cache_position,
			&disable_cache_size,
			&disable_cache_enabled,
			&disable_cache_show
		};
		bool cache_options_default_value[] = {
			true,
			false,
			false,
			true,
			true
		};
		int cache_cnt = 0;
		for (int i = 0; i < ARRAYSIZE(cache_options_strs); i++)
		{
			PCSTR cache_str = cache_options_strs[i];
			if (d.HasMember(cache_str))
			{
				auto& ref = d[cache_str];
				if (false == ref.IsBool())
				{
					MessageBox(NULL, L"One of enable_cache disable_cacahe_* options is not bool type error!",
						L"Type Error",
						MB_OK | MB_ICONERROR
					);
					SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
				}
				else
				{
					*(cache_options_global_pointer[i]) = ref.GetBool();
				}
				cache_cnt++;
			}
			else
			{
				*(cache_options_global_pointer[i]) = cache_options_default_value[i];
			}
		}
		if (cache_cnt == 0) // if no cache options, then override the default value `"enable_cache": true,`
		{
			enable_cache = false;
		}
	} // End cache options

	PCSTR size_postion_strs[] = {
		"position",
		"size"
	};
	//int screen_fullx = GetSystemMetrics(SM_CXFULLSCREEN);
	//int screen_fully = GetSystemMetrics(SM_CYFULLSCREEN);
	int screen_full_ints[] = {
		GetSystemMetrics(SM_CXFULLSCREEN),
		GetSystemMetrics(SM_CYFULLSCREEN)
	};

	assert(screen_full_ints[0] == GetSystemMetrics(SM_CXFULLSCREEN));
	assert(screen_full_ints[1] == GetSystemMetrics(SM_CYFULLSCREEN));

	int cnt = 0;

	for (auto& m : d["configs"].GetArray())
	{
#ifdef _DEBUG
		StringBuffer sb;
		Writer<StringBuffer> writer(sb);
		m.Accept(writer);
		std::string ss = sb.GetString();
		LOGMESSAGE(L"Type of member %s is %S\n",
			//ss.c_str(),
			utf8_to_wstring(ss).c_str(),
			kTypeNames[m.GetType()]);
		//LOGMESSAGE(L"Type of member %S is %S\n",
		//m.GetString(), kTypeNames[m.GetType()]);
#endif
		assert(m.IsObject());

		assert(m.HasMember("name"));
		assert(m["name"].IsString());

		assert(m.HasMember("path"));
		assert(m["path"].IsString());

		assert(m.HasMember("cmd"));
		assert(m["cmd"].IsString());

		assert(m.HasMember("working_directory"));
		assert(m["working_directory"].IsString());

		assert(m.HasMember("addition_env_path"));
		assert(m["addition_env_path"].IsString());

		assert(m.HasMember("use_builtin_console"));
		assert(m["use_builtin_console"].IsBool());

		assert(m.HasMember("is_gui"));
		assert(m["is_gui"].IsBool());

		assert(m.HasMember("enabled"));
		assert(m["enabled"].IsBool());

		/*if (m.IsObject() &&
		m.HasMember("name") && m["name"].IsString() &&
		m.HasMember("path") && m["path"].IsString() &&
		m.HasMember("cmd") && m["cmd"].IsString() &&
		m.HasMember("working_directory") && m["working_directory"].IsString() &&
		m.HasMember("addition_env_path") && m["addition_env_path"].IsString() &&
		m.HasMember("use_builtin_console") && m["use_builtin_console"].IsBool() &&
		m.HasMember("is_gui") && m["is_gui"].IsBool() &&
		m.HasMember("enabled") && m["enabled"].IsBool()
		)*/
		/*if (m.IsObject() &&
		rapidjson_check_exist_type<JsonStr>(m, "name", false) &&
		rapidjson_check_exist_type<JsonStr>(m, "path", false) &&
		rapidjson_check_exist_type<JsonStr>(m, "cmd", false) &&
		rapidjson_check_exist_type<JsonStr>(m, "working_directory", false) &&
		rapidjson_check_exist_type<JsonStr>(m, "addition_env_path", false) &&
		rapidjson_check_exist_type<bool>(m, "use_builtin_console", false) &&
		rapidjson_check_exist_type<bool>(m, "is_gui", false) &&
		rapidjson_check_exist_type<bool>(m, "enabled", false)
		)*/
		if (m.IsObject() &&
			rapidjson_check_exist_type2(m, "name", iStringType, false) &&
			rapidjson_check_exist_type2(m, "path", iStringType, false) &&
			rapidjson_check_exist_type2(m, "cmd", iStringType, false) &&
			rapidjson_check_exist_type2(m, "working_directory", iStringType, false) &&
			rapidjson_check_exist_type2(m, "addition_env_path", iStringType, false) &&
			rapidjson_check_exist_type2(m, "use_builtin_console", iBoolType, false) &&
			rapidjson_check_exist_type2(m, "is_gui", iBoolType, false) &&
			rapidjson_check_exist_type2(m, "enabled", iBoolType, false)
			)
		{
			/*//we donnot need it now, it can be easy implemented in create_process
			if (m["working_directory"] == "")
			{
			m["working_directory"] = StringRef(m["path"].GetString());
			}*/

			std::wstring wname = utf8_to_wstring(m["name"].GetString());

			// type check for optional items
			/*if (m.HasMember("require_admin") && !(m["require_admin"].IsBool()) ||
			m.HasMember("start_show") && !(m["start_show"].IsBool()) ||
			m.HasMember("icon") && !(m["icon"].IsString()) ||
			m.HasMember("alpha") && !(m["alpha"].IsInt()) ||
			m.HasMember("is_topmost") && !(m["is_topmost"].IsBool()) ||
			m.HasMember("position") && !(m["position"].IsArray()) ||
			m.HasMember("size") && !(m["size"].IsArray()) ||
			m.HasMember("ignore_all") && !(m["ignore_all"].IsBool())
			)*/

			/*if (!rapidjson_check_exist_type<bool>(m, "require_admin") ||
			!rapidjson_check_exist_type<bool>(m, "start_show") ||
			!rapidjson_check_exist_type<JsonStr>(m, "icon") ||
			!rapidjson_check_exist_type<int>(m, "alpha") ||
			!rapidjson_check_exist_type<bool>(m, "is_topmost") ||
			!rapidjson_check_exist_type<JsonArray>(m, "position") ||
			!rapidjson_check_exist_type<JsonArray>(m, "size") ||
			!rapidjson_check_exist_type<bool>(m, "ignore_all")
			)*/

			if (!rapidjson_check_exist_type2(m, "require_admin", iBoolType) ||
				!rapidjson_check_exist_type2(m, "start_show", iBoolType) ||
				!rapidjson_check_exist_type2(m, "icon", iStringType) ||
				!rapidjson_check_exist_type2(m, "alpha", iIntType, true, [](const Value& val)->bool {int v = val.GetInt(); LOGMESSAGE(L"lambda! getint:%d", v); return v >= 0 && v < 256; }) ||
				!rapidjson_check_exist_type2(m, "is_topmost", iBoolType) ||
				!rapidjson_check_exist_type2(m, "position", iArrayType) ||
				!rapidjson_check_exist_type2(m, "size", iArrayType) ||
				!rapidjson_check_exist_type2(m, "ignore_all", iBoolType)
				)
			{
				MessageBox(NULL, (wname + L" One of require_admin(bool) start_show(bool) ignore_all(bool)"
					L" is_topmost(bool) icon(str) alpha(0-255)"
					L" has a type error!").c_str(),
					L"Type Error",
					MB_OK | MB_ICONERROR
				);
				SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
			}

			//alpha extra check
			/*if (m.HasMember("alpha"))
			{
			int alpha = m["alpha"].GetInt();
			if (alpha < 0 || alpha>255)
			{
			MessageBox(NULL, (wname + L" alpha must be 0-255 integer").c_str(),
			L"Type Error",
			MB_OK | MB_ICONERROR
			);
			SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
			}
			}*/

			//size position extra check
			for (auto str_item : size_postion_strs)
			{
				if (m.HasMember(str_item))
				{
					auto& ref = m[str_item];
					if (false == ref.IsArray() || ref.GetArray().Size() != 2)
					{
						MessageBox(NULL, L"One of position size is not array of two double numbers!", L"Type Error", MB_OK | MB_ICONERROR);
						SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
					}
					int cord_xy_cnt = 0, cords[2];
					for (auto&itm : ref.GetArray())
					{
						if (false == itm.IsNumber() || itm.GetDouble() < 0)
						{
							MessageBox(NULL, L"Number in position size array must be not less than zero!", L"Type Error", MB_OK | MB_ICONERROR);
							SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
						}
						double val = itm.GetDouble();
						if (val <= 1)
						{
							//itm.SetDouble(val*(cord_xy_cnt ? screen_fully : screen_fullx));
							//cords[cord_xy_cnt] = static_cast<int>(val*(cord_xy_cnt ? screen_fully : screen_fullx));
							cords[cord_xy_cnt] = static_cast<int>(val*screen_full_ints[cord_xy_cnt]);
						}
						else
						{
							cords[cord_xy_cnt] = static_cast<int>(val);
						}
						cord_xy_cnt++;
					}
					ref.Clear();

					ref.PushBack(cords[0], allocator);
					ref.PushBack(cords[1], allocator);
				}
			}

			//empty icon string to remove
			if (m.HasMember("icon") && m["icon"] == "")
			{
				m.RemoveMember("icon");
			}

			cnt++;
		}
		else
		{
			SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
			/*free(readBuffer);
			fclose(fp);
			return NULL;*/
		}
	}
	int left_click_cnt = 0;
	if (d.HasMember("left_click"))
	{
		for (auto& m : d["left_click"].GetArray())
		{
			if (m.IsInt())
			{
				int ans = m.GetInt();
				if (ans < 0 || ans >= cnt)
				{
					SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
				}
			}
			else
			{
				SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, NULL);
			}
			left_click_cnt++;
		}
	}

	if (left_click_cnt > 0)
	{
		enable_left_click = true;
	}
	else
	{
		enable_left_click = false;
	}
	StringBuffer sb;
	Writer<StringBuffer> writer(sb);
	d.Accept(writer);
	out = sb.GetString();
	//std::string ss = sb.GetString();
	//out = ss;
	SAFE_RETURN_VAL_FREE_FCLOSE(readBuffer, fp, cnt);
	/*free(readBuffer);
	fclose(fp);
	return cnt;*/
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
		PCSTR names_to_check[] = {
			"x",
			"y",
			"width",

		};
	}
}



/*bool rapidjson_check_exist_type2(
rapidjson::Value& val,
PCSTR name,
RapidJsonType type,
bool not_exist_return = true
);*/
/*bool rapidjson_check_exist_type2(
	rapidjson::Value& val,
	PCSTR name,
	RapidJsonType type,
	bool not_exist_return
)
{
	if (val.HasMember(name))
	{
		auto& ref = val[name];
		bool ret;
		if (type == iBoolType)
		{
			int val_type = ref.GetType();
			ret = val_type == iTrueType || val_type == iFalseType;
		}
		else if (type == iIntType)
		{
			ret = ref.IsInt();
		}
		else
		{
			ret = ref.GetType() == type;
		}
		LOGMESSAGE(L"%S result:%d type:%d GetType:%d\n", name, ret, type, ref.GetType());
		return ret;
	}
	return not_exist_return;
}*/


/*struct JsonArray {};
struct JsonStr {};
struct JsonNumber {};
struct JsonObject {};*/

/*
* when not_exist_return is true
* return false only when exist and type not correct
*/
/*template<typename Type>
bool rapidjson_check_exist_type(rapidjson::Value& val, PCSTR name, bool not_exist_return = true, void* fuc = nullpt)
{
	if (val.HasMember(name))
	{
		if (std::is_same<Type, bool>::value)
		{
			if (val[name].IsBool())
			{
				//if (fuc != nullptr)
				//{
				//	typedef bool(*BoolFuc) (bool);
				//	return reinterpret_cast<BoolFuc>(fuc)(val[name].GetBool());
				//}
				return true;
			}
			return false;
		}
		if (std::is_same<Type, int>::value)
		{
			if (val[name].IsInt())
			{
				//if (fuc != nullptr)
				//{
				//	typedef bool(*BoolFuc) (int);
				//	return reinterpret_cast<BoolFuc>(fuc)(val[name].GetInt());
				//}
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonStr>::value)
		{
			if (val[name].IsString())
			{
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonArray>::value)
		{
			if (val[name].IsArray())
			{
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonNumber>::value)
		{
			if (val[name].IsNumber())
			{
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonObject>::value)
		{
			if (val[name].IsObject())
			{
				return true;
			}
			return false;
		}
		assert(false);
	}
	return not_exist_return;
}*/

/*// I cannot make the following code to be compiled
template<typename Type,typename Param>
bool rapidjson_check_exist_type(rapidjson::Value& val, PCSTR name, bool not_exist_return = true, std::function<bool(Param)> func = nullptr)
{
	if (val.HasMember(name))
	{
		if (std::is_same<Type, bool>::value)
		{
			if (val[name].IsBool())
			{
				if (func != nullptr)
				{
					bool bool_val = val[name].GetBool();
					return func(bool_val);
				}
				return true;
			}
			return false;
		}
		if (std::is_same<Type, int>::value)
		{
			if (val[name].IsInt())
			{
				if (func != nullptr)
				{
					std::function<int(int)> f = func;
					int val_int = val[name].GetInt();
					return f(val_int);
				}
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonStr>::value)
		{
			if (val[name].IsString())
			{
				if (func != nullptr)
				{
					return func(val[name].GetString());
				}
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonArray>::value)
		{
			if (val[name].IsArray())
			{
				if (func != nullptr)
				{
					return func(val[name].GetArray());
				}
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonNumber>::value)
		{
			if (val[name].IsNumber())
			{
				if (func != nullptr)
				{
					return func(val[name].GetDouble());
				}
				return true;
			}
			return false;
		}
		if (std::is_same<Type, JsonObject>::value)
		{
			if (val[name].IsObject())
			{
				if (func != nullptr)
				{
					return func(val[name].GetObject());
				}
				return true;
			}
			return false;
		}
	}
	return not_exist_return;
}*/
