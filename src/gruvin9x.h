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

#ifndef gruvin9x_h
#define gruvin9x_h

#define VERS 1
#define SUB_VERS 2

#include <inttypes.h>
#include <string.h>

#if defined(PCBV3)
#include "time.h"
#endif

#ifdef SIMU
  #include "simpgmspace.h"
  #define APM
  #include "stdio.h"
#else
///opt/cross/avr/include/avr/pgmspace.h
#include <stddef.h>
#include <avr/io.h>
#define assert(x)
//disable whole pgmspace functionality for all avr-gcc because
//avr-gcc > 4.2.1 does not work anyway
//http://www.mail-archive.com/gcc-bugs@gcc.gnu.org/msg239240.html
//http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
//
//Workarounds:
//
//PSTR is fixed below
//all prog_xx definitions must use APM explicitely

//#define __ATTR_PROGMEM__
#include <avr/pgmspace.h>
#ifdef __cplusplus
#define APM __attribute__(( section(".progmem.data") ))
#undef PSTR
#define PSTR(s) (__extension__({static prog_char APM __c[] = (s);&__c[0];}))
#endif

#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL  // 16 MHz
#include <util/delay.h>
#define pgm_read_adr(address_short) pgm_read_word(address_short)
#include <avr/wdt.h>
#endif

#include "file.h"

extern RlcFile theFile;  //used for any file operation

// G: The following comments relate to the original stock PCB only
//
//                  elev                        thr
//                   LV                         RV
//                 2 ^                        4 ^
//                   1                          2
//                   |     rudd                 |     aile
//              <----X--3-> LH             <----X--0-> RH
//              6    |    7                1    |    0
//                   |                          |
//                 3 v                        5 v
//
//PORTA  7      6       5       4       3       2       1       0
//       O      O       O       O       O       O       O       O
//       ------------------------ LCD_DAT -----------------------
//
//PORTB  7      6       5       4       3       2       1       0
//       O      i       i       i       i       i       i       O
//       light  KEY_LFT KEY_RGT KEY_UP  KEY_DWN KEY_EXT KEY_MEN  PPM
//
//PORTC  7      6       5       4       3       2       1       0
//       -      -       O       O       O       O       O       -
//       NC     NC    LCD_E   LCD_RNW  LCD_A0  LCD_RES LCD_CS1  NC
//
//PORTD  7      6       5       4       3       2       1       0
//       i      i       i       i       i       i       i       i
//     TRM_D_DWN _UP  TRM_C_DWN _UP   TRM_B_DWN _UP   TRM_A_DWN _UP
//
//PORTE  7      6       5       4       3       2       1       0
//       i      i       i       i       O       i       i       i
//     PPM_IN  ID2    Trainer  Gear   Buzzer   ElevDR  AileDR  THRCT
//
//PORTF  7      6       5       4       3       2       1       0
//       ai     ai      ai      ai      ai      ai      ai      ai
// ANA_ BAT   PITT_TRM HOV_THR HOV_PIT  STCK_LH STCK_RV STCK_LV STCK_RH
//                                      rud    thro   elev   aile
//PORTG  7      6       5       4       3       2       1       0
//       -      -       -       O       i               i       i
//                            SIM_CTL  ID1      NC      RF_POW RuddDR

#define PORTA_LCD_DAT  PORTA
#define PORTC_LCD_CTRL PORTC
#define OUT_C_LCD_E     5
#define OUT_C_LCD_RnW   4
#define OUT_C_LCD_A0    3
#define OUT_C_LCD_RES   2
#define OUT_C_LCD_CS1   1

extern uint16_t DEBUG1;
extern uint16_t DEBUG2;

#if defined (PCBV4)

#  define INP_P_SPARE6    7
#  define INP_P_SPARE5    6
#  define INP_P_KEY_EXT   5
#  define INP_P_KEY_MEN   4
#  define INP_P_KEY_LFT   3
#  define INP_P_KEY_RGT   2
#  define INP_P_KEY_UP    1
#  define INP_P_KEY_DWN   0

