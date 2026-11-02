
#ifndef HTTP_QUEUE_H_
#define HTTP_QUEUE_H_

#include <iostream>
#include <queue>
#include <string>
#include <cstring>
#include "cjson_interface.h"



int32_t PushHttpDataToQue(ComponentData *data);
int32_t PopHttpDataFromQue(ComponentData *&data);



#endif
