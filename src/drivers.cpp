/*
 * Authors (alphabetical order)
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 *
 * gruvin9x is based on code named er9x by
 * Author - Erez Raviv <erezraviv@gmail.com>, which is in turn
 * was based on the original (and ongoing) project by Thomas Husterer,
 * th9x -- http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "gruvin9x.h"

#ifndef SIMU

uint16_t eeprom_pointer;
const char* eeprom_buffer_data;
volatile int8_t eeprom_buffer_size = 0;

inline void eeprom_write_byte()
{
  EEAR = eeprom_pointer;
  EEDR = *eeprom_buffer_data;
#if defined (PCBV3)
  EECR |= 1<<EEMPE;
  EECR |= 1<<EEPE;
#else
  EECR |= 1<<EEMWE;
  EECR |= 1<<EEWE;
#endif
  eeprom_pointer++;
  eeprom_buffer_data++;
}

ISR(EE_READY_vect)
{
  if (--eeprom_buffer_size > 0) {
    eeprom_write_byte();
  }
  else {
#if defined (PCBV3)
    EECR &= ~(1<<EERIE);
#else
    EECR &= ~(1<<EERIE);
#endif
  }
}

void eeWriteBlockCmp(const void *i_pointer_ram, void *i_pointer_eeprom, size_t size)
{
  assert(!eeprom_buffer_size);

  eeprom_pointer = (uint16_t)i_pointer_eeprom;
  eeprom_buffer_data = (const char*)i_pointer_ram;
  eeprom_buffer_size = size+1;

#if defined (PCBV3)
  EECR |= (1<<EERIE);
#else
  EECR |= (1<<EERIE);
#endif

  if (s_sync_write) {
    while (eeprom_buffer_size > 0) wdt_reset();
  }
}

#endif


static uint8_t s_evt;
void putEvent(uint8_t evt)
{
  s_evt = evt;
}
uint8_t getEvent()
{
  uint8_t evt = s_evt;
  s_evt=0;
  return evt;
}

class Key
{
#define FILTERBITS      4
#ifdef SIMU
#define FFVAL 1
#else
#define FFVAL          ((1<<FILTERBITS)-1)
#endif
#define KSTATE_OFF      0
#define KSTATE_RPTDELAY 95 // gruvin: delay state before key repeating starts
//#define KSTATE_SHORT   96
#define KSTATE_START    97
#define KSTATE_PAUSE    98
#define KSTATE_KILLED   99
  uint8_t m_vals:FILTERBITS;   // key debounce?  4 = 40ms
  uint8_t m_dblcnt:2;
  uint8_t m_cnt;
  uint8_t m_state;
public:
  void input(bool val, EnumKeys enuk);
  bool state()       { return m_vals==FFVAL;                }
  void pauseEvents() { m_state = KSTATE_PAUSE;  m_cnt   = 0;}
  void killEvents()  { m_state = KSTATE_KILLED; m_dblcnt=0; }
  uint8_t getDbl()   { return m_dblcnt;                     }
};


Key keys[NUM_KEYS];
void Key::input(bool val, EnumKeys enuk)
{
  //  uint8_t old=m_vals;
  m_vals <<= 1;  if(val) m_vals |= 1; //portbit einschieben
  m_cnt++;

  if(m_state && m_vals==0){  //gerade eben sprung auf 0
    if(m_state!=KSTATE_KILLED) {
      putEvent(EVT_KEY_BREAK(enuk));
      if(!( m_state == 16 && m_cnt<16)){
        m_dblcnt=0;
      }
        //      }
    }
    m_cnt   = 0;
    m_state = KSTATE_OFF;
  }
  switch(m_state){
    case KSTATE_OFF:
      if(m_vals==FFVAL){ //gerade eben sprung auf ff
        m_state = KSTATE_START;
        if(m_cnt>16) m_dblcnt=0; //pause zu lang fuer double
        m_cnt   = 0;
      }
      break;
      //fallthrough
    case KSTATE_START:
      putEvent(EVT_KEY_FIRST(enuk));
      m_dblcnt++;
      m_state   = KSTATE_RPTDELAY;
      m_cnt     = 0;
      break;

    case KSTATE_RPTDELAY: // gruvin: delay state before first key repeat
      if(m_cnt == 24) putEvent(EVT_KEY_LONG(enuk)); 
      if (m_cnt == 40) {
        m_state = 16;
        m_cnt = 0;
      }
      break;

    case 16:
    case 8:
    case 4:
    case 2:
      if(m_cnt >= 48)  { //3 6 12 24 48 pulses in every 480ms
        m_state >>= 1;
        m_cnt     = 0;
      }
      //fallthrough
    case 1:
      if( (m_cnt & (m_state-1)) == 0)  putEvent(EVT_KEY_REPT(enuk));
      break;

    case KSTATE_PAUSE: //pause
      if(m_cnt >= 64)      {
        m_state = 8;
        m_cnt   = 0;
      }
      break;

    case KSTATE_KILLED: //killed
      break;
  }
}

bool keyState(EnumKeys enuk)
{
  if(enuk < (int)DIM(keys))  return keys[enuk].state() ? 1 : 0;

#if defined (PCBV4)
  switch(enuk){
    case SW_ElevDR : return PINC & (1<<INP_C_ElevDR);
    
    case SW_AileDR : return PINC & (1<<INP_C_AileDR);

    case SW_RuddDR : return PING & (1<<INP_G_RuddDR);
      //     INP_G_ID1 INP_B_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0    : return !(PING & (1<<INP_G_ID1));
    case SW_ID1    : return (PING & (1<<INP_G_ID1))&& (PINB & (1<<INP_B_ID2));
    case SW_ID2    : return !(PINB & (1<<INP_B_ID2));

    case SW_Gear   : return PING & (1<<INP_G_Gear);

    case SW_ThrCt  : return PING & (1<<INP_G_ThrCt);

    case SW_Trainer: return PINB & (1<<INP_B_Trainer);

    case SW_RE1: return (~PIND & 0b00100000);

    case SW_RE2: return (~PIND & 0b00010000);

    default:;
  }
#else
  switch(enuk){
    case SW_ElevDR : return PINE & (1<<INP_E_ElevDR);
    
#if defined(JETI) || defined(FRSKY)
    case SW_AileDR : return PINC & (1<<INP_C_AileDR); //shad974: rerouted inputs to free up UART0
#else
    case SW_AileDR : return PINE & (1<<INP_E_AileDR);
#endif

    case SW_RuddDR : return PING & (1<<INP_G_RuddDR);
      //     INP_G_ID1 INP_E_ID2
      // id0    0        1
      // id1    1        1
      // id2    1        0
    case SW_ID0    : return !(PING & (1<<INP_G_ID1));
    case SW_ID1    : return (PING & (1<<INP_G_ID1))&& (PINE & (1<<INP_E_ID2));
    case SW_ID2    : return !(PINE & (1<<INP_E_ID2));
    case SW_Gear   : return PINE & (1<<INP_E_Gear);
    //case SW_ThrCt  : return PINE & (1<<INP_E_ThrCt);

#if defined(JETI) || defined(FRSKY)
    case SW_ThrCt  : return PINC & (1<<INP_C_ThrCt); //shad974: rerouted inputs to free up UART0
#else
    case SW_ThrCt  : return PINE & (1<<INP_E_ThrCt);
#endif

    case SW_Trainer: return PINE & (1<<INP_E_Trainer);
    default:;
  }
#endif // defined (PCBV4)
  return 0;
}

void pauseEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].pauseEvents();
}
void killEvents(uint8_t event)
{
  event=event & EVT_KEY_MASK;
  if(event < (int)DIM(keys))  keys[event].killEvents();
}

//uint16_t g_anaIns[8];
volatile uint16_t g_tmr10ms;
volatile uint8_t  g_blinkTmr10ms;


#if defined (PCBV3)
uint8_t g_ms100 = 0; // global to allow time set function to reset to zero
#endif
void per10ms()
{
    g_tmr10ms++;
    g_blinkTmr10ms++;

#if defined (PCBV3)
    /* Update gloabal Date/Time every 100 per10ms cycles */
    if (++g_ms100 == 100)
    {
      g_unixTime++; // inc global unix timestamp one second
      g_ms100 = 0;
    }