#  define INP_J_TRM_RH_UP   7
#  define INP_J_TRM_RH_DWN  6
#  define INP_J_TRM_RV_UP   5
#  define INP_J_TRM_RV_DWN  4
#  define INP_J_TRM_LV_UP   3
#  define INP_J_TRM_LV_DWN  2
#  define INP_J_TRM_LH_UP   1
#  define INP_J_TRM_LH_DWN  0

#  define INP_E_PPM_IN      7
#  define INP_E_ROT_ENC_1_B 6
#  define INP_E_ROT_ENC_1_A 5
#  define INP_E_USB_D_PLS   4
#  define OUT_E_BUZZER      3
#  define INP_E_USB_D_NEG   2
#  define INP_E_TELEM_RX    1
#  define OUT_E_TELEM_TX    0

#  define INP_D_SPARE3         7
#  define INP_D_SPARE4         6
#  define INP_D_ROT_ENC_2_PUSH 5
#  define INP_D_ROT_ENC_1_PUSH 4
#  define OUT_D_ROT_ENC_2_B    3
#  define INP_D_ROT_ENC_2_A    2
#  define INP_D_I2C_SCL        1
#  define INP_D_I2C_SDA        0

#  define INP_G_Gear     5
#  define INP_G_ThrCt    2
#  define OUT_G_SIM_CTL  4 //1 : phone-jack=ppm_in
#  define INP_G_ID1      3
#  define INP_G_RF_POW   1
#  define INP_G_RuddDR   0

#  define INP_C_AileDR   7
#  define INP_C_ElevDR   6
#  define OUT_C_LIGHT    0

#  define OUT_B_Speaker  7
#  define OUT_B_PPM      6 // will be switched by TCNT1==OCR1B in hardware
#  define INP_B_Trainer  5
#  define INP_B_ID2      4


#else // boards prior to v4 ...


#  define INP_B_KEY_LFT 6
#  define INP_B_KEY_RGT 5
#  define INP_B_KEY_UP  4
#  define INP_B_KEY_DWN 3
#  define INP_B_KEY_EXT 2
#  define INP_B_KEY_MEN 1
//vinceofdrink@gmail harwared ppm
//Orginal bitbang port for PPM
#  ifndef DPPMPB7_HARDWARE
#    define OUT_B_PPM 0
#  else
#    define	OUT_B_PPM 7 // will not be used
#  endif

#  define INP_D_TRM_LH_UP   7
#  define INP_D_TRM_LH_DWN  6
#  define INP_D_TRM_RV_DWN  5
#  define INP_D_TRM_RV_UP   4
#  define INP_D_TRM_LV_DWN  3
#  define INP_D_TRM_LV_UP   2
#  define INP_D_TRM_RH_DWN  1
#  define INP_D_TRM_RH_UP   0

#  if defined (PCBV3)
#    define OUT_C_LIGHT   0
#  else
#    ifndef DPPMPB7_HARDWARE
#      define OUT_B_LIGHT   7
#    else
#      define OUT_B_LIGHT   0
#    endif
#  endif

#  define INP_E_PPM_IN  7
#  define INP_E_ID2     6
#  define INP_E_Trainer 5
#  define INP_E_Gear    4
#  define OUT_E_BUZZER  3
#  define INP_E_ElevDR  2
#  define INP_E_AileDR  1
#  define INP_E_ThrCt   0

#  if defined(JETI) || defined(FRSKY)
#    undef INP_E_ThrCt
#    undef INP_E_AileDR
#    define INP_C_ThrCt   6
#    define INP_C_AileDR  7
#  endif

#  define OUT_G_SIM_CTL  4 //1 : phone-jack=ppm_in
#  define INP_G_ID1      3
#  define INP_G_RF_POW   1
#  define INP_G_RuddDR   0

// Legacy support for USART1 hardware mod [DEPRECATED]
// G: This board will be retired before much longer. But I still need it for now.
#if defined(USART1FREED)
// do not undef the original INP_D_TRM_LV_DWN/UP as we need them later
  #define INP_C_TRM_LV_UP  0
  #define INP_G_TRM_LV_DWN 2
#endif

#endif // defined (PCBV4)

#if defined (BEEPSPKR)
#define BEEP_KEY_TIME 5
#define BEEP_DEFAULT_FREQ 50
#define BEEP_KEY_UP_FREQ 55
#define BEEP_KEY_DOWN_FREQ 45
#endif

