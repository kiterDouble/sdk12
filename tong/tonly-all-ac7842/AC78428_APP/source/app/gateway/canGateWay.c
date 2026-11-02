#include "FreeRTOS.h"
#include "task.h"
#include "remoteDiagnosticSdk.h"
#include "canGateWay.h"
#include "canHal.h"

#define CAN_TX_CAN1_FLAG         (0x01<<0)
#define CAN_TX_CAN2_FLAG         (0x01<<1)
#define CAN_TX_CAN3_FLAG         (0x01<<2)
#define CAN_TX_CAN4_FLAG         (0x01<<3)
#define CAN_TX_CAN5_FLAG         (0x01<<4)
#define CAN_TX_CAN6_FLAG         (0x01<<5)
#define CAN_TX_CAN_CHANNEL_INVALID      0x00

#define CAN_DIAG_REPORT_NONE            0x00
//#define CAN_DIAG_REPORT_CAN_P           0x01
#define CAN_DIAG_REPORT_CAN_B           0x02
#define CAN_DIAG_REPORT_CAN_E           0x03
#define CAN_DIAG_REPORT_CAN_C           0x04
#define CAN_DIAG_REPORT_CAN_I           0x05
#define CAN_DIAG_REPORT_CAN_A           0x06

typedef struct 
{
	uint32_t canId;	
    uint8_t dlc;
	uint32_t txChannel;//bit0:channel0,bit1:channel1	
}CanRoutTable_t;


#define CAN_TX_CAN_CHANNEL_A      (0x01<<TBOX_CAN_CHANNEL_A)
#define CAN_TX_CAN_CHANNEL_B      (0x01<<TBOX_CAN_CHANNEL_B)
#define CAN_TX_CAN_CHANNEL_E      (0x01<<TBOX_CAN_CHANNEL_E)              
#define CAN_TX_CAN_CHANNEL_C      (0x01<<TBOX_CAN_CHANNEL_C)
#define CAN_TX_CAN_CHANNEL_D      (0x01<<TBOX_CAN_CHANNEL_D)
#define CAN_TX_CAN_CHANNEL_I      (0x01<<TBOX_CAN_CHANNEL_I)

#define CAN_ROUTE_TABLE_LIST(a)    g_can##a##_rout_table_list

#if(TBOX_CAN_CHANNEL_A==TBOX_CAN_CHANNEL_1)
#define CAN_ROUTE_TABLE_LIST_CH_A    CAN_ROUTE_TABLE_LIST(1)
#define CAN_DIAG_REPORT_CAN_1        CAN_DIAG_REPORT_CAN_A
#elif(TBOX_CAN_CHANNEL_A==TBOX_CAN_CHANNEL_2)
#define CAN_ROUTE_TABLE_LIST_CH_A    CAN_ROUTE_TABLE_LIST(2)
#define CAN_DIAG_REPORT_CAN_2        CAN_DIAG_REPORT_CAN_A
#elif(TBOX_CAN_CHANNEL_A==TBOX_CAN_CHANNEL_3)
#define CAN_ROUTE_TABLE_LIST_CH_A    CAN_ROUTE_TABLE_LIST(3)
#define CAN_DIAG_REPORT_CAN_3        CAN_DIAG_REPORT_CAN_A
#elif(TBOX_CAN_CHANNEL_A==TBOX_CAN_CHANNEL_4)
#define CAN_ROUTE_TABLE_LIST_CH_A    CAN_ROUTE_TABLE_LIST(4)
#define CAN_DIAG_REPORT_CAN_4        CAN_DIAG_REPORT_CAN_A
#elif(TBOX_CAN_CHANNEL_A==TBOX_CAN_CHANNEL_5)
#define CAN_ROUTE_TABLE_LIST_CH_A    CAN_ROUTE_TABLE_LIST(5)
#define CAN_DIAG_REPORT_CAN_5        CAN_DIAG_REPORT_CAN_A
#elif(TBOX_CAN_CHANNEL_A==TBOX_CAN_CHANNEL_6)
#define CAN_ROUTE_TABLE_LIST_CH_A    CAN_ROUTE_TABLE_LIST(6)
#define CAN_DIAG_REPORT_CAN_6        CAN_DIAG_REPORT_CAN_A
#endif

