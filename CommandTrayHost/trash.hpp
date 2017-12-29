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



/*
//https://stackoverflow.com/questions/4191465/how-to-run-only-one-instance-of-application
void makeSingleInstance2()
{
	TCHAR szPathToExe[MAX_PATH * 2];
	if (GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe)))
	{
		//size_t length = 0;
		//StringCchLength(szPathToExe, ARRAYSIZE(szPathToExe), &length);
		for (int i = 0; i < ARRAYSIZE(szPathToExe); i++)
		{
			if (L'\\' == szPathToExe[i] || L':' == szPathToExe[i])
			{
				szPathToExe[i] = L'_';
			}
			else if (L'\x0' == szPathToExe[i])
			{
				LOGMESSAGE(L"GetModuleFileName changed to :%s, length:%d\n", szPathToExe, i);
				break;
			}
		}
		LOGMESSAGE(L"makeSingleInstance2 %s\n", szPathToExe);
		// Try to open the mutex.
		ghMutex = OpenMutex(MUTEX_ALL_ACCESS, 0, szPathToExe);
		if (!ghMutex)
		{
			// Mutex doesn't exist. This is
			// the first instance so create
			// the mutex.
			ghMutex = CreateMutex(0, 0, szPathToExe);
			if (ghMutex == NULL)
			{
				MessageBox(NULL, L"CommandTrayHost cannot CreateMutex, please report to Author!",
					L"Error",
					MB_OK | MB_ICONERROR);
			}
			LOGMESSAGE(L"makeSingleInstance2 CreateMutex 0x%x 0x%0x\n", ghMutex, GetLastError());
		}
		else
		{
			// The mutex exists so this is the
			// the second instance so return.
			LOGMESSAGE(L"makeSingleInstance2 found!\n");
			MessageBox(NULL, L"CommandTrayHost is already running!\n"
				L"If you are sure not, you can reboot your computer \n"
				L"or move CommandTrayHost.exe to other folder \n"
				L"or rename CommandTrayHost.exe",
				L"Error",
				MB_OK | MB_ICONERROR);
			exit(-1);
			return;
		}

		// The app is closing so release
		// the mutex.
		//ReleaseMutex(ghMutex);

	}
}
*/

/*
void makeSingleInstance()
{
	PCWSTR lock_filename = LOCK_FILE_NAME;
	int txt_pid = -1;
	if (TRUE == PathFileExists(lock_filename))
	{
		std::ifstream fi(lock_filename);
		if (fi.good())
		{
			fi >> txt_pid;
		}
		fi.close();
	}
	LOGMESSAGE(L"txt_pid %d\n", txt_pid);
	if (txt_pid > 0)
	{
		//https://stackoverflow.com/questions/4570174/how-to-get-the-process-name-in-c
		//https://stackoverflow.com/questions/1933113/c-windows-how-to-get-process-path-from-its-pid
		HANDLE Handle = OpenProcess(
			PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE,
			txt_pid // This is the PID, you can find one from windows task manager
		);
		if (Handle)
		{
			TCHAR Buffer[MAX_PATH * 10];
			if (GetModuleFileNameEx(Handle, NULL, Buffer, ARRAYSIZE(Buffer)))
			{
				// At this point, buffer contains the full path to the executable
				TCHAR szPathToExe[MAX_PATH * 10];
				if (GetModuleFileName(NULL, szPathToExe, ARRAYSIZE(szPathToExe))
					&& (0 == wcscmp(Buffer, szPathToExe)))
				{
					LOGMESSAGE(L"makeSingleInstance found!\n");
					MessageBox(NULL, L"CommandTrayHost is already running!", L"Error", MB_OK | MB_ICONERROR);
					exit(-1);
					return;
				}
				else
				{
					LOGMESSAGE(L"pid file path doesn't match! error code:0x%x \n", GetLastError());
				}
			}
			else
			{
				// You better call GetLastError() here
				LOGMESSAGE(L"GetModuleFileNameEx failed! error code:0x%x \n", GetLastError());
			}
			CloseHandle(Handle);
		}
	}
	DWORD pid = GetCurrentProcessId();

	std::ofstream fo(lock_filename);
	if (fo.good())
	{
		fo << pid;
		LOGMESSAGE(L"pid has wrote\n");
	}
	fo.close();
}
*/