#define SLAVE_MODE (PING & (1<<INP_G_RF_POW))

const uint8_t modn12x3[4][4]= {
  {1, 2, 3, 4},
  {1, 3, 2, 4},
  {4, 2, 3, 1},
  {4, 3, 2, 1} };

//R=1
//E=2
//T=3
//A=4

const uint8_t chout_ar[24][4] = { //First number is 0..23 -> template setup,  Second is relevant channel out
{1,2,3,4},{1,2,4,3},{1,3,2,4},{1,3,4,2},{1,4,2,3},{1,4,3,2},
{2,1,3,4},{2,1,4,3},{2,3,1,4},{2,3,4,1},{2,4,1,3},{2,4,3,1},
{3,1,2,4},{3,1,4,2},{3,2,1,4},{3,2,4,1},{3,4,1,2},{3,4,2,1},
{4,1,2,3},{4,1,3,2},{4,2,1,3},{4,2,3,1},{4,3,1,2},{4,3,2,1}    };

//convert from mode 1 to mode g_eeGeneral.stickMode
//NOTICE!  =>  1..4 -> 1..4
#define CONVERT_MODE(x)  (((x)<=4) ? modn12x3[g_eeGeneral.stickMode][((x)-1)] : (x))
#define CHANNEL_ORDER(x) (chout_ar[g_eeGeneral.templateSetup][(x)-1])
#define THR_STICK       (2-(g_eeGeneral.stickMode&1))
#define ELE_STICK       (1+(g_eeGeneral.stickMode&1))
#define AIL_STICK       ((g_eeGeneral.stickMode&2) ? 0 : 3)
#define RUD_STICK       ((g_eeGeneral.stickMode&2) ? 3 : 0)

enum EnumKeys {
  KEY_MENU ,
  KEY_EXIT ,
  KEY_DOWN ,
  KEY_UP  ,
  KEY_RIGHT ,
  KEY_LEFT ,
  TRM_LH_DWN  ,
  TRM_LH_UP   ,
  TRM_LV_DWN  ,
  TRM_LV_UP   ,
  TRM_RV_DWN  ,
  TRM_RV_UP   ,
  TRM_RH_DWN  ,
  TRM_RH_UP   ,
  //SW_NC     ,
  //SW_ON     ,
  SW_ThrCt  ,
  SW_RuddDR ,
  SW_ElevDR ,
  SW_ID0    ,
  SW_ID1    ,
  SW_ID2    ,
  SW_AileDR ,
  SW_Gear   ,
  SW_Trainer
};

#define SWITCHES_STR "THR""RUD""ELE""ID0""ID1""ID2""AIL""GEA""TRN""SW1""SW2""SW3""SW4""SW5""SW6""SW7""SW8""SW9""SWA""SWB""SWC"

#define CURV_STR "---x>0x<0|x|f>0f<0|f|c1 c2 c3 c4 c5 c6 c7 c8 c9 c10c11c12c13c14c15c16"
#define CURVE_BASE 7

#define CSWITCH_STR  "----   v>ofs  v<ofs  |v|>ofs|v|<ofsAND    OR     XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 "
#define CSW_LEN_FUNC 7

#define FSWITCH_STR  "----          ""Trainer       ""Trainer RUD   ""Trainer ELE   ""Trainer THR   ""Trainer AIL   ""Instant Trim  ""Trims2Offsets ""Telemetry View"
#define FSW_LEN_FUNC 14

#define SWASH_TYPE_STR   "---   ""120   ""120X  ""140   ""90    "

#define SWASH_TYPE_120   1
#define SWASH_TYPE_120X  2
#define SWASH_TYPE_140   3
#define SWASH_TYPE_90    4
#define SWASH_TYPE_NUM   4

#define CS_OFF       0
#define CS_VPOS      1  //v>offset
#define CS_VNEG      2  //v<offset
#define CS_APOS      3  //|v|>offset
#define CS_ANEG      4  //|v|<offset
#define CS_AND       5
#define CS_OR        6
#define CS_XOR       7
#define CS_EQUAL     8
#define CS_NEQUAL    9
#define CS_GREATER   10
#define CS_LESS      11
#define CS_EGREATER  12
#define CS_ELESS     13
#define CS_MAXF      13  //max function