#if(TBOX_CAN_CHANNEL_B==TBOX_CAN_CHANNEL_1)
#define CAN_ROUTE_TABLE_LIST_CH_B    CAN_ROUTE_TABLE_LIST(1)
#define CAN_DIAG_REPORT_CAN_1        CAN_DIAG_REPORT_CAN_B
#elif(TBOX_CAN_CHANNEL_B==TBOX_CAN_CHANNEL_2)
#define CAN_ROUTE_TABLE_LIST_CH_B    CAN_ROUTE_TABLE_LIST(2)
#define CAN_DIAG_REPORT_CAN_2        CAN_DIAG_REPORT_CAN_B
#elif(TBOX_CAN_CHANNEL_B==TBOX_CAN_CHANNEL_3)
#define CAN_ROUTE_TABLE_LIST_CH_B    CAN_ROUTE_TABLE_LIST(3)
#define CAN_DIAG_REPORT_CAN_3        CAN_DIAG_REPORT_CAN_B
#elif(TBOX_CAN_CHANNEL_B==TBOX_CAN_CHANNEL_4)
#define CAN_ROUTE_TABLE_LIST_CH_B    CAN_ROUTE_TABLE_LIST(4)
#define CAN_DIAG_REPORT_CAN_4        CAN_DIAG_REPORT_CAN_B
#elif(TBOX_CAN_CHANNEL_B==TBOX_CAN_CHANNEL_5)
#define CAN_ROUTE_TABLE_LIST_CH_B    CAN_ROUTE_TABLE_LIST(5)
#define CAN_DIAG_REPORT_CAN_5        CAN_DIAG_REPORT_CAN_B
#elif(TBOX_CAN_CHANNEL_B==TBOX_CAN_CHANNEL_6)
#define CAN_ROUTE_TABLE_LIST_CH_B    CAN_ROUTE_TABLE_LIST(6)
#define CAN_DIAG_REPORT_CAN_6        CAN_DIAG_REPORT_CAN_B
#endif

#if(TBOX_CAN_CHANNEL_C==TBOX_CAN_CHANNEL_1)
#define CAN_ROUTE_TABLE_LIST_CH_C    CAN_ROUTE_TABLE_LIST(1)
#define CAN_DIAG_REPORT_CAN_1        CAN_DIAG_REPORT_CAN_C
#elif(TBOX_CAN_CHANNEL_C==TBOX_CAN_CHANNEL_2)
#define CAN_ROUTE_TABLE_LIST_CH_C    CAN_ROUTE_TABLE_LIST(2)
#define CAN_DIAG_REPORT_CAN_2        CAN_DIAG_REPORT_CAN_C
#elif(TBOX_CAN_CHANNEL_C==TBOX_CAN_CHANNEL_3)
#define CAN_ROUTE_TABLE_LIST_CH_C    CAN_ROUTE_TABLE_LIST(3)
#define CAN_DIAG_REPORT_CAN_3        CAN_DIAG_REPORT_CAN_C
#elif(TBOX_CAN_CHANNEL_C==TBOX_CAN_CHANNEL_4)
#define CAN_ROUTE_TABLE_LIST_CH_C    CAN_ROUTE_TABLE_LIST(4)
#define CAN_DIAG_REPORT_CAN_4        CAN_DIAG_REPORT_CAN_C
#elif(TBOX_CAN_CHANNEL_C==TBOX_CAN_CHANNEL_5)
#define CAN_ROUTE_TABLE_LIST_CH_C    CAN_ROUTE_TABLE_LIST(5)
#define CAN_DIAG_REPORT_CAN_5        CAN_DIAG_REPORT_CAN_C
#elif(TBOX_CAN_CHANNEL_C==TBOX_CAN_CHANNEL_6)
#define CAN_ROUTE_TABLE_LIST_CH_C    CAN_ROUTE_TABLE_LIST(6)
#define CAN_DIAG_REPORT_CAN_6        CAN_DIAG_REPORT_CAN_C
#endif

#if(TBOX_CAN_CHANNEL_D==TBOX_CAN_CHANNEL_1)
#define CAN_ROUTE_TABLE_LIST_CH_D    CAN_ROUTE_TABLE_LIST(1)
#define CAN_DIAG_REPORT_CAN_1        CAN_DIAG_REPORT_CAN_D
#elif(TBOX_CAN_CHANNEL_D==TBOX_CAN_CHANNEL_2)
#define CAN_ROUTE_TABLE_LIST_CH_D    CAN_ROUTE_TABLE_LIST(2)
#define CAN_DIAG_REPORT_CAN_2        CAN_DIAG_REPORT_CAN_D
#elif(TBOX_CAN_CHANNEL_D==TBOX_CAN_CHANNEL_3)
#define CAN_ROUTE_TABLE_LIST_CH_D    CAN_ROUTE_TABLE_LIST(3)
#define CAN_DIAG_REPORT_CAN_3        CAN_DIAG_REPORT_CAN_D
#elif(TBOX_CAN_CHANNEL_D==TBOX_CAN_CHANNEL_4)
#define CAN_ROUTE_TABLE_LIST_CH_D    CAN_ROUTE_TABLE_LIST(4)
#define CAN_DIAG_REPORT_CAN_4        CAN_DIAG_REPORT_CAN_D
#elif(TBOX_CAN_CHANNEL_D==TBOX_CAN_CHANNEL_5)
#define CAN_ROUTE_TABLE_LIST_CH_D    CAN_ROUTE_TABLE_LIST(5)
#define CAN_DIAG_REPORT_CAN_5        CAN_DIAG_REPORT_CAN_D
#elif(TBOX_CAN_CHANNEL_D==TBOX_CAN_CHANNEL_6)
#define CAN_ROUTE_TABLE_LIST_CH_D    CAN_ROUTE_TABLE_LIST(6)
#define CAN_DIAG_REPORT_CAN_6        CAN_DIAG_REPORT_CAN_D
#endif