/*
BOOL ShowPopupMenuJson2()
{
	POINT pt;
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	LPCTSTR lpCurrentProxy = GetWindowsProxy();
	std::vector<HMENU> vctHmenu = get_command_submenu(global_stat);


	if (lpProxyList[1] != NULL)
	{
		hSubMenu = CreatePopupMenu();
		for (int i = 0; lpProxyList[i]; i++)
		{
			UINT uFlags = wcscmp(lpProxyList[i], lpCurrentProxy) == 0 ? MF_STRING | MF_CHECKED : MF_STRING;
			LPCTSTR lpText = wcslen(lpProxyList[i]) ? lpProxyList[i] : (isZHCN ? L"\x7981\x7528\x4ee3\x7406" : L"<None>");
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE + i, lpText);
		}
	}

	HMENU hMenu = CreatePopupMenu();
	//其实是UTF8的十六进制,修正下，我发现UTF16与UTF8编码是一样的。
	//Windows API其实只支持UTF16LE UCS-2
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	if (hSubMenu != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu),
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	if (vctHmenu[0] != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(vctHmenu[0]),
			(isZHCN ? L"应用" : L"Daemon"));
	}
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : L"Exit"));
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	if (hSubMenu != NULL)
		DestroyMenu(hSubMenu);
	for (auto it : vctHmenu)
	{
		if (it != NULL)
		{
			DestroyMenu(it);
		}
	}
	//free vctHmenu memory now
	DestroyMenu(hMenu);
	return TRUE;
}

BOOL ShowPopupMenuJson()
{
	LPCTSTR MENUS_LEVEL2_CN[] = {
		L"命令" ,
		L"启用" ,
		L"重启命令"
	};
	LPCTSTR MENUS_LEVEL2_EN[] = {
		L"Command" ,
		L"Enable" ,
		L"Restart Command"
	};
	POINT pt;
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	LPCTSTR lpCurrentProxy = GetWindowsProxy();
	std::vector<HMENU> vctHmenu;
	hSubMenu = CreatePopupMenu();
	vctHmenu.push_back(hSubMenu);
	for (int i = 0; i < 8; i++)
	{
		hSubMenu = CreatePopupMenu();
		for (int j = 0; j < 3; j++)
		{
			UINT uFlags = MF_STRING | MF_CHECKED;
			LPCTSTR lpText = isZHCN ? MENUS_LEVEL2_CN[j] : MENUS_LEVEL2_EN[j];
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + j, lpText);
		}
		AppendMenu(vctHmenu[0], MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu,
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
		vctHmenu.push_back(hSubMenu);
	}

	if (lpProxyList[1] != NULL)
	{
		hSubMenu = CreatePopupMenu();
		for (int i = 0; lpProxyList[i]; i++)
		{
			UINT uFlags = wcscmp(lpProxyList[i], lpCurrentProxy) == 0 ? MF_STRING | MF_CHECKED : MF_STRING;
			LPCTSTR lpText = wcslen(lpProxyList[i]) ? lpProxyList[i] : (isZHCN ? L"\x7981\x7528\x4ee3\x7406" : L"<None>");
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE + i, lpText);
		}
	}

	HMENU hMenu = CreatePopupMenu();
	//其实是UTF8的十六进制,修正下，我发现UTF16与UTF8编码是一样的。
	//Windows API其实只支持UTF16LE UCS-2
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	if (hSubMenu != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu,
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	if (vctHmenu[0] != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)vctHmenu[0],
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : L"Exit"));
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	if (hSubMenu != NULL)
		DestroyMenu(hSubMenu);
	for (auto it : vctHmenu)
	{
		if (it != NULL)
		{
			DestroyMenu(it);
		}
	}
	//free vctHmenu memory now
	DestroyMenu(hMenu);
	return TRUE;
}

BOOL ShowPopupMenu()
{
	POINT pt;
	HMENU hSubMenu = NULL;
	BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	LPCTSTR lpCurrentProxy = GetWindowsProxy();
	if (lpProxyList[1] != NULL)
	{
		hSubMenu = CreatePopupMenu();
		for (int i = 0; lpProxyList[i]; i++)
		{
			UINT uFlags = wcscmp(lpProxyList[i], lpCurrentProxy) == 0 ? MF_STRING | MF_CHECKED : MF_STRING;
			LPCTSTR lpText = wcslen(lpProxyList[i]) ? lpProxyList[i] : (isZHCN ? L"\x7981\x7528\x4ee3\x7406" : L"<None>");
			AppendMenu(hSubMenu, uFlags, WM_TASKBARNOTIFY_MENUITEM_PROXYLIST_BASE + i, lpText);
		}
	}

	HMENU hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	if (hSubMenu != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu,
			(isZHCN ? L"\x8bbe\x7f6e IE \x4ee3\x7406" : L"Set IE Proxy"));
	}
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(hMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : L"Exit"));
	GetCursorPos(&pt);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	if (hSubMenu != NULL)
		DestroyMenu(hSubMenu);
	DestroyMenu(hMenu);
	return TRUE;
}

*/


