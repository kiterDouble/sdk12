#include "tsp_oder_charging.h"
#include "tsp_parser_diagnosis.h"
#include "mcu_api.h"
#include "queue_msg.h"
gpointer OderChargingHandleTaskEntry(gpointer usrData)
{
    struct timeval newTime;
    ReserverCharging reserverChargingTime;
    guchar controlData[4] = {0};
    guchar start = 0;
    while(1)
    {
        /*控制指令超时判断 有usleep*/
        ControlCmdTimeOut();

        gettimeofday(&newTime, NULL);

        /*获取预约充电时间*/
        GetReserverChargingTime(&reserverChargingTime);
        LOG_Debug("reserverChargingTime.flag %d", reserverChargingTime.flag);

#if 0
        LOG_Debug("%d", reserverChargingTime.version);
        LOG_Debug("%d", reserverChargingTime.appointmentType);
        LOG_Debug("%lld", reserverChargingTime.startTime);
        LOG_Debug("%d", reserverChargingTime.startTimeH);
        LOG_Debug("%d", reserverChargingTime.startTimeM);
        LOG_Debug("%d", reserverChargingTime.endTimeH);
        LOG_Debug("%d", reserverChargingTime.endTimeM);
#endif        
        /*如果开始时间为零或者预约充电没有开启，则不向MCU发送数据*/
        if(0 == reserverChargingTime.flag || 0 == reserverChargingTime.mcuSetState)
        {
            continue;
        }

        if(1 == reserverChargingTime.version)
        {
            LOG_Debug("%lld", reserverChargingTime.startTime);
            if(newTime.tv_sec >= (reserverChargingTime.startTime / 1000))
            {
                start = 1;
            }
        }
        else if((2 == reserverChargingTime.version) && (1 == reserverChargingTime.appointmentType))
        {

            // 单次
            LOG_Debug("%lld", reserverChargingTime.singleStartTime);
            if(newTime.tv_sec < reserverChargingTime.singleStartTime)
            {
                continue;
            }               
            start = 1;
            controlData[0] = 0;
            LOG_Debug("Arrive at the appointment time and allow charging,signle");
        }
        else if((2 == reserverChargingTime.version) && (2 == reserverChargingTime.appointmentType))
        {
            // 每日
            time_t now = time(NULL);
            tm* local_time = localtime(&now);
            int nowMinute =local_time->tm_hour * 60 + local_time->tm_min;
            int reserverChargingTimeStartM = reserverChargingTime.startTimeH * 60 + reserverChargingTime.startTimeM;
            int reserverChargingTimeEndM = reserverChargingTime.endTimeH * 60 + reserverChargingTime.endTimeM;
            LOG_Debug("now:%d start:%d end:%d",nowMinute,reserverChargingTimeStartM,reserverChargingTimeEndM);                
            start = 2;

            // 结束时间小于开始时间，代表跨天情况下
            if(2 == reserverChargingTime.appointmentType && (
                (reserverChargingTimeEndM < reserverChargingTimeStartM && !(reserverChargingTimeEndM <= nowMinute && nowMinute < reserverChargingTimeStartM)) || 
                (reserverChargingTimeEndM > reserverChargingTimeStartM && reserverChargingTimeStartM <= nowMinute && nowMinute < reserverChargingTimeEndM)
                ))
            {
                reserverChargingTime.sendFlag = 2;
                controlData[0] = 0;
            }
            else
            {
                reserverChargingTime.sendFlag = 1;
                controlData[0] = 1;  
            }
        }
        

        if(1 == start)
        {
            // 单次预约充电，发送后清空后预约信息
            start = 0;
            /*判断是否是睡眠状态*/
            if(SGMW_PM_STATE_SLEEP == GetPmState())
            {
                //Wakeup(PM_WAKEUP_SOURCE_TCP);
                PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
            }
            usleep(200000);
            ControlReserverCharging(controlData, 4);
        }
        else if(2 == start && reserverChargingTime.sendFlag != reserverChargingTime.sendFlagTmp)
        {
            // 多次预约充电，发送后不清空预约信息
            start = 0;
            /*判断是否是睡眠状态*/
            if(SGMW_PM_STATE_SLEEP == GetPmState())
            {
                //Wakeup(PM_WAKEUP_SOURCE_TCP);
                PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_DATA);
            }
            usleep(200000);
            reserverChargingTime.sendFlagTmp = reserverChargingTime.sendFlag;
            SetReserverChargingTime(&reserverChargingTime);
            ControlReserverChargingOnlySend(controlData, 4);
        }

    }
}