#if(TBOX_CAN_CHANNEL_E==TBOX_CAN_CHANNEL_1)
#define CAN_ROUTE_TABLE_LIST_CH_E    CAN_ROUTE_TABLE_LIST(1)
#define CAN_DIAG_REPORT_CAN_1        CAN_DIAG_REPORT_CAN_E
#elif(TBOX_CAN_CHANNEL_E==TBOX_CAN_CHANNEL_2)
#define CAN_ROUTE_TABLE_LIST_CH_E    CAN_ROUTE_TABLE_LIST(2)
#define CAN_DIAG_REPORT_CAN_2        CAN_DIAG_REPORT_CAN_E
#elif(TBOX_CAN_CHANNEL_E==TBOX_CAN_CHANNEL_3)
#define CAN_ROUTE_TABLE_LIST_CH_E    CAN_ROUTE_TABLE_LIST(3)
#define CAN_DIAG_REPORT_CAN_3        CAN_DIAG_REPORT_CAN_E
#elif(TBOX_CAN_CHANNEL_E==TBOX_CAN_CHANNEL_4)
#define CAN_ROUTE_TABLE_LIST_CH_E    CAN_ROUTE_TABLE_LIST(4)
#define CAN_DIAG_REPORT_CAN_4        CAN_DIAG_REPORT_CAN_E
#elif(TBOX_CAN_CHANNEL_E==TBOX_CAN_CHANNEL_5)
#define CAN_ROUTE_TABLE_LIST_CH_E    CAN_ROUTE_TABLE_LIST(5)
#define CAN_DIAG_REPORT_CAN_5        CAN_DIAG_REPORT_CAN_E
#elif(TBOX_CAN_CHANNEL_E==TBOX_CAN_CHANNEL_6)
#define CAN_ROUTE_TABLE_LIST_CH_E    CAN_ROUTE_TABLE_LIST(6)
#define CAN_DIAG_REPORT_CAN_6        CAN_DIAG_REPORT_CAN_E
#endif

#if(TBOX_CAN_CHANNEL_I==TBOX_CAN_CHANNEL_1)
#define CAN_ROUTE_TABLE_LIST_CH_I    CAN_ROUTE_TABLE_LIST(1)
#define CAN_DIAG_REPORT_CAN_1        CAN_DIAG_REPORT_CAN_I
#elif(TBOX_CAN_CHANNEL_I==TBOX_CAN_CHANNEL_2)
#define CAN_ROUTE_TABLE_LIST_CH_I    CAN_ROUTE_TABLE_LIST(2)
#define CAN_DIAG_REPORT_CAN_2        CAN_DIAG_REPORT_CAN_I
#elif(TBOX_CAN_CHANNEL_I==TBOX_CAN_CHANNEL_3)
#define CAN_ROUTE_TABLE_LIST_CH_I    CAN_ROUTE_TABLE_LIST(3)
#define CAN_DIAG_REPORT_CAN_3        CAN_DIAG_REPORT_CAN_I
#elif(TBOX_CAN_CHANNEL_I==TBOX_CAN_CHANNEL_4)
#define CAN_ROUTE_TABLE_LIST_CH_I    CAN_ROUTE_TABLE_LIST(4)
#define CAN_DIAG_REPORT_CAN_4        CAN_DIAG_REPORT_CAN_I
#elif(TBOX_CAN_CHANNEL_I==TBOX_CAN_CHANNEL_5)
#define CAN_ROUTE_TABLE_LIST_CH_I    CAN_ROUTE_TABLE_LIST(5)
#define CAN_DIAG_REPORT_CAN_5        CAN_DIAG_REPORT_CAN_I
#elif(TBOX_CAN_CHANNEL_I==TBOX_CAN_CHANNEL_6)
#define CAN_ROUTE_TABLE_LIST_CH_I    CAN_ROUTE_TABLE_LIST(6)
#define CAN_DIAG_REPORT_CAN_6        CAN_DIAG_REPORT_CAN_I
#endif