BOOL ShowPopupMenuJson3()
{
	POINT pt;
	HMENU hSubMenu = NULL;
	//const LCID cur_lcid = GetSystemDefaultLCID();
	//const BOOL isZHCN = cur_lcid == 2052;
	//LPCTSTR lpCurrentProxy = GetWindowsProxy();
	std::vector<HMENU> vctHmenu;
	get_command_submenu(vctHmenu);

	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);
	AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDEALL, (isZHCN ? L"隐藏全部" : translate_w2w(L"Hide All").c_str()));
	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_DISABLEALL, (isZHCN ? L"全部禁用" : translate_w2w(L"Disable All").c_str()));
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_ENABLEALL, (isZHCN ? L"全部启动" : translate_w2w(L"Enable All").c_str()));
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOWALL, (isZHCN ? L"全部显示" : translate_w2w(L"Show All").c_str()));
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RESTARTALL, (isZHCN ? L"全部重启" : translate_w2w(L"Restart All").c_str()));
	AppendMenu(vctHmenu[0], MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), (isZHCN ? L"全部" : translate_w2w(L"All").c_str()));
	vctHmenu.push_back(hSubMenu);
	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);

	UINT uFlags = IsMyProgramRegisteredForStartup(szPathToExeToken) ? (MF_STRING | MF_CHECKED) : (MF_STRING);
	AppendMenu(vctHmenu[0], uFlags, WM_TASKBARNOTIFY_MENUITEM_STARTUP, (isZHCN ? L"开机启动" : translate_w2w(L"Start on Boot").c_str()));
	{
		AppendMenu(vctHmenu[0], is_runas_admin ? (MF_STRING | MF_CHECKED) : MF_STRING, WM_TASKBARNOTIFY_MENUITEM_ELEVATE, (isZHCN ? L"提权" : translate_w2w(L"Elevate").c_str()));
		/*HICON shieldIcon;
		if (GetStockIcon(shieldIcon))
		{
			AppendMenu(vctHmenu[0], is_runas_admin ? (MF_BITMAP | MF_CHECKED) : MF_BITMAP, WM_TASKBARNOTIFY_MENUITEM_ELEVATE, reinterpret_cast<LPCTSTR>(BitmapFromIcon(shieldIcon)));
		}*/

	}
	//AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_SHOW, (isZHCN ? L"\x663e\x793a" : L"Show"));
	//AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_HIDE, (isZHCN ? L"\x9690\x85cf" : L"Hide"));
	//AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_RELOAD, (isZHCN ? L"\x91cd\x65b0\x8f7d\x5165" : L"Reload"));
	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_OPENURL, (isZHCN ? L"主页" : translate_w2w(L"Home").c_str()));
	AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_ABOUT, (isZHCN ? L"关于" : translate_w2w(L"About").c_str()));
	AppendMenu(vctHmenu[0], MF_STRING | MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), (isZHCN ? L"帮助" : translate_w2w(L"Help").c_str()));
	vctHmenu.push_back(hSubMenu);

	AppendMenu(vctHmenu[0], MF_SEPARATOR, NULL, NULL);
	AppendMenu(vctHmenu[0], MF_STRING, WM_TASKBARNOTIFY_MENUITEM_EXIT, (isZHCN ? L"\x9000\x51fa" : translate_w2w(L"Exit").c_str()));

	GetCursorPos(&pt);
	TrackPopupMenu(vctHmenu[0], TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
	PostMessage(hWnd, WM_NULL, 0, 0);
	LOGMESSAGE(L"hWnd:0x%x\n", hWnd);

	for (auto it : vctHmenu)
	{
		if (it != NULL)
		{
			DestroyMenu(it);
		}
	}
	//free vctHmenu memory now
	return TRUE;
}


