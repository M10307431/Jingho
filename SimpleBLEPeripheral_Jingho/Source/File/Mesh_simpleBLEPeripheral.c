//#include<ioCC2540.h>
#include "stdio.h"
#include "stdlib.h"

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"

#if defined( CC2540_MINIDK )
  #include "simplekeys.h"
#endif

#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

#include "gapbondmgr.h"

#include "simpleBLEPeripheral.h"
#include "MeshProfile.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

#include "central.h"
#include "ll.h"
#include "observer.h"
/*********************************************************************
 * MACROS
 */
#define HAL_ADC_CHANNEL_7          0x07
#define HAL_ADC_RESOLUTION_8       0x01
/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   1000

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          825

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

#if defined ( CC2540_MINIDK )
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
#else
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL
#endif  // defined ( CC2540_MINIDK )

//240==>300ms

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
//416=520ms, 432=540ms, 448=560ms
//264=330ms, 528=660ms, 792=990ms
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80//240

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
//416=520ms, 432=540ms, 448=560ms
//264=330ms, 528=660ms, 792=990ms
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     80//240

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE//TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

#if defined ( PLUS_BROADCASTER )
  #define ADV_IN_CONN_WAIT                    500 // delay 500 ms
#endif

#define MAX_CONNECTIONS                       3
#define DEFAULT_SCAN_DURATION                 4000//ms
#define DEFAULT_SCAN_INTERVAL		      32 //N*625us 32
#define DEFAULT_SCAN_WINDOW                   32 //N*625us 32

#define ROLE_CENTRAL                          1
#define ROLE_PERIPHERAL                       2
// TRUE to use high scan duty cycle when creating link
#define DEFAULT_LINK_HIGH_DUTY_CYCLE          FALSE
// TRUE to use white list when creating link
#define DEFAULT_LINK_WHITE_LIST               FALSE

// ======================================Observer Application states
// Discovey mode (limited, general, all)
#define DEFAULT_DISCOVERY_MODE                DEVDISC_MODE_GENERAL //	DEVDISC_MODE_LIMITED ,DEVDISC_MODE_GENERAL  ,DEVDISC_MODE_ALL
// TRUE to use active scan
#define DEFAULT_DISCOVERY_ACTIVE_SCAN         TRUE
// TRUE to use white list during discovery
#define DEFAULT_DISCOVERY_WHITE_LIST          FALSE
// Maximum number of scan responses
#define DEFAULT_MAX_SCAN_RES                  8
// Scan result list
static gapDevRec_t simpleBLEDevList[DEFAULT_MAX_SCAN_RES];
// Number of scan results and scan result index
static uint8 simpleBLEScanRes;
static uint8 simpleBLEScanIdx;

// Scanning state
static uint8 simpleBLEScanning = FALSE;


enum
{
  BLE_STATE_IDLE,
  BLE_STATE_CONNECTING,
  BLE_STATE_CONNECTED,
  BLE_STATE_DISCONNECTING,
  BLE_STATE_ADVERTISING 
};
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Connection handle of current connection 
static uint16 simpleBLEConnHandle = GAP_CONNHANDLE_INIT;
static bool simpleBLEDoWrite = FALSE;

static uint8 deviceRole = ROLE_PERIPHERAL;
static uint8 masterSlave_State = BLE_STATE_ADVERTISING;
uint16 recevdata;

static bool simple_status = FALSE;
static uint16 gapConnHandle;

static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys );
static void simpleBLEPeripheralProcessGattMsg( gattMsgEvent_t *pMsg );

/************************************************  
                Setting Parameter
************************************************/
#define SBP_BURST_EVT 0x0008    // Send Event Handler
#define AMOUNT_OF_EVENT 2       // Packet event number

