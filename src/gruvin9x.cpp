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
#include "s9xsplash.lbm"
#include "menus.h"

// MM/SD card Disk IO Support
#if defined (PCBV3)
#include "integer.h"
#include "time.h"
#include "rtc.h"
#include "ff.h"
#include "diskio.h"
time_t g_unixTime; // Global date/time register, incremented each second in per10ms()
#endif

/*
mode1 rud ele thr ail
mode2 rud thr ele ail
mode3 ail ele thr rud
mode4 ail thr ele rud
*/

EEGeneral  g_eeGeneral;
ModelData  g_model;

uint16_t g_tmr1Latency_max;
uint16_t g_tmr1Latency_min = 0x7ff;
uint16_t g_timeMain;
uint16_t g_time_per10;

#if defined (PCBSTD) && defined (BEEPSPKR)
uint8_t toneFreq = BEEP_DEFAULT_FREQ;
uint8_t toneOn = false;
#endif

bool warble = false;

const prog_char APM modi12x3[]=
  "RUD ELE THR AIL "
  "RUD THR ELE AIL "
  "AIL ELE THR RUD "
  "AIL THR ELE RUD ";

const prog_char APM s_charTab[] = "_-.,";

#ifdef TRANSLATIONS
int8_t char2idx(char c)
{
  if (c==' ') return 0;
  if (c>='A' && c<='Z') return 1+c-'A';
  if (c>='a' && c<='z') return -1-c+'a';
  if (c>='0' && c<='9') return 27+c-'0';
  for (int8_t i=0;;i++) {
    char cc = pgm_read_byte(s_charTab+i);
    if(cc==c) return 37+i;
    if(cc==0) return 0;
  }
}
#endif

char idx2char(int8_t idx)
{
  if (idx == 0) return ' ';
  if (idx < 0) {
    if (idx > -27) return 'a' - idx - 1;
    idx = -idx;
  }
  if (idx < 27) return 'A' + idx - 1;
  if (idx < 37) return '0' + idx - 27;
  if (idx <= ZCHAR_MAX) return pgm_read_byte(s_charTab+idx-37);
  return ' ';
}

PhaseData *phaseaddress(uint8_t idx)
{
  return &g_model.phaseData[idx];
}

ExpoData *expoaddress(uint8_t idx )
{
  return &g_model.expoData[idx];
}

MixData *mixaddress(uint8_t idx)
{
  return &g_model.mixData[idx];
}

LimitData *limitaddress(uint8_t idx)
{
  return &g_model.limitData[idx];
}

int16_t intpol(int16_t x, uint8_t idx) // -100, -75, -50, -25, 0 ,25 ,50, 75, 100
{
#define D9 (RESX * 2 / 8)
#define D5 (RESX * 2 / 4)
  bool    cv9 = idx >= MAX_CURVE5;
  int8_t *crv = cv9 ? g_model.curves9[idx-MAX_CURVE5] : g_model.curves5[idx];
  int16_t erg;

  x+=RESXu;
  if(x < 0) {
    erg = (int16_t)crv[0] * (RESX/4);
  } else if(x >= (RESX*2)) {
    erg = (int16_t)crv[(cv9 ? 8 : 4)] * (RESX/4);
  } else {
    int16_t a,dx;
    if(cv9){
      a   = (uint16_t)x / D9;
      dx  =((uint16_t)x % D9) * 2;
    } else {
      a   = (uint16_t)x / D5;
      dx  = (uint16_t)x % D5;
    }
    erg  = (int16_t)crv[a]*((D5-dx)/2) + (int16_t)crv[a+1]*(dx/2);
  }
  return erg / 25; // 100*D5/RESX;
}

int16_t applyCurve(int16_t x, uint8_t idx, uint8_t srcRaw)
{
  switch(idx) {
  case 0:
    return x;
  case 1:
    if (srcRaw == MIX_FULL) { //FULL
      if (x<0 ) x=-RESX;   //x|x>0
      else x=-RESX+2*x;
    }
    else {
      if (x<0) x=0;   //x|x>0
    }
    return x;
  case 2:
    if (srcRaw == MIX_FULL) { //FULL
      if (x>0) x=RESX;   //x|x<0
      else x=RESX+2*x;
    }
    else {
      if (x>0) x=0;   //x|x<0
    }
    return x;
  case 3:       // x|abs(x)
    return abs(x);
  case 4:       //f|f>0
    return x>0 ? RESX : 0;
  case 5:       //f|f<0
    return x<0 ? -RESX : 0;
  case 6:       //f|abs(f)
    return x>0 ? RESX : -RESX;
  }
  return intpol(x, idx-7);
}

// expo-funktion:
// ---------------
// kmplot
// f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
// f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
// f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]

uint16_t expou(uint16_t x, uint16_t k)
{
  // k*x*x*x + (1-k)*x
  return ((unsigned long)x*x*x/0x10000*k/(RESXul*RESXul/0x10000) + (RESKul-k)*x+RESKul/2)/RESKul;
}

int16_t expo(int16_t x, int16_t k)
{
  if(k == 0) return x;
  int16_t   y;
  bool    neg =  x < 0;
  if(neg)   x = -x;
  if(k<0){
    y = RESXu-expou(RESXu-x,-k);
  }else{
    y = expou(x,k);
  }
  return neg? -y:y;
}

#ifdef EXTENDED_EXPO
/// expo with y-offset
class Expo
{
  uint16_t   c;
  int16_t    d,drx;
public:
  void     init(uint8_t k, int8_t yo);
  static int16_t  expou(uint16_t x,uint16_t c, int16_t d);
  int16_t  expo(int16_t x);
};
void    Expo::init(uint8_t k, int8_t yo)
{
  c = (uint16_t) k  * 256 / 100;
  d = (int16_t)  yo * 256 / 100;
  drx = d * ((uint16_t)RESXu/256);
}
int16_t Expo::expou(uint16_t x,uint16_t c, int16_t d)
{
  uint16_t a = 256 - c - d;
  if( (int16_t)a < 0 ) a = 0;
  // a x^3 + c x + d
  //                         9  18  27        11  20   18
  uint32_t res =  ((uint32_t)x * x * x / 0x10000 * a / (RESXul*RESXul/0x10000) +
                   (uint32_t)x                   * c
  ) / 256;
  return (int16_t)res;
}
int16_t  Expo::expo(int16_t x)
{
  if(c==256 && d==0) return x;
  if(x>=0) return expou(x,c,d) + drx;
  return -expou(-x,c,-d) + drx;
}
#endif


void applyExpos(int16_t *anas)
{
  static int16_t anas2[4]; // values before expo, to ensure same expo base when multiple expo lines are used
  memcpy(anas2, anas, sizeof(anas2));

  uint8_t phase = getFlightPhase();

  for (uint8_t i=0; i<DIM(g_model.expoData); i++) {
    ExpoData &ed = g_model.expoData[i];
    if (ed.mode==0) break; // end of list
    if (ed.phase != 0) {
      if (ed.negPhase) {
        if (phase+1 == -ed.phase)
          continue;
      }
      else {
        if (phase+1 != ed.phase)
          continue;
      }
    }
    if (getSwitch(ed.swtch, 1)) {
      int16_t v = anas2[ed.chn];
      if((v<0 && ed.mode&1) || (v>=0 && ed.mode&2)) {
        int16_t k = ed.expo;
        if (IS_THROTTLE(ed.chn) && g_model.thrExpo)
          v = 2*expo((v+RESX)/2, k);
        else
          v = expo(v, k);
        if (ed.curve) v = applyCurve(v, ed.curve, 0);
        v = ((int32_t)v * ed.weight) / 100;
        if (IS_THROTTLE(ed.chn) && g_model.thrExpo) v -= RESX;
        anas[ed.chn] = v;
      }
    }
  }
}

const prog_char *get_switches_string()
{
  return PSTR(SWITCHES_STR);
}

static bool s_noStickInputs = false;
inline int16_t getValue(uint8_t i)
{
    if(i<NUM_STICKS+NUM_POTS) return (s_noStickInputs ? 0 : calibratedStick[i]);
    else if(i<MIX_FULL/*srcRaw is shifted +1!*/) return 1024; //FULL/MAX
    else if(i<PPM_BASE+NUM_CAL_PPM) return (g_ppmIns[i-PPM_BASE] - g_eeGeneral.trainer.calib[i-PPM_BASE])*2;
    else if(i<PPM_BASE+NUM_PPM) return g_ppmIns[i-PPM_BASE]*2;
    else if(i<CHOUT_BASE+NUM_CHNOUT) return ex_chans[i-CHOUT_BASE];
#ifdef FRSKY
    else if(i<CHOUT_BASE+NUM_CHNOUT+NUM_TELEMETRY) return frskyTelemetry[i-CHOUT_BASE-NUM_CHNOUT].value;
#endif
    else return 0;
}

uint8_t getFlightPhase()
{
  for (uint8_t i=1; i<MAX_PHASES; i++) {
    PhaseData *phase = &g_model.phaseData[i];
    if (phase->swtch && getSwitch(phase->swtch, 0)) {
      return i;
    }
  }
  return 0;
}

uint8_t getTrimFlightPhase(uint8_t idx, int8_t phase)
{
  if (phase == -1) phase = getFlightPhase();

  for (uint8_t i=0; i<MAX_PHASES; i++) {
    if (phase == 0) return 0;
    int8_t trim = g_model.phaseData[phase].trim[idx];
    if (trim > TRIM_MAX) return 0;
    if (trim >= TRIM_MIN) return phase;
    uint8_t result = 129 + trim;
    if (result >= phase) result++;
    phase = result;
  }
  return 0;
}

bool getSwitch(int8_t swtch, bool nc, uint8_t level)
{
  if(level>5) return false; //prevent recursive loop going too deep

  switch(swtch){
    case  0:            return  nc;
    case  MAX_SWITCH: return  true;
    case -MAX_SWITCH: return  false;
  }

  uint8_t dir = swtch>0;
  if(abs(swtch)<(MAX_SWITCH-NUM_CSW)) {
    if(!dir) return ! keyState((EnumKeys)(SW_BASE-swtch-1));
    return            keyState((EnumKeys)(SW_BASE+swtch-1));
  }

  //custom switch, Issue 78
  //use putsChnRaw
  //input -> 1..4 -> sticks,  5..8 pots
  //MAX,FULL - disregard
  //ppm
  CustomSwData &cs = g_model.customSw[abs(swtch)-(MAX_SWITCH-NUM_CSW)];
  if(!cs.func) return false;


  int8_t a = cs.v1;
  int8_t b = cs.v2;
  int16_t x = 0;
  int16_t y = 0;

  // init values only if needed
  uint8_t s = CS_STATE(cs.func);
  if(s == CS_VOFS)
  {
      x = getValue(cs.v1-1);
#ifdef FRSKY
      if (cs.v1 > CHOUT_BASE+NUM_CHNOUT)
        y = 125+cs.v2;
      else
#endif
        y = calc100toRESX(cs.v2);
  }
  else if(s == CS_VCOMP)
  {
      x = getValue(cs.v1-1);
      y = getValue(cs.v2-1);
  }

  switch (cs.func) {
  case (CS_VPOS):
      return swtch>0 ? (x>y) : !(x>y);
      break;
  case (CS_VNEG):
      return swtch>0 ? (x<y) : !(x<y);
      break;
  case (CS_APOS):
      {
        bool res = (abs(x)>y) ;
        return swtch>0 ? res : !res ;
      }
      break;
  case (CS_ANEG):
      {
        bool res = (abs(x)<y) ;
        return swtch>0 ? res : !res ;
      }
      break;

  case (CS_AND):
  case (CS_OR):
  case (CS_XOR):
  {
    bool res1 = getSwitch(a,0,level+1) ;
    bool res2 = getSwitch(b,0,level+1) ;
    if ( cs.func == CS_AND )
    {
      return res1 && res2 ;
    }
    else if ( cs.func == CS_OR )
    {
      return res1 || res2 ;
    }
    else  // CS_XOR
    {
      return res1 ^ res2 ;
    }
  }
  break;
  case (CS_EQUAL):
      return (x==y);
      break;
  case (CS_NEQUAL):
      return (x!=y);
      break;
  case (CS_GREATER):
      return (x>y);
      break;
  case (CS_LESS):
      return (x<y);
      break;
  case (CS_EGREATER):
      return (x>=y);
      break;
  case (CS_ELESS):
      return (x<=y);
      break;
  default:
      return false;
      break;
  }

}


//#define CS_EQUAL     8
//#define CS_NEQUAL    9
//#define CS_GREATER   10
//#define CS_LESS      11
//#define CS_EGREATER  12
//#define CS_ELESS     13

#if defined (PCBV3) && !defined (PCBV4)
// The ugly scanned keys thing should be gone for PCBV4+. In the meantime ...
uint8_t keyDown()
{
  uint8_t in;
  PORTD = ~1; // select KEY_Y0 row (Bits 3:2 EXIT:MENU)
  _delay_us(1);
  in = (~PIND & 0b11000000) >> 5;
  PORTD = ~2; // select Y1 row. (Bits 3:0 Left/Right/Up/Down)
  _delay_us(1);
  in |= (~PIND & 0xf0) >> 1;
  PORTD = 0xff;
  return (in);
}
#else
inline uint8_t keyDown()
{
#if defined (PCBV4)
  return (~PINL) & 0x3F;
#else
  return (~PINB) & 0x7E;
#endif
}
#endif

void clearKeyEvents()
{
    while(keyDown());  // loop until all keys are up
    putEvent(0);
}

void doSplash()
{
    if(!g_eeGeneral.disableSplashScreen)
    {
      if(getSwitch(g_eeGeneral.lightSw,0) || g_eeGeneral.lightAutoOff)
        BACKLIGHT_ON;
      else
        BACKLIGHT_OFF;

      lcd_clear();
      lcd_img(0, 0, s9xsplash,0,0);
      refreshDiplay();
      lcdSetRefVolt(g_eeGeneral.contrast);
      clearKeyEvents();

#ifndef SIMU
      for(uint8_t i=0; i<32; i++)
        getADC_filt(); // init ADC array
#endif

#define INAC_DEVISOR 256   // Bypass splash screen with stick movement
      uint16_t inacSum = 0;
      for(uint8_t i=0; i<4; i++)
        inacSum += anaIn(i)/INAC_DEVISOR;

      uint16_t tgtime = get_tmr10ms() + SPLASH_TIMEOUT;  //2sec splash screen
      while(tgtime != get_tmr10ms())
      {
#ifndef SIMU
        getADC_filt();
#endif
        uint16_t tsum = 0;
        for(uint8_t i=0; i<4; i++)
          tsum += anaIn(i)/INAC_DEVISOR;

        if(keyDown() || (tsum!=inacSum))   return;  //wait for key release

        if(getSwitch(g_eeGeneral.lightSw,0) || g_eeGeneral.lightAutoOff)
          BACKLIGHT_ON;
        else
          BACKLIGHT_OFF;
      }
    }
}

void checkMem()
{
  if(g_eeGeneral.disableMemoryWarning) return;
  if(EeFsGetFree() < 200)
  {
    alert(PSTR("EEPROM low mem"));
  }
}

void alertMessages( const prog_char * s, const prog_char * t )
{
  lcd_clear();
  lcd_putsAtt(64-5*FW,0*FH,PSTR("ALERT"),DBLSIZE);
  lcd_puts_P(0,4*FH,s);
  lcd_puts_P(0,5*FH,t);
  lcd_puts_P(0,6*FH,  PSTR("Press any key to skip") ) ;
  refreshDiplay();
  lcdSetRefVolt(g_eeGeneral.contrast);

  clearKeyEvents();
}

void checkTHR()
{
  if(g_eeGeneral.disableThrottleWarning) return;

  int thrchn=(2-(g_eeGeneral.stickMode&1));//stickMode=0123 -> thr=2121

  int16_t lowLim = THRCHK_DEADBAND + g_eeGeneral.calibMid[thrchn] - g_eeGeneral.calibSpanNeg[thrchn];

#ifndef SIMU
  getADC_single();   // if thr is down - do not display warning at all
#endif
  int16_t v = anaIn(thrchn);
  if(v<=lowLim) return;

  // first - display warning
  alertMessages( PSTR("Throttle not idle"), PSTR("Reset throttle") ) ;

  //loop until all switches are reset
  while (1)
  {
#ifndef SIMU
      getADC_single();
#endif
      int16_t v = anaIn(thrchn);

      if(v<=lowLim || keyDown()) {
        clearKeyEvents();
        return;
      }

      if(getSwitch(g_eeGeneral.lightSw,0) || g_eeGeneral.lightAutoOff)
          BACKLIGHT_ON;
      else
          BACKLIGHT_OFF;
  }
}

void checkAlarm() // added by Gohst
{
  if(g_eeGeneral.disableAlarmWarning) return;
  if(!g_eeGeneral.beeperVal) alert(PSTR("Alarms Disabled"));
}

void checkSwitches()
{
  if(!g_eeGeneral.switchWarning) return; // if warning is on

  // first - display warning
  alertMessages( PSTR("Switches not off"), PSTR("Please reset them") ) ;

  bool state = (g_eeGeneral.switchWarning > 0);

  //loop until all switches are reset
  while (1)
  {
    uint8_t i;
    for(i=SW_BASE; i<SW_Trainer; i++)
    {
        if(i==SW_ID0) continue;
        if(getSwitch(i-SW_BASE+1,0) != state) break;
    }
    if(i==SW_Trainer || keyDown()) return;

    if(getSwitch(g_eeGeneral.lightSw,0) || g_eeGeneral.lightAutoOff)
      BACKLIGHT_ON;
    else
      BACKLIGHT_OFF;
  }
}

void checkQuickSelect()
{
    uint8_t i = keyDown(); //check for keystate
    uint8_t j;
    for(j=1; j<8; j++)
        if(i & (1<<j)) break;
    j--;

    if(j<6) {
        if(!eeModelExists(j)) return;

        eeLoadModel(g_eeGeneral.currModel = j);
        eeDirty(EE_GENERAL);

        lcd_clear();
        lcd_putsAtt(64-7*FW,0*FH,PSTR("LOADING"),DBLSIZE);

        for(uint8_t i=0;i<sizeof(g_model.name);i++)
            lcd_putcAtt(FW*2+i*2*FW-i-2, 3*FH, g_model.name[i],DBLSIZE);

        refreshDiplay();
        lcdSetRefVolt(g_eeGeneral.contrast);

        if(g_eeGeneral.lightSw || g_eeGeneral.lightAutoOff) // if lightswitch is defined or auto off
            BACKLIGHT_ON;
        else
            BACKLIGHT_OFF;

        clearKeyEvents(); // wait for user to release key
    }
}

uint8_t  g_beepCnt;
uint8_t  g_beepVal[5];

void message(const prog_char * s)
{
  lcd_clear();
  lcd_putsAtt(64-5*FW,0*FH,PSTR("MESSAGE"),DBLSIZE);
  lcd_puts_P(0,4*FW,s);
  refreshDiplay();
  lcdSetRefVolt(g_eeGeneral.contrast);
}

void alert(const prog_char * s, bool defaults)
{
  lcd_clear();
  lcd_putsAtt(64-5*FW,0*FH,PSTR("ALERT"),DBLSIZE);
  lcd_puts_P(0,4*FH,s);
  lcd_puts_P(64-6*FW,7*FH,PSTR("press any Key"));
  refreshDiplay();
  lcdSetRefVolt(defaults ? 25 : g_eeGeneral.contrast);
  beepErr();
  clearKeyEvents();
  while(1)
  {
    if(keyDown())   return;  //wait for key release

    if(getSwitch(g_eeGeneral.lightSw,0) || g_eeGeneral.lightAutoOff || defaults)
        BACKLIGHT_ON;
      else
        BACKLIGHT_OFF;
  }
}

void incSubtrim(uint8_t idx, int16_t inc)
{
  for (int8_t i=0; i<MAX_PHASES; i++) {
    PhaseData *phase = phaseaddress(i);
    int16_t trim = phase->trim[idx];
    if (trim >= TRIM_MIN && trim <= TRIM_MAX) {
      int16_t newtrim = trim - inc;
      if (newtrim < TRIM_MIN) {
        newtrim = TRIM_MIN;
      }
      if (newtrim > TRIM_MAX) {
        newtrim = TRIM_MAX;
      }
      phase->trim[idx] = newtrim;
    }
  }
  int16_t subtrim = g_model.subtrim[idx] + inc;
  if (subtrim > SUBTRIM_MAX)
    subtrim = SUBTRIM_MAX;
  if (subtrim < SUBTRIM_MIN)
    subtrim = SUBTRIM_MIN;
  g_model.subtrim[idx] = subtrim;
}