void get_command_submenu2(std::vector<HMENU>& outVcHmenu)
{
	LOGMESSAGE(L"enable_groups_menu:%d json %s\n", enable_groups_menu, utf8_to_wstring(global_stat.dump()).c_str());
	//return {};

#define RUNAS_ADMINISRATOR_INDEX 5

	LPCTSTR MENUS_LEVEL2_CN[] = {
		L"显示",
		L"隐藏" ,
		L"启用",
		L"停用",
		L"重启命令",
		L"管理员运行"  //index is 5, magic number
	};
	LPCTSTR MENUS_LEVEL2_EN[] = {
		L"Show",
		L"Hide" ,
		L"Enable" ,
		L"Disable",
		L"Restart Command",
		L"Run As Administrator" //index is 5, magic number
	};
	HMENU hSubMenu = NULL;
	//const LCID cur_lcid = GetSystemDefaultLCID();
	//const BOOL isZHCN = cur_lcid == 2052;
	//std::vector<HMENU> vctHmenu;
	if (!enable_groups_menu)
	{
		hSubMenu = CreatePopupMenu();
		outVcHmenu.push_back(hSubMenu);
	}
	int i = 0;
	//std::wstring local_wstring;
	for (auto& itm : (*global_configs_pointer))
	{
		hSubMenu = CreatePopupMenu();

		bool is_enabled = static_cast<bool>(itm["enabled"]);
		bool is_running = static_cast<bool>(itm["running"]);
		bool is_show = static_cast<bool>(itm["show"]);
		bool is_en_job = static_cast<bool>(itm["en_job"]);

		int64_t handle = itm["handle"];
		if (is_running)
		{
			DWORD lpExitCode;
			BOOL retValue = GetExitCodeProcess(reinterpret_cast<HANDLE>(handle), &lpExitCode);
			if (retValue != 0 && lpExitCode != STILL_ACTIVE)
			{
				itm["running"] = false;
				itm["en_job"] = false;
				itm["handle"] = 0;
				itm["pid"] = -1;
				itm["hwnd"] = 0;
				itm["win_num"] = 0;
				itm["show"] = false;
				itm["enabled"] = false;

				is_running = false;
				is_show = false;
				is_enabled = false;
			}
		}

		UINT uSubFlags = (is_en_job && is_running) ? (MF_STRING) : (MF_STRING | MF_GRAYED);
		AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 0,
			utf8_to_wstring(itm["path"]).c_str());
		AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 1,
			utf8_to_wstring(itm["cmd"]).c_str());
		//AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 2,
			//utf8_to_wstring(itm["working_directory"]).c_str());
		AppendMenu(hSubMenu, MF_SEPARATOR, NULL, NULL);

		const int info_items_cnt = 2;
		uSubFlags = is_enabled ? (MF_STRING) : (MF_STRING | MF_GRAYED);
		for (int j = 0; j < 3; j++)
		{
			int menu_name_item;// = j + (j == 0 && is_running) + (j == 1 && is_show) + (j == 2 ? 0 : 2);
			if (j == 0)
			{
				if (is_show) { menu_name_item = 1; }
				else { menu_name_item = 0; }
			}
			else if (j == 1)
			{
				if (is_enabled) { menu_name_item = 3; }
				else { menu_name_item = 2; }
			}
			else
			{
				menu_name_item = 4;
			}
			/*LPCTSTR lpText;

			if (isZHCN)
			{
				lpText = MENUS_LEVEL2_CN[menu_name_item];
			}
			else
			{
				local_wstring = translate_w2w(MENUS_LEVEL2_EN[menu_name_item], cur_lcid);
				lpText = local_wstring.c_str();
			}*/
			if (j != 1)
			{
				AppendMenu(hSubMenu, uSubFlags, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					isZHCN ? MENUS_LEVEL2_CN[menu_name_item] :
					translate_w2w(MENUS_LEVEL2_EN[menu_name_item]).c_str()
				);
			}
			else
			{
				AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + info_items_cnt + j,
					isZHCN ? MENUS_LEVEL2_CN[menu_name_item] :
					translate_w2w(MENUS_LEVEL2_EN[menu_name_item]).c_str()
				);
			}
		}
		if (!is_runas_admin)
		{
			/*LPCTSTR lpText;// = isZHCN ? MENUS_LEVEL2_CN[RUNAS_ADMINISRATOR_INDEX] : MENUS_LEVEL2_EN[RUNAS_ADMINISRATOR_INDEX];
			if (isZHCN)
			{
				lpText = MENUS_LEVEL2_CN[RUNAS_ADMINISRATOR_INDEX];
			}
			else
			{
				local_wstring = translate_w2w(MENUS_LEVEL2_EN[RUNAS_ADMINISRATOR_INDEX], cur_lcid);
				lpText = local_wstring.c_str();
			}*/
			AppendMenu(hSubMenu, MF_SEPARATOR, NULL, NULL);
			AppendMenu(hSubMenu, MF_STRING, WM_TASKBARNOTIFY_MENUITEM_COMMAND_BASE + i * 0x10 + 5,
				isZHCN ? MENUS_LEVEL2_CN[RUNAS_ADMINISRATOR_INDEX] :
				translate_w2w(MENUS_LEVEL2_EN[RUNAS_ADMINISRATOR_INDEX]).c_str()
			);
		}
		if (!enable_groups_menu)
		{
			UINT uFlags = is_enabled ? (MF_STRING | MF_CHECKED | MF_POPUP) : (MF_STRING | MF_POPUP);
			AppendMenu(outVcHmenu[0], uFlags, reinterpret_cast<UINT_PTR>(hSubMenu), utf8_to_wstring(itm["name"]).c_str());
		}
		outVcHmenu.push_back(hSubMenu);
		i++;
	}
	if (enable_groups_menu)
	{
		std::wstring menu_symbol_wstring = utf8_to_wstring(global_stat["groups_menu_symbol"]) + L" ";
		hSubMenu = CreatePopupMenu();
		vector_hemnu_p = &outVcHmenu;
		level_menu_symbol_p = menu_symbol_wstring.c_str();
		create_group_level_menu(*global_groups_pointer, hSubMenu);
		outVcHmenu.insert(outVcHmenu.begin(), hSubMenu);
	}
	//return true;
	//return vctHmenu;
}


