
#include "mcuinte.h"
#include "mcuinterbase.h"
#include "sr_log.h"



gint ControlRemotePowerDown(void)
{
    LOG_Debug("远程下电");
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    retValue = SendRemoteControlDataToMcu(0x00024009, controlBuff, 4);
    return retValue;
}

gint ControlrestartPowerUp(void)
{
    LOG_Debug("远程重启");
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    controlBuff[0] = 0x03;//controlBuff[0]=2
    
    retValue = SendRemoteControlDataToMcu(0x00024047, controlBuff, 4);
    return retValue;
}

gint ControlRemotePowerUp(void)
{

    LOG_Debug("远程上电");
    gint retValue = RET_ERR; 
    guchar controlBuff[4] = {0};
    controlBuff[0] = 0x03;

    retValue = SendRemoteControlDataToMcu(0x00024008, controlBuff, 4);
    return retValue;
}

gint Set_ecu_adcu(int num)
{
    LOG_Debug("%s num:%02x",__FUNCTION__,num);
    gint retValue = RET_ERR; 
    guchar controlBuff[1] = {0};
    controlBuff[0] = num;
    retValue = SendRemoteControlDataToMcu(0x00033000, controlBuff, 1);
    return retValue;
}
gint Set_da_mode(int num)
{
    LOG_Debug("%s num:%02x",__FUNCTION__,num);
    gint retValue = RET_ERR; 
    guchar controlBuff[1] = {0};
    controlBuff[0] = num;
    
    retValue = SendRemoteControlDataToMcu(0x00033100, controlBuff, 1);
    return retValue;
}
gint Set_upgrade_mode(int cmd)
{
    LOG_Debug("%s cmd:%02x",__FUNCTION__,cmd); //cmd 1 : 主,2 : 副
    gint retValue = RET_ERR; 
    guchar controlBuff[1] = {0};
    controlBuff[0] = cmd;
    
    retValue = SendRemoteControlDataToMcu(0x00033200, controlBuff, 1);
    return retValue;
}