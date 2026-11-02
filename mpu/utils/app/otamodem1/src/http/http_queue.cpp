#include "http_queue.h"
#include "cjson_interface.h"
#include <pthread.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
static std::queue<ComponentData *>  g_Data; 
static std::mutex g_DataMutex;


int32_t PushHttpDataToQue(ComponentData *data)
{ 
	std::lock_guard<std::mutex> lock(g_DataMutex);
	g_Data.push(data);
	return 0;
}

int32_t PopHttpDataFromQue(ComponentData *&data)
{
	std::lock_guard<std::mutex> lock(g_DataMutex); 
	if(g_Data.empty())
	{
		return -1;
	}

	data = g_Data.front(); 
	g_Data.pop();

	return 0;
}