//获取内存占用率
int get_oemdata_usage() {
    FILE *pipe;
    char line[256];
    int used, total, percent;

    // 执行 df 命令并捕获输出
    pipe = popen("df | grep oemdata", "r");
    if (!pipe) {
        perror("popen");
        return -1;
    }

    // 读取输出的第一行
    if (fgets(line, sizeof(line), pipe)) {
        // 解析出第3个和第2个字段
        if (sscanf(line, "%*s %d %d", &total, &used) == 2) {
            // 计算百分比
            if (total != 0) {
                percent = (used * 100) / total;
                pclose(pipe);
                return percent;
            } else {
                pclose(pipe);
                return -1; // Total space is 0, cannot calculate percentage
            }
        } else {
            pclose(pipe);
            return -1; // Failed to parse the output
        }
    } else {
        pclose(pipe);
        return -1; // No output from command
    }
}
//获取CPU使用率
int get_cpu_usage() {
    FILE *pipe;
    char line[256];
    int user_cpu, sys_cpu, total_cpu;

    // 执行命令并捕获输出
    pipe = popen("top -bn1 | grep 'CPU' | head -n 1", "r");
    if (!pipe) {
        perror("popen");
        return -1;
    }

    // 读取输出的第一行
    if (fgets(line, sizeof(line), pipe)) {
        // 解析出用户态和系统态CPU使用率
        LOG_Debug("%s",line);
        if (sscanf(line, "CPU: %d%% usr %d%% sys", &user_cpu, &sys_cpu) == 2) {
            // 计算总和
            total_cpu = user_cpu + sys_cpu;
            pclose(pipe);
            //LOG_Debug("chen %d %d %d",user_cpu,sys_cpu,total_cpu);
            return total_cpu;
        } else {
            pclose(pipe);
            LOG_Error("Failed to parse the output");
            return -1; // Failed to parse the output
        }
    } else {
        pclose(pipe);
        LOG_Error("No output from command");
        return -1; // No output from command
    }
}


gpointer CPUStatusThread(gpointer pArg)
{
    uint8_t data[CPU_STATUS_LEN] = {0};
    McuMesg_t McuMsgCPUstatus;
    McuMesg_t McuMsgTspStatus;
    char out[128] = {0};
    char tmpTemp[4096] = {0};
    uint32_t cpuTemp = 0; 
    uint8_t data_tsp[TSP_STATUS_LEN] = {0};

    char *endptr;
    long long_value = 0;
    while(1)
    {
        sleep(1);
        if (SGMW_PM_STATE_SLEEP == GetPmState())
        {
            continue;
        }
        //内存占用率
        if(get_oemdata_usage() > -1)
        {
            data[0] = get_oemdata_usage() & 0xFF;
        }
        else
        {
            data[0] = 0;
        }
        //CPU使用率
        if(get_cpu_usage() > -1)
        {
            data[1] = get_cpu_usage() & 0xFF;
        }
        else
        {
            data[1] = 0;
        }
        //模组温度
        GetFileContent(CPU_TEMPRATURE_FILE, tmpTemp, 4096);

        // 使用 strtol 转换字符串
        long_value = strtol(tmpTemp, &endptr, 10);
        cpuTemp = uint32_t(long_value/100);
        //LOG_Debug("chen %d %d",long_value,cpuTemp);
        data[2] = (cpuTemp >> 8) & 0xFF;
        data[3] = cpuTemp & 0xFF;
        McuMsgCPUstatus.type = SYS_QUE_TYPE_MCU_SYNC_CPU_STATUS;
        McuMsgCPUstatus.payloadLen = CPU_STATUS_LEN;
        McuMsgCPUstatus.payload = data;
        LOG_DebugHex(data , 4 ,"CPU_STATUS_LEN");
        if (RET_ERR == McuSrvSendMesg(&McuMsgCPUstatus))
        {
            LOG_Error(" %s:McuSrvSendMesg faild!", __func__);
        }
        GetFileContent(TSP_CONNECT_STATUS_FILE, out, 8);
        if(atoi(out) != 0)
        {
            data_tsp[0] = 0x01;
        }
        else
        {
            data_tsp[0] = 0x00;
        }

        GetFileContent(GB_CONNECT_STATUS_FILE, out, 8);
        if(atoi(out) != 0)
        {
            data_tsp[1] = 0x01;
        }
        else
        {
            data_tsp[1] = 0x00;
        }

        McuMsgTspStatus.type = SYS_QUE_TYPE_MCU_SYNC_TSP_STATUS;
        McuMsgTspStatus.payloadLen = TSP_STATUS_LEN;
        McuMsgTspStatus.payload = data_tsp;
        LOG_DebugHex(data_tsp , 2 ,"TSP_STATUS_LEN");
        if (RET_ERR == McuSrvSendMesg(&McuMsgTspStatus))
        {
            LOG_Error(" %s:McuSrvSendMesg faild!", __func__);
        }
    }
}