#endif

/**** BEGIN KEY STATE READ ****/
  uint8_t enuk = KEY_MENU;

// User buttons ...
#if defined (PCBV3)
  /* Original keys were connected to PORTB as follows:

     Bit  Key
      7   other use
      6   LEFT
      5   RIGHT
      4   UP
      3   DOWN
      2   EXIT
      1   MENU
      0   other use
  */

#  if defined (PCBV4)
  uint8_t tin = ~PINL;
  uint8_t in;
  in = (tin & 0x0f) << 3;
  in |= (tin & 0x30) >> 3;

#  else

// Gruvin's PCBv2.14/v3 key scanning ...
#define KEY_Y0 1 // EXIT / MENU
#define KEY_Y1 2 // LEFT / RIGHT / UP / DOWN
#define KEY_Y2 4 // LV_Trim_Up / Down / LH_Trim_Up / Down 
#define KEY_Y3 8 // RV_Trim_Up / Down / RH_Trim_Up / Down 
#define TRIM_M_LH_DWN 0 // KEY_X0
#define TRIM_M_LH_UP  1 // KEY_X1
#define TRIM_M_LV_DWN 2 // KEY_X2
#define TRIM_M_LV_UP  3 // KEY_X3
#define TRIM_M_RV_DWN 4 // KEY_X0
#define TRIM_M_RV_UP  5 // KEY_X1
#define TRIM_M_RH_DWN 6 // KEY_X2
#define TRIM_M_RH_UP  7 // KEY_3X

  uint8_t in, tin;

  in = keyDown(); // in gruvin9x.cpp

