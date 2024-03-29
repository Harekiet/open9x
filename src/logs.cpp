/*
 * Authors (alphabetical order)
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Jean-Pierre Parisy
 * - Karl Szmutny <shadow@privy.de>
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * open9x is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
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

#include "open9x.h"
#include "FatFs/ff.h"

char g_logFilename[21]; //
FIL g_oLogFile;
const pm_char * g_logError = NULL;
uint8_t logDelay;

const pm_char * openLogs()
{
  // Determine and set log file filename
  FRESULT result;
  DIR folder;
  char filename[24];

  // close any file left open. E.G. Changing models with log switch still on.
  if (g_oLogFile.fs) f_close(&g_oLogFile);

  strcpy_P(filename, STR_LOGS_PATH);

  result = f_opendir(&folder, filename);
  if (result != FR_OK) {
    if (result == FR_NO_PATH)
      result = f_mkdir(filename);
    if (result != FR_OK)
      return SDCARD_ERROR(result);
  }

  filename[sizeof(LOGS_PATH)-1] = '/';
  memcpy(&filename[sizeof(LOGS_PATH)], g_model.name, sizeof(g_model.name));
  filename[sizeof(LOGS_PATH)+sizeof(g_model.name)] = '\0';

  uint8_t i = sizeof(LOGS_PATH)+sizeof(g_model.name)-1;
  uint8_t len = 0;
  while (i>sizeof(LOGS_PATH)-1) {
    if (!len && filename[i])
      len = i+1;
    if (len) {
      if (filename[i])
        filename[i] = idx2char(filename[i]);
      else
        filename[i] = '_';
    }
    i--;
  }

  if (len == 0) {
    uint8_t num = g_eeGeneral.currModel + 1;
    strcpy_P(&filename[sizeof(LOGS_PATH)], STR_MODEL);
    filename[sizeof(LOGS_PATH) + PSIZE(TR_MODEL)] = (char)((num / 10) + '0');
    filename[sizeof(LOGS_PATH) + PSIZE(TR_MODEL) + 1] = (char)((num % 10) + '0');
    len = sizeof(LOGS_PATH) + PSIZE(TR_MODEL) + 2;
  }

  strcpy_P(&filename[len], STR_LOGS_EXT);

  result = f_open(&g_oLogFile, filename, FA_OPEN_ALWAYS | FA_WRITE);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  if (g_oLogFile.fsize == 0) {
#if defined(PCBV4)
    f_puts("Date,Time,", &g_oLogFile);
#endif

#if defined(FRSKY)
    f_puts("Buffer,RX,TX,A1,A2,", &g_oLogFile);
#endif

#if defined(FRSKY_HUB)
    if (g_model.frsky.usrProto == USR_PROTO_FRSKY_HUB)
      f_puts("GPS Date,GPS Time,Long,Lat,Course,GPS Speed,GPS Alt,Baro Alt,Temp1,Temp2,RPM,Fuel,Volts,AccelX,AccelY,AccelZ,", &g_oLogFile);
#endif

    f_puts("Rud,Ele,Thr,Ail,P1,P2,P3,THR,RUD,ELE,ID0,ID1,ID2,AIL,GEA,TRN\n", &g_oLogFile);
  }

  result = f_lseek(&g_oLogFile, g_oLogFile.fsize); // append
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  return NULL;
}

uint16_t lastLogTime = 0;

void closeLogs()
{
  f_close(&g_oLogFile);
  lastLogTime = 0;
}

void writeLogs()
{
  if (isFunctionActive(FUNC_LOGS) && logDelay > 0) {
    uint16_t tmr10ms = get_tmr10ms();
    if (lastLogTime == 0 || tmr10ms - lastLogTime >= 10*logDelay) {
      lastLogTime = tmr10ms;

      if (!g_oLogFile.fs) {
        if (openLogs())
          return;
      }

#if defined(PCBV4)
      static struct gtm t;
      struct gtm *at = &t;
      filltm(&g_unixTime, &t);

      f_printf(&g_oLogFile, "%4d-%02d-%02d,", at->tm_year+1900, at->tm_mon+1, at->tm_mday);
      f_printf(&g_oLogFile, "%02d:%02d:%02d,", at->tm_hour, at->tm_min, at->tm_sec);
#endif

#if defined(FRSKY)
      f_printf(&g_oLogFile, "%d,", frskyStreaming);
      f_printf(&g_oLogFile, "%d,", frskyData.rssi[0].value);
      f_printf(&g_oLogFile, "%d,", frskyData.rssi[1].value);
      f_printf(&g_oLogFile, "%d,", frskyData.analog[0].value);
      f_printf(&g_oLogFile, "%d,", frskyData.analog[1].value);
#endif

#if defined(FRSKY_HUB)
      if (g_model.frsky.usrProto == USR_PROTO_FRSKY_HUB) {
        f_printf(&g_oLogFile, "%4d-%02d-%02d,", frskyData.hub.year+2000, frskyData.hub.month, frskyData.hub.day);
        f_printf(&g_oLogFile, "%02d:%02d:%02d,", frskyData.hub.hour, frskyData.hub.min, frskyData.hub.sec);
        f_printf(&g_oLogFile, "%03d.%04d%c,", frskyData.hub.gpsLongitude_bp, frskyData.hub.gpsLongitude_ap,
            frskyData.hub.gpsLongitudeEW ? frskyData.hub.gpsLongitudeEW : '-');
        f_printf(&g_oLogFile, "%03d.%04d%c,", frskyData.hub.gpsLatitude_bp, frskyData.hub.gpsLatitude_ap,
            frskyData.hub.gpsLatitudeNS ? frskyData.hub.gpsLatitudeNS : '-');
        f_printf(&g_oLogFile, "%03d.%d,", frskyData.hub.gpsCourse_bp, frskyData.hub.gpsCourse_ap);
        f_printf(&g_oLogFile, "%d.%d,", frskyData.hub.gpsSpeed_bp, frskyData.hub.gpsSpeed_ap);
        f_printf(&g_oLogFile, "%03d.%d,", frskyData.hub.gpsAltitude_bp, frskyData.hub.gpsAltitude_ap);
        f_printf(&g_oLogFile, "%d.%d,", frskyData.hub.baroAltitude_bp, frskyData.hub.baroAltitude_ap);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.temperature1);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.temperature2);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.rpm);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.fuelLevel);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.volts);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.accelX);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.accelY);
        f_printf(&g_oLogFile, "%d,", frskyData.hub.accelZ);
      }
#endif

      for (uint8_t i=0; i<NUM_STICKS+NUM_POTS; i++)
        f_printf(&g_oLogFile, "%d,", calibratedStick[i]);

      f_printf(&g_oLogFile, "%d,", keyState(SW_ThrCt));
      f_printf(&g_oLogFile, "%d,", keyState(SW_RuddDR));
      f_printf(&g_oLogFile, "%d,", keyState(SW_ElevDR));
      f_printf(&g_oLogFile, "%d,", keyState(SW_ID0));
      f_printf(&g_oLogFile, "%d,", keyState(SW_ID1));
      f_printf(&g_oLogFile, "%d,", keyState(SW_ID2));
      f_printf(&g_oLogFile, "%d,", keyState(SW_AileDR));
      f_printf(&g_oLogFile, "%d,", keyState(SW_Gear));
      f_printf(&g_oLogFile, "%d\n", keyState(SW_Trainer));
    }
  }
}