uint8_t checkTrim(uint8_t event)
{
  int8_t  k = (event & EVT_KEY_MASK) - TRM_BASE;
  int8_t  s = g_model.trimInc;

  if (k>=0 && k<8) { // && (event & _MSK_KEY_REPT))
    //LH_DWN LH_UP LV_DWN LV_UP RV_DWN RV_UP RH_DWN RH_UP
    uint8_t idx = k/2;
    uint8_t phase = getTrimFlightPhase(idx);
    int8_t  t = g_model.phaseData[phase].trim[idx];
    int16_t before = g_model.subtrim[idx] + t;
    int8_t  v = (s==0) ? min(32, abs(before)/4+1) : 1 << (s-1); // 1=>1  2=>2  3=>4  4=>8
    bool thro = (((2-(g_eeGeneral.stickMode&1)) == idx) && g_model.thrTrim);
    if (thro) v = 4; // if throttle trim and trim trottle then step=4
    int16_t trim = (k&1) ? t + v : t - v;   // positive = k&1
    int16_t after = g_model.subtrim[idx] + trim;

    bool beepTrim = false;
    for (int8_t mark=TRIM_MIN; mark!=-6; mark+=TRIM_MAX) { // because (int8_t)125+125==-6
      if ((mark!=0 || !thro) && ((mark!=TRIM_MIN && after>=mark && before<mark) || (mark!=TRIM_MAX && after<=mark && before>mark))) {
        trim = mark - g_model.subtrim[idx];
        beepTrim = true;
      }
    }

    if ((before<after && after>TRIM_MAX) || (before>after && after<TRIM_MIN)) {
      if (!g_model.extendedTrims) trim = t;
      beepTrim = true; // no repetition, it could be dangerous
    }

    if (trim < TRIM_MIN) {
      incSubtrim(idx, trim-TRIM_MIN);
      trim = TRIM_MIN;
    }
    else if (trim > TRIM_MAX) {
      incSubtrim(idx, trim-TRIM_MAX);
      trim = TRIM_MAX;
    }

    g_model.phaseData[phase].trim[idx] = (int8_t)trim;
    STORE_MODELVARS;

#if defined (BEEPSPKR)
    // toneFreq higher/lower according to trim position
    // limit the frequency, range -125 to 125 = toneFreq: 19 to 101
    if (after > TRIM_MAX)
      after = TRIM_MAX;
    if (after < TRIM_MIN)
      after = TRIM_MIN;
    after /= 4;
    after += 60;
#endif

    if (beepTrim) {
      killEvents(event);
      warble = false;
#if defined (BEEPSPKR)
      beepWarn2Spkr(after);
#else
      beepWarn2();
#endif
    }
    else {
      if (event & _MSK_KEY_REPT) warble = true;
#if defined (BEEPSPKR)
      beepTrimSpkr(after);
#else
      beepWarn1();
#endif
    }
    return 0;
  }
  return event;
}

#ifndef SIMU
class AutoLock
{
  uint8_t m_saveFlags;
public:
  AutoLock(){
    m_saveFlags = SREG;
    cli();
  };
  ~AutoLock(){
    if(m_saveFlags & (1<<SREG_I)) sei();
    //SREG = m_saveFlags;// & (1<<SREG_I)) sei();
  };
};

// #define STARTADCONV (ADCSRA  = (1<<ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2) | (1<<ADSC) | (1 << ADIE))
// G: Note that the above would have set the ADC prescaler to 128, equating to
// 125KHz sample rate. We now sample at 500KHz, with oversampling and other
// filtering options to produce 11-bit results.
uint16_t BandGap ;
static uint16_t s_anaFilt[8];
uint16_t anaIn(uint8_t chan)
{
  //                     ana-in:   3 1 2 0 4 5 6 7
  //static prog_char APM crossAna[]={4,2,3,1,5,6,7,0}; // wenn schon Tabelle, dann muss sich auch lohnen
  //                        Google Translate (German): // if table already, then it must also be worthwhile
  static prog_char APM crossAna[]={3,1,2,0,4,5,6,7};
  volatile uint16_t *p = &s_anaFilt[pgm_read_byte(crossAna+chan)];
  AutoLock autoLock;
  return *p;
}

#define ADC_VREF_TYPE 0x40 // AVCC with external capacitor at AREF pin
void getADC_filt()
{
  static uint16_t t_ana[3][8];
  for (uint8_t adc_input=0;adc_input<8;adc_input++)
  {
      ADMUX=adc_input|ADC_VREF_TYPE;
      // Start the AD conversion
      ADCSRA|=0x40;
      // Wait for the AD conversion to complete
      while ((ADCSRA & 0x10)==0);
      ADCSRA|=0x10;

      s_anaFilt[adc_input] = (s_anaFilt[adc_input]/2 + t_ana[1][adc_input]) & 0xFFFE; //gain of 2 on last conversion - clear last bit
      //t_ana[2][adc_input]  =  (t_ana[2][adc_input]  + t_ana[1][adc_input]) >> 1;
      t_ana[1][adc_input]  = (t_ana[1][adc_input]  + t_ana[0][adc_input]) >> 1;
      t_ana[0][adc_input]  = (t_ana[0][adc_input]  + ADCW               ) >> 1;
  }
}
/*
  s_anaFilt[chan] = (s_anaFilt[chan] + sss_ana[chan]) >> 1;
  sss_ana[chan] = (sss_ana[chan] + ss_ana[chan]) >> 1;
  ss_ana[chan] = (ss_ana[chan] + s_ana[chan]) >> 1;
  s_ana[chan] = (ADC + s_ana[chan]) >> 1;
  */

void getADC_osmp()
{
  uint16_t temp_ana[8] = {0};
  for (uint8_t adc_input=0;adc_input<8;adc_input++)
  {
    ADMUX=adc_input|ADC_VREF_TYPE; // TODO now it is done only one time before the loop, is it good?
    for (uint8_t i=0; i<4;i++) {  // Going from 10bits to 11 bits.  Addition = n.  Loop 4^n times
      // Start the AD conversion
      ADCSRA|=0x40;
      // Wait for the AD conversion to complete
      while ((ADCSRA & 0x10)==0);
      ADCSRA|=0x10;
      temp_ana[adc_input] += ADCW;
    }
    s_anaFilt[adc_input] = temp_ana[adc_input] / 2; // divide by 2^n to normalize result.
  }
}

void getADC_single()
{
  for (uint8_t adc_input=0;adc_input<8;adc_input++)
  {
      ADMUX=adc_input|ADC_VREF_TYPE;
      // Start the AD conversion
      ADCSRA|=0x40;
      // Wait for the AD conversion to complete
      while ((ADCSRA & 0x10)==0);
      ADCSRA|=0x10;
      s_anaFilt[adc_input]= ADCW * 2; // use 11 bit numbers
    }
}

void getADC_bandgap()
{
#if defined(PCBSTD)
  ADMUX=0x1E|ADC_VREF_TYPE; // Switch MUX to internal 1.22V reference
  _delay_us(5); // short delay to stablise reference voltage
  ADCSRA|=0x40; while ((ADCSRA & 0x10)==0); ADCSRA|=0x10; // grab a sample
  ADCSRA|=0x40; while ((ADCSRA & 0x10)==0); ADCSRA|=0x10; // again becasue first one is usually inaccurate
  BandGap=ADCW;
#else
  //BandGap=225; // gruvin: 1.1V internal Vref doesn't seem to work on the ATmega2561. :/ Weird.
                 // See http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=847208#847208
  // In the end, simply using a longer delay (presumably to account for the higher
  // impedance Vbg internal source) solved the problem. NOTE: Does NOT adversely affect PPM_out latency.
#if defined (PCBV4)
  ADCSRB &= ~(1<<MUX5);
#endif
  ADMUX=0x1E|ADC_VREF_TYPE; // Switch MUX to internal 1.1V reference
  _delay_us(300); // this somewhat costly delay is the only remedy for stable results on the Atmega2560/1 chips
  ADCSRA|=0x40; while ((ADCSRA & 0x10)==0); ADCSRA|=0x10; // again becasue first one is usually inaccurate
  BandGap=ADCW;
#if defined (PCBV4)
  ADCSRB |= (1<<MUX5);
#endif
#endif
}

getADCp getADC[3] = {
  getADC_single,
  getADC_osmp,
  getADC_filt
};
#else
uint16_t BandGap = 225;
#endif

#ifndef BATT_UNSTABLE_BANDGAP
uint16_t abRunningAvg = 0;
uint8_t  g_vbat100mV;
#else
uint16_t g_vbat100mV = 0;
#endif

volatile uint8_t tick10ms = 0;
uint16_t g_LightOffCounter;

uint8_t beepAgain = 0;
uint8_t beepAgainOrig = 0;
uint8_t beepOn = false;
int16_t p1val;

inline bool checkSlaveMode()
{
  // no power -> only phone jack = slave mode

#if defined(BUZZER_MOD) || defined(BEEPSPKR)
  return SLAVE_MODE;
#else
  static bool lastSlaveMode = false;
  static uint8_t checkDelay = 0;
  if (g_beepCnt || beepAgain || beepOn) {
    checkDelay = 20;
  }
  else if (checkDelay) {
    --checkDelay;
  }
  else {
    lastSlaveMode = SLAVE_MODE;
  }
  return lastSlaveMode;
#endif
}

uint16_t s_timeCumTot;
uint16_t s_timeCumAbs;  //laufzeit in 1/16 sec
uint16_t s_timeCumSw;  //laufzeit in 1/16 sec
uint16_t s_timeCumThr;  //gewichtete laufzeit in 1/16 sec
uint16_t s_timeCum16ThrP; //gewichtete laufzeit in 1/16 sec

uint8_t  s_timerState;
int16_t  s_timerVal[2];
uint8_t  Timer1_pre = 0 ;

uint8_t  Timer2_running = 0 ;
uint8_t  Timer2_pre = 0 ;
uint16_t timer2 = 0 ;

uint8_t  trimsCheckTimer = 0;

