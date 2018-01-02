#pragma once

//void to_json(nlohmann::json& j, const cron_expr& p);

//void from_json(const nlohmann::json& j, cron_expr& p);

void crontab_log(const nlohmann::json& jsp_crontab_config, time_t, time_t, PCSTR, PCSTR, PCSTR, int, int);

cron_expr* get_cron_expr(const nlohmann::json& jsp, cron_expr& result);


void handle_crontab(size_t idx);