#define CS_VOFS       0
#define CS_VBOOL      1
#define CS_VCOMP      2
#define CS_STATE(x)   ((x)<CS_AND ? CS_VOFS : ((x)<CS_EQUAL ? CS_VBOOL : CS_VCOMP))

//#define SW_BASE      SW_NC
#define SW_BASE      SW_ThrCt
#define SW_BASE_DIAG SW_ThrCt
//#define SWITCHES_STR "  NC  ON THR RUD ELE ID0 ID1 ID2 AILGEARTRNR"
#define MAX_PSWITCH   (SW_Trainer-SW_ThrCt+1)  // 9 physical switches
#define MAX_SWITCH    (1+MAX_PSWITCH+NUM_CSW)  // 22(1+9+12) !switches + 0 + 22 switches: 6 bits needed
#define MAX_DRSWITCH  (MAX_PSWITCH+NUM_CSW/2)  // 15(9+6) !switches + 0 + 15 switches: 5 bits needed

#define NUM_STICKS      4
#define NUM_POTS        3
#define PPM_BASE        (MIX_FULL+3/*CYC1-CYC3*/) // because srcRaw is shifted +1!
#define NUM_CAL_PPM     4
#define NUM_PPM         8
#define CHOUT_BASE      (PPM_BASE+NUM_PPM)

#ifdef FRSKY
#define NUM_TELEMETRY 2
#define TELEMETRY_CHANNELS "AD1 AD2 "
#else
#define NUM_TELEMETRY 0
#define TELEMETRY_CHANNELS ""
#endif

#define DSW_THR  1
#define DSW_RUD  2
#define DSW_ELE  3
#define DSW_ID0  4
#define DSW_ID1  5
#define DSW_ID2  6
#define DSW_AIL  7
#define DSW_GEA  8
#define DSW_TRN  9
#define DSW_SW1  10
#define DSW_SW2  11
#define DSW_SW3  12
#define DSW_SW4  13
#define DSW_SW5  14
#define DSW_SW6  15

#define THRCHK_DEADBAND 16
#define SPLASH_TIMEOUT  (4*100)  //400 msec - 4 seconds

#define NUM_KEYS TRM_RH_UP+1
#define TRM_BASE TRM_LH_DWN

//#define _MSK_KEY_FIRST (_MSK_KEY_REPT|0x20)
//#define EVT_KEY_GEN_BREAK(key) ((key)|0x20)
#define _MSK_KEY_REPT    0x40
#define _MSK_KEY_DBL     0x10
#define IS_KEY_BREAK(key)  (((key)&0xf0)        ==  0x20)
#define EVT_KEY_BREAK(key) ((key)|                  0x20)
#define EVT_KEY_FIRST(key) ((key)|    _MSK_KEY_REPT|0x20)
#define EVT_KEY_REPT(key)  ((key)|    _MSK_KEY_REPT     )
#define EVT_KEY_LONG(key)  ((key)|0x80)
#define EVT_KEY_DBL(key)   ((key)|_MSK_KEY_DBL)
//#define EVT_KEY_DBL(key)   ((key)|0x10)
#define EVT_ENTRY               (0xff - _MSK_KEY_REPT)
#define EVT_ENTRY_UP            (0xfe - _MSK_KEY_REPT)
#define EVT_KEY_MASK             0x0f


#define TMRMODE_NONE     0
#define TMRMODE_ABS      1
#define TMRMODE_THR      2
#define TMRMODE_THR_REL  3
#define MAX_ALERT_TIME   60

#define PROTO_PPM        0
#define PROTO_SILV_A     1
#define PROTO_SILV_B     2
#define PROTO_SILV_C     3
#define PROTO_TRACER_CTP1009 4
#define PROT_MAX         4
#define PROT_STR "PPM   SILV_ASILV_BSILV_CTRAC09"
#define PROT_STR_LEN     6

typedef void (*getADCp)();

#define ZCHAR_MAX 40

#ifdef TRANSLATIONS
extern int8_t char2idx(char c);
#endif
extern char idx2char(int8_t idx);

