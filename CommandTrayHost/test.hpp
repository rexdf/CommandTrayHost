#include <chrono>
#ifdef _DEBUG
void experimental_test()
{
	cron_expr expr;
	ZeroMemory(&expr, sizeof(expr));
	const char* err = NULL;
	const int test_times = 50000;
	{
		auto begin = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < test_times; i++) {
			cron_parse_expr("8 */2 15-16 29 2 *", &expr, &err);
		}
		auto end = std::chrono::high_resolution_clock::now();
		LOGMESSAGE(L"cron_parse_expr %d times, cost %lld miliseconds\n", test_times, std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
	}
	if (err)LOGMESSAGE(L"cron_parse_expr err: %S\n", err);
	else LOGMESSAGE(L"cron_parse_expr ok!\n");
	assert(0 == err);
	time_t cur = time(NULL);
	time_t next = cron_next(&expr, cur);

	LOGMESSAGE(L"%lld -> %lld diff:%lld", next, cur, next - cur);
	double dif = difftime(next, cur);
	char buffer[80];
	tm t1, t2;
	localtime_s(&t1, &cur);
	localtime_s(&t2, &next);
	strftime(buffer, ARRAYSIZE(buffer), "%Y-%m-%d_%H:%M:%S", &t1);
	LOGMESSAGE(L"t1:%S %f\n", buffer, dif);
	strftime(buffer, ARRAYSIZE(buffer), "%Y-%m-%d_%H:%M:%S", &t2);
	LOGMESSAGE(L"t2:%S %f\n", buffer, dif);
	LOGMESSAGE(L"%llu %llu\n", ((time_t)-1), static_cast<time_t>(-1));
	LOGMESSAGE(L"ARRAYSIZE %d\n", ARRAYSIZE("start") - 1);
	LOGMESSAGE(L"%d\n", sizeof(cron_expr));
	{
		LOGMESSAGE(L"cron_expr_begin");
		for (int _i = 0; _i < 26; _i++)
		{
			LOGMESSAGE(L"%d", (reinterpret_cast<const char*>(&expr))[_i]);
		}
		LOGMESSAGE(L"cron_expr_end\n");
		uint8_t* pointer_st = reinterpret_cast<uint8_t*>(&expr);
		std::vector<uint8_t> vector_instance = std::vector<uint8_t>(pointer_st, pointer_st + 26);
		nlohmann::json j = vector_instance;
		LOGMESSAGE(L"0x%x 0x%x 0x%x 0x%x\n",
			pointer_st,
			vector_instance.data(),
			(j.get<std::vector<uint8_t>>().data()),
			(j.get<std::vector<uint8_t>>().data())
		);
		{
			auto begin = std::chrono::high_resolution_clock::now();
			for (int i = 0; i<test_times; i++)
			{
				std::vector<uint8_t> a = j.get<std::vector<uint8_t>>();
				a.data();
			}
			auto end = std::chrono::high_resolution_clock::now();
			LOGMESSAGE(L"j.get<std::vector<uint8_t>>().data() %d times, cost %lld miliseconds\n", test_times, std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
		}
	}
}
#endif