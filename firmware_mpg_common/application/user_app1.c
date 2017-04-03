/**********************************************************************************************************************
File: user_app1.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app1.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */
volatile bool G_bIsButton0Pushed = FALSE;         
volatile bool G_bIsButton1Pushed = FALSE;    
volatile bool G_bIsButton2Pushed = FALSE;    
volatile bool G_bIsButton3Pushed = FALSE;    

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/

// Globals for passing data from the ANT application to the API (import these to application)
extern AntSetupDataType G_stAntSetupData;
extern u32 G_u32AntApiCurrentDataTimeStamp;                       // From ant_api.c
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    // From ant_api.c
extern u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];  // From ant_api.c

void UserApp1Initialize(void)
{
  G_stAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_USERAPP;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_USERAPP;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;
  
  /* If good initialization, set state to Idle */
  if( AntChannelConfig(ANT_MASTER) )
  {
    AntOpenChannel();
    DebugPrintf("IntoAntMaster\r\n");
    UserApp1_StateMachine = UserApp1SM_ButtonCheck;
    LCDClearChars(LINE1_START_ADDR, 20); 
    LCDMessage(LINE1_START_ADDR, "Push Button to Start");
    LCDMessage(LINE2_START_ADDR, "BassHiHatCrashSnare");
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_FailedInit;
  }

} /* end UserApp1Initialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* StateMachine: UserApp1SM_ButtonCheck

Requires: 
ANT Channel is open, first button of the song has been pressed

Promises:
If a button has been pressed, puts 1 in a specific byte in an 8 byte array.
This array will be put in AntQueueBroadcastMesssage

Button 0 = byte 0 = Bass Drum
Button 1 = byte 1 = Closed Hi Hat
Button 2 = byte 2 = Crash Cymbol
Button 3 = byte 3 = Acostic Snare

*/
static void UserApp1SM_ButtonCheck(void)
{
  static u8 u8LCDCleared = 0;
  if(u8LCDCleared)
  {
      LCDClearChars(LINE1_START_ADDR, 20);
      LCDMessage(LINE1_START_ADDR,"To stop press all");
      u8LCDCleared = 0;
  }
    
   u8 au8TestMessage[] = {0,0,0,0,0,0,0,0};
   u8 au8DataContent[] = "xxxxxxxx";
   if( AntReadData())
   {
     if(G_eAntApiCurrentMessageClass == ANT_DATA)
     {
       
        for(u8 i = 0; i < ANT_DATA_BYTES; i++)
        {
          au8DataContent[i] = G_au8AntApiCurrentData[i];
        }
        LCDMessage(LINE2_START_ADDR, au8DataContent);
     }
     else if(G_eAntApiCurrentMessageClass == ANT_TICK)
     {
       au8TestMessage[7]++;
       if(au8TestMessage[7] == 0)
       {
         au8TestMessage[6]++;
         if(au8TestMessage[6] == 0)
         {
           au8TestMessage[5]++;
         }
       }
     }
     if(IsButtonPressed(BUTTON0))
     { 
       au8TestMessage[0] = 0x01;
       u8LCDCleared = 1;
     }
     else
     {
        au8TestMessage[0] = 0x00;
     }
  
     if(IsButtonPressed(BUTTON1))
     { 
       au8TestMessage[1] = 0x01;
       u8LCDCleared = 1;
     }
     else
     {
        au8TestMessage[1] = 0x00;
     }
     
     if(IsButtonPressed(BUTTON2))
     {
       au8TestMessage[2] = 0x01;
       u8LCDCleared = 1;
     }
     else
     {
        au8TestMessage[2] = 0x00;
     }     
     
     if(IsButtonPressed(BUTTON3))
     {
       au8TestMessage[3] = 0x01;
       u8LCDCleared = 1;
     }
     else
     {
        au8TestMessage[3] = 0x00;
     }     
     
     AntQueueBroadcastMessage(au8TestMessage);
     
   }
} /* end UserApp1SM_Idle() */

static void UserApp1SM_EndOfSong(void)
{
  
}

static void UserApp1SM_ButtonOff(void)
{
  
}


#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserApp1SM_FailedInit(void)          
{
    
} /* end UserApp1SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