/*******************************************************

******************************************************/
//DEBUG
static const CanRoutTable_t CAN_ROUTE_TABLE_LIST_CH_D[] = 
{
    {0x702,8,CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_A|CAN_TX_CAN_CHANNEL_I},
    {0x7DF,8,CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_A},
    //
    {0x770,8,CAN_TX_CAN_CHANNEL_E},
    {0x772,8,CAN_TX_CAN_CHANNEL_E},
    {0x771,8,CAN_TX_CAN_CHANNEL_E},
    {0x742,8,CAN_TX_CAN_CHANNEL_E},
    {0x7E3,8,CAN_TX_CAN_CHANNEL_E},
    {0x90000766,8,CAN_TX_CAN_CHANNEL_E}, //XCP
    
    //
    {0x745,8,CAN_TX_CAN_CHANNEL_B},
    {0x751,8,CAN_TX_CAN_CHANNEL_B},
    {0x723,8,CAN_TX_CAN_CHANNEL_B},
    {0x761,8,CAN_TX_CAN_CHANNEL_B},
    {0x755,8,CAN_TX_CAN_CHANNEL_B},
    {0x760,8,CAN_TX_CAN_CHANNEL_B},
    {0x721,8,CAN_TX_CAN_CHANNEL_B},
    {0x744,8,CAN_TX_CAN_CHANNEL_B},
    //
    {0x750,8,CAN_TX_CAN_CHANNEL_C},
    {0x753,8,CAN_TX_CAN_CHANNEL_C},
    {0x730,8,CAN_TX_CAN_CHANNEL_C},
    {0x725,8,CAN_TX_CAN_CHANNEL_C},
    {0x720,8,CAN_TX_CAN_CHANNEL_C},
    
    {0x735,8,CAN_TX_CAN_CHANNEL_A},
    {0x734,8,CAN_TX_CAN_CHANNEL_A},
    {0x726,8,CAN_TX_CAN_CHANNEL_A},
};



//ECAN
static const CanRoutTable_t CAN_ROUTE_TABLE_LIST_CH_E[] = 
{
    {0x706,8,CAN_TX_CAN_CHANNEL_D|CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_A|CAN_TX_CAN_CHANNEL_I},
    {0x287,8,CAN_TX_CAN_CHANNEL_C},
    {0x155,8,CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_B},     
    {0x4C5,8,CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_B},
    {0x36A,8,CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_B},
    {0x4C6,8,CAN_TX_CAN_CHANNEL_C},
    {0x16C,8,CAN_TX_CAN_CHANNEL_C},
    {0x486,8,CAN_TX_CAN_CHANNEL_C},
    {0x165,8,CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_A},     
    //
    {0x3C1,8,CAN_TX_CAN_CHANNEL_B},
    {0x4C7,8,CAN_TX_CAN_CHANNEL_B},
    {0x36D,8,CAN_TX_CAN_CHANNEL_B},
    {0x36E,8,CAN_TX_CAN_CHANNEL_B},
    {0x271,8,CAN_TX_CAN_CHANNEL_B},
    {0x297,8,CAN_TX_CAN_CHANNEL_B},
    {0x4F6,8,CAN_TX_CAN_CHANNEL_B},
    
    {0x778,8,CAN_TX_CAN_CHANNEL_D},
    {0x77A,8,CAN_TX_CAN_CHANNEL_D},
    {0x779,8,CAN_TX_CAN_CHANNEL_D},
    {0x74A,8,CAN_TX_CAN_CHANNEL_D},
    {0x7EB,8,CAN_TX_CAN_CHANNEL_D},
    {0x90000777,8,CAN_TX_CAN_CHANNEL_D},//XCP resp
     
};