/// stoppt alle events von dieser taste bis eine kurze Zeit abgelaufen ist
void pauseEvents(uint8_t enuk);
/// liefert die Zahl der schnellen Wiederholungen dieser Taste
uint8_t getEventDbl(uint8_t event);
/// stoppt alle events von dieser taste bis diese wieder losgelassen wird
void    killEvents(uint8_t enuk);
/// liefert den Wert einer beliebigen Taste KEY_MENU..SW_Trainer
bool    keyState(EnumKeys enuk);
/// Liefert das naechste Tasten-Event, auch trim-Tasten.
/// Das Ergebnis hat die Form:
/// EVT_KEY_BREAK(key), EVT_KEY_FIRST(key), EVT_KEY_REPT(key) oder EVT_KEY_LONG(key)
uint8_t getEvent();
void putEvent(uint8_t evt);
#if defined (PCBV3)
extern uint8_t keyDown();
#endif

/// Gibt Alarm Maske auf lcd aus.
/// Die Maske wird so lange angezeigt bis eine beliebige Taste gedrueckt wird.
void alert(const prog_char * s, bool defaults=false);
void message(const prog_char * s);
/// periodisches Hauptprogramm
void    perMain();
/// Bearbeitet alle zeitkritischen Jobs.
/// wie z.B. einlesen aller Eingaenge, Entprellung, Key-Repeat..
void    per10ms();
/// Erzeugt periodisch alle Outputs ausser Bildschirmausgaben.
void zeroVariables();

#define NO_TRAINER 0x01
#define NO_INPUT   0x02
extern void perOut(int16_t *chanOut, uint8_t att);

///   Liefert den Zustand des Switches 'swtch'. Die Numerierung erfolgt ab 1
///   (1=SW_ON, 2=SW_ThrCt, 10=SW_Trainer). 0 Bedeutet not conected.
///   Negative Werte  erzeugen invertierte Ergebnisse.
///   Die Funktion putsSwitches(..) erzeugt den passenden Ausdruck.
///
///   \param swtch
///     0                : not connected. Liefert den Wert 'nc'
///     1.. MAX_SWITCH : SW_ON .. SW_Trainer
///    -1..-MAX_SWITCH : negierte Werte
///   \param nc Wert, der bei swtch==0 geliefert wird.
bool    getSwitch(int8_t swtch, bool nc, uint8_t level=0);
/// Zeigt den Namen des Switches 'swtch' im display an
///   \param x     x-koordinate 0..127
///   \param y     y-koordinate 0..63 (nur durch 8 teilbar)
///   \param swtch -MAX_SWITCH ..  MAX_SWITCH
///   \param att   NO_INV,INVERS,BLINK
///

uint8_t getFlightPhase();
uint8_t getTrimFlightPhase(uint8_t idx, int8_t phase=-1);
extern int16_t getTrimValue(uint8_t phase, uint8_t idx);
extern void setTrimValue(uint8_t phase, uint8_t idx, int16_t trim);

extern uint16_t s_timeCumTot;
extern uint16_t s_timeCumAbs;  //laufzeit in 1/16 sec
extern uint16_t s_timeCumSw;  //laufzeit in 1/16 sec
extern uint16_t s_timeCumThr;  //gewichtete laufzeit in 1/16 sec
extern uint16_t s_timeCum16ThrP; //gewichtete laufzeit in 1/16 sec
extern uint8_t  s_timerState;
extern int16_t  s_timerVal[2];

extern uint8_t trimsCheckTimer;

#define TMR_OFF     0
#define TMR_RUNNING 1
#define TMR_BEEPING 2
#define TMR_STOPPED 3
void resetTimer1();

extern uint8_t Timer2_running ;
extern uint16_t timer2 ;
void resetTimer2() ;

extern uint16_t g_tmr1Latency_max;
extern uint16_t g_tmr1Latency_min;
extern uint16_t g_timeMain;
extern uint16_t g_time_per10;

#define MAXTRACE 120
extern uint8_t s_traceBuf[MAXTRACE];
extern uint16_t s_traceWr;
extern uint16_t s_traceCnt;

const prog_char *get_switches_string() ;

uint16_t getTmr16KHz();
uint16_t stack_free();

void checkLowEEPROM();
void checkTHR();
void checkSwitches();

#define GETADC_SING = 0
#define GETADC_OSMP = 1
#define GETADC_FILT = 2

