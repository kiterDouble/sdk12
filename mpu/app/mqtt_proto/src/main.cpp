#include "mqtt_log_sync.h"
#include <iostream>
#include <thread>

int main() {
    std::cout << "Main: Starting SyncLogFileThread" << std::endl;
    std::thread t(SyncLogFileThread);
    t.join();
    std::cout << "Main: SyncLogFileThread joined" << std::endl;
    return 0;
}
