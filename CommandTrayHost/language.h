#pragma once

bool json_object_has_member(const nlohmann::json&, PCSTR);

void initialize_local();

std::string translate(std::string);

std::wstring translate_w2w(std::wstring);