static int center_mode=0;   //mode0->Advertising(slave), mode1->Discovery(master), mode2->tx power setting
uint8 burstData[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Send data load (20bytes)
const uint8 seqpkt=4;

static void ValueNotification();

Buffer *HEAD_BUFFER;     //store packet
uint8 B_size=30;
uint8 BufferQueue[30][20];

static uint16 counter=0;

PeriodicEvent_Creat PERIODIC_EVENT[AMOUNT_OF_EVENT];

volatile int SCANDATA_Flag=0;

bool SendFlag;
/*********************************************************************
 * Packet Information
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x14,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  0x53,   // 'S'
  0x69,   // 'i'
  0x6d,   // 'm'
  0x70,   // 'p'
  0x6c,   // 'l'
  0x65,   // 'e'
  0x42,   // 'B'
  0x4c,   // 'L'
  0x45,   // 'E'
  0x50,   // 'P'
  0x65,   // 'e'
  0x72,   // 'r'
  0x69,   // 'i'
  0x70,   // 'p'
  0x68,   // 'h'
  0x65,   // 'e'
  0x72,   // 'r'
  0x61,   // 'a'
  0x6c,   // 'l'

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};
static uint8 RSPData[] =
{
  // complete name
  0x00,   // length of this data
  0x00,   // 'S'
  0x00,   // 'i'
  0x00,   // 'm'
  0x00,   // 'p'
  0x00,   // 'l'
  0x00,   // 'e'
  0x00,   // 'B'
  0x00,   // 'L'
  0x00,   // 'E'
  0x00,   // 'P'
  0x00,   // 'e'
  0x00,   // 'r'
  0x00,   // 'i'
  0x00,   // 'p'
  0x00,   // 'h'
  0x00,   // 'e'
  0x00,   // 'r'
  0x00,   // 'a'
  0x00,   // 'l'

};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( SIMPLEPROFILE_SERV_UUID ),
  HI_UINT16( SIMPLEPROFILE_SERV_UUID ),

};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple BLE Peripheral";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void simpleProfileChangeCB( uint8 paramID );

static void simpleBLECentralRssiCB( uint16 connHandle, int8  rssi );
static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent );
static void simpleBLECentralPasscodeCB( uint8 *deviceAddr, uint16 connectionHandle,uint8 uiInputs, uint8 uiOutputs );
static void simpleBLECentralPairStateCB( uint16 connHandle, uint8 state, uint8 status );

static void observerEventCB(observerRoleEvent_t *pEvent);
static void simpleBLEAddDeviceInfo(uint8 *pAddr, uint8 addrType);

static const gapCentralRoleCB_t simpleBLERoleCB={
  simpleBLECentralRssiCB,       // RSSI callback
  simpleBLECentralEventCB       // Event callback
};
// Bond Manager Callbacks
static const gapBondCBs_t simpleBLEBondCB =
{
  simpleBLECentralPasscodeCB,
  simpleBLECentralPairStateCB
};

typedef struct
{
  uint8 state;
  uint8 connHandle;
  uint8 addr[B_ADDR_LEN]; 
} deviceList_t;
static deviceList_t deviceList[3];
static uint8 currentDevice=0;

static void performPeriodicTask( void );
static void periodicCentralTask( void );

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
static char *bdAddr2Str ( uint8 *pAddr );
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)



/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL,                            // When a valid RSSI is read from controller (not used by application)
  observerEventCB
};

// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
  simpleBLEPeripheral_TaskID = task_id;

  /*===========================================================
					PERIPHERAL APP STARTUP
  ===========================================================*/
  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    #if defined( CC2540_MINIDK )
      // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
      uint8 initial_advertising_enable = FALSE;
    #else
      // For other hardware platforms, device starts advertising upon initialization
      uint8 initial_advertising_enable = TRUE;
    #endif

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );
  //GAP_SetParamValue( TGAP_CONN_EST_INT_MIN, DEFAULT_DESIRED_MIN_CONN_INTERVAL );
  //GAP_SetParamValue( TGAP_CONN_EST_INT_MAX, DEFAULT_DESIRED_MAX_CONN_INTERVAL );
  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }
  
  
  /*===========================================================
					GAP Bond Manager
  ===========================================================*/  

  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
	
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  
  DevInfo_AddService();                           // Device Information Service
  
  SimpleProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
  MeshProfile_AddService( GATT_ALL_SERVICES );    // Mesh GATT Profile
  
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif

  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
  }


#if defined( CC2540_MINIDK )

  SK_AddService( GATT_ALL_SERVICES ); // Simple Keys Profile

  // Register for all key events - This app will handle all key events
  RegisterForKeys( simpleBLEPeripheral_TaskID );

  // makes sure LEDs are off
  HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );

  // For keyfob board set GPIO pins into a power-optimized state
  // Note that there is still some leakage current from the buzzer,
  // accelerometer, LEDs, and buttons on the PCB.

  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO

  /*
  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
  */            
  P0DIR = 0x7C; // Port 0 pins P0.0 and P0.1 as input (buttons) P0.7 as input (light),
                // all others  as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low

#endif // #if defined( CC2540_MINIDK )

#if (defined HAL_LCD) && (HAL_LCD == TRUE)