#  endif // PCBV4

#else
  uint8_t in = ~PINB;
#endif

  for(int i=1; i<7; i++)
  {
    //INP_B_KEY_MEN 1  .. INP_B_KEY_LFT 6
    keys[enuk].input(in & (1<<i),(EnumKeys)enuk);
    ++enuk;
  }
// End User buttons

// Trim switches ...
#if defined (PCBV3)
#  if defined (PCBV4)
  static  prog_uchar  APM crossTrim[]={
    1<<INP_J_TRM_LH_DWN,
    1<<INP_J_TRM_LH_UP,
    1<<INP_J_TRM_LV_DWN,
    1<<INP_J_TRM_LV_UP,
    1<<INP_J_TRM_RV_DWN,
    1<<INP_J_TRM_RV_UP,
    1<<INP_J_TRM_RH_DWN,
    1<<INP_J_TRM_RH_UP
  };
#  else
  static  prog_uchar  APM crossTrim[]={
    1<<TRIM_M_RV_DWN,
    1<<TRIM_M_RV_UP,
    1<<TRIM_M_RH_DWN,
    1<<TRIM_M_RH_UP,
    1<<TRIM_M_LH_DWN,
    1<<TRIM_M_LH_UP,
    1<<TRIM_M_LV_DWN,
    1<<TRIM_M_LV_UP
  };
#  endif

#else // stock original board ...

  static  prog_uchar  APM crossTrim[]={
    1<<INP_D_TRM_LH_DWN,  // bit 7
    1<<INP_D_TRM_LH_UP,
    1<<INP_D_TRM_LV_DWN,
    1<<INP_D_TRM_LV_UP,
    1<<INP_D_TRM_RV_DWN,
    1<<INP_D_TRM_RV_UP,
    1<<INP_D_TRM_RH_DWN,
    1<<INP_D_TRM_RH_UP    // bit 0
  };
