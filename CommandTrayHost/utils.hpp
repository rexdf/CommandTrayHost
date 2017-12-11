#pragma once

std::wstring utf8_to_wstring(const std::string&);
std::string wstring_to_utf8(const std::wstring&);

int64_t FileSize(PCWSTR);

bool json_object_has_member(const nlohmann::json&, PCSTR);

/*
* Make sure out is initialized with default value before call try_read_optional_json
*/
template<typename Type>
#ifdef _DEBUG
bool try_read_optional_json(const nlohmann::json& root, Type& out, PCSTR query_string, PCSTR caller_fuc_name)
#else
bool try_read_optional_json(const nlohmann::json& root, Type& out, PCSTR query_string)
#endif
{
	//Type ignore_all = false; // Do it before call try_read_optional_json
	try
	{
		out = root.at(query_string);
	}
#ifdef _DEBUG
	catch (std::out_of_range& e)
#else
	catch (std::out_of_range&)
#endif
	{
		LOGMESSAGE(L"%S %S out_of_range %S\n", caller_fuc_name, query_string, e.what());
		return false;
	}
	catch (...)
	{
		MessageBox(NULL,
			(utf8_to_wstring(query_string) + L" type check failed!").c_str(),
			L"Type Error",
			MB_OK | MB_ICONERROR
		);
		return false;
	}
	return true;
}

bool rapidjson_check_exist_type2(rapidjson::Value&, PCSTR, rapidjson::Type, bool not_exist_return = true);

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

BOOL get_hicon(PCWSTR, int, HICON&, bool share = false);

#ifdef _DEBUG
void ChangeIcon(const HICON);
#endif
