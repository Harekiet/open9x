/*
 * Authors (alphabetical order)
 * - Cameron Weeks <th9xer@gmail.com>
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

/*
 I did think about implimenting this with an array for the timing durations
 like the original bit bang DSM2 code was done but that meant another array 
 on top of the playlist one, therefore more memory waisted, and because 
 the playlist must be executed one after another it would mean putting 
 the code to fill the array in the interrupt (This would be unlike the 
 old DSM2 code) which seemed to much for the interrupt. 
 */

#include "open9x.h"

// Start and stop bits need to be 2ms in duration. Start bit is low, stop bit is high
#define SOMOSSBIT    4 //The 2ms of a stop/start bit
#define SOMOSTOP    48 //This is the needed 2ms (4) + 20ms (40) to allow for the 
                       //point at which the busy flag is checkable + 2ms for saftey (4)
#define SOMOSTART   60 //This is the needed 2ms (4) + 2ms (4) for safety + 26ms (52) for the 
                       //undocumented delay that must exist between playbacks.
#define	SOMOBUSY		(PINH & 0x40)	/* SOMO14D Busy flag */
#define	SOMOCLK 		(PINH & 0x10)	/* SOMO14D CLK */

#define QUEUE_LENGTH  10
uint16_t somo14playlist[QUEUE_LENGTH] = {0};
uint16_t somo14command = 0;
uint8_t somo14RIdx = 0;
volatile uint8_t somo14WIdx = 0;

#ifdef SIMU
#define BUSY busy
bool TIMER4_COMPA_vect()
#else
#define BUSY
ISR(TIMER4_COMPA_vect) //Every 0.5ms
#endif
{
  static uint16_t somo14_current;
  static uint8_t i=0;
  static uint8_t busy=0; // debugging make BOOL when done
  static uint8_t startstop=SOMOSSBIT; // Only the first playback can have this short delay

  OCR4A = 0x7d; //another 0.5ms 

  // printf("startstop=%d i=%d\n", startstop, i); fflush(stdout);

  // Send no data unless we are either not busy or it's a command

    // Only when stop bit is sent do we increment read pointer
    // This will happen before the SOMO14D busy flag gets set
    // Busy flag happens 20ms after the last data is clocked in
    if (i==0 && !busy) {
      if(somo14command) { // Allow a command to be send even if the SOMO is busy, i.e. STOP or PAUSE
        somo14_current=somo14command;
      }
      else if (!SOMOBUSY) {
        if (somo14RIdx == somo14WIdx) {
          // TODO disable the interrupt (and re-enable when a new prompt is pushed)?
          return BUSY;
        }
        somo14_current = somo14playlist[somo14RIdx];
        somo14RIdx = (somo14RIdx + 1) % QUEUE_LENGTH;
      }
      busy = 1;
    }

    // Start and stop bits
    if (busy && startstop) {
      // Start bit
      if (i==0) {
        if (startstop < SOMOSSBIT) { // This stretches the time before we start sending data to allow for the
          // undocumented delay that must exist between playbacks. Unfortunately the
          // device isn't ready even though the busy flag says it is.
          PORTH &= ~(1<<OUT_H_14DCLK); // Start Bit, CLK low for 2ms
        }
        startstop--;
        return BUSY;
      }
      // Stop bit
      if (i==16) {
        PORTH &= ~(1<<OUT_H_14DDATA); // Data low
        PORTH |= (1<<OUT_H_14DCLK); // Stop Bit, CLK high for 2ms
        startstop--;
        return BUSY;
      }
    }

    // After stop bit is sent
    // we are done sending so prepair for next
    if (i==16 && !startstop) {
      // Cam, here I think there is a bug (if we send a "command") somo14RPtr++;
      i=0;
      busy=0;
      startstop=SOMOSTART;
      return BUSY;
    }

    // Send the data
    if (i<16 && busy) {
      if (!SOMOCLK) { // Only change data when the CLK is low
        startstop =SOMOSTOP;
        if (somo14_current & 0x8000) {
          PORTH |= (1<<OUT_H_14DDATA); // Data high
        }
        somo14_current = (somo14_current<<1);
        i++;
        // Strictly speaking there should be a data setup delay in here of 1us
        // Be we don't like no stinking delays !
        PORTH |= (1<<OUT_H_14DCLK); // CLK high
      }

      else { // Don't alter after sending last bit in preparation for sending stop bit
        PORTH &= ~(1<<OUT_H_14DCLK); // CLK low
        PORTH &= ~(1<<OUT_H_14DDATA); // Data low
      }
    }
    
    return BUSY;
}

void somoPushPrompt(uint16_t prompt)
{
  somo14playlist[somo14WIdx] = prompt;
  somo14WIdx = (somo14WIdx + 1) % QUEUE_LENGTH;
}
