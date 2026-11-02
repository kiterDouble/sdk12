
#include <list>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include "queue.h"
#include "mqtt.h"
#include "mqtt_data_handler.h"


static std::queue<can_queue *>  g_Data; 
static std::mutex g_DataMutex;


static std::queue<mqtt_data *>  g_MqttData; 
static std::mutex g_MqttMutex;

int32_t PushMqttDataToQue(mqtt_data *data)
{
	std::lock_guard<std::mutex> lock(g_MqttMutex); 
	g_MqttData.push(data);

	return 0;
}

int32_t PopMqttDataFromQue(mqtt_data *&data)
{
	
	std::lock_guard<std::mutex> lock(g_MqttMutex); 
	if(g_MqttData.empty())
	{
		//data->payloadlen = 0;
		return -1;
	}

	data = g_MqttData.front(); 
	g_MqttData.pop();

	return 0;
}


int32_t PushCanDataToQue(can_queue *data)
{ 
	std::lock_guard<std::mutex> lock(g_DataMutex);
	g_Data.push(data);

	return 0;
}

int32_t PopCanDataFromQue(can_queue *&data)
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

int32_t GetCanDataFromQueSize()
{
	std::lock_guard<std::mutex> lock(g_DataMutex); 
	return g_Data.size();
}