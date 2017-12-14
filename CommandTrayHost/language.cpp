#include "stdafx.h"
#include "language.h"
//#include "configure.h"
#include "language_data.h"
#include "utils.hpp"


//extern nlohmann::json global_stat;
extern CHAR locale_name[LOCALE_NAME_MAX_LENGTH];
extern BOOL isZHCN, isENUS;

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
	LOGMESSAGE(L"success: %s -> %s\n",
		utf8_to_wstring(en).c_str(),
		utf8_to_wstring(language_data[locale_name][en.c_str()]).c_str()
	);
	return language_data[locale_name][en.c_str()];
}

std::wstring translate_w2w(std::wstring en)
{
	if (isENUS)return en;
	return  utf8_to_wstring(translate(wstring_to_utf8(en)));
}

void update_isZHCN(bool check_system_acp)
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
		isZHCN = FALSE;
	}
	if (FALSE == isZHCN && true == check_system_acp)
	{
		// GetSystemDefaultLCID https://msdn.microsoft.com/en-us/library/windows/desktop/dd318693(v=vs.85).aspx
		// GetACP https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx
		// I think that GetACP is better than GetUserDefaultLocaleName
		isZHCN = GetSystemDefaultLCID() == 2052 || GetACP() == 936;
	}
}

void update_locale_name_by_alias()
{
	if (json_object_has_member(language_alias, locale_name))
	{
		std::string locale_alias = language_alias[locale_name];
		// strcpy_s(locale_name, LOCALE_NAME_MAX_LENGTH, locale_alias.c_str());
		if (FAILED(StringCchCopyA(locale_name, ARRAYSIZE(locale_name), locale_alias.c_str())))
		{
			LOGMESSAGE(L"StringCchCopyA Failed\n");
		}
		LOGMESSAGE(L"locale_alias:%S\n", locale_alias);
	}
	else
	{
		LOGMESSAGE(L"locale_name not found!\n");
	}
}

void update_locale_name_by_system()
{
#if VER_PRODUCTBUILD == 7600
	PCSTR locale_pointer = NULL;
	if (GetSystemDefaultLCID() == 2052 || GetACP() == 936)
	{
		locale_pointer = "zh-CN";
	}
	else
	{
		locale_pointer = "en-US";
	}
	if (FAILED(StringCchCopyA(locale_name, ARRAYSIZE(locale_name), locale_pointer)))
	{
		LOGMESSAGE(L"update_locale_name_by_alias StringCchCopyA Failed\n");
	}
#else
	WCHAR wlocale_name[LOCALE_NAME_MAX_LENGTH];
	if (GetUserDefaultLocaleName(wlocale_name, LOCALE_NAME_MAX_LENGTH))
	{
		LOGMESSAGE(L"GetUserDefaultLocaleName %s\n", wlocale_name);
		// strcpy_s(locale_name, LOCALE_NAME_MAX_LENGTH, wstring_to_utf8(wlocale_name).c_str());
		if (FAILED(StringCchCopyA(locale_name, ARRAYSIZE(locale_name), wstring_to_utf8(wlocale_name).c_str())))
		{
			LOGMESSAGE(L"StringCchCopyA Failed\n");
		}
		//update_isZHCN();
	}
	else
	{
		//isZHCN = GetSystemDefaultLCID() == 2052;
		LOGMESSAGE(L"initialize_local failed!\n");
	}
#endif
}

void initialize_local(bool has_lang, PCSTR lang_str)
{
	LOGMESSAGE(L"GetUserDefaultUILanguage: 0x%x GetSystemDefaultUILanguage: 0x%0x GetACP: %d\n",
		GetUserDefaultUILanguage(),
		GetSystemDefaultUILanguage(),
		GetACP()
	);
	LOGMESSAGE(L"has_lang:%d lang_str:%S\n", has_lang, lang_str);
	//if (false == json_object_has_member(global_stat, "lang") || global_stat["lang"] == "auto")
	if (false == has_lang || 0 == strcmp(lang_str, "auto"))
	{
		update_locale_name_by_system();
		update_locale_name_by_alias();
		update_isZHCN(true); //use system acp check
	}
	else
	{
		//assert(json_object_has_member(global_stat, "lang"));
		assert(lang_str);
		//if (json_object_has_member(global_stat, "lang"))  // it must be sure
		{
			std::string local = lang_str;
			// strcpy_s(locale_name, LOCALE_NAME_MAX_LENGTH, local.c_str());
			if (FAILED(StringCchCopyA(locale_name, ARRAYSIZE(locale_name), local.c_str())))
			{
				LOGMESSAGE(L"StringCchCopyA Failed\n");
			}
		}
		update_locale_name_by_alias();
		update_isZHCN(false); //use user defined
	}
	LOGMESSAGE(L"final locale_name %S\n", locale_name);

	/*if (json_object_has_member(global_stat, "lang"))
	{
		std::string local = global_stat["lang"];
		//wcscpy(local_name, utf8_to_wstring(local).c_str());
		strcpy_s(locale_name, LOCALE_NAME_MAX_LENGTH, local.c_str());
		if (0 == strcmp(locale_name, "auto"))
		{
			update_locale_name_by_system();
			update_locale_name_by_alias();
			update_isZHCN(true); //use system acp check
		}
		else
		{
			update_locale_name_by_alias();
			update_isZHCN(false); //use user defined
		}
		LOGMESSAGE(L"json_object_has_member %S\n", locale_name);
	}
	else // no lang items
	{
		update_locale_name_by_system();
		update_locale_name_by_alias();
		update_isZHCN(true); //use system acp check
	}*/

	if (isZHCN == FALSE && (0 == strcmp(locale_name, "en-US") ||
		false == json_object_has_member(language_data, locale_name))
		)
	{
		isENUS = TRUE;
	}
	else
	{
		isENUS = FALSE;
	}
	LOGMESSAGE(L"locale_name: %S isZHCN: %d isENUS: %d\n", locale_name, isZHCN, isENUS);
	// BOOL isZHCN = GetSystemDefaultLCID() == 2052;
	extern WCHAR szBalloon[512];
	if (not isZHCN)
	{
		ZeroMemory(szBalloon, sizeof(szBalloon));
		if (FAILED(StringCchCopy(szBalloon,
			ARRAYSIZE(szBalloon),
			translate_w2w(L"CommandTrayHost Started，Click Tray icon to Hide/Show Console.").c_str()))
			)
		{
			LOGMESSAGE(L"init_global StringCchCopy failed\n");
		}
		// wcscpy_s(szBalloon, translate_w2w(L"CommandTrayHost Started，Click Tray icon to Hide/Show Console.").c_str());
	}
}
