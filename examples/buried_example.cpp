#include "include/buried.h"
#include <chrono>
#include <thread>
#include <iostream>
int main() {
	Buried* buried = BuriedCreate("C:/Users/asus/Downloads");
	if (buried == nullptr) {
		return -1;
	}

	BuriedConfig config;
	config.host = "127.0.0.1";
	config.port = "5678";
	config.topic = "test_topic";
	config.user_id = "test_user";
	config.app_version = "1.0.0";
	config.app_name = "test_app";
	config.custom_data = "{\"test\":\"test\"}";

	BuriedStart(buried, &config);

	std::thread t1([&]() {
		std::cout << "thread1 start\n";
		for (int i = 0; i < 100; ++i) {
			BuriedReport(buried, "test_title", "test_data", i);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		std::cout << "thread1 finish\n";
		});
	std::thread t2([&]() {
		std::cout << "thread2 start\n";
		for (int i = 0; i < 100; ++i) {
			BuriedReport(buried, "test_2title", "test_2data", i);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		std::cout << "thread2 finish\n";
		});

	t1.join();
	t2.join();
	std::this_thread::sleep_for(std::chrono::hours(1));
	BuriedDestroy(buried);
	return 0;
}