#if defined FEATURE_OAD
  #if defined (HAL_IMAGE_A)
    HalLcdWriteStringValue( "BLE Peri-A", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #else
    HalLcdWriteStringValue( "BLE Peri-B", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #endif // HAL_IMAGE_A
#else
  HalLcdWriteString( "BLE Peripheral", HAL_LCD_LINE_1 );
#endif // FEATURE_OAD

#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

  // Register callback with SimpleGATTprofile
  VOID SimpleProfile_RegisterAppCBs( &simpleBLEPeripheral_SimpleProfileCBs );

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

#if defined ( DC_DC_P0_7 )

  // Enable stack to toggle bypass control on TPS62730 (DC/DC converter)
  HCI_EXT_MapPmIoPortCmd( HCI_EXT_PM_IO_PORT_P0, HCI_EXT_PM_IO_PORT_PIN7 );

#endif // defined ( DC_DC_P0_7 )


  /*===========================================================
					CENTRAL APP STARTUP
  ===========================================================*/  
  {
	uint8 scanRes=DEFAULT_MAX_SCAN_RES;
	GAPRole_SetParameter( GAPOBSERVERROLE_MAX_SCAN_RES, sizeof( uint8 ), &scanRes );
  }
  // Set SCAN interval
  {
	GAP_SetParamValue( TGAP_GEN_DISC_SCAN, DEFAULT_SCAN_DURATION );
	GAP_SetParamValue( TGAP_LIM_DISC_SCAN, DEFAULT_SCAN_DURATION );
		
    GAP_SetParamValue( TGAP_CONN_SCAN_WIND , DEFAULT_SCAN_WINDOW );
	GAP_SetParamValue( TGAP_CONN_SCAN_INT , DEFAULT_SCAN_INTERVAL );
	
	GAP_SetParamValue( TGAP_GEN_DISC_SCAN_WIND , DEFAULT_SCAN_WINDOW );
	GAP_SetParamValue( TGAP_GEN_DISC_SCAN_INT  , DEFAULT_SCAN_INTERVAL );
	
	GAP_SetParamValue( TGAP_LIM_DISC_SCAN_WIND , DEFAULT_SCAN_WINDOW );
	GAP_SetParamValue( TGAP_LIM_DISC_SCAN_INT, DEFAULT_SCAN_INTERVAL );
  }
  
  //devices the central will try and connect to
  uint8 addr1[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x02};
  uint8 addr2[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x03};
  uint8 addr3[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x04};

  //uint8 addr3[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x00}; //MASTER FOB
  
  VOID osal_memcpy( deviceList[0].addr, addr1, 6 );
  VOID osal_memcpy( deviceList[1].addr, addr2, 6 );
  VOID osal_memcpy( deviceList[2].addr, addr3, 6 );
  
  deviceList[0].state = BLE_STATE_IDLE;
  deviceList[1].state = BLE_STATE_IDLE;
  deviceList[2].state = BLE_STATE_IDLE;

  deviceList[0].connHandle = 1;         
  deviceList[1].connHandle = 2;
  deviceList[2].connHandle = 3;

  /*===========================================================
					Setting process event
  ===========================================================*/
  
  //Open Gatt Indications/Notifications
  VOID GATT_InitClient();
  
  //Signal SYS_EVENT_MSG
  GATT_RegisterForInd( simpleBLEPeripheral_TaskID);//Gatt register-->GATT_MSG_EVENT
  RegisterForKeys( simpleBLEPeripheral_TaskID );//Keys register-->KEY_CHANGE
  
    //-----------------------Init Buffer
    HEAD_BUFFER->size=B_size;
    HEAD_BUFFER->count=0;
    HEAD_BUFFER->cur=0;
    
    //--------------------------------------------------EVENT 1
	
	PERIODIC_EVENT[0].ID=SBP_PERIODIC_TASK1;//SBP_PERIODIC_TASK1
	PERIODIC_EVENT[0].ARRIVAL=0;
	PERIODIC_EVENT[0].SIZE=100;
	PERIODIC_EVENT[0].PERIOD=200;
	PERIODIC_EVENT[0].ARRIVAL_COUNT=0;
	PERIODIC_EVENT[0].EXESIZE=PERIODIC_EVENT[0].SIZE;
	PERIODIC_EVENT[0].DEADLINE=PERIODIC_EVENT[0].PERIOD/1000;
	
	//--------------------------------------------------EVENT 2
	
	PERIODIC_EVENT[1].ID=SBP_PERIODIC_TASK2;
	PERIODIC_EVENT[1].ARRIVAL=0;
	PERIODIC_EVENT[1].SIZE=100;
	PERIODIC_EVENT[1].PERIOD=200;
	PERIODIC_EVENT[1].ARRIVAL_COUNT=0;
	PERIODIC_EVENT[1].EXESIZE=PERIODIC_EVENT[1].SIZE;
	PERIODIC_EVENT[1].DEADLINE=PERIODIC_EVENT[1].PERIOD/1000;
	
	//--------------------------------------------------EVENT 3
	/*
	PERIODIC_EVENT[2].ID=SBP_PERIODIC_TASK3;
	PERIODIC_EVENT[2].ARRIVAL=0;
	PERIODIC_EVENT[2].SIZE=100;
	PERIODIC_EVENT[2].PERIOD=3000;
	PERIODIC_EVENT[2].ARRIVAL_COUNT=0;
	PERIODIC_EVENT[2].EXESIZE=PERIODIC_EVENT[2].SIZE;
	PERIODIC_EVENT[2].DEADLINE=PERIODIC_EVENT[2].PERIOD/1000;
    */
		
	/*---------------------------
		Set BLE init event
	---------------------------*/
	osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );
    osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT );   //TimeSlot count
    
  //osal_set_event( simpleBLEPeripheral_TaskID, SBP_BURST_EVT );      //TX Event
  //osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
  
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */

uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID task_id;
  
  /*==========================================
	SYS_EVENT_MSG(Handle Key & Receive MSG)
  ==========================================*/
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL ){		
      simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );
      VOID osal_msg_deallocate( pMsg );// Release the OSAL message
    }
    return (events ^ SYS_EVENT_MSG);
  }

  /*==========================================
		SBP_START_DEVICE_EVT (Master/Slave mode)
  ==========================================*/
  if ( events & SBP_START_DEVICE_EVT )
  {
    if(center_mode==0){
		VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );// Start the Slave mode
		masterSlave_State = BLE_STATE_ADVERTISING; 
		VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );// Start Bond Manager(Init)
		GAPCentralRole_TerminateLink( 0XFFFE );//cancel current link request 
		uint8 turnOnAdv = TRUE;
		GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );
		
		HalLcdWriteString( "Start Slave EVT",  HAL_LCD_LINE_6 );
	}else if(center_mode==1){
		VOID GAPCentralRole_StartDevice( (gapCentralRoleCB_t *) &simpleBLERoleCB );// Start the Master mode
		GAPBondMgr_Register( (gapBondCBs_t *) &simpleBLEBondCB );
		masterSlave_State = BLE_STATE_CONNECTING;
		deviceRole = ROLE_CENTRAL;
		uint8 new_adv_enabled_status = FALSE;// PERIPHERAL --> CENTRAL
		GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
		
		HalLcdWriteString( "Start Master EVT",  HAL_LCD_LINE_6 );
	}
	
    return ( events ^ SBP_START_DEVICE_EVT );
  }
  
  /*==========================================
    SBP_PERIODIC_EVT(Periodic event)
  ==========================================*/
  
  if ( events & SBP_PERIODIC_EVT )
  {
    // Restart timer
    if ( SBP_PERIODIC_EVT_PERIOD ){
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );	
    }
	
	performPeriodicTask();// Perform periodic application task
	
	return (events ^ SBP_PERIODIC_EVT);
  }
  
  /*==========================================
			�j�MArrival Event
  ==========================================*/
   for (int i=0; i<AMOUNT_OF_EVENT;i++){
		if(events & PERIODIC_EVENT[i].ID){
        
            /*****************
              Arrival time
            *****************/
               
            uint32 Arrival_Clock=osal_GetSystemClock();
            uint8 Load[20];
            
            BufferQueue[HEAD_BUFFER->count][4]=(Arrival_Clock & 0xFF000000)>>24;
            BufferQueue[HEAD_BUFFER->count][3]=(Arrival_Clock & 0xFF0000)>>16;
            BufferQueue[HEAD_BUFFER->count][2]=(Arrival_Clock & 0xFF00)>>8;
            BufferQueue[HEAD_BUFFER->count][1]=(Arrival_Clock & 0xFF);
            BufferQueue[HEAD_BUFFER->count][0]=PERIODIC_EVENT[i].ID;
            
            /*****************
              Buffer Size add
            *****************/
            HEAD_BUFFER->count++;
            if(HEAD_BUFFER->count>=HEAD_BUFFER->size){
                HEAD_BUFFER->count=0;
            }
            
            HalLcdWriteStringValue( "Count:", HEAD_BUFFER->count, 10,  HAL_LCD_LINE_6);
            
			osal_start_timerEx( simpleBLEPeripheral_TaskID, PERIODIC_EVENT[i].ID, PERIODIC_EVENT[i].PERIOD);
			return (events ^ PERIODIC_EVENT[i].ID);//XOR
		}
   }
   
  /*==========================================
	SBP_BURST_EVT(Send msg)
  ==========================================*/
 if ( events & SBP_BURST_EVT ){  
    return (events ^ SBP_BURST_EVT);
  }  
  
#if defined ( PLUS_BROADCASTER )
  if ( events & SBP_ADV_IN_CONNECTION_EVT )
  {
    uint8 turnOnAdv = TRUE;
    // Turn on advertising while in a connection
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );

    return (events ^ SBP_ADV_IN_CONNECTION_EVT);
  }
#endif // PLUS_BROADCASTER

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
		
  switch ( pMsg->event )
  {
  
    case KEY_CHANGE:
      simpleBLEPeripheral_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
    break;
  
    case GATT_MSG_EVENT://HCI_LE_ExtEvent ATT_HandleValueNotification
      simpleBLEPeripheralProcessGattMsg((gattMsgEvent_t *) pMsg);
    break;
  
	default:
    // do nothing
    break;
  }
}

/*=============================================
                Master Request
                <Recv Notify data>
=============================================*/
static void simpleBLEPeripheralProcessGattMsg(gattMsgEvent_t *pMsg)
{

 if ( pMsg->method == ATT_HANDLE_VALUE_NOTI || pMsg->method == ATT_HANDLE_VALUE_IND ){
	volatile uint8 recvmsg=pMsg->msg.handleValueNoti.value[0];
	HalLcdWriteStringValue( "", recvmsg, 10,  HAL_LCD_LINE_4 );
	

	/*==============================
       Trigger Periodoc Event work
    ==============================*/
	if(recvmsg==14){
		  osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK1 ); //Event1
		  osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK2 ); //Event2
		  //osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK3 ); //Event3
	}
    	
    /*==============================
             Response Data
    ==============================*/
	if(recvmsg==13){
		osal_set_event( simpleBLEPeripheral_TaskID, SBP_BURST_EVT );
        ValueNotification();
	}
	
    /*==============================
       Discovery for adv node
       (Mesh)
    ==============================*/
	if(recvmsg==12){
		SCANDATA_Flag=0;
		HalLcdWriteString( "Discovering...", HAL_LCD_LINE_1 );
		GAPObserverRole_StartDiscovery( DEFAULT_DISCOVERY_MODE,
										  DEFAULT_DISCOVERY_ACTIVE_SCAN,
										  DEFAULT_DISCOVERY_WHITE_LIST ); 
	}
	
 }
}

/*=============================================
				Handle Key function
===============================================*/
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys )
{
  uint8 SK_Keys = 0;

  VOID shift;  // Intentionally unreferenced parameter
  /*=========================================
  
              UP Key
  
  ===========================================*/
  if ( keys & HAL_KEY_UP ){

	if(!simpleBLEScanning){
		//HalLcdWriteString( "Discovering...", HAL_LCD_LINE_1 );
		//HalLcdWriteString( "", HAL_LCD_LINE_2 );
		//HalLcdWriteString( "", HAL_LCD_LINE_4 );
		
		GAPObserverRole_StartDiscovery( DEFAULT_DISCOVERY_MODE,
										  DEFAULT_DISCOVERY_ACTIVE_SCAN,
										  DEFAULT_DISCOVERY_WHITE_LIST ); 
		simpleBLEScanning=TRUE;
	}else{
		//HalLcdWriteString( "Disable Discovering", HAL_LCD_LINE_1 );
		simpleBLEScanning=FALSE;
		GAPobserverRole_CancelDiscovery();
	}
	
  }
  /*=========================================  
              DOWN Key
  ===========================================*/
  if ( keys & HAL_KEY_DOWN ){
  
    HalLcdWriteString( "Interval Change 1", HAL_LCD_LINE_6 );
    uint16 desired_min_interval = 40;
    uint16 desired_max_interval = 40;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;
    
    GAPRole_SendUpdateParam( desired_min_interval, desired_max_interval, desired_slave_latency, desired_conn_timeout,GAPROLE_TERMINATE_LINK);
  }
  /*=========================================
              LEFT Key
  ===========================================*/
  if ( keys & HAL_KEY_LEFT ){
    HalLcdWriteString( "Interval Change 2", HAL_LCD_LINE_6 );
    
    uint16 desired_min_interval = 160;
    uint16 desired_max_interval = 160;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;
    
    GAPRole_SendUpdateParam( desired_min_interval, desired_max_interval, desired_slave_latency, desired_conn_timeout,GAPROLE_TERMINATE_LINK);
  }
  /*=========================================
              RIGHT Key
  ===========================================*/
  if ( keys & HAL_KEY_RIGHT ){
    HalLcdWriteString( "TX power set 1", HAL_LCD_LINE_7 );
    HCI_EXT_SetTxPowerCmd(HCI_EXT_TX_POWER_4_DBM);
  }
  
  /*=========================================
              CENTER Key
  ===========================================*/
  if(keys & HAL_KEY_CENTER){
    HalLcdWriteString( "TX power set 2", HAL_LCD_LINE_7 );
    HCI_EXT_SetTxPowerCmd(LL_EXT_TX_POWER_MINUS_23_DBM);
  }
 
}

