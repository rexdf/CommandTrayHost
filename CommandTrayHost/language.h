#pragma once

void update_isZHCN(bool);
void update_locale_name_by_alias();
void update_locale_name_by_system();

void initialize_local(bool has_lang, PCSTR lang_str);

// std::string translate(std::string);

//std::wstring translate_w2w(const std::wstring&);
std::string translate(const std::string&);
