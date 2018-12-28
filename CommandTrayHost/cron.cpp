#include "stdafx.h"
#include "cron.h"
#include "utils.hpp"
#include "CommandTrayHost.h"
#include "configure.h"
#include "cache.h"

extern nlohmann::json* global_cache_configs_pointer;
extern nlohmann::json* global_configs_pointer;

extern bool enable_cache;
extern bool disable_cache_show;

/*void to_json(nlohmann::json& j, const cron_expr& p) {
	j = nlohmann::json{ std::string(reinterpret_cast<const char*>(&p), sizeof(cron_expr)) };
}

void from_json(const nlohmann::json& j, cron_expr& p) {
	memcpy(reinterpret_cast<char*>(&p), j.get<std::string>().data(), sizeof(cron_expr));
}

cron_expr* get_cron_expr(const nlohmann::json& jsp, cron_expr& result)
{
	if (json_object_has_member(jsp, "crontab_config") && json_object_has_member(jsp["crontab_config"], "cron_expr"))
	{
		result = jsp["crontab_config"]["cron_expr"];
		return &result;
	}
	return nullptr;
}*/

void rotate_file(PCWSTR filename)
{
	TCHAR buffer[MAX_PATH * 10];
	for (int i = 1; i < 500; i++)
	{
		StringCchPrintf(buffer, ARRAYSIZE(buffer), L"%s.%d", filename, i);
		if (TRUE != PathFileExists(buffer))
		{
			if (MoveFile(filename, buffer))
			{

			}
			else
			{
				msg_prompt(
					L"cannot rename filename!",
					L"Logrotate failed!",
					MB_OK
				);
			}
			return;
		}
	}
	msg_prompt(L"There are too many log files, Please delete or move them elsewhere.", L"Logrotate error", MB_OK);
}

/*
 * Make sure jsp has "crontab_config", before call crontab_log
 * @param time_cur,current time.
 * @param time_next,next schedule time. If both are 0, just output log_msg & cron_msg
 */
void crontab_log(const nlohmann::json& jsp_crontab_config,
	time_t time_cur,
	time_t time_next,
	PCSTR name,
	PCSTR log_msg,
	PCSTR cron_msg,
	int log_count,
	int log_level_limit
)
{
	//if (json_object_has_member(jsp, "crontab_config"))
	{
		//auto& crontab_config_ref = jsp["crontab_config"];
		int log_level = jsp_crontab_config.value("log_level", 0);
		if (log_level < log_level_limit) { return; }
		const size_t buffer_len = 256;
		char buffer[buffer_len];
		size_t idx = 0, len;
		tm t1;
		bool is_crontab_trigger_msg = true;
		if (time_cur == 0)
		{
			time_cur = time(NULL);
			if (0 == time_next)is_crontab_trigger_msg = false;
		}
		localtime_s(&t1, &time_cur);
		idx = strftime(buffer, ARRAYSIZE(buffer), "%Y-%m-%d %H:%M:%S ", &t1);
		if (is_crontab_trigger_msg)
		{
			printf_to_bufferA(buffer, buffer_len - idx, idx,
				"[%s] [%s] [left count: %d] [%s]",
				name,
				log_msg,
				log_count,
				//log_count == 0 ? " infinite" : "",
				cron_msg
			);
			if (time_next)
			{
				localtime_s(&t1, &time_next);
				len = strftime(buffer + idx, ARRAYSIZE(buffer), " %Y-%m-%d %H:%M:%S ", &t1);
				idx += len;
			}
		}
		else
		{
			printf_to_bufferA(buffer, buffer_len - idx, idx,
				"[%s] [%s] [%s]",
				name,
				log_msg,
				cron_msg
			);
		}

		std::string crontab_log_filename = jsp_crontab_config["log"];
		std::wstring crontab_log_filename_w = utf8_to_wstring(crontab_log_filename);
		if (TRUE == PathFileExists(crontab_log_filename_w.c_str()) && FileSize(crontab_log_filename_w.c_str()) > 1024 * 1024 * 10)
		{
			rotate_file(crontab_log_filename_w.c_str());
		}
		std::ofstream o_log(crontab_log_filename.c_str(), std::ios_base::app | std::ios_base::out);
		o_log << buffer << std::endl;
	}
}

cron_expr* get_cron_expr(const nlohmann::json& jsp, cron_expr& result)
{
	if (json_object_has_member(jsp, "crontab_config"))
	{
		auto& crontab_config_ref = jsp["crontab_config"];
		if (crontab_config_ref["enabled"])
		{
			//cron_expr expr;
			ZeroMemory(&result, sizeof(cron_expr)); // if not do this, always get incorrect result
			const char* err = NULL;
			cron_parse_expr(crontab_config_ref["crontab"].get<std::string>().c_str(), &result, &err);
			if (err)
			{
				LOGMESSAGE(L"cron_parse_expr failed! %S\n", err);
			}
			else
			{
				return &result;
			}
		}
	}
	return nullptr;
}