static void ValueNotification(){
    
    uint8 NotifyFlag=0;
	attHandleValueNoti_t Pkt;
	uint32 NotifyClock;
    NotifyClock=osal_GetSystemClock();
    uint8 SeqPkt=4;
    /*******************************
    *******************************/
    
    while(HEAD_BUFFER->count!=HEAD_BUFFER->cur && (SeqPkt--)!=0){
        NotifyFlag=1;
        
        BufferQueue[HEAD_BUFFER->cur][8]=(NotifyClock & 0xFF000000)>>24;
        BufferQueue[HEAD_BUFFER->cur][7]=(NotifyClock & 0xFF0000)>>16;
        BufferQueue[HEAD_BUFFER->cur][6]=(NotifyClock & 0xFF00)>>8;
        BufferQueue[HEAD_BUFFER->cur][5]=(NotifyClock & 0xFF);
    
        Pkt.len=20;
        Pkt.handle=BufferQueue[HEAD_BUFFER->cur][0];
        osal_memcpy(&Pkt.value, BufferQueue[HEAD_BUFFER->cur], 20);
        //HalLcdWriteStringValue( "Cur:", HEAD_BUFFER->cur, 10,  HAL_LCD_LINE_7);
        
        if(GATT_Notification(gapConnHandle, &Pkt, FALSE)==SUCCESS){}
        
        HEAD_BUFFER->cur++;
        if(HEAD_BUFFER->cur>=HEAD_BUFFER->size){
            HEAD_BUFFER->cur=0;
        }
    }
    
    //No Packet
    /*
    if(NotifyFlag==0){
        Pkt.len=20;
        Pkt.handle=00;
        osal_memcpy(&Pkt.value, &burstData, 20);
        if(GATT_Notification(gapConnHandle, &Pkt, FALSE)==SUCCESS){}    
    }
    */
}
/*********************************************************************
 * @fn      Central CallBack
 *
 * @brief   simpleBLECentralEventCB,simpleBLECentralRssiCB,simpleBLECentralPasscodeCB,simpleBLECentralPairStateCB
 *
 * @param   
 *
 */
 static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent )
{
	uint8 i;  
	//HalLcdWriteString( "Central CB",  HAL_LCD_LINE_7 );
	
  switch ( pEvent->gap.opcode )
  {
      
   case GAP_LINK_ESTABLISHED_EVENT:
      {
       
        if ( pEvent->gap.hdr.status == SUCCESS )
        {          
          for(i=0; i<MAX_CONNECTIONS;i++)
          {
            //Check against addresses we want to connect to
            if(deviceList[i].addr[5] == pEvent->linkCmpl.devAddr[5])
            {
              deviceList[i].state = BLE_STATE_CONNECTED;
              deviceList[i].connHandle = pEvent->linkCmpl.connectionHandle;
            }
          }
         }        
      }
        
      break;

      case GAP_LINK_TERMINATED_EVENT:
      {
          for(i=0; i<MAX_CONNECTIONS;i++)
          {
            //Check against addresses we want to connect to
            if(deviceList[i].connHandle == pEvent->linkTerminate.connectionHandle)
            {
              deviceList[i].state = BLE_STATE_IDLE;
            }
          }     
      }
      break;
	  
	  default:
	  break;
  }
}

static void simpleBLECentralRssiCB( uint16 connHandle, int8 rssi )
{
    //LCD_WRITE_STRING_VALUE( "RSSI -dB:", (uint8) (-rssi), 10, HAL_LCD_LINE_1 );
}

static void simpleBLECentralPasscodeCB( uint8 *deviceAddr, uint16 connectionHandle,
                                        uint8 uiInputs, uint8 uiOutputs )
{
#if (HAL_LCD == TRUE)

  uint32  passcode;
  uint8   str[7];

  // Create random passcode
  LL_Rand( ((uint8 *) &passcode), sizeof( uint32 ));
  passcode %= 1000000;
  
  // Display passcode to user
  if ( uiOutputs != 0 )
  {
	HalLcdWriteString( "Passcode:",  HAL_LCD_LINE_1 );
    
  }
  
  // Send passcode response
  GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, passcode );
