#include "stdafx.h"
#include "language.h"
#include "configure.h"
#include "language_data.h"

////////////////// CODE AREA ////////////////////////
bool json_object_has_member(const nlohmann::json& root, PCSTR query_string)
{
	try
	{
		root.at(query_string);
	}
#ifdef _DEBUG
	catch (std::out_of_range& e)
#else
	catch (std::out_of_range&)
#endif
	{
		LOGMESSAGE(L"json_object_has_member out_of_range %S\n", e.what());
		return false;
	}
	catch (...)
	{
		::MessageBox(NULL,
			L"json_object_has_member error",
			L"Type Error",
			MB_OK | MB_ICONERROR
		);
		return false;
	}
	return true;
}

extern nlohmann::json global_stat;
extern CHAR locale_name[];
extern BOOL isZHCN;

void update_locale_name_by_alias()
{
	if (json_object_has_member(language_alias, locale_name))
	{
		std::string locale_alias = language_alias[locale_name];
		strcpy_s(locale_name, LOCALE_NAME_MAX_LENGTH, locale_alias.c_str());
	}
}

std::string translate(std::string en)
{
	if (language_data == nullptr)return en;
	/*char lang[10];
	snprintf(lang, 8, "0x%04x", language_code);
	LOGMESSAGE(L"lang: %S\n", lang);*/

	if (false == json_object_has_member(language_data, locale_name) ||
		false == json_object_has_member(language_data[locale_name], en.c_str())
		)
	{
		return en;
	}
	LOGMESSAGE(L"translation success: %s -> %s\n",
		utf8_to_wstring(en).c_str(),
		utf8_to_wstring(language_data[locale_name][en.c_str()]).c_str()
	);
	return language_data[locale_name][en.c_str()];
}

std::wstring translate_w2w(std::wstring en)
{
	return  utf8_to_wstring(translate(wstring_to_utf8(en)));
}

void update_isZHCN()
{
	if (0 == strcmp(locale_name, "zh-CN") ||
		0 == strcmp(locale_name, "zh-Hans") ||
		0 == strcmp(locale_name, "zh") ||
		0 == strcmp(locale_name, "zh-SG")
		)
	{
		isZHCN = TRUE;
	}
	else
	{
		isZHCN = GetSystemDefaultLCID() == 2052 || GetACP() == 936;
	}
}

void update_locale_name_by_system()
{
	WCHAR wlocale_name[LOCALE_NAME_MAX_LENGTH];
	if (GetUserDefaultLocaleName(wlocale_name, LOCALE_NAME_MAX_LENGTH))
	{
		LOGMESSAGE(L"initialize_local GetUserDefaultLocaleName %s\n", wlocale_name);
		strcpy_s(locale_name, LOCALE_NAME_MAX_LENGTH, wstring_to_utf8(wlocale_name).c_str());
		update_isZHCN();
	}
	else
	{
		isZHCN = GetSystemDefaultLCID() == 2052;
		LOGMESSAGE(L"initialize_local failed!\n");
	}
}

void initialize_local()
{
	LOGMESSAGE(L"GetUserDefaultUILanguage: %d GetSystemDefaultUILanguage: %d GetACP: %d\n",
		GetUserDefaultUILanguage(),
		GetSystemDefaultUILanguage(),
		GetACP()
	);
	if (json_object_has_member(global_stat, "lang"))
	{
		std::string local = global_stat["lang"];
		//wcscpy(local_name, utf8_to_wstring(local).c_str());
		strcpy_s(locale_name, LOCALE_NAME_MAX_LENGTH, local.c_str());
		if (0 == strcmp(locale_name, "auto"))
		{
			update_locale_name_by_system();
		}
		LOGMESSAGE(L"initialize_local json_object_has_member %S\n", locale_name);
		update_isZHCN();
	}
	else
	{
		update_locale_name_by_system();
	}
	update_locale_name_by_alias();
	LOGMESSAGE(L"initialize_local %S isZHCN: %d\n", locale_name, isZHCN);
}
