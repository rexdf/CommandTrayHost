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
		LOGMESSAGE(L"translate out_of_range %S\n", e.what());
		return false;
	}
	catch (...)
	{
		::MessageBox(NULL,
			L"translate error",
			L"Type Error",
			MB_OK | MB_ICONERROR
		);
		return false;
	}
	return true;
}

std::string translate(std::string en, LCID language_code)
{
	if (language_data == nullptr)return en;
	char lang[10];
	snprintf(lang, 8, "0x%04x", language_code);
	LOGMESSAGE(L"lang: %S\n", lang);
	if (false == json_object_has_member(language_data, lang) ||
		false == json_object_has_member(language_data[lang], en.c_str())
		)
	{
		return en;
	}
	LOGMESSAGE(L"translation success: %s -> %s\n",
		utf8_to_wstring(en).c_str(),
		utf8_to_wstring(language_data[lang][en.c_str()]).c_str()
	);
	return language_data[lang][en.c_str()];
}

std::wstring translate_w2w(std::wstring en, LCID language_code)
{
	return  utf8_to_wstring(translate(wstring_to_utf8(en), language_code));
}