/****************************************************************************
*  Copyright (c) 2011 by Michael Blandford. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may
*     be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*  SUCH DAMAGE.
*
****************************************************************************
*  History:
*
****************************************************************************/

#ifndef sound_driver_h
#define sound_driver_h

#define NUM_VOL_LEVELS	24

extern void startSound( void ) ;
extern void buzzer_on( void ) ;
extern void buzzer_off( void ) ;
extern void buzzer_sound( uint8_t time ) ;
extern void setFrequency( uint32_t frequency ) ;
extern void start_timer1( void ) ;
extern void initDac( void ) ;
extern void end_sound( void ) ;

#define WAV_BUFFER_SIZE 1200
extern uint16_t wavSamplesArray[2*WAV_BUFFER_SIZE]; /* 2 buffers of 100ms at 12kHz */
extern uint16_t *wavSamplesBuffer;

inline void toneStart()
{
  PMC->PMC_PCER0 |= 0x40000000L ; // Enable peripheral clock to DAC
  DACC->DACC_IER = DACC_IER_ENDTX ;
}

inline void toneStop()
{
  DACC->DACC_IDR = DACC_IDR_ENDTX ; // Disable interrupt
}

extern volatile uint8_t Buzzer_count ;

extern void init_twi( void ) ;
extern void set_volume( register uint8_t volume ) ;
extern "C" void TWI0_IRQHandler (void) ;

#endif