//BCAN
static const CanRoutTable_t CAN_ROUTE_TABLE_LIST_CH_B[] = 
{
    {0x701,8,CAN_TX_CAN_CHANNEL_D|CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_A|CAN_TX_CAN_CHANNEL_I},
    {0x120,5,CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_E},
    {0x565,8,CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_A},     
    {0x12A,8,CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_E},
    {0x122,4,CAN_TX_CAN_CHANNEL_C},
    // 
    {0x3C9,8,CAN_TX_CAN_CHANNEL_E},
    {0x57E,8,CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_A},
    {0x279,8,CAN_TX_CAN_CHANNEL_E},
    {0x335,8,CAN_TX_CAN_CHANNEL_E},
    
    {0x332,8,CAN_TX_CAN_CHANNEL_A},
    {0x125,8,CAN_TX_CAN_CHANNEL_A},
    {0x1F1,8,CAN_TX_CAN_CHANNEL_A},
    {0x140,4,CAN_TX_CAN_CHANNEL_A},
    {0x1E1,8,CAN_TX_CAN_CHANNEL_A},
    {0x333,8,CAN_TX_CAN_CHANNEL_A},
    {0x336,8,CAN_TX_CAN_CHANNEL_A},
    {0x337,8,CAN_TX_CAN_CHANNEL_A},
    {0x338,8,CAN_TX_CAN_CHANNEL_A},
    {0x402,8,CAN_TX_CAN_CHANNEL_A},
    {0x403,8,CAN_TX_CAN_CHANNEL_A},
    {0x404,8,CAN_TX_CAN_CHANNEL_A},

    {0x43B,8,CAN_TX_CAN_CHANNEL_A},
    {0x343,8,CAN_TX_CAN_CHANNEL_A},
    {0x344,8,CAN_TX_CAN_CHANNEL_A},
    {0x345,8,CAN_TX_CAN_CHANNEL_A},
    //diag
    {0x74D,8,CAN_TX_CAN_CHANNEL_D},
    {0x759,8,CAN_TX_CAN_CHANNEL_D},
    {0x72B,8,CAN_TX_CAN_CHANNEL_D},
    {0x75D,8,CAN_TX_CAN_CHANNEL_D},
    {0x768,8,CAN_TX_CAN_CHANNEL_D},
    {0x769,8,CAN_TX_CAN_CHANNEL_D},
    {0x729,8,CAN_TX_CAN_CHANNEL_D},
    {0x74C,8,CAN_TX_CAN_CHANNEL_D},
};

//CCAN
static const CanRoutTable_t CAN_ROUTE_TABLE_LIST_CH_C[] = 
{     
    {0x704,8,CAN_TX_CAN_CHANNEL_D|CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_A|CAN_TX_CAN_CHANNEL_I},
    {0x1E9,8,CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_E},
    {0x17D,8,CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_E},
    {0x34C,6,CAN_TX_CAN_CHANNEL_B},
    {0x34E,6,CAN_TX_CAN_CHANNEL_B},
    {0x230,8,CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_E},
    {0x132,8,CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_E},
    {0x1E5,8,CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_E},
    {0x141,8,CAN_TX_CAN_CHANNEL_B},
    {0x1D3,8,CAN_TX_CAN_CHANNEL_B},
    {0x0E1,8,CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_E},
    //
    {0x1C6,8,CAN_TX_CAN_CHANNEL_E},
    {0x348,6,CAN_TX_CAN_CHANNEL_E},
    {0x34A,6,CAN_TX_CAN_CHANNEL_E},
    {0x128,8,CAN_TX_CAN_CHANNEL_E},
    {0x156,8,CAN_TX_CAN_CHANNEL_E},
    
    {0x758,8,CAN_TX_CAN_CHANNEL_D},
    {0x738,8,CAN_TX_CAN_CHANNEL_D},
    {0x728,8,CAN_TX_CAN_CHANNEL_D},
    {0x75B,8,CAN_TX_CAN_CHANNEL_D},
};

static const CanRoutTable_t CAN_ROUTE_TABLE_LIST_CH_A[] = 
{
    {0x703,8,CAN_TX_CAN_CHANNEL_D|CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_I},
    {0x329,8,CAN_TX_CAN_CHANNEL_B},
    {0x32A,8,CAN_TX_CAN_CHANNEL_B},
    {0x32B,8,CAN_TX_CAN_CHANNEL_B},
    {0x32C,8,CAN_TX_CAN_CHANNEL_B},
    {0x32D,8,CAN_TX_CAN_CHANNEL_B},
    {0x32E,8,CAN_TX_CAN_CHANNEL_B},
    {0x32F,8,CAN_TX_CAN_CHANNEL_B},
    {0x370,8,CAN_TX_CAN_CHANNEL_B},
    {0x371,8,CAN_TX_CAN_CHANNEL_B},
    {0x372,8,CAN_TX_CAN_CHANNEL_B},
    {0x373,8,CAN_TX_CAN_CHANNEL_B},
    {0x374,8,CAN_TX_CAN_CHANNEL_B},
    {0x375,8,CAN_TX_CAN_CHANNEL_B},
    {0x376,8,CAN_TX_CAN_CHANNEL_B},
    {0x377,8,CAN_TX_CAN_CHANNEL_B},
    {0x378,8,CAN_TX_CAN_CHANNEL_B},
    {0x379,8,CAN_TX_CAN_CHANNEL_B},
    {0x37A,8,CAN_TX_CAN_CHANNEL_B},
    {0x37B,8,CAN_TX_CAN_CHANNEL_B},
    {0x37C,8,CAN_TX_CAN_CHANNEL_B},
    {0x37D,8,CAN_TX_CAN_CHANNEL_B},
    {0x37E,8,CAN_TX_CAN_CHANNEL_B},
    {0x37F,8,CAN_TX_CAN_CHANNEL_B},
    {0x35B,8,CAN_TX_CAN_CHANNEL_B},
    {0x35C,8,CAN_TX_CAN_CHANNEL_B},
    {0x35D,8,CAN_TX_CAN_CHANNEL_B},
    {0x35E,8,CAN_TX_CAN_CHANNEL_B},
    {0x390,8,CAN_TX_CAN_CHANNEL_B},
    {0x391,8,CAN_TX_CAN_CHANNEL_B},
    {0x392,8,CAN_TX_CAN_CHANNEL_B},
    {0x393,8,CAN_TX_CAN_CHANNEL_B},
    {0x394,8,CAN_TX_CAN_CHANNEL_B},
    {0x395,8,CAN_TX_CAN_CHANNEL_B},
    {0x396,8,CAN_TX_CAN_CHANNEL_B},
    {0x35F,8,CAN_TX_CAN_CHANNEL_B},
    //
    {0x1D2,8,CAN_TX_CAN_CHANNEL_E},
    
    {0x73D,8,CAN_TX_CAN_CHANNEL_D},
    {0x73C,8,CAN_TX_CAN_CHANNEL_D},
    {0x72E,8,CAN_TX_CAN_CHANNEL_D},
};