#endif
}

 
/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
	//HalLcdWriteString( "Slave CB",  HAL_LCD_LINE_6 );
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          // Display device address
          HalLcdWriteString( bdAddr2Str( ownAddress ),  HAL_LCD_LINE_2 );
          HalLcdWriteString( "Initialized",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
	  
	  //Restart Advertisement
	  uint8 turnOnAdv = TRUE;
	  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );
	  
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		simpleBLEDoWrite = FALSE;
      }
      break;

    case GAPROLE_CONNECTED:
      {
		//Get connection handle
		GAPRole_GetParameter( GAPROLE_CONNHANDLE, &gapConnHandle );
        
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Connected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        
        simpleBLEDoWrite=true;
		masterSlave_State = BLE_STATE_CONNECTED;

      }
    break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		masterSlave_State = BLE_STATE_ADVERTISING;
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		masterSlave_State = BLE_STATE_ADVERTISING;
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
		masterSlave_State = BLE_STATE_ADVERTISING;
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif


}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          OSAL event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask( void )
{

    //Get light sensor data
    uint16 adc;
    int8 buf[10];
    
    adc=HalAdcRead(HAL_ADC_CHANNEL_7, HAL_ADC_RESOLUTION_8);
    HalLcdWriteStringValue( "Sensing data", (uint16)(adc), 10 , HAL_LCD_LINE_4);
    
    /*
    arrivalcount_t1++;
    HalLcdWriteStringValue( "Task1 arrival", (uint16)(arrivalcount_t1), 10 , HAL_LCD_LINE_4);
	if(arrivalcount_t1==100)
		arrivalcount_t1=0;
	*/
  uint8 valueToCopy;
  uint8 stat;

  // Call to retrieve the value of the third characteristic in the profile
  stat = SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &valueToCopy);

  if( stat == SUCCESS )
  {
    /*
     * Call to set that value of the fourth characteristic in the profile. Note
     * that if notifications of the fourth characteristic have been enabled by
     * a GATT client device, then a notification will be sent every time this
     * function is called.
     */
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof(uint8), &valueToCopy);
  }
}

static void periodicCentralTask( void )
{
    uint8 numDevices=1;
    uint8 i;

    HalLedSet( HAL_LED_ALL, HAL_LED_MODE_OFF );
  

     if(deviceRole == ROLE_CENTRAL)
     {
        //Calc number of connections
        for(i=0;i<MAX_CONNECTIONS;i++)
        {
          if(deviceList[i].state == BLE_STATE_CONNECTED)
          {
            numDevices = numDevices + 1;
          }
        }
     }
    
    switch (masterSlave_State)
    {
 
       case BLE_STATE_CONNECTING:
       case BLE_STATE_CONNECTED:
         
         if(deviceRole == ROLE_CENTRAL)
         {
           
             //cancel current link request - only give each fob 1 sec to complet connection
             GAPCentralRole_TerminateLink( 0XFFFE );
            
                GAPCentralRole_EstablishLink( DEFAULT_LINK_HIGH_DUTY_CYCLE,
                                              DEFAULT_LINK_WHITE_LIST,
                                              ADDRTYPE_PUBLIC, deviceList[currentDevice].addr );
                
         }                
          //green
         //HalLedBlink (1, 3, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME); 
         osal_pwrmgr_device( PWRMGR_BATTERY );
          HalLedBlink (1, numDevices, 50, 500);
         osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
          
          break;
      case BLE_STATE_DISCONNECTING:
      case BLE_STATE_IDLE:

         if(deviceRole == ROLE_CENTRAL)
         {
           //are we finished disconnecting
           if(numDevices == 1)
           {  
              // set new role to peripheral           
              deviceRole = ROLE_PERIPHERAL;
              
              masterSlave_State = BLE_STATE_ADVERTISING;
              
              //restet to peripheral and advertise
              osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT, 100 );
           }
           else  //continue to terminate links
           {
            if(deviceList[currentDevice].state == BLE_STATE_CONNECTED)
            {
              //terminate link
              GAPCentralRole_TerminateLink(deviceList[currentDevice].connHandle);
            }
           }
           
           //red
          HalLedBlink (2, 2, 10, 100);
         }
        break;
        
      case BLE_STATE_ADVERTISING: 

           // HalLedBlink (2, 1, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME);    
           HalLedBlink (2, 4, 10, 100);
        
        break;
        
          
    default:
      
        break;
    }

    currentDevice = currentDevice +1;
    
    if(currentDevice >2)
      currentDevice = 0;
}


/*********************************************************************
 * @fn      observerCB
 *
 * @brief   Notification from the Observer Role of an event.
 *
 * @param   newState - new state
 *
 * @return  none
 */