void handle_crontab(size_t idx)
{
	auto& config_i_ref = (*global_configs_pointer)[idx]; // ["crontab_config"]
	if (json_object_has_member(config_i_ref, "crontab_config"))
	{
		auto& crontab_ref = config_i_ref["crontab_config"];
		extern HWND hWnd;
		KillTimer(hWnd, VM_TIMER_BASE + idx);

		//bool crontab_write_log = false;
		time_t log_time_cur = time(NULL), log_time_next = 0;
		PCSTR log_msg = nullptr, log_cron_msg = nullptr;
		int log_count = 0;
		std::string crontab_method = crontab_ref["method"];
		bool need_renew = crontab_ref["need_renew"];
		extern HANDLE ghJob;
		bool enable_cache_backup = enable_cache;
		enable_cache = false;

		bool to_start;
		if (false == need_renew)
		{
			if (crontab_method.compare(0, ARRAYSIZE("start") - 1, "start") == 0)
			{
				to_start = true;

				if (config_i_ref["running"])
				{
					int64_t handle = config_i_ref["handle"];
					DWORD lpExitCode;
					BOOL retValue = GetExitCodeProcess(reinterpret_cast<HANDLE>(handle), &lpExitCode);
					if (retValue != 0 && lpExitCode == STILL_ACTIVE)
					{
						to_start = false;
						log_msg = "method:start program is still running.";
					}
				}
				if (to_start)
				{
					/*config_i_ref["enabled"] = true;
					create_process(config_i_ref, ghJob, true);*/
					log_msg = "method:start started.";
					//crontab_write_log = true;
				}
			}
			/*else if (crontab_method == "restart")
			{

			}
			else if (crontab_method == "stop")*/
			else
			{
				to_start = false;
				if (config_i_ref["enabled"] && config_i_ref["running"] && config_i_ref["en_job"])
				{
					disable_enable_menu(config_i_ref, ghJob);
					log_msg = "method:stop killed.";
					//crontab_write_log = true;
				}
				if (crontab_method.compare(0, ARRAYSIZE("restart") - 1, "restart") == 0) // should I minus 1
				{
					to_start = true;
					/*config_i_ref["enabled"] = true;
					create_process(config_i_ref, ghJob, false, true);*/
					log_msg = "method:restart done.";
					//crontab_write_log = true;
				}
			}
			if (to_start)
			{
				config_i_ref["enabled"] = true;
				bool start_show = false, config_i_start_show_backup = false;
				if (json_object_has_member(crontab_ref, "start_show"))
				{
					start_show = crontab_ref["start_show"];
				}
				else
				{
					if (enable_cache_backup && !disable_cache_show)
					{
						auto& ref = (*global_cache_configs_pointer)[idx];
						if (check_cache_valid(ref["valid"].get<int>(), cShow))
						{
							start_show = ref["start_show"];
							LOGMESSAGE(L"start_show cache hit!");
						}
					}
				}
				if (json_object_has_member(config_i_ref, "start_show"))
				{
					config_i_start_show_backup = config_i_ref["start_show"];
				}
				config_i_ref["start_show"] = start_show;
				create_process(config_i_ref, ghJob, false, true);
				config_i_ref["start_show"] = config_i_start_show_backup;
			}
		}
		else
		{
			log_msg = "method:renew";
		}
		enable_cache = enable_cache_backup;


		int crontab_count = crontab_ref["count"];

		if (need_renew || crontab_count != 1)
		{
			cron_expr c;
			if (nullptr != get_cron_expr(config_i_ref, c))
			{
				time_t next_t = 0, now_t = time(NULL);
				next_t = cron_next(&c, now_t); // return -1 when failed
				LOGMESSAGE(L"next_t %llu now_t %llu\n", next_t, now_t);
				if (next_t != static_cast<time_t>(-1) && next_t > now_t)
				{
					log_time_next = next_t; // logging

					next_t -= now_t;
					if (next_t > CRONTAB_MAXIUM_SECONDS)
					{
						next_t = CRONTAB_RENEW_MARKER;
						if (!need_renew)crontab_ref["need_renew"] = true;
					}
					else if (need_renew)
					{
						crontab_ref["need_renew"] = false;
					}
					next_t *= 1000;
					//if (next_t > USER_TIMER_MAXIMUM)next_t = USER_TIMER_MAXIMUM;
					SetTimer(hWnd, VM_TIMER_BASE + idx, static_cast<UINT>(next_t), NULL);

					log_cron_msg = "schedule next"; // logging

					if (false == need_renew && crontab_count > 1)
					{
						crontab_ref["count"] = crontab_count - 1;
						log_count = crontab_count - 1;
					}
				}
			}
		}
		else
		{
			crontab_ref["enabled"] = false;

			log_count = -1;
			// *_stop
			size_t crontab_method_len = crontab_method.length();
			if (crontab_method.compare(crontab_method_len - ARRAYSIZE("_stop"), ARRAYSIZE("_stop"), "_stop") == 0)
			{
				config_i_ref["enabled"] = true;
				disable_enable_menu(config_i_ref, ghJob);
				log_cron_msg = "count limited,crontab stopped. program stopped.";
			}
			else
			{
				log_cron_msg = "count limited,crontab stopped.";
			}
		}
		if (json_object_has_member(crontab_ref, "log"))
		{
			crontab_log(crontab_ref, log_time_cur, log_time_next, config_i_ref["name"].get<std::string>().c_str(), log_msg, log_cron_msg, log_count, 0);
		}
	}
	else
	{
		msg_prompt(L"Crontab has no crontab_config! Please report this windows screenshot to author!",
			L"Crontab Error",
			MB_OK
		);
	}
}