static const CanRoutTable_t CAN_ROUTE_TABLE_LIST_CH_I[] = 
{
    {0x705,8,CAN_TX_CAN_CHANNEL_D|CAN_TX_CAN_CHANNEL_B|CAN_TX_CAN_CHANNEL_C|CAN_TX_CAN_CHANNEL_E|CAN_TX_CAN_CHANNEL_A},
    {0x125,8,CAN_TX_CAN_CHANNEL_INVALID},
};

/*********************************************

********************************************/
  


static uint8_t g_diagCanReportControl = CAN_DIAG_REPORT_NONE;//CAN_DIAG_REPORT_CAN_B  CAN_DIAG_REPORT_NONE
static uint16_t g_dcanDiagMsgRxFlag = 0;

void CanGateWayCan1Rx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
  
  uint32_t num,i;
  const CanRoutTable_t *pRouteTable;
  
  num = sizeof(CAN_ROUTE_TABLE_LIST(1))/sizeof(CAN_ROUTE_TABLE_LIST(1)[0]);  
  pRouteTable = CAN_ROUTE_TABLE_LIST(1);
  

  for(i=0;i<num;i++)
  {
    if((pRouteTable[i].canId==canId)&&(pRouteTable[i].dlc==dlc))
    {
      //if(pRouteTable[i].txChannel&CAN_TX_CAN1_FLAG)
      //{
      //  CanHalTransmitFromIsr((0x0000),canId,data,dlc,0);
      //}
      if(pRouteTable[i].txChannel&CAN_TX_CAN2_FLAG)
      {
        CanHalTransmitFromIsr((0x0001),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN3_FLAG)
      {
        CanHalTransmitFromIsr((0x0002),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN4_FLAG)
      {
        CanHalTransmitFromIsr((0x0003),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN5_FLAG)
      {
        CanHalTransmitFromIsr((0x0004),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN6_FLAG)
      {
        CanHalTransmitFromIsr((0x0005),canId,data,dlc,0);
      }
    }
  }
//#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_1)
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_1) && (canId < 0x700))
  {
    CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
  }
//#else
//  g_dcanDiagMsgRxFlag = 1000;
//  RemoteDiagnosticDisableFlagSet();
//#endif
  
}

void CanGateWayCan2Rx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
  uint32_t num,i;
  const CanRoutTable_t *pRouteTable;  
  
  num = sizeof(g_can2_rout_table_list)/sizeof(g_can2_rout_table_list[0]);  
  pRouteTable = g_can2_rout_table_list;
  
  for(i=0;i<num;i++)
  {
    if((pRouteTable[i].canId==canId)&&(pRouteTable[i].dlc==dlc))
    {
      if(pRouteTable[i].txChannel&CAN_TX_CAN1_FLAG)
      {
        CanHalTransmitFromIsr((0x0000),canId,data,dlc,0);
      }
      //if(pRouteTable[i].txChannel&CAN_TX_CAN2_FLAG)
      //{
      //  CanHalTransmitFromIsr((0x0001),canId,data,dlc,0);
      //}
      if(pRouteTable[i].txChannel&CAN_TX_CAN3_FLAG)
      {
        CanHalTransmitFromIsr((0x0002),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN4_FLAG)
      {
        CanHalTransmitFromIsr((0x0003),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN5_FLAG)
      {
        CanHalTransmitFromIsr((0x0004),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN6_FLAG)
      {
        CanHalTransmitFromIsr((0x0005),canId,data,dlc,0);
      }
    }
  }
//#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_2)
//  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_2) && (canId < 0x700))
//  {
//    CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
//  }
//#else
  g_dcanDiagMsgRxFlag = 1000;
  RemoteDiagnosticSdkShortDisable();  
//#endif
}

void CanGateWayCan3Rx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
  uint32_t num,i;
  const CanRoutTable_t *pRouteTable;
  
  num = sizeof(g_can3_rout_table_list)/sizeof(g_can3_rout_table_list[0]);  
  pRouteTable = g_can3_rout_table_list;
  
  
  for(i=0;i<num;i++)
  {
    if((pRouteTable[i].canId==canId)&&(pRouteTable[i].dlc==dlc))
    {
      if(pRouteTable[i].txChannel&CAN_TX_CAN1_FLAG)
      {
        CanHalTransmitFromIsr((0x0000),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN2_FLAG)
      {
        CanHalTransmitFromIsr((0x0001),canId,data,dlc,0);
      }
      //if(pRouteTable[i].txChannel&CAN_TX_CAN3_FLAG)
      //{
      //  CanHalTransmitFromIsr((0x0002),canId,data,dlc,0);
      //}
      if(pRouteTable[i].txChannel&CAN_TX_CAN4_FLAG)
      {
        CanHalTransmitFromIsr((0x0003),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN5_FLAG)
      {
        CanHalTransmitFromIsr((0x0004),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN6_FLAG)
      {
        CanHalTransmitFromIsr((0x0005),canId,data,dlc,0);
      }
    }
  } 
//#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_3)
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_3) && (canId < 0x700))
  {
    CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
  }
//#else
//  g_dcanDiagMsgRxFlag = 1000;
//  RemoteDiagnosticSdkShortDisable();
//#endif
}

void CanGateWayCan4Rx(uint32_t canId,uint8_t *data,uint8_t dlc)
{   
  uint32_t num,i;
  const CanRoutTable_t *pRouteTable;
  
  num = sizeof(g_can4_rout_table_list)/sizeof(g_can4_rout_table_list[0]);  
  pRouteTable = g_can4_rout_table_list;
  
  for(i=0;i<num;i++)
  {
    if((pRouteTable[i].canId==canId)&&(pRouteTable[i].dlc==dlc))
    {
      if(pRouteTable[i].txChannel&CAN_TX_CAN1_FLAG)
      {
        CanHalTransmitFromIsr((0x0000),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN2_FLAG)
      {
        CanHalTransmitFromIsr((0x0001),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN3_FLAG)
      {
        CanHalTransmitFromIsr((0x0002),canId,data,dlc,0);
      }
      //if(pRouteTable[i].txChannel&CAN_TX_CAN4_FLAG)
      //{
      //  CanHalTransmitFromIsr((0x0003),canId,data,dlc,0);
      //}
      if(pRouteTable[i].txChannel&CAN_TX_CAN5_FLAG)
      {
        CanHalTransmitFromIsr((0x0004),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN6_FLAG)
      {
        CanHalTransmitFromIsr((0x0005),canId,data,dlc,0);
      }
    }
  }
//#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_4)
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_4) && (canId < 0x700))
  {
    CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
  }
//#else
//  g_dcanDiagMsgRxFlag = 1000;
//  RemoteDiagnosticSdkShortDisable();
//#endif
}

void CanGateWayCan5Rx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
  uint32_t num,i;
  const CanRoutTable_t *pRouteTable;
  
  num = sizeof(g_can5_rout_table_list)/sizeof(g_can5_rout_table_list[0]);  
  pRouteTable = g_can5_rout_table_list;
  
  for(i=0;i<num;i++)
  {
    if((pRouteTable[i].canId==canId)&&(pRouteTable[i].dlc==dlc))
    {
      if(pRouteTable[i].txChannel&CAN_TX_CAN1_FLAG)
      {
        CanHalTransmitFromIsr((0x0000),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN2_FLAG)
      {
        CanHalTransmitFromIsr((0x0001),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN3_FLAG)
      {
        CanHalTransmitFromIsr((0x0002),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN4_FLAG)
      {
        CanHalTransmitFromIsr((0x0003),canId,data,dlc,0);
      }
      //if(pRouteTable[i].txChannel&CAN_TX_CAN5_FLAG)
      //{
      //  CanHalTransmitFromIsr((0x0004),canId,data,dlc,0);
      //}
      if(pRouteTable[i].txChannel&CAN_TX_CAN6_FLAG)
      {
        CanHalTransmitFromIsr((0x0005),canId,data,dlc,0);
      }
    }
  } 
//#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_5)
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_5) && (canId < 0x700))
  {
    CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
  } 
//#else
//  g_dcanDiagMsgRxFlag = 1000;
//  RemoteDiagnosticSdkShortDisable();
//#endif
}

void CanGateWayCan6Rx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
  uint32_t num,i;
  const CanRoutTable_t *pRouteTable;
  
  num = sizeof(g_can6_rout_table_list)/sizeof(g_can6_rout_table_list[0]);  
  pRouteTable = g_can6_rout_table_list;  

  for(i=0;i<num;i++)
  {
    if((pRouteTable[i].canId==canId)&&(pRouteTable[i].dlc==dlc))
    {
      if(pRouteTable[i].txChannel&CAN_TX_CAN1_FLAG)
      {
        CanHalTransmitFromIsr((0x0000),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN2_FLAG)
      {
        CanHalTransmitFromIsr((0x0001),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN3_FLAG)
      {
        CanHalTransmitFromIsr((0x0002),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN4_FLAG)
      {
        CanHalTransmitFromIsr((0x0003),canId,data,dlc,0);
      }
      if(pRouteTable[i].txChannel&CAN_TX_CAN5_FLAG)
      {
        CanHalTransmitFromIsr((0x0004),canId,data,dlc,0);
      }
      //if(pRouteTable[i].txChannel&CAN_TX_CAN6_FLAG)
      //{
      //  CanHalTransmitFromIsr((0x0005),canId,data,dlc,0);
      //}
    }
  }
//#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_6)
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_6) && (canId < 0x700))
  {
    CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
  }
//#else
//  g_dcanDiagMsgRxFlag = 1000;
//  RemoteDiagnosticSdkShortDisable();
//#endif
}

void CanGateWayCan1Tx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_1)   
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_1))
  {
    if(g_dcanDiagMsgRxFlag)
    {
      if(canId<0x700)
      {
        CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
      }
      /*else
      {
        m_dcanDiagMsgRxFlag = 0;
      }*/
    }
    else
    {
      CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
    }    
  }  
#endif
}