void getADC_single();
void getADC_osmp();
void getADC_filt();

// checkIncDec flags
#define   EE_GENERAL 0x01
#define   EE_MODEL   0x02

extern bool warble;

extern uint8_t  s_eeDirtyMsk;

#define STORE_MODELVARS eeDirty(EE_MODEL)
#define STORE_GENERALVARS eeDirty(EE_GENERAL)

#if defined (PCBV3)
#define BACKLIGHT_ON    PORTC |=  (1<<OUT_C_LIGHT)
#define BACKLIGHT_OFF   PORTC &= ~(1<<OUT_C_LIGHT)
#else
#define BACKLIGHT_ON    PORTB |=  (1<<OUT_B_LIGHT)
#define BACKLIGHT_OFF   PORTB &= ~(1<<OUT_B_LIGHT)
#endif

#define BITMASK(bit) (1<<(bit))

/// liefert Dimension eines Arrays
#define DIM(arr) (sizeof((arr))/sizeof((arr)[0]))

/// liefert Betrag des Arguments
template<class t> inline t abs(t a){ return a>0?a:-a; }
/// liefert das Minimum der Argumente
template<class t> inline t min(t a, t b){ return a<b?a:b; }
/// liefert das Maximum der Argumente
template<class t> inline t max(t a, t b){ return a>b?a:b; }
template<class t> inline int8_t sgn(t a){ return a>0 ? 1 : (a < 0 ? -1 : 0); }

/// Markiert einen EEPROM-Bereich als dirty. der Bereich wird dann in
/// eeCheck ins EEPROM zurueckgeschrieben.
void eeWriteBlockCmp(const void *i_pointer_ram, void *i_pointer_eeprom, size_t size);
void eeDirty(uint8_t msk);
#ifdef EEPROM_ASYNC_WRITE
inline void eeFlush() { theFile.flush(); }
#endif
void eeCheck(bool immediately=false);
void eeReadAll();
bool eeModelExists(uint8_t id);
uint16_t eeLoadModelName(uint8_t id, char *name);
void eeLoadModel(uint8_t id);
int8_t eeDuplicateModel(uint8_t id, bool down=true);

///number of real input channels (1-9) plus virtual input channels X1-X4
#define NUM_XCHNRAW (NUM_STICKS+NUM_POTS+2/*MAX/FULL*/+3/*CYC1-CYC3*/+NUM_PPM+NUM_CHNOUT+NUM_TELEMETRY)
///number of real output channels (CH1-CH8) plus virtual output channels X1-X4
#define NUM_XCHNOUT (NUM_CHNOUT) //(NUM_CHNOUT)//+NUM_VIRT)

extern inline int16_t calc100toRESX(int8_t x)
{
  // return (int16_t)x*10 + x/4 - x/64;
  return ((x*41)>>2) - x/64;
}

extern inline int16_t calc1000toRESX(int16_t x)
{
  // return x + x/32 - x/128 + x/512;
  int16_t y = x>>5;
  x+=y;
  y=y>>2;
  x-=y;
  return x+(y>>2);
}

extern volatile uint16_t g_tmr10ms;

extern inline uint16_t get_tmr10ms()
{
  uint16_t time  ;
  cli();
  time = g_tmr10ms ;  
  sei();
  return time ;
}

#define TMR_VAROFS  16

#define SUB_MODE_V     1
#define SUB_MODE_H     2
#define SUB_MODE_H_DBL 3

void setupPulses();
void setupPulsesPPM();
void setupPulsesSilver();
void setupPulsesTracerCtp1009();

void initTemplates();

#include "lcd.h"
extern const char stamp1[];
extern const char stamp2[];
extern const char stamp3[];
extern const char stamp4[];
extern const char stamp5[];
#include "myeeprom.h"

#ifdef JETI
// Jeti-DUPLEX Telemetry
extern uint16_t jeti_keys;
#include "jeti.h"
#endif

#if defined (FRSKY)
// FrSky Telemetry
#include "frsky.h"
#endif