#endif

#if defined (PCBV3)

#  if defined (PCBV4)
  in = ~PINJ;
#  else
  PORTD = ~KEY_Y2; // select Y2 row. (Bits 3:0 LVD / LVU / LHU / LHD)
  _delay_us(1);
  in = ~PIND & 0xf0; // mask out outputs

  PORTD = ~KEY_Y3; // select Y3 row. (Bits 3:0 RHU / RHD / RVD / RVU)
  _delay_us(1);
  in |= ((~PIND & 0xf0) >> 4); 

  PORTD = 0xFF;
#  endif

#else

  in = ~PIND;

// Legacy support for USART1 free hardware mod [DEPRECATED]
#  if defined(USART1FREED)
  // mask out original INP_D_TRM_LV_UP and INP_D_TRM_LV_DWN bits
  in &= ~((1<<INP_D_TRM_LV_UP) | (1<<INP_D_TRM_LV_DWN));

  // merge in the two new switch port values
  in |= (~PINC & (1<<INP_C_TRM_LV_UP)) ? (1<<INP_D_TRM_LV_UP) : 0;
  in |= (~PING & (1<<INP_G_TRM_LV_DWN)) ? (1<<INP_D_TRM_LV_DWN) : 0;
#  endif
#endif

  for(int i=0; i<8; i++)
  {
    // INP_D_TRM_RH_UP   0 .. INP_D_TRM_LH_UP   7
    keys[enuk].input(in & pgm_read_byte(crossTrim+i),(EnumKeys)enuk);
    ++enuk;
  }
// End Trim Switches

/**** END KEY STATE READ ****/

#if defined (FRSKY)

  // Attempt to transmit any waiting Fr-Sky alarm set packets every 50ms (subject to packet buffer availability)
  static uint8_t FrskyDelay = 5;
  if (FrskyAlarmSendState && (--FrskyDelay == 0))
  {
    FrskyDelay = 5; // 50ms
    FRSKY10mspoll();
  }
  
  if (frskyStreaming > 0)
    frskyStreaming--;
  else if (g_eeGeneral.enableTelemetryAlarm && (g_model.frsky.channels[0].ratio || g_model.frsky.channels[1].ratio)) {
#if defined (BEEPSPKR)
    if (!(g_tmr10ms % 30)) beepWarn2Spkr((g_tmr10ms % 60) ? 25 : 20);
#else
    if (!(g_tmr10ms % 30)) 
    {
      warble = !(g_tmr10ms % 60);
      beepWarn2();
    }
#endif
  }
#endif

  // These moved here from perOut() [gruvin9x.cpp] to improve beep trigger reliability.
#if defined (BEEPSPKR)
  if(mixWarning & 1) if(((g_tmr10ms&0xFF)==  0)) beepWarn1Spkr(BEEP_DEFAULT_FREQ+7);
  if(mixWarning & 2) if(((g_tmr10ms&0xFF)== 64) 
      || ((g_tmr10ms&0xFF)== 72)) beepWarn1Spkr(BEEP_DEFAULT_FREQ+9);
  if(mixWarning & 4) if(((g_tmr10ms&0xFF)==128) || ((g_tmr10ms&0xFF)==136) 
      || ((g_tmr10ms&0xFF)==144)) beepWarn1Spkr(BEEP_DEFAULT_FREQ+11);
#else
  if(mixWarning & 1) if(((g_tmr10ms&0xFF)==  0)) beepWarn1();
  if(mixWarning & 2) if(((g_tmr10ms&0xFF)== 64) || ((g_tmr10ms&0xFF)== 72)) beepWarn1();
  if(mixWarning & 4) if(((g_tmr10ms&0xFF)==128) || ((g_tmr10ms&0xFF)==136) || ((g_tmr10ms&0xFF)==144)) beepWarn1();
#endif
}