void CanGateWayCan2Tx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_2)   
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_2))
  {
    if(g_dcanDiagMsgRxFlag)
    {
      if(canId<0x700)
      {
        CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
      }
      /*else
      {
        m_dcanDiagMsgRxFlag = 0;
      }*/
    }
    else
    {
      CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
    } 
  }  
#endif  
}

void CanGateWayCan3Tx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_3)   
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_3))
  {
    if(g_dcanDiagMsgRxFlag)
    {
      if(canId<0x700)
      {
        CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
      }
      /*else
      {
        m_dcanDiagMsgRxFlag = 0;
      }*/
    }
    else
    {
      CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
    } 
  }  
#endif  
}

void CanGateWayCan4Tx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_4)   
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_4))
  {
    if(g_dcanDiagMsgRxFlag)
    {
      if(canId<0x700)
      {
        CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
      }
      /*else
      {
        m_dcanDiagMsgRxFlag = 0;
      }*/
    }
    else
    {
      CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
    } 
  }  
#endif  
}
void CanGateWayCan5Tx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_5)   
  if((g_diagCanReportControl==CAN_DIAG_REPORT_CAN_5))
  {
    if(g_dcanDiagMsgRxFlag)
    {
      if(canId<0x700)
      {
        CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
      }
      /*else
      {
        m_dcanDiagMsgRxFlag = 0;
      }*/
    }
    else
    {
      CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
    } 
  }  
