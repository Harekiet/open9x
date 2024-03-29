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

#ifndef audio_h
#define audio_h

#define AUDIO_QUEUE_LENGTH (20)
#define BEEP_DEFAULT_FREQ  (70)
#define BEEP_OFFSET        (10)
#define BEEP_KEY_UP_FREQ   (BEEP_DEFAULT_FREQ+5)
#define BEEP_KEY_DOWN_FREQ (BEEP_DEFAULT_FREQ-5)

class ToneFragment {
  public:
    uint8_t freq;
    uint8_t duration;
    uint8_t pause;
    uint8_t repeat;
    int8_t  freqIncr;
};

class AudioFragment : public ToneFragment {
  public:
    char file[32+1];
};

extern "C" void DAC_IRQHandler();

#define AUDIO_SLEEPING     0
#define AUDIO_RESUMING     1
#define AUDIO_PLAYING_TONE 2
#define AUDIO_PLAYING_WAV  3

class AudioQueue {

  friend void audioTask(void* pdata);
  friend void DAC_IRQHandler();

  public:

    AudioQueue();

    void play(uint8_t tFreq, uint8_t tLen, uint8_t tPause, uint8_t tFlags=0, int8_t tFreqIncr=0);

    void playFile(const char *filename, uint8_t tFlags=0);

    void pause(uint8_t tLen);

    bool busy()
    {
#ifdef SIMU
      return false;
#else
      return (state != AUDIO_SLEEPING);
#endif
    }

  protected:

    void wakeup();

    uint8_t state;
    uint8_t ridx;
    uint8_t widx;
    int8_t prioIdx;
    AudioFragment fragments[AUDIO_QUEUE_LENGTH];
    ToneFragment background; // for vario

    AudioFragment current;

};

extern AudioQueue audioQueue;

void alawInit();
extern "C" void retrieveAvailableAudioFiles();

void audioEvent(uint8_t e, uint8_t f=BEEP_DEFAULT_FREQ);

#define AUDIO_KEYPAD_UP()   audioEvent(AU_KEYPAD_UP)
#define AUDIO_KEYPAD_DOWN() audioEvent(AU_KEYPAD_DOWN)
#define AUDIO_MENUS()       audioEvent(AU_MENUS)
#define AUDIO_WARNING1()    audioEvent(AU_WARNING1)
#define AUDIO_WARNING2()    audioEvent(AU_WARNING2)
#define AUDIO_TX_BATTERY_LOW() audioEvent(AU_TX_BATTERY_LOW)
#define AUDIO_ERROR()       audioEvent(AU_ERROR)
#define AUDIO_ERROR_MESSAGE(e) audioEvent(e)
#define AUDIO_TIMER_30()    audioEvent(AU_TIMER_30)
#define AUDIO_TIMER_20()    audioEvent(AU_TIMER_20)
#define AUDIO_TIMER_10()    audioEvent(AU_TIMER_10)
#define AUDIO_TIMER_LT3()   audioEvent(AU_TIMER_LT3)
#define AUDIO_MINUTE_BEEP() audioEvent(AU_WARNING1)
#define AUDIO_INACTIVITY()  audioEvent(AU_INACTIVITY)
#define AUDIO_MIX_WARNING_1() audioEvent(AU_MIX_WARNING_1)
#define AUDIO_MIX_WARNING_2() audioEvent(AU_MIX_WARNING_2)
#define AUDIO_MIX_WARNING_3() audioEvent(AU_MIX_WARNING_3)
#define AUDIO_POT_STICK_MIDDLE() audioEvent(AU_POT_STICK_MIDDLE)
#define AUDIO_VARIO_UP()    audioEvent(AU_KEYPAD_UP)
#define AUDIO_VARIO_DOWN()  audioEvent(AU_KEYPAD_DOWN)
#define AUDIO_TRIM_MIDDLE(f) audioEvent(AU_TRIM_MIDDLE, f)
#define AUDIO_TRIM_END(f)    audioEvent(AU_TRIM_END, f)
#define AUDIO_TRIM(event, f) audioEvent(AU_TRIM_MOVE, f)
#define AUDIO_PLAY(p)        audioEvent(p)
#define AUDIO_VARIO(f, t)    audioQueue.play(f, t, 0, PLAY_SOUND_VARIO)

#define AUDIO_HEARTBEAT()

extern void pushPrompt(uint16_t prompt);

#endif
