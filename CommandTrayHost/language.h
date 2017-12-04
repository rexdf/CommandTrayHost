#pragma once

bool json_object_has_member(const nlohmann::json&, PCSTR);

void update_isZHCN(bool);
void update_locale_name_by_alias();
void update_locale_name_by_system();

void initialize_local();

std::string translate(std::string);

std::wstring translate_w2w(std::wstring);