bool is_cache_not_expired(bool is_from_flush/*,bool is_from_other_thread*/)
{
	LOGMESSAGE(L"GetCurrentThreadId:%d\n", GetCurrentThreadId());
	/*if (enable_critialsection)
	{
		EnterCriticalSection(&CriticalSection);
		if(is_from_flush)
		{
			LOGMESSAGE(L"EnterCriticalSection\n");
		}
	}*/
	PCWSTR json_filename = CONFIG_FILENAMEW;
	PCWSTR cache_filename = CACHE_FILENAMEW;

#define RETURN_LEAVECRITIALCAL(val) { \
	/*if(enable_critialsection)LeaveCriticalSection(&CriticalSection);*/ \
	return val; \
}

	if (TRUE != PathFileExists(json_filename))
	{
		extern HANDLE ghJob;
		extern HICON gHicon;
		if (NULL == init_global(ghJob, gHicon))
		{
			//MessageBox(NULL, L"Initialization failed!", L"Error", MB_OK | MB_ICONERROR);
			//enable_cache = true;
			//return true;
			RETURN_LEAVECRITIALCAL(true);
		}
	}
	if (enable_cache && TRUE != PathFileExists(cache_filename))
	{
		if (is_from_flush && global_stat != nullptr)
		{
			if (conform_cache_expire)
			{
				const int result = msg_prompt(//NULL,
					isZHCN ? L"缓存文件被删除了，是否要写入旧缓存！\n\n选择 是 则临时禁用缓存"
					L"\n\n选择 否 则继续缓存数据，如果改动了" CONFIG_FILENAMEW L"同时删除了缓存，选 否 使用内存的缓存"
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
				else if (IDYES == result) // global_stat != nullptr
				{
					enable_cache = false;
				}
			}
		}
		LOGMESSAGE(L"PathFileExists failed\n");
		//return false;
		RETURN_LEAVECRITIALCAL(false);
	}
	HANDLE json_hFile = CreateFile(json_filename, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE cache_hFile = nullptr;
	if (enable_cache)
	{
		cache_hFile = CreateFile(cache_filename, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
	}

#define CLOSE_CREATEFILE(fp) { \
	if(fp)CloseHandle(fp); \
	fp=NULL; \
}

#define RETURN_AND_CLOSE_CREATEFILE(ret) { \
	CLOSE_CREATEFILE(json_hFile);\
	if(enable_cache)CLOSE_CREATEFILE(cache_hFile);\
	RETURN_LEAVECRITIALCAL(ret); \
}
	if (!json_hFile || (enable_cache && !cache_hFile))
	{
		LOGMESSAGE(L"CreateFile failed\n");
		RETURN_AND_CLOSE_CREATEFILE(false);
	}
	FILETIME json_write_timestamp, cache_write_timestamp;
	if (!GetFileTime(json_hFile, NULL, NULL, &json_write_timestamp) ||
		(enable_cache && !GetFileTime(cache_hFile, NULL, NULL, &cache_write_timestamp)))
	{
		LOGMESSAGE(L"GetFileTime failed\n");
		RETURN_AND_CLOSE_CREATEFILE(false);
	}
	CLOSE_CREATEFILE(json_hFile);
	if (enable_cache)CLOSE_CREATEFILE(cache_hFile);
	if (!enable_cache || (enable_cache && CompareFileTime(&json_write_timestamp, &cache_write_timestamp) >= 0))
	{
		LOGMESSAGE(L"json_write_timestamp is later than cache_write_timestamp\n");
		bool return_val = false;
		if (conform_cache_expire)
		{
			//bool isZHCN = GetSystemDefaultLCID() == 2052 || GetACP() == 936;
			LOGMESSAGE(L"isZHCN:%d isENUS:%d\n", isZHCN, isENUS);
			//extern HWND hWnd;
			//SetForegroundWindow(hWnd);
			int result;
			bool is_reloading_config = is_from_flush && global_stat != nullptr;
			if (!auto_hot_reloading_config)
			{
				result = msg_prompt(//NULL,
					isZHCN ? (is_reloading_config ?
						L"config.json被编辑过了,缓存可能已经失效！\n\n选择 是 则清空缓存，关闭全部在运行的程序，重新读取配置。"
						L"\n\n选择 否 则重新加载配置,但是并不删除缓存,cmd path working_directory未修改的运行中的程序不会被关闭"
						L"\n\n选择 取消 则保留缓存数据,下次启动CommandTrayHost才加载config.json"
						:
						L"config.json被编辑过了，缓存可能已经失效！\n\n选择 是 则清空缓存"
						L"\n\n选择 否 则保留缓存数据"
						)
					:
					translate_w2w(L"You just edit config.json!\n\nChoose Yes to clear"
						L" cache\n\nChoose No to keep expired cache.").c_str(),
					isZHCN ? L"是否要清空缓存？" : translate_w2w(L"Clear cache?").c_str(),
					is_reloading_config ? MB_YESNOCANCEL : MB_YESNO
				);
			}
			else
			{
				//result = is_reloading_config ? IDCANCEL : IDNO;
				result = IDNO;
			}
			if (enable_cache && (IDNO == result || IDCANCEL == result))
			{
				return_val = true;
				if (global_stat == nullptr || IDCANCEL == result)
				{
					std::ofstream o_cache(CACHE_FILENAMEA, std::ios_base::app | std::ios_base::out);
					o_cache << std::endl;
				}
			}
			if (IDYES == result || IDCANCEL == result)
			{
				if (global_stat == nullptr)
				{
					LOGMESSAGE(L"IDYES nullptr\n");
				}
				else
				{
					LOGMESSAGE(L"IDYES\n");

					enable_cache = false;
					/*if (is_from_other_thread)
					{
						LOGMESSAGE(L"is_from_other_thread GetCurrentThreadId:%d\n", GetCurrentThreadId());
						extern HWND hWnd;
						SendMessage(hWnd, WM_COMMAND, WM_TASKBARNOTIFY_MENUITEM_UNREGISTRYHOTKEY_CRONTAB, NULL);
					}
					else
					{
						unregisterhotkey_killtimer_all();
					}*/
					unregisterhotkey_killtimer_all();
					if (IDYES == result)
					{
						if (NULL == DeleteFile(CACHE_FILENAMEW))
						{
							LOGMESSAGE(L"DeleteFile GetLastError:%d\n", GetLastError());
							msg_prompt(/*NULL,*/ L"Delete " CACHE_FILENAMEW L" Failed!", L"Delete failed", MB_OK);
						}
						kill_all();
					}
					extern HANDLE ghJob;
					extern HICON gHicon;
					if (NULL == init_global(ghJob, gHicon))
					{
						//MessageBox(NULL, L"Initialization failed!", L"Error", MB_OK | MB_ICONERROR);
						//enable_cache = true;
						//return true;
						RETURN_LEAVECRITIALCAL(true);
					}
					start_all(ghJob);
					DeleteTrayIcon();
					ShowTrayIcon(CONFIG_FILENAMEW L" has been reloaded.", NIM_ADD);
					//enable_cache = false;
					//return false;
					RETURN_LEAVECRITIALCAL(false);
				}

			}
		}
		RETURN_AND_CLOSE_CREATEFILE(return_val);
	}
	RETURN_AND_CLOSE_CREATEFILE(true);
}