void resetTimer1()
{
  s_timerState = TMR_OFF; // is changed to RUNNING dep from mode
  s_timeCumAbs=0;
  s_timeCumThr=0;
  s_timeCumSw=0;
  s_timeCum16ThrP=0;
  Timer1_pre = 0 ;
}

void resetTimer2()
{
  Timer2_pre = 0 ;
  timer2 = 0 ;
}

inline void incTimers()
{
  TimerData *timer = &g_model.timer1;

  for (uint8_t i=0; i<1; i++, timer=&g_model.timer2) {
    int8_t tm = timer->mode;
    uint8_t atm = abs(tm);

    // value for time described in timer->mode
    // OFFABSRUsRU%ELsEL%THsTH%ALsAL%P1P1%P2P2%P3P3%
    uint16_t val = 0;
    if (atm>1 && atm<TMR_VAROFS) {
      val = calibratedStick[CONVERT_MODE(atm/2)-1];
      val = (tm<0 ? RESX-val : val+RESX ) / (RESX/16);
    }

    static uint16_t s_time;
    static uint16_t s_cnt;
    static uint16_t s_sum;
    static uint8_t sw_toggled[2] = {false, false};

    if (atm>=(TMR_VAROFS+MAX_SWITCH-1)){ // toggeled switch
      static uint8_t lastSwPos[2] = {0, 0};
      if(!(sw_toggled[i] | s_sum | s_cnt | s_time | lastSwPos[i])) lastSwPos[i] = tm < 0;  // if initializing then init the lastSwPos
      uint8_t swPos = getSwitch(tm>0 ? tm-(TMR_VAROFS+MAX_SWITCH-1-1) : tm+(TMR_VAROFS+MAX_SWITCH-1-1), 0);
      if (swPos && !lastSwPos[i]) sw_toggled[i] = !sw_toggled[i];  // if switch is flipped first time -> change counter state
      lastSwPos[i] = swPos;
    }

    s_cnt++;
    s_sum+=val;

    if ( (Timer1_pre += 1 ) >= 100 ) {
      Timer1_pre -= 100 ;
      s_time += 100;
      val     = s_sum/s_cnt;
      s_sum  -= val*s_cnt; //rest
      s_cnt   = 0;

      if (atm<TMR_VAROFS)
        sw_toggled[i] = false; // not switch - sw timer off
      else if (atm<(TMR_VAROFS+MAX_SWITCH-1))
        sw_toggled[i] = getSwitch((tm>0 ? tm-(TMR_VAROFS-1) : tm+(TMR_VAROFS-1)), 0); // normal switch

      s_timeCumTot               += 1;
      s_timeCumAbs               += 1;
      if(val) s_timeCumThr       += 1;
      if(sw_toggled) s_timeCumSw += 1;
      s_timeCum16ThrP            += val/2;

      s_timerVal[i] = timer->val;
      if (atm==TMRMODE_NONE) s_timerState = TMR_OFF;
      else if (atm==TMRMODE_ABS) s_timerVal[i] -= s_timeCumAbs;
      else if(atm<TMR_VAROFS) s_timerVal[i] -= (atm&1) ? s_timeCum16ThrP/16 : s_timeCumThr;// stick% : stick
      else s_timerVal[i] -= s_timeCumSw; //switch

      switch(s_timerState)
      {
        case TMR_OFF:
          if (tm != TMRMODE_NONE) s_timerState=TMR_RUNNING;
          break;
        case TMR_RUNNING:
          if (s_timerVal[i]<=0 && timer->val) s_timerState=TMR_BEEPING;
          break;
        case TMR_BEEPING:
          if (s_timerVal[i] <= -MAX_ALERT_TIME)   s_timerState=TMR_STOPPED;
          if (timer->val == 0)             s_timerState=TMR_RUNNING;
          break;
        case TMR_STOPPED:
          break;
      }

      static int16_t last_tmr;

      if (last_tmr != s_timerVal[i])  //beep only if seconds advance
      {
          if(s_timerState==TMR_RUNNING)
          {
              if(g_eeGeneral.preBeep && timer->val) // beep when 30, 15, 10, 5,4,3,2,1 seconds remaining
              {
                  if(s_timerVal[i]==30) {beepAgain=2; beepWarn2();} //beep three times
                  if(s_timerVal[i]==20) {beepAgain=1; beepWarn2();} //beep two times
                  if(s_timerVal[i]==10)  beepWarn2();
                  if(s_timerVal[i]<= 3)  beepWarn2();

                  if(g_eeGeneral.flashBeep && (s_timerVal[i]==30 || s_timerVal[i]==20 || s_timerVal[i]==10 || s_timerVal[i]<=3))
                      g_LightOffCounter = FLASH_DURATION;
              }

              if(g_eeGeneral.minuteBeep && (((timer->dir ? timer->val-s_timerVal[i] : s_timerVal[i])%60)==0)) //short beep every minute
              {
                  beepWarn2();
                  if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
              }
          }
          else if(s_timerState==TMR_BEEPING)
          {
              beepWarn();
              if(g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
          }
      }
      last_tmr = s_timerVal[i];
      if(timer->dir) s_timerVal[i] = timer->val-s_timerVal[i]; //if counting backwards - display backwards
    }
  }

  if (Timer2_running) {
    if ( (Timer2_pre += 1 ) >= 100 ) {
      Timer2_pre -= 100 ;
      timer2 += 1 ;
    }
  }
}

uint8_t s_traceBuf[MAXTRACE];
uint16_t s_traceWr;
uint16_t s_traceCnt;
inline void trace()   // called in perOut - once envery 0.01sec
{
  incTimers();

  uint16_t val = calibratedStick[CONVERT_MODE(3)-1]; // get throttle channel value
  val = (g_eeGeneral.throttleReversed ? RESX-val : val+RESX);
  val /= (RESX/16); // calibrate it

  static uint16_t s_time;
  static uint16_t s_cnt;
  static uint16_t s_sum;
  s_cnt++;
  s_sum+=val;
  if((get_tmr10ms()-s_time)<1000) //10 sec
    return;
  s_time= get_tmr10ms();
  val   = s_sum/s_cnt;
  s_sum = 0;
  s_cnt = 0;

  s_traceCnt++;
  s_traceBuf[s_traceWr++] = val;
  if(s_traceWr>=MAXTRACE) s_traceWr=0;
}

#ifdef HELI
uint16_t isqrt32(uint32_t n)
{
    uint16_t c = 0x8000;
    uint16_t g = 0x8000;

    for(;;) {
        if((uint32_t)g*g > n)
            g ^= c;
        c >>= 1;
        if(c == 0)
            return g;
        g |= c;
    }
}
#endif

// static variables used in perOut - moved here so they don't interfere with the stack
// It's also easier to initialize them here.
uint16_t pulses2MHz[120] = {0};
int16_t  anas [NUM_XCHNRAW] = {0};
int16_t  trims[NUM_STICKS] = {0};
int32_t  chans[NUM_CHNOUT] = {0};
uint32_t inacCounter = 0;
uint16_t inacSum = 0;
uint8_t  bpanaCenter = 0;
int16_t  sDelay[MAX_MIXERS] = {0};
int32_t  act   [MAX_MIXERS] = {0};
uint8_t  swOn  [MAX_MIXERS] = {0};
uint8_t mixWarning;

inline void evalTrims()
{
  for (uint8_t i=0; i<NUM_STICKS; i++) {
    // do trim -> throttle trim if applicable
    int16_t v = anas[i];
    int32_t vv = 2*RESX;
    int16_t trim = g_model.subtrim[i] + g_model.phaseData[getTrimFlightPhase(i)].trim[i];
    if (IS_THROTTLE(i) && g_model.thrTrim) {
      vv = (g_eeGeneral.throttleReversed) ?
                ((int32_t)trim+TRIM_MIN)*(RESX+v)/(2*RESX) :
                ((int32_t)trim-TRIM_MIN)*(RESX-v)/(2*RESX);
    }
    else if (trimsCheckTimer > 0) {
      trim = 0;
    }

    trims[i] = (vv==2*RESX) ? trim*2 : (int16_t)vv*2; // if throttle trim -> trim low end
  }
}

uint8_t evalSticks()
{
#ifdef HELI
  uint16_t d = 0;
  if(g_model.swashR.value) {
    uint32_t v = (int32_t(calibratedStick[ELE_STICK])*calibratedStick[ELE_STICK] +
        int32_t(calibratedStick[AIL_STICK])*calibratedStick[AIL_STICK]);
    uint32_t q = int32_t(RESX)*g_model.swashR.value/100;
    q *= q;
    if(v>q)
      d = isqrt32(v);
  }
#endif

  uint8_t anaCenter = 0;

  for(uint8_t i=0; i<NUM_STICKS+NUM_POTS; i++) {

    // normalization [0..2048] -> [-1024..1024]

    int16_t v = anaIn(i);
    v -= g_eeGeneral.calibMid[i];
    v  =  v * (int32_t)RESX /  (max((int16_t)100,(v>0 ?
                                     g_eeGeneral.calibSpanPos[i] :
                                     g_eeGeneral.calibSpanNeg[i])));
    if(v <= -RESX) v = -RESX;
    if(v >=  RESX) v =  RESX;
    calibratedStick[i] = v; //for show in expo
    if(!(v/16)) anaCenter |= 1<<(CONVERT_MODE((i+1))-1);


    if (i < NUM_STICKS) { //only do this for sticks
      if (!s_noStickInputs && isFunctionActive(FUNC_TRAINER)) {
        // trainer mode
        TrainerMix* td = &g_eeGeneral.trainer.mix[i];
        if (td->mode) {
          uint8_t chStud = td->srcChn;
          int32_t vStud  = (g_ppmIns[chStud]- g_eeGeneral.trainer.calib[chStud]);
          vStud *= td->studWeight;
          vStud /= 50;
          switch (td->mode) {
            case 1: v += vStud;   break; // add-mode
            case 2: v  = vStud;   break; // subst-mode
          }
        }
      }

#ifdef HELI
      if(d && (i==ELE_STICK || i==AIL_STICK))
        v = int32_t(v)*g_model.swashR.value*RESX/(int32_t(d)*100);
#endif

    }
    anas[i] = v; //set values for mixer
  }

  /* EXPOs */
  applyExpos(anas);

  /* TRIMs */
  evalTrims();

  return anaCenter;
}

uint16_t active_functions = 0; // current max = 16 functions

void evalFunctions()
{
  assert(sizeof(active_functions)*8 >= FUNC_LAST);

  for (uint8_t i=0; i<NUM_FSW; i++) {
    FuncSwData *sd = &g_model.funcSw[i];
    if (sd->swtch && sd->func) {
      uint16_t mask = (1 << (sd->func-1));
      if (getSwitch(sd->swtch, 0))
        active_functions |= mask;
      else
        active_functions &= (~mask);
    }
  }
}

void perOut(int16_t *chanOut)
{
  uint8_t phase = getFlightPhase();
  uint8_t anaCenter = evalSticks();

  //===========BEEP CENTER================
  anaCenter &= g_model.beepANACenter;
  if(((bpanaCenter ^ anaCenter) & anaCenter)) beepWarn1();
  bpanaCenter = anaCenter;

  anas[MIX_MAX-1]  = RESX;     // MAX
  anas[MIX_FULL-1] = RESX;     // FULL

#ifdef HELI
  if(g_model.swashR.value)
  {
      uint32_t v = ((int32_t)anas[ELE_STICK]*anas[ELE_STICK] + (int32_t)anas[AIL_STICK]*anas[AIL_STICK]);
      uint32_t q = (int32_t)RESX*g_model.swashR.value/100;
      q *= q;
      if(v>q)
      {
          uint16_t d = isqrt32(v);
          anas[ELE_STICK] = (int32_t)anas[ELE_STICK]*g_model.swashR.value*RESX/((int32_t)d*100);
          anas[AIL_STICK] = (int32_t)anas[AIL_STICK]*g_model.swashR.value*RESX/((int32_t)d*100);
      }
  }

#define REZ_SWASH_X(x)  ((x) - (x)/8 - (x)/128 - (x)/512)   //  1024*sin(60) ~= 886
#define REZ_SWASH_Y(x)  ((x))   //  1024 => 1024

  if(g_model.swashR.type)
  {
      int16_t vp = anas[ELE_STICK]+trims[ELE_STICK];
      int16_t vr = anas[AIL_STICK]+trims[AIL_STICK];
      int16_t vc = 0;
      if(g_model.swashR.collectiveSource)
          vc = anas[g_model.swashR.collectiveSource-1];

      if(g_model.swashR.invertELE) vp = -vp;
      if(g_model.swashR.invertAIL) vr = -vr;
      if(g_model.swashR.invertCOL) vc = -vc;

      switch (g_model.swashR.type)
      {
      case (SWASH_TYPE_120):
          vp = REZ_SWASH_Y(vp);
          vr = REZ_SWASH_X(vr);
          anas[MIX_CYC1-1] = vc - vp;
          anas[MIX_CYC2-1] = vc + vp/2 + vr;
          anas[MIX_CYC3-1] = vc + vp/2 - vr;
          break;
      case (SWASH_TYPE_120X):
          vp = REZ_SWASH_X(vp);
          vr = REZ_SWASH_Y(vr);
          anas[MIX_CYC1-1] = vc - vr;
          anas[MIX_CYC2-1] = vc + vr/2 + vp;
          anas[MIX_CYC3-1] = vc + vr/2 - vp;
          break;
      case (SWASH_TYPE_140):
          vp = REZ_SWASH_Y(vp);
          vr = REZ_SWASH_Y(vr);
          anas[MIX_CYC1-1] = vc - vp;
          anas[MIX_CYC2-1] = vc + vp + vr;
          anas[MIX_CYC3-1] = vc + vp - vr;
          break;
      case (SWASH_TYPE_90):
          vp = REZ_SWASH_Y(vp);
          vr = REZ_SWASH_Y(vr);
          anas[MIX_CYC1-1] = vc - vp;
          anas[MIX_CYC2-1] = vc + vr;
          anas[MIX_CYC3-1] = vc - vr;
          break;
      default:
          break;
      }
  }
#endif

  if (s_noStickInputs) {
    for (uint8_t i=0; i<NUM_STICKS; i++) {
      if (!IS_THROTTLE(i)) {
        anas[i]  = 0;
      }
    }
    for (uint8_t i=0; i<NUM_PPM; i++) anas[i+PPM_BASE] = 0;
  }
  else {
    for (uint8_t i=0; i<NUM_CAL_PPM; i++)       anas[i+PPM_BASE] = (g_ppmIns[i] - g_eeGeneral.trainer.calib[i])*2; // add ppm channels
    for (uint8_t i=NUM_CAL_PPM; i<NUM_PPM; i++) anas[i+PPM_BASE] = g_ppmIns[i]*2; // add ppm channels
  }
  
  for (uint8_t i=CHOUT_BASE; i<NUM_XCHNRAW; i++) anas[i] = chans[i-CHOUT_BASE]; // other mixes previous outputs

  if(tick10ms) trace(); //trace thr 0..32  (/32)

  memset(chans,0,sizeof(chans));        // All outputs to 0

    //========== MIXER LOOP ===============
    mixWarning = 0;
    for(uint8_t i=0;i<MAX_MIXERS;i++){
      MixData *md = mixaddress( i ) ;

      if((md->destCh==0) || (md->destCh>NUM_CHNOUT)) break;

      if (md->phase != 0) {
        if (md->phase > 0) {
          if (phase+1 != md->phase)
            continue;
        }
        else {
          if (phase+1 == -md->phase)
            continue;
        }
      }

      //Notice 0 = NC switch means not used -> always on line
      int16_t v  = 0;
      uint8_t swTog;

      //swOn[i]=false;
      if(!getSwitch(md->swtch,1)){ // switch on?  if no switch selected => on
        swTog = swOn[i];
        swOn[i] = false;
        if(md->srcRaw!=MIX_MAX && md->srcRaw!=MIX_FULL) continue;// if not MAX or FULL - next loop
        if(md->mltpx==MLTPX_REP) continue; // if switch is off and REPLACE then off
        v = (md->srcRaw == MIX_FULL ? -RESX : 0); // switch is off and it is either MAX=0 or FULL=-512
      }
      else {
        swTog = !swOn[i];
        swOn[i] = true;
        uint8_t k = md->srcRaw-1;
        v = anas[k]; //Switch is on. MAX=FULL=512 or value.
        if(k>=CHOUT_BASE && (k<i)) v = chans[k]; // if we've already calculated the value - take it instead // anas[i+CHOUT_BASE] = chans[i]
        if(md->mixWarn) mixWarning |= 1<<(md->mixWarn-1); // Mix warning
      }

      //========== INPUT OFFSET ===============
      if(md->sOffset) v += calc100toRESX(md->sOffset);

      //========== DELAY and PAUSE ===============
      if (md->speedUp || md->speedDown || md->delayUp || md->delayDown)  // there are delay values
      {
#define DEL_MULT 256

        //if(init) {
          //act[i]=(int32_t)v*DEL_MULT;
          //swTog = false;
        //}
        int16_t diff = v-act[i]/DEL_MULT;

        if(swTog) {
            //need to know which "v" will give "anas".
            //curves(v)*weight/100 -> anas
            // v * weight / 100 = anas => anas*100/weight = v
          if(md->mltpx==MLTPX_REP)
          {
              act[i] = (int32_t)anas[md->destCh-1+CHOUT_BASE]*DEL_MULT;
              act[i] *=100;
              if(md->weight) act[i] /= md->weight;
          }
          diff = v-act[i]/DEL_MULT;
          if(diff) sDelay[i] = (diff<0 ? md->delayUp :  md->delayDown) * 100;
        }

        if(sDelay[i]){ // perform delay
            if(tick10ms) sDelay[i]--;
            v = act[i]/DEL_MULT;
            diff = 0;
        }

        if(diff && (md->speedUp || md->speedDown)){
          //rate = steps/sec => 32*1024/100*md->speedUp/Down
          //act[i] += diff>0 ? (32768)/((int16_t)100*md->speedUp) : -(32768)/((int16_t)100*md->speedDown);
          //-100..100 => 32768 ->  100*83886/256 = 32768,   For MAX we divide by 2 sincde it's asymmetrical
          if(tick10ms) {
              int32_t rate = (int32_t)DEL_MULT*2048*100;
              if(md->weight) rate /= abs(md->weight);
              act[i] = (diff>0) ? ((md->speedUp>0)   ? act[i]+(rate)/((int16_t)100*md->speedUp)   :  (int32_t)v*DEL_MULT) :
                                  ((md->speedDown>0) ? act[i]-(rate)/((int16_t)100*md->speedDown) :  (int32_t)v*DEL_MULT) ;
          }

          if(((diff>0) && (v<(act[i]/DEL_MULT))) || ((diff<0) && (v>(act[i]/DEL_MULT)))) act[i]=(int32_t)v*DEL_MULT; //deal with overflow
          v = act[i]/DEL_MULT;
        }
      }

      //========== CURVES ===============
      if (md->curve)
        v = applyCurve(v, md->curve, md->srcRaw);

      //========== TRIM ===============
      if((md->carryTrim==0) && (md->srcRaw>0) && (md->srcRaw<=4)) v += trims[md->srcRaw-1];  //  0 = Trim ON  =  Default

      //========== MULTIPLEX ===============
      int32_t dv = (int32_t)v*md->weight;
      switch(md->mltpx){
        case MLTPX_REP:
          chans[md->destCh-1] = dv;
          break;
        case MLTPX_MUL:
          chans[md->destCh-1] *= dv/100l;
          chans[md->destCh-1] /= RESXl;
          break;
        default:  // MLTPX_ADD
          chans[md->destCh-1] += dv; //Mixer output add up to the line (dv + (dv>0 ? 100/2 : -100/2))/(100);
          break;
        }
    }

  //========== MIXER WARNING ===============
  // 1,2 or 3 "bips" (short beeps)
  //1= 00,08
  //2= 24,32,40
  //3= 56,64,72,80
  // Gruvin:  Sometimes, one or more of these beeps do not fire. That will be because the tmr10ms counter
  //          may not necessarily be exactly (==) the below figures when queried from inside perOut().
  //          But we only ever want a beep to fire once, so we have to use anexact counter match (not a range).
  //          My solution was to make mixWarning global and have the counter checks done inside per10ms();
  /* {
    uint16_t tmr10ms ;
    tmr10ms = get_tmr10ms() ;
    if(mixWarning & 1) if(((tmr10ms&0xFF)==  0)) beepWarn1();
    if(mixWarning & 2) if(((tmr10ms&0xFF)== 64) || ((tmr10ms&0xFF)== 72)) beepWarn1();
    if(mixWarning & 4) if(((tmr10ms&0xFF)==128) || ((tmr10ms&0xFF)==136) || ((tmr10ms&0xFF)==144)) beepWarn1();
  } */

  //========== LIMITS ===============
  for (uint8_t i=0;i<NUM_CHNOUT;i++) {
      // chans[i] holds data from mixer.   chans[i] = v*weight => 1024*100
      // later we multiply by the limit (up to 100) and then we need to normalize
      // at the end chans[i] = chans[i]/100 =>  -1024..1024
      // interpolate value with min/max so we get smooth motion from center to stop
      // this limits based on v original values and min=-1024, max=1024  RESX=1024

      int32_t q = chans[i];// + (int32_t)g_model.limitData[i].offset*100; // offset before limit

      chans[i] /= 100; // chans back to -1024..1024
      ex_chans[i] = chans[i]; //for getswitch

      int16_t ofs = g_model.limitData[i].offset;
      int16_t lim_p = 10*(g_model.limitData[i].max+100);
      int16_t lim_n = 10*(g_model.limitData[i].min-100); //multiply by 10 to get same range as ofs (-1000..1000)
      if(ofs>lim_p) ofs = lim_p;
      if(ofs<lim_n) ofs = lim_n;

      if(q) q = (q>0) ?
                q*((int32_t)lim_p-ofs)/100000 :
               -q*((int32_t)lim_n-ofs)/100000 ; //div by 100000 -> output = -1024..1024

      q += calc1000toRESX(ofs);
      lim_p = calc1000toRESX(lim_p);
      lim_n = calc1000toRESX(lim_n);
      if(q>lim_p) q = lim_p;
      if(q<lim_n) q = lim_n;
      if(g_model.limitData[i].revert) q=-q;// finally do the reverse.

      if(g_model.safetySw[i].swtch)  //if safety sw available for channel check and replace val if needed
          if(getSwitch(g_model.safetySw[i].swtch,0)) q = calc100toRESX(g_model.safetySw[i].val);

      chanOut[i] = q; //copy consistent word to int-level
  }
}

void perMain()
{
  static uint16_t lastTMR;
  tick10ms = (get_tmr10ms() != lastTMR);
  lastTMR = get_tmr10ms();

  int16_t last_chans512[NUM_CHNOUT];
  int16_t next_chans512[NUM_CHNOUT];

  static uint8_t last_phase = 0;
  uint8_t phase = getFlightPhase();

  static uint8_t fading_out_timer = 0;
  if (last_phase != phase) {
    fading_out_timer = 10 * max(g_model.phaseData[last_phase].fadeOut, g_model.phaseData[phase].fadeIn);
    last_phase = phase;
  }

  if (fading_out_timer) {
    memcpy(last_chans512, g_chans512, sizeof(g_chans512));
  }

  perOut(next_chans512);

  cli();
  if (fading_out_timer) {
    for (uint8_t i=0; i<NUM_CHNOUT; i++) {
      g_chans512[i] = last_chans512[i] + (next_chans512[i] - last_chans512[i]) / fading_out_timer;
    }
    fading_out_timer--;
  }
  else {
    memcpy(g_chans512, next_chans512, sizeof(g_chans512));
  }
  sei();

#ifdef ASYNC_WRITE
  if (!eeprom_buffer_size) {
    if (theFile.isWriting())
      theFile.nextWriteStep();
    else if (s_eeDirtyMsk)
      eeCheck();
  }
#endif

  if(!tick10ms) return; //make sure the rest happen only every 10ms.

  evalFunctions();

  if (s_noHi) s_noHi--;
  if (g_eeGeneral.inactivityTimer && g_vbat100mV>49) {
    inacCounter++;
    uint16_t tsum = 0;
    for(uint8_t i=0;i<4;i++) tsum += anaIn(i)/64;  // reduce sensitivity
    if(tsum!=inacSum){
      inacSum = tsum;
      inacCounter=0;
    }
    if(inacCounter>((uint32_t)g_eeGeneral.inactivityTimer*100*60))
      if((inacCounter&0x3F)==10) beepWarn();
  }

  if (trimsCheckTimer > 0)
    trimsCheckTimer -= 1;

#ifndef ASYNC_WRITE
  eeCheck();
#endif

  lcd_clear();
  uint8_t evt=getEvent();
  evt = checkTrim(evt);

  if(g_LightOffCounter) g_LightOffCounter--;
  if(evt) g_LightOffCounter = g_eeGeneral.lightAutoOff*500; // on keypress turn the light on 5*100

  if( getSwitch(g_eeGeneral.lightSw,0) || g_LightOffCounter)
    BACKLIGHT_ON;
  else
    BACKLIGHT_OFF;

  ////////////////
  // G: TODO This shouldn't be in perMain(). It should be in the same place
  // all the other ADC samples happen
  static int16_t p1valprev;
  p1valdiff = (p1val-calibratedStick[6])/32;
  if(p1valdiff) {
      p1valdiff = (p1valprev-calibratedStick[6])/2;
      p1val = calibratedStick[6];
  }
  p1valprev = calibratedStick[6];
  /////////////////

  g_menuStack[g_menuStackPtr](evt);
  refreshDiplay();

  // PPM signal on phono-jack. In or out? ...
  if(checkSlaveMode()) {
    PORTG &= ~(1<<OUT_G_SIM_CTL); // 0=ppm out
  }
  else{
    PORTG |=  (1<<OUT_G_SIM_CTL); // 1=ppm-in
  }

  switch( get_tmr10ms() & 0x1f ) { //alle 10ms*32

    case 2:
      {
/*
Gruvin:
  Interesting fault with new unit. Sample is reading 0x06D0 (around 12.3V) but
  we're only seeing around 0.2V displayed! (Calibrate = 0)

  Long story short, the higher voltage of the new 8-pack of AA alkaline cells I put in the stock
  '9X, plus just a tiny bit of calibration applied, were causing an overflow in the 16-bit math,
  causing a wrap-around to a very small voltage.

  See the wiki (VoltageAveraging) if you're interested in my long-winded analysis.
*/

#ifndef BATT_UNSTABLE_BANDGAP
        // initialize to first sample if on first averaging cycle
        if (abRunningAvg == 0) abRunningAvg = anaIn(7);

        // G: Running average (virtual 7 stored plus current sample) for batt volts to stablise display
        // Average the raw samples so the calibrartion screen updates instantly
        int32_t ab = ((abRunningAvg * 7) + anaIn(7)) / 8;
        abRunningAvg = (uint16_t)ab;

        // Calculation By Mike Blandford
        // Resistor divide on battery voltage is 5K1 and 2K7 giving a fraction of 2.7/7.8
        // If battery voltage = 10V then A2D voltage = 3.462V
        // 11 bit A2D count is 1417 (3.462/5*2048).
        // 1417*18/256 = 99 (actually 99.6) to represent 9.9 volts.
        // Erring on the side of low is probably best.

        g_vbat100mV = (ab*16 + (ab*g_eeGeneral.vBatCalib)/8)/BandGap;
#else
        int32_t instant_vbat = anaIn(7);
        instant_vbat = (instant_vbat*16 + instant_vbat*g_eeGeneral.vBatCalib/8) / BandGap;
        if (g_vbat100mV == 0 || g_menuStack[0] != menuMainView) g_vbat100mV = instant_vbat;
        g_vbat100mV = (instant_vbat + g_vbat100mV*7) / 8;
#endif

        static uint8_t s_batCheck;
        s_batCheck+=32;
        if(s_batCheck==0 && g_vbat100mV<g_eeGeneral.vBatWarn && g_vbat100mV>49) {
          beepErr();
          if (g_eeGeneral.flashBeep) g_LightOffCounter = FLASH_DURATION;
        }

      }
      break;


    case 3:
      {
        // The various "beep" tone lengths
        static prog_uint8_t APM beepTab[]= {
       // 0   1   2   3    4
          0,  0,  0,  0,   0, //quiet
          0,  1,  8, 30, 100, //silent
          1,  1,  8, 30, 100, //normal
          1,  1, 15, 50, 150, //for motor
         10, 10, 30, 50, 150, //for motor
        };
        memcpy_P(g_beepVal,beepTab+5*g_eeGeneral.beeperVal,5);
          //g_beepVal = BEEP_VAL;
      }
      break;
  }

}
int16_t g_ppmIns[8];
uint8_t ppmInState = 0; //0=unsync 1..8= wait for value i-1

#ifndef SIMU

#define HEART_TIMER2Mhz 1;
#define HEART_TIMER10ms 2;

uint8_t heartbeat;


ISR(TIMER1_COMPA_vect) //2MHz pulse generation
{
  static uint8_t   pulsePol;
  static uint16_t *pulsePtr = pulses2MHz;

  // TODO understand what Bryan did here
  uint8_t i = 0;
  while((TCNT1L < 10) && (++i < 50))  // Timer does not read too fast, so i
    ;
  uint16_t dt=TCNT1;//-OCR1A;

//vinceofdrink@gmail harwared ppm
//Orginal bitbang for PPM
#ifndef DPPMPB7_HARDWARE
  if(pulsePol)
  {
    PORTB |=  (1<<OUT_B_PPM); // GCC optimisation should result in a single SBI instruction
    pulsePol = 0;
  }else{
    PORTB &= ~(1<<OUT_B_PPM); // GCC optimisation should result in a single CBI instruction
    pulsePol = 1;
  }
  g_tmr1Latency_max = max(dt,g_tmr1Latency_max);    // max has leap, therefore vary in length
  g_tmr1Latency_min = min(dt,g_tmr1Latency_min);    // min has leap, therefore vary in length
#endif
  OCR1A  = *pulsePtr++;

//vinceofdrink@gmail harwared ppm
#if defined (DPPMPB7_HARDWARE)
OCR1C=OCR1A;            //just copy the value of the OCR1A to OCR1C to test PPM out without to much change in the code not optimum but will not alter ppm precision
#endif

  if( *pulsePtr == 0) {
    //currpulse=0;
    pulsePtr = pulses2MHz;
    pulsePol = g_model.pulsePol;//0;

    cli();
#if defined (PCBV3)
    TIMSK1 &= ~(1<<OCIE1A); //stop reentrance
#else
    TIMSK &= ~(1<<OCIE1A); //stop reentrance
#endif
    sei();

    setupPulses();

    cli();
#if defined (PCBV3)
    TIMSK1 |= (1<<OCIE1A);
#else
    TIMSK |= (1<<OCIE1A);
#endif
    sei();
  }
  heartbeat |= HEART_TIMER2Mhz;
}

volatile uint8_t g_tmr16KHz; //continuous timer 16ms (16MHz/1024/256) -- 8-bit counter overflow
#if defined (PCBV3)
ISR(TIMER2_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
  g_tmr16KHz++; // gruvin: Not 16KHz. Overflows occur at 61.035Hz (1/256th of 15.625KHz)
                // to give *16.384ms* intervals. Kind of matters for accuracy elsewhere. ;)
                // g_tmr16KHz is used to software-construct a 16-bit timer
                // from TIMER-0 (8-bit). See getTmr16KHz, below.
}

uint16_t getTmr16KHz()
{
  while(1){
    uint8_t hb  = g_tmr16KHz;
#if defined (PCBV3)
    uint8_t lb  = TCNT2;
#else
    uint8_t lb  = TCNT0;
#endif
    if(hb-g_tmr16KHz==0) return (hb<<8)|lb;
  }
}

extern uint16_t g_time_per10; // instantiated in menus.cpp

#if defined (PCBV3)
ISR(TIMER2_COMPA_vect, ISR_NOBLOCK) //10ms timer
#else
ISR(TIMER0_COMP_vect, ISR_NOBLOCK) //10ms timer
#endif
{
  cli();
  static uint8_t accuracyWarble = 4; // becasue 16M / 1024 / 100 = 156.25. So bump every 4.
  uint8_t bump = (!(accuracyWarble++ & 0x03)) ? 157 : 156;
#if defined (PCBV3)
  TIMSK2 &= ~(1<<OCIE2A); //stop reentrance
  OCR2A += bump;
#else
  TIMSK &= ~(1<<OCIE0); //stop reentrance
#if defined (BEEPSPKR)
  OCR0 += 2; // run much faster, to generate speaker tones
#else
  OCR0 += bump;
#endif
#endif
  sei();

#if defined (PCBSTD) && defined (BEEPSPKR)
  // gruvin: Begin Tone Generator
  static uint8_t toneCounter;

  if (toneOn)
  {
    toneCounter += toneFreq;
    if ((toneCounter & 0x80) == 0x80)
      PORTE |=  (1<<OUT_E_BUZZER); // speaker output 'high'
    else
      PORTE &=  ~(1<<OUT_E_BUZZER); // speaker output 'low'
  }
  else
      PORTE &=  ~(1<<OUT_E_BUZZER); // speaker output 'low'
  // gruvin: END Tone Generator

  static uint8_t cnt10ms = 77; // execute 10ms code once every 78 ISRs
  if (cnt10ms-- == 0) // BEGIN { ... every 10ms ... }
  {
    // Begin 10ms event
    cnt10ms = 77;

#endif

    // Record start time from TCNT1 to record excution time
    cli();
    uint16_t dt=TCNT1;// TCNT1 (used for PPM out pulse generation) is running at 2MHz
    sei();

    //cnt >/=0
    //beepon/off
    //beepagain y/n
    if(g_beepCnt) {
        if(!beepAgainOrig) {
            beepAgainOrig = g_beepCnt;
            beepOn = true;
        }
        g_beepCnt--;
    }
    else
    {
        if(beepAgain && beepAgainOrig) {
            beepOn = !beepOn;
            g_beepCnt = beepOn ? beepAgainOrig : 8;
            if(beepOn) beepAgain--;
        }
        else {
            beepAgainOrig = 0;
            beepOn = false;
            warble = false;
        }
    }

#if defined (PCBV3)
    // G: use timer0 WGM mode tone generator for beeps
    if(beepOn)
    {
      static bool warbleC;
      warbleC = warble && !warbleC;
      if(warbleC)
        TCCR0A  &= ~(0b01<<COM0A0); // tone off
      else
        TCCR0A  |= (0b01<<COM0A0);  // tone on
    }else{
      TCCR0A  &= ~(0b01<<COM0A0);   // tone off
    }
#else
#if defined (BEEPSPKR)
    // G: use speaker tone generator for beeps
    if(beepOn)
    {
      static bool warbleC;
      warbleC = warble && !warbleC;
      if(warbleC)
        toneOn = false;
      else
        toneOn = true;
    }else{
      toneOn = false;
    }

#else
    // G: use original external buzzer for beeps
    if(beepOn){
    static bool warbleC;
    warbleC = warble && !warbleC;
    if(warbleC)
      PORTE &= ~(1<<OUT_E_BUZZER);//buzzer off
    else
      PORTE |=  (1<<OUT_E_BUZZER);//buzzer on
    }else{
      PORTE &= ~(1<<OUT_E_BUZZER);
    }
#endif // BEEPSPKR
#endif // PCBV3

    per10ms();

#if defined (PCBV3)
    disk_timerproc();
#endif

    heartbeat |= HEART_TIMER10ms;

    // Record per10ms ISR execution time, in us(x2) for STAT2 page
    cli();
    uint16_t dt2 = TCNT1; // capture end time
    sei();
    g_time_per10 = dt2 - dt; // NOTE: These spike to nearly 65535 just now and then. Why? :/

#if defined (PCBSTD) && defined (BEEPSPKR)
  } // end 10ms event
#endif

  cli();
#if defined (PCBV3)
  TIMSK2 |= (1<<OCIE2A);
#else
  TIMSK |= (1<<OCIE0);
#endif
  sei();
}



// Timer3 used for PPM_IN pulse width capture. Counter running at 16MHz / 8 = 2MHz
// equating to one count every half millisecond. (2 counts = 1ms). Control channel
// count delta values thus can range from about 1600 to 4400 counts (800us to 2200us),
// corresponding to a PPM signal in the range 0.8ms to 2.2ms (1.5ms at center).
// (The timer is free-running and is thus not reset to zero at each capture interval.)
ISR(TIMER3_CAPT_vect, ISR_NOBLOCK) //capture ppm in 16MHz / 8 = 2MHz
{
  static uint16_t lastCapt;

  uint16_t capture=ICR3;

  cli(); // gruvin: are these global int disables really needed? Consult data sheet.
#if defined (PCBV3)
  TIMSK3 &= ~(1<<ICIE3);
#else
  ETIMSK &= ~(1<<TICIE3);
#endif
  sei();

  uint16_t val = (capture - lastCapt) / 2;

  // G: We prcoess g_ppmInsright here to make servo movement as smooth as possible
  //    while under trainee control
  if (val>4000 && val < 16000) // G: Priorotise reset pulse. (Needed when less than 8 incoming pulses)
    ppmInState = 1; // triggered
  else
  {
    if (ppmInState && ppmInState<=8)
    {
      if (val>800 && val<2200) // if valid pulse-width range
      {
        g_ppmIns[ppmInState++ - 1] =
          (int16_t)(val - 1500)*(g_eeGeneral.PPM_Multiplier+10)/10; //+-500 != 512, but close enough.
      }
      else
        ppmInState = 0; // not triggered
    }
  }

  lastCapt = capture;

  cli();
#if defined (PCBV3)
  TIMSK3 |= (1<<ICIE3);
#else
  ETIMSK |= (1<<TICIE3);
#endif
  sei();
}

#if defined (PCBV3)
/*---------------------------------------------------------*/
/* User Provided Date/Time Function for FatFs module       */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */
/* This is not required in read-only configuration.        */

uint32_t get_fattime(void)
{
  struct tm t;
  filltm(&g_unixTime, &t); // create a struct tm date/time structure from global unix time stamp

  /* Pack date and time into a DWORD variable */
  return    ((DWORD)(t.tm_year - 80) << 25)
    | ((uint32_t)(t.tm_mon+1) << 21)
    | ((uint32_t)t.tm_mday << 16)
    | ((uint32_t)t.tm_hour << 11)
    | ((uint32_t)t.tm_min << 5)
    | ((uint32_t)t.tm_sec >> 1);
}
#endif

extern uint16_t g_timeMain;

/*
// gruvin: Fuse declarations work if we use the .elf file for AVR Studio (v4)
// instead of the Intel .hex files.  They should also work with AVRDUDE v5.10
// (reading from the .hex file), since a bug relating to Intel HEX file record
// interpretation was fixed. However, I leave these commented out, just in case
// it causes trouble for others.
#if defined (PCBV3)
// See fuses_2561.txt
  FUSES =
  {
    // BOD=4.3V, WDT OFF (enabled in code), Boot Flash 4096 bytes at 0x1F000,
    // JTAG and OCD enabled, EESAVE enabled, BOOTRST/CKDIV8/CKOUT disabled,
    // Full swing Xtal oscillator. Start-up 16K clks + 0ms. BOD enabled.
    0xD7, // .low
    0x11, // .high
    0xFC  // .extended
  };
#else
  FUSES =
  {
    // G: Changed 2011-07-04 to include EESAVE. Tested OK on stock 9X
    0x1F, // LFUSE
    0x11, // HFUSE
    0xFF  // EFUSE
  };
#endif
*/

#ifdef DEBUG
uint16_t DEBUG1 = 0;
uint16_t DEBUG2 = 0;
#endif

extern unsigned char __bss_end ;

unsigned int stack_free()
{
  unsigned char *p ;

  p = &__bss_end + 1 ;
  while ( *p == 0x55 )
  {
    p+= 1 ;
  }
  return p - &__bss_end ;
}

#endif


void instantTrim()
{
  for (uint8_t i=0; i<NUM_STICKS; i++) {
    if (!IS_THROTTLE(i)) {
      // don't instant trim the throttle stick
      uint8_t phase = getTrimFlightPhase(i);
      s_noStickInputs = true;
      evalSticks();
      s_noStickInputs = false;
      int16_t trim = (anas[i] + trims[i]) / 2;
      trim -= g_model.subtrim[i];
      // printf("anas[%d]=%d, trims[%d]=%d, trim=%d, subtrim=%d => trim=%d\n", i, anas[i], i, trims[i], g_model.phaseData[phase].trim[i], g_model.subtrim[i], trim);
      if (trim < TRIM_MIN) {
        incSubtrim(i, trim-TRIM_MIN);
        trim = TRIM_MIN;
      }
      if (trim > TRIM_MAX) {
        incSubtrim(i, trim-TRIM_MAX);
        trim = TRIM_MAX;
      }
      g_model.phaseData[phase].trim[i] = (int8_t)trim;
    }
  }

  STORE_MODELVARS;
  beepWarn1();
}

void moveTrimsToOffsets() // copy state of 3 primary to subtrim
{
  int16_t zero_chans512[NUM_CHNOUT];

  s_noStickInputs = true;
  perOut(zero_chans512); // do output loop - zero input sticks
  s_noStickInputs = false;

  for (uint8_t i=0; i<NUM_CHNOUT; i++)
    g_model.limitData[i].offset = max(min(zero_chans512[i], (int16_t)1000), (int16_t)-1000); // make sure the offset doesn't go haywire

  // reset all trims, except throttle
  for (uint8_t i=0; i<4; i++) {
    if (!IS_THROTTLE(i)) {
      g_model.subtrim[i] = 0;
      for (uint8_t phase=0; phase<MAX_PHASES; phase++) {
        int8_t trim = g_model.phaseData[phase].trim[i];
        if (trim >= TRIM_MIN && trim <= TRIM_MAX)
          g_model.phaseData[phase].trim[i] = 0;
      }
    }
  }

  STORE_MODELVARS;
  beepWarn1();
}

#ifndef SIMU
int main(void)
{
  // Set up I/O port data directions and initial states
  DDRA = 0xff;  PORTA = 0x00; // LCD data

#if defined (PCBV4)
  DDRB = 0b10010111;  PORTB = 0b01101000; // 7:SPKR, 6:IDL2_S|PPM,  5:TrainSW,  SDCARD[4:CS 3:MISO 2:MOSI 1:SCK], 0:PPM_OUT|IDL2_SW
  DDRC = 0x3f;  PORTC = 0xc0; // 7:AilDR, 6:EleDR, LCD[5,4,3,2,1[, 0:BackLight
  DDRD = 0x01;  PORTD = 0xfe; // 7/6:Spare3/4, 5:RENC2_PUSH, 4:RENC1_PUSH, 3:RENC2_B, 2:RENC2_A, 1:I2C_SDA, 0:I2C_SCL
  DDRE = 0b00001010;  PORTE = 0b11110101; // 7:PPM_IN, 6: RENC1_B, 5:RENC1_A, 4:USB_DNEG, 3:BUZZER, 2:USB_DPOS, 1:TELEM_TX, 0:TELEM_RX
  DDRF = 0x00;  PORTF = 0x00; // 7-4:JTAG, 3:ADC_REF_1.2V input, 2-0:ADC_SPARE_2-0
  DDRG = 0b00010000;  PORTG = 0xff; // 7-6:N/A, 5:GearSW, 4: Sim_Ctrl[out], 3:IDL1_Sw, 2:TCut_Sw, 1:RF_Power[in], 0: RudDr_Sw
  DDRH = 0x00;  PORTH = 0xff; // 7:0 Spare port -- all inputer for now [Bit 2:VIB_OPTION -- setting to input for now]
  DDRJ = 0x00;  PORTJ = 0xff; // 7-0:Trim switch inputs
  DDRK = 0x00;  PORTK = 0x00; // anain. No pull-ups!
  DDRL = 0x00;  PORTL = 0xff; // 7-6:Spare6/5 (inputs), 5-0: User Button inputs
#else
#  if defined (PCBV3)
  DDRB = 0x97;  PORTB = 0x1e; // 7:AUDIO, SD_CARD[6:SDA 5:SCL 4:CS 3:MISO 2:MOSI 1:SCK], 0:PPM_OUT
  DDRC = 0x3f;  PORTC = 0xc0; // PC0 used for LCD back light control
  DDRD = 0x0F;  PORTD = 0xff; // 7:4=inputs (keys/trims, pull-ups on), 3:0=outputs (keyscan row select)
#  else
  DDRB = 0x81;  PORTB = 0x7e; //pullups keys+nc
  DDRC = 0x3e;  PORTC = 0xc1; //pullups nc
  DDRD = 0x00;  PORTD = 0xff; //pullups keys
#  endif
  DDRE = (1<<OUT_E_BUZZER);  PORTE = 0xff-(1<<OUT_E_BUZZER); //pullups + buzzer 0
  DDRF = 0x00;  PORTF = 0x00; //anain
  DDRG = 0x10;  PORTG = 0xff; //pullups + SIM_CTL=1 = phonejack = ppm_in
#endif

  lcd_init();

  ADMUX=ADC_VREF_TYPE;
  ADCSRA=0x85; // ADC enabled, pre-scaler division=32 (no interrupt, no auto-triggering)
#if defined (PCBV4)
  ADCSRB=(1<<MUX5);
#endif

  /**** Set up timer/counter 0 ****/
#if defined (PCBV3)
  /** Move old 64A Timer0 functions to Timer2 and use WGM on OC0(A) (PB7) for spkear tone output **/

  // TCNT0  10ms = 16MHz/1024/156(.25) periodic timer (100ms interval)
  //        cycles at 9.984ms but includes 1:4 duty cycle correction to /157 to average at 10.0ms
  TCCR2B  = (0b111 << CS20); // Norm mode, clk/1024 (differs from ATmega64 chip)
  OCR2A   = 156;
  TIMSK2 |= (1<<OCIE2A) |  (1<<TOIE2); // Enable Output-Compare and Overflow interrrupts

  // Set up Phase correct Waveform Gen. mode, at clk/64 = 250,000 counts/second
  // (Higher speed allows for finer control of frquencies in the audio range.)
  // Used for audio tone generation
  TCCR0B  = (1<<WGM02) | (0b011 << CS00);
  TCCR0A  = (0b01<<WGM00);

#else

# if defined (BEEPSPKR)
  // TCNT0  10ms = 16MHz/1024/2(/78) periodic timer (for speaker tone generation)
  //        Capture ISR 7812.5/second -- runs per-10ms code segment once every 78
  //        cycles (9.984ms). Timer overflows at about 61Hz or once every 16ms.
  TCCR0  = (0b111 << CS00);//  Norm mode, clk/1024
  OCR0 = 2;
# else
  // TCNT0  10ms = 16MHz/1024/156 periodic timer (9.984ms)
  // (with 1:4 duty at 157 to average 10.0ms)
  // Timer overflows at about 61Hz or once every 16ms.
  TCCR0  = (0b111 << CS00);//  Norm mode, clk/1024
  OCR0 = 156;
# endif

  TIMSK |= (1<<OCIE0) |  (1<<TOIE0); // Enable Output-Compare and Overflow interrrupts
  /********************************/

#endif

  // TCNT1 2MHz counter (auto-cleared) plus Capture Compare int.
  //       Used for PPM pulse generator
  TCCR1B = (1 << WGM12) | (2<<CS10); // CTC OCR1A, 16MHz / 8
  // not here ... TIMSK1 |= (1<<OCIE1A); ... enable immediately before mainloop

  // TCNT3 (2MHz) used for PPM_IN pulse width capture
#if defined (PPMIN_MOD1) || defined (PCBV3)
  // Noise Canceller enabled, pos. edge, clock at 16MHz / 8 (2MHz)
  TCCR3B  = (1<<ICNC3) | (1<<ICES3) | (0b010 << CS30);
#else
  // Noise Canceller enabled, neg. edge, clock at 16MHz / 8 (2MHz)
  TCCR3B  = (1<<ICNC3) | (0b010 << CS30);
#endif

#if defined (PCBV3)
  TIMSK3 |= (1<<ICIE3);         // Enable capture event interrupt
#else
  ETIMSK |= (1<<TICIE3);
#endif

  sei(); // interrupts needed for eeReadAll function (soon).

  g_menuStack[0] = menuMainView;
  g_menuStack[1] = menuProcModelSelect;

  lcdSetRefVolt(25);

#if defined (FRSKY)
  FRSKY_Init();
#endif

#ifdef JETI
  JETI_Init();
#endif

  eeReadAll();

  uint8_t cModel = g_eeGeneral.currModel;
  checkQuickSelect();

  doSplash();
  checkMem();

  getADC_single();
  checkTHR();

  checkSwitches();
  checkAlarm();

  clearKeyEvents(); //make sure no keys are down before proceeding

  //addon Vinceofdrink@gmail (hardware ppm)
  #if defined (DPPMPB7_HARDWARE)
    TCCR1A |=(1<<COM1C0); // (COM1C1=0 and COM1C0=1 in TCCR1A)  toogle the state of PB7  on each TCNT1=OCR1C
  #endif

  setupPulses();

  wdt_enable(WDTO_500MS);

  perOut(g_chans512);

  lcdSetRefVolt(g_eeGeneral.contrast);
  g_LightOffCounter = g_eeGeneral.lightAutoOff*500; //turn on light for x seconds - no need to press key Issue 152

  if(cModel!=g_eeGeneral.currModel) eeDirty(EE_GENERAL); // if model was quick-selected, make sure it sticks



#if defined (PCBV3)
  TIMSK1 |= (1<<OCIE1A); // Pulse generator enable immediately before mainloop
#else
  TIMSK |= (1<<OCIE1A); // Pulse generator enable immediately before mainloop
#endif

#if defined (PCBV3)
// Initialise global unix timestamp with current time from RTC chip on SD card interface
  RTC rtc;
  struct tm utm;
  rtc_gettime(&rtc);
  utm.tm_year = rtc.year - 1900;
  utm.tm_mon =  rtc.month - 1;
  utm.tm_mday = rtc.mday;
  utm.tm_hour = rtc.hour;
  utm.tm_min =  rtc.min;
  utm.tm_sec =  rtc.sec;
  utm.tm_wday = rtc.wday - 1;
  g_unixTime = mktime(&utm);
#endif

  while(1){
    uint16_t t0 = getTmr16KHz();
    getADC[g_eeGeneral.filterInput]();
    getADC_bandgap() ;
    perMain();

    if(heartbeat == 0x3)
    {
      wdt_reset();
      heartbeat = 0;
    }
    t0 = getTmr16KHz() - t0;
    g_timeMain = max(g_timeMain,t0);
  }
}
#endif

