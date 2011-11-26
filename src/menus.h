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

#ifndef menus_h
#define menus_h

#include <inttypes.h>
#include "gruvin9x.h"

#define IS_THROTTLE(x)  (((2-(g_eeGeneral.stickMode&1)) == x) && (x<4))

#define NO_HI_LEN 25

#define RESX    1024
#define RESXu   1024u
#define RESXul  1024ul
#define RESXl   1024l
#define RESKul  100ul
#define RESX_PLUS_TRIM (RESX+128)

typedef void (*MenuFuncP)(uint8_t event);

void DisplayScreenIndex(uint8_t index, uint8_t count, uint8_t attr);

extern uint8_t s_pgOfs;
extern uint8_t s_noHi;
extern uint8_t s_noScroll;

void menu_lcd_onoff(uint8_t x, uint8_t y, uint8_t value, uint8_t mode);
void menu_lcd_HYPHINV(uint8_t x, uint8_t y, uint8_t value, uint8_t mode);

extern MenuFuncP g_menuStack[5];
extern uint8_t g_menuStackPtr;

/// goto given Menu, but substitute current menu in menuStack
void chainMenu(MenuFuncP newMenu);
/// goto given Menu, store current menu in menuStack
void pushMenu(MenuFuncP newMenu);
/// return to last menu in menustack
void popMenu();
///deliver address of last menu which was popped from
inline MenuFuncP lastPopMenu()
{
  return g_menuStack[g_menuStackPtr+1];
}

void doMainScreenGrphics();
void menuMainView(uint8_t event);
void menuProcSetup(uint8_t event);
void menuProcModelSelect(uint8_t event);
void menuProcStatistic(uint8_t event);
void menuProcStatistic2(uint8_t event);

extern int16_t p1valdiff;
extern int8_t  checkIncDec_Ret;  // global helper vars
extern uint8_t s_editMode;    // global editmode

int16_t checkIncDec(uint8_t event, int16_t i_pval, int16_t i_min, int16_t i_max, uint8_t i_flags);
int8_t checkIncDecModel(uint8_t event, int8_t i_val, int8_t i_min, int8_t i_max);
int8_t checkIncDecGen(uint8_t event, int8_t i_val, int8_t i_min, int8_t i_max);

#define CHECK_INCDEC_MODELVAR( event, var, min, max)     \
  var = checkIncDecModel(event,var,min,max)

#define CHECK_INCDEC_GENVAR( event, var, min, max)     \
  var = checkIncDecGen(event,var,min,max)

// Menus related stuff ...
extern uint8_t m_posVert;
extern uint8_t m_posHorz;
inline void minit(){m_posVert=m_posHorz=0;}
bool check(uint8_t event, uint8_t curr, MenuFuncP *menuTab, uint8_t menuTabSize, prog_uint8_t *subTab, uint8_t subTabMax, uint8_t maxrow);
bool check_simple(uint8_t event, uint8_t curr, MenuFuncP *menuTab, uint8_t menuTabSize, uint8_t maxrow);
bool check_submenu_simple(uint8_t event, uint8_t maxrow);

typedef PROGMEM void (*MenuFuncP_PROGMEM)(uint8_t event);

#define TITLEP(pstr) lcd_putsAtt(0,0,pstr,INVERS)
#define TITLE(str)   TITLEP(PSTR(str))

#define MENU(title, tab, menu, lines_count, lines...) \
TITLE(title); \
static prog_uint8_t APM mstate_tab[] = lines; \
if (!check(event,menu,tab,DIM(tab),mstate_tab,DIM(mstate_tab)-1,lines_count-1)) return;

#define SIMPLE_MENU_NOTITLE(tab, menu, lines_count) \
if (!check_simple(event,menu,tab,DIM(tab),lines_count-1)) return;

#define SIMPLE_MENU(title, tab, menu, lines_count) \
TITLE(title); \
SIMPLE_MENU_NOTITLE(tab, menu, lines_count)

#define SUBMENU(title, lines_count, lines...) \
TITLE(title); \
static prog_uint8_t APM mstate_tab[] = lines; \
if (!check(event,0,NULL,0,mstate_tab,DIM(mstate_tab)-1,lines_count-1)) return;

#define SIMPLE_SUBMENU_NOTITLE(lines_count) \
if (!check_submenu_simple(event,lines_count-1)) return;

#define SIMPLE_SUBMENU(title, lines_count) \
TITLE(title); \
SIMPLE_SUBMENU_NOTITLE(lines_count)

#endif