#ifndef BATT_UNSTABLE_BANDGAP
extern uint16_t           abRunningAvg;
extern uint8_t            g_vbat100mV;
#else
extern uint16_t           g_vbat100mV;
#endif
extern volatile uint16_t  g_tmr10ms;
extern volatile uint8_t   g_blinkTmr10ms;
extern uint8_t            g_beepCnt;
extern uint8_t            g_beepVal[5];
extern const PROGMEM char modi12x3[];
extern uint16_t           pulses2MHz[120];
extern int16_t            g_ppmIns[8];
extern int16_t            g_chans512[NUM_CHNOUT];
extern volatile uint8_t   tick10ms;
extern uint16_t           BandGap;

extern uint16_t expou(uint16_t x, uint16_t k);
extern int16_t expo(int16_t x, int16_t k);
extern int16_t intpol(int16_t, uint8_t);
extern int16_t applyCurve(int16_t, uint8_t, uint8_t srcRaw);
extern void applyExpos(int16_t *anas);

extern uint16_t anaIn(uint8_t chan);
extern int16_t calibratedStick[7];
extern int16_t ex_chans[NUM_CHNOUT];

#define FLASH_DURATION 50

extern uint8_t  beepAgain;
extern uint16_t g_LightOffCounter;
extern uint8_t mixWarning;

/// Erzeugt einen beep der laenge b
inline void _beep(uint8_t b) {
  g_beepCnt=b;
}

extern uint8_t toneFreq;
#if defined (PCBV3) && defined(BEEPSPKR)
inline void _beepSpkr(uint8_t d, uint8_t f)
{
  g_beepCnt=d;
  OCR0A = (5000 / f); // sticking with old values approx 20(abs. min) to 90, 60 being the default tone(?).
}
#elif defined (BEEPSPKR)
inline void _beepSpkr(uint8_t d, uint8_t f)
{
  g_beepCnt=d;
  toneFreq=f;
}
#endif

#if defined (BEEPSPKR)

#define beepKeySpkr(freq) _beepSpkr(g_beepVal[0],freq)
#define beepTrimSpkr(freq) _beepSpkr(g_beepVal[0],freq)
#define beepWarn1Spkr(freq) _beepSpkr(g_beepVal[1],freq)
#define beepWarn2Spkr(freq) _beepSpkr(g_beepVal[2],freq)
#define beepKey() _beepSpkr(g_beepVal[0],BEEP_DEFAULT_FREQ)
#define beepWarn() _beepSpkr(g_beepVal[3],BEEP_DEFAULT_FREQ)
#define beepWarn1() _beepSpkr(g_beepVal[1],BEEP_DEFAULT_FREQ)
#define beepWarn2() _beepSpkr(g_beepVal[2],BEEP_DEFAULT_FREQ)
#define beepErr()  _beepSpkr(g_beepVal[4],BEEP_DEFAULT_FREQ)

#else

/// Erzeugt einen kurzen beep
#define beepKey()   _beep(g_beepVal[0])
#define beepWarn() _beep(g_beepVal[3])
#define beepWarn1() _beep(g_beepVal[1])
#define beepWarn2() _beep(g_beepVal[2])
#define beepErr()  _beep(g_beepVal[4])

#endif

// MM/SD card Disk IO Support
#if defined (PCBV3)
#include "rtc.h"
extern void disk_timerproc(void);
extern time_t g_unixTime; // global unix timestamp -- hold current time in seconds since 1970-01-01 00:00:00 
extern uint8_t g_ms100; // defined in drivers.cpp
#endif

extern PhaseData *phaseaddress(uint8_t idx);
extern ExpoData *expoaddress(uint8_t idx);
extern MixData *mixaddress(uint8_t idx);
extern LimitData *limitaddress(uint8_t idx);

extern void incSubtrim(uint8_t idx, int16_t inc);
extern void instantTrim();
extern void moveTrimsToOffsets(); // move state of 3 primary trims to offsets

extern uint16_t active_functions;
inline bool isFunctionActive(uint8_t func)
{
  return active_functions & (1 << (func-1));
}

#if defined (PCBV3)
extern char g_logFilename[21]; // pers.cpp::resetTelemetry()
extern FATFS FATFS_Obj; // pers.cpp::resetTelemetry()
extern FIL g_oLogFile; // pers.cpp::resetTelemetry()
#endif

#endif // gruvin9x_h
/*eof*/