#endif  
}

void CanGateWayCan6Tx(uint32_t canId,uint8_t *data,uint8_t dlc)
{
#if(TBOX_CAN_CHANNEL_D!=TBOX_CAN_CHANNEL_6)   
  if(g_diagCanReportControl==CAN_DIAG_REPORT_CAN_6)
  {
    if(g_dcanDiagMsgRxFlag)
    {
      if(canId<0x700)
      {
        CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
      }
      /*else
      {
        m_dcanDiagMsgRxFlag = 0;
      }*/
    }
    else
    {
      CanHalTransmitFromIsr((TBOX_CAN_CHANNEL_D),canId,data,dlc,0);
    } 
  }
#endif 
}

int16_t CanGateWaySetCanReportControl(uint8_t value)
{
  g_diagCanReportControl = value;
  //m_diagCanReportControl = 2;
  return 0;
}

uint8_t CanGateWayGetCanReportControl(void)
{
  return g_diagCanReportControl;
}

void CanGateWayCycleProcess(uint32_t time)
{
    Common_DisableInterrupts();
    if(g_dcanDiagMsgRxFlag>=time)
    {
        g_dcanDiagMsgRxFlag -= time;
    }
    else
    {
    if(g_dcanDiagMsgRxFlag!=0)
    {
        g_dcanDiagMsgRxFlag = 0;
    }
    }
    Common_EnableInterrupts();
}