static void observerEventCB( observerRoleEvent_t *pEvent )
{
  switch ( pEvent->gap.opcode )
  {
    case GAP_DEVICE_INIT_DONE_EVENT:  
      {
        //HalLcdWriteString( "BLE Observer", HAL_LCD_LINE_1 );
        //HalLcdWriteString( bdAddr2Str( pEvent->initDone.devAddr ),  HAL_LCD_LINE_2 );
      }
      break;
	  
	//============================================���DEVICES
    case GAP_DEVICE_INFO_EVENT:
      {
        simpleBLEAddDeviceInfo( pEvent->deviceInfo.addr, pEvent->deviceInfo.addrType ); 
		
		/*---------------------------------------
					Response Data
		---------------------------------------*/
		
		uint8 RSParray[20];
		//osal_memcpy( RSParray, pEvent->deviceInfo.pEvtData, sizeof(RSParray) );
		//HalLcdWriteString( RSParray+2, HAL_LCD_LINE_4 );	
		
		if(pEvent->deviceInfo.pEvtData[2]=='S'){
			SCANDATA_Flag=1;
			//osal_set_event( simpleBLEPeripheral_TaskID, SBP_BURST_EVT );
			
			//HalLcdWriteString( "Get Response", HAL_LCD_LINE_4 );	
			GAPobserverRole_CancelDiscovery();
		}
      }
      break;
		
	//============================================����SCAN (SCAN DURATION)
    case GAP_DEVICE_DISCOVERY_EVENT:
      {
        // discovery complete
        simpleBLEScanning = FALSE;

        // Copy results
        simpleBLEScanRes = pEvent->discCmpl.numDevs;
        osal_memcpy( simpleBLEDevList, pEvent->discCmpl.pDevList,
                     (sizeof( gapDevRec_t ) * pEvent->discCmpl.numDevs) );
        
        HalLcdWriteStringValue( "Devices Found", simpleBLEScanRes,
                                10, HAL_LCD_LINE_1 );
		
		/*
		if(!SCANDATA_Flag){
			GAPObserverRole_StartDiscovery( DEFAULT_DISCOVERY_MODE,
										  DEFAULT_DISCOVERY_ACTIVE_SCAN,
										  DEFAULT_DISCOVERY_WHITE_LIST ); 
		}
		*/
        if ( simpleBLEScanRes > 0 )
        {
          //HalLcdWriteString( "<- To Select", HAL_LCD_LINE_2 );
        }

        // initialize scan index to last device
        simpleBLEScanIdx = simpleBLEScanRes;
      }
      break;
      
    default:
      break;
  }
}

/*********************************************************************
 * @fn      simpleBLEAddDeviceInfo
 *
 * @brief   Add a device to the device discovery result list
 *
 * @return  none
 */
static void simpleBLEAddDeviceInfo( uint8 *pAddr, uint8 addrType )
{
  uint8 i;
  
  // If result count not at max
  if ( simpleBLEScanRes < DEFAULT_MAX_SCAN_RES )
  {
    // Check if device is already in scan results
    for ( i = 0; i < simpleBLEScanRes; i++ )
    {
      if ( osal_memcmp( pAddr, simpleBLEDevList[i].addr , B_ADDR_LEN ) )
      {
        return;
      }
    }
    
    // Add addr to scan result list
    osal_memcpy( simpleBLEDevList[simpleBLEScanRes].addr, pAddr, B_ADDR_LEN );
    simpleBLEDevList[simpleBLEScanRes].addrType = addrType;
    
    // Increment scan result count
    simpleBLEScanRes++;
  }
}

/*==============================================
			Simple Profile CallBack
==============================================*/
static void simpleProfileChangeCB( uint8 paramID )
{
  uint8 newValue;
  
  switch( paramID )
  {
    case SIMPLEPROFILE_CHAR1:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR1, &newValue );

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 1:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        
	  /*----------------------------
				RECV MSG
	  ----------------------------*/
	  recevdata=newValue;
	  if(newValue==0){
		osal_stop_timerEx(simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK1);
		osal_stop_timerEx(simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK2);
		osal_stop_timerEx(simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK3);			
      }
	  
	  if (newValue==1){
		osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT );   //TimeSlot count
		
	  }
	  
      break;

    case SIMPLEPROFILE_CHAR3:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &newValue );

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 3:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif 

      break;

    default:
	  break;
  }
}

static void simpleBLECentralPairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
  if ( state == GAPBOND_PAIRING_STATE_STARTED )
  {
	//HalLcdWriteString( "Pairing started",  HAL_LCD_LINE_1 );
  }
  else if ( state == GAPBOND_PAIRING_STATE_COMPLETE )
  {
    if ( status == SUCCESS )
    {
		//HalLcdWriteString( "Pairing success",  HAL_LCD_LINE_1 );
    }
    else
    {
		//HalLcdWriteString( "Pairing fail",  HAL_LCD_LINE_1 );
    }
  }
  else if ( state == GAPBOND_PAIRING_STATE_BONDED )
  {
    if ( status == SUCCESS )
    {
		//HalLcdWriteString( "Bonding success",  HAL_LCD_LINE_1 );
    }
  }
}

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string. Only needed when
 *          LCD display is used.
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
  uint8       i;
  char        hex[] = "0123456789ABCDEF";
  static char str[B_ADDR_STR_LEN];
  char        *pStr = str;

  *pStr++ = '0';
  *pStr++ = 'x';

  // Start from end of addr
  pAddr += B_ADDR_LEN;

  for ( i = B_ADDR_LEN; i > 0; i-- )
  {
    *pStr++ = hex[*--pAddr >> 4];
    *pStr++ = hex[*pAddr & 0x0F];
  }

  *pStr = 0;

  return str;
}
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

/*********************************************************************
*********************************************************************/
