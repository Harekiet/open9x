# !!!! BETA Makefile !!!!
# !!!! Use at own risk !!!!
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# make filename.i = Create a preprocessed source file for use in submitting
#                   bug reports to the GCC project.
#
# To rebuild project do "make clean" then "make all".
#----------------------------------------------------------------------------

#----------- BUILD OPTIONS ---------------------------

# PCB version
# Values: STD, V4, ARM
PCB = STD

# PCB revision
# Values: REVA, REVB
PCBREV = REVB

# Enable JETI-Telemetry or FrSky Telemetry reception on UART0
# For this option you need to modify your hardware!
# More information at [insertURLhere]
# Values = STD, FRSKY, JETI, NMEA, ARDUPILOT, MAVLINK
EXT = STD

# Enable heli menu
# Values = YES, NO
HELI = NO

# Enable templates menu
# Values = YES, NO
TEMPLATES = YES

# Enable navigation with Pots / Rotary encoders
# Values = NO
#          POT1 (only POT1 available for fields modification),
#          POT2 (menus scroll),
#          POT3 (cursor down/up),
#          POTS (POT1, POT2, POT3),
NAVIGATION = NO

# Switches autmatic selection
# Values = YES, NO 
AUTOSWITCH = NO

# AUDIO Mods
# Values = YES, NO 
AUDIO = NO

# HAPTIC Mods
# Values = YES, NO 
HAPTIC = NO

# SPLASH on START
SPLASH = YES

# GRAPHICS (sliders / checkboxes / bold)
GRAPHICS = YES
BOLD = YES

# THR trace (in STATS screen)
THR_TRACE = YES

# PPM center adjustable
# Values = YES, NO
PPM_CENTER_ADJUSTABLE = NO

# Limits display in the LIMITS
# Values = PERCENT, US
PPM_LIMITS_UNIT = PERCENT

# Decimals display in the main view / PPM calibration, 
# Values = YES, NO
DECIMALS = YES

# DISPLAY_USER_DATA to display on screen data from FrSky module (testing/demo purpose)
# Values = YES, NO
DISPLAY_USER_DATA = NO

# FrSky Hub 
# Values = YES, NO
FRSKY_HUB = YES

# WS HowHigh 
# Values = YES, NO
WS_HOW_HIGH = YES

# WS HowHigh 
# Values = YES, NO
VARIO = YES

# SDCARD
# Values = YES, NO
SDCARD = NO

# PXX (FrSky PCM) protocol
PXX = NO

# DSM2 (Spektrum) protocol
DSM2 = NO

#IR protocols (SILV / TRAC09 / PICZ / SWIFT)
IRPROTOS = NO

# SOMO-14D module (V4 board only)
SOMO = NO

# TRANSLATIONS
# Values = cz, de, en, fr, it, se  
TRANSLATIONS = EN

# TTS
# Values = en, fr
TTS = $(TRANSLATIONS)

# UNITS
# Values = imperial, metric 
UNITS = METRIC

# EEPROM_PROGRESS_BAR
# Values = YES, NO
EEPROM_PROGRESS_BAR = NO

# Enable extra rotary encoders (V4 board only)
# Values = YES, NO
EXTRA_ROTARY_ENCODERS = NO

# DEBUG mode
DEBUG = NO

# Flight Phases
# Values = YES, NO
FLIGHT_PHASES = YES

#------- END BUILD OPTIONS ---------------------------

# Define programs and commands.
SHELL = sh
IMG2LBM = python ../util/img2lbm.py
SVNREV = $(shell sh -c "svnversion | egrep -o '[[:digit:]]+[[:alpha:]]*$$'")

CPPDEFS = 

# MCU name
ifeq ($(PCB), STD)
  TRGT = avr-
  MCU = atmega64  
  CPPDEFS += -DF_CPU=$(F_CPU)UL
endif

ifeq ($(PCB), V4)
  ifeq ($(PCBREV), REV0)
    CPPDEFS += -DREV0
  else
    CPPDEFS += -DREV1
  endif
  TRGT = avr-
  MCU = atmega2560
  CPPDEFS += -DF_CPU=$(F_CPU)UL
endif

ifeq ($(PCB), ARM)
  ifeq ($(PCBREV), REVA)
    CPPDEFS += -DREVA
    LDSCRIPT = ersky9x/sam3s2c_flash.ld
  else
    CPPDEFS += -DREVB
    LDSCRIPT = ersky9x/sam3s4c_flash.ld
  endif
  TRGT = arm-none-eabi-
  MCU  = cortex-m3
endif

CC      = $(TRGT)gcc
AS      = $(TRGT)as
OBJCOPY = $(TRGT)objcopy
OBJDUMP = $(TRGT)objdump
SIZE    = $(TRGT)size
NM      = $(TRGT)nm

AVRDUDE = avrdude
REMOVE = rm -f
REMOVEDIR = rm -rf

# Processor frequency.
F_CPU = 16000000

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Target file name (without extension).
TARGET = open9x

# List C++ source files here. (C dependencies are automatically generated.)
SRC =
STD_TTS_SRC = translations/tts_$(shell sh -c "echo $(TTS) | tr '[:upper:]' '[:lower:]'").cpp
TTS_SRC = $(shell sh -c "if test -f $(STD_TTS_SRC); then echo $(STD_TTS_SRC); else echo translations/tts_en.cpp; fi")

CPPSRC = open9x.cpp $(PULSESSRC) stamp.cpp menus.cpp model_menus.cpp general_menus.cpp main_views.cpp statistics_views.cpp $(EEPROMSRC) lcd.cpp drivers.cpp translations.cpp $(TTS_SRC)

ifeq ($(EXT), JETI)
 CPPSRC += jeti.cpp
endif

ifeq ($(EXT), ARDUPILOT)
 CPPSRC += ardupilot.cpp
endif

ifeq ($(EXT), NMEA)
 CPPSRC += nmea.cpp
endif

# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DBGFMT = dwarf-2

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRAINCDIRS = . translations

# NOTE: PCB version now overrides all the earlier individual settings
#       These individual settings work only for PCB=STD

CPPDEFS += -DTRANSLATIONS_$(TRANSLATIONS)

ifeq ($(UNITS), IMPERIAL)
  CPPDEFS += -DIMPERIAL_UNITS
endif

# If POTS are used for fields modification
ifeq ($(NAVIGATION), POT1)
  CPPDEFS += -DNAVIGATION_POT1
endif
ifeq ($(NAVIGATION), POT2)
  CPPDEFS += -DNAVIGATION_POT2
endif
ifeq ($(NAVIGATION), POT3)
  CPPDEFS += -DNAVIGATION_POT3
endif
ifeq ($(NAVIGATION), POTS)
  CPPDEFS += -DNAVIGATION_POT1 -DNAVIGATION_POT2 -DNAVIGATION_POT3
endif

ifeq ($(AUTOSWITCH), YES)
  CPPDEFS += -DAUTOSWITCH
endif

ifeq ($(SPLASH), YES)
  CPPDEFS += -DSPLASH
endif

ifeq ($(GRAPHICS), YES)
  CPPDEFS += -DGRAPHICS
endif

ifeq ($(BOLD), YES)
  CPPDEFS += -DBOLD_FONT
endif

ifeq ($(THR_TRACE), YES)
  CPPDEFS += -DTHRTRACE
endif

ifeq ($(FLIGHT_PHASES), YES)
  CPPDEFS += -DFLIGHT_PHASES
endif

# If STD-Support is enabled
ifeq ($(EXT), STD)
  CPPDEFS += -DEXTSTD
endif

# If ARDUPILOT-Support is enabled
ifeq ($(EXT), ARDUPILOT)
  CPPDEFS += -DARDUPILOT
endif

# If NMEA-Support is enabled
ifeq ($(EXT), NMEA)
  CPPDEFS += -DNMEA
endif

# If JETI-Support is enabled
ifeq ($(EXT), JETI)
  MODS:=${MODS}J
  CPPDEFS += -DJETI
endif

ifeq ($(EXT), MAVLINK)
 MODS:=${MODS}M
 CPPDEFS += -DMAVLINK
 CPPSRC += mavlink.cpp rotarysw.cpp serial.cpp
endif

# If FRSKY-Support is enabled
ifeq ($(EXT), FRSKY)
  MODS:=${MODS}F
  CPPDEFS += -DFRSKY
  CPPSRC += frsky.cpp
  # If FRSKY-Hub is enabled
  ifeq ($(FRSKY_HUB), YES)
    CPPDEFS += -DFRSKY_HUB
  endif
  # If WS HowHigh is enabled
  ifeq ($(WS_HOW_HIGH), YES)
    MODS:=${MODS}W
    CPPDEFS += -DWS_HOW_HIGH
  endif
  # If Vario is enabled
  ifeq ($(VARIO), YES)
    CPPDEFS += -DVARIO
  endif
endif

ifeq ($(DEBUG), YES)
  CPPDEFS += -DDEBUG
endif

ifeq ($(EEPROM_PROGRESS_BAR), YES)
  CPPDEFS += -DEEPROM_PROGRESS_BAR
endif

ifeq ($(SDCARD), YES)
  MODS:=${MODS}S
  CPPDEFS += -DSDCARD
  CPPSRC += logs.cpp 
endif

RUN_FROM_FLASH = 1

ifeq ($(PCB), ARM)
  # V4 ARM, so ...
  OPT = 2
  CPPDEFS += -DPCBARM -DAUDIO -DHAPTIC -DPXX -DDSM2 -DDSM2_PPM -DROTARY_ENCODERS=1
  EXTRAINCDIRS += ersky9x CoOS/kernel CoOS/portable
  BOARDSRC = board_ersky9x.cpp 
  SRC += ersky9x/core_cm3.c ersky9x/board_lowlevel.c ersky9x/crt.c ersky9x/vectors_sam3s.c
  SRC += CoOS/kernel/core.c CoOS/kernel/hook.c CoOS/kernel/task.c CoOS/kernel/event.c CoOS/kernel/time.c CoOS/kernel/timer.c CoOS/kernel/flag.c CoOS/kernel/mutex.c CoOS/kernel/serviceReq.c CoOS/portable/GCC/port.c CoOS/portable/arch.c
  EEPROMSRC = eeprom_arm.cpp
  PULSESSRC = pulses_arm.cpp
  CPPSRC += ersky9x/audio.cpp haptic.cpp
  CPPSRC += ersky9x/sound_driver.cpp ersky9x/haptic_driver.cpp ersky9x/sdcard_driver.cpp
  
  ifeq ($(SDCARD), YES)
    SRC += FatFs/ff.c FatFs/fattime.c FatFs/option/ccsbcs.c ersky9x/diskio.c
  endif
endif

ifeq ($(PCB), V4)
  # V4 PCB, so ...
  OPT = 2
  CPPDEFS += -DPCBV4 -DAUDIO -DHAPTIC -DROTARY_ENCODERS=2
  EXTRAINCDIRS += gruvin9x stock FatFs FatFs/option
  BOARDSRC += board_gruvin9x.cpp
  EEPROMSRC = eeprom_avr.cpp
  PULSESSRC = pulses_avr.cpp  
  CPPSRC += stock/audio.cpp haptic.cpp

  ifeq ($(SDCARD), YES)
    CPPSRC += FatFs/fattime.c gruvin9x/rtc.cpp gruvin9x/diskio.cpp
    EXTRABOARDSRC += FatFs/ff.c FatFs/option/ccsbcs.c 
  endif
    
  ifeq ($(SOMO), YES)
    CPPSRC += gruvin9x/somo14d.cpp
    CPPDEFS += -DSOMO
  endif
  
  ifeq ($(EXTRA_ROTARY_ENCODERS), YES)
    CPPDEFS += -DEXTRA_ROTARY_ENCODERS
    MODS:=${MODS}X
  endif
  
endif

ifeq ($(PCB), STD)
  # STD PCB, so ...
  OPT = s
  EXTRAINCDIRS += stock
  CPPDEFS += -DPCBSTD
  BOARDSRC = board_stock.cpp
  EEPROMSRC = eeprom_avr.cpp
  PULSESSRC = pulses_avr.cpp
   
  ifeq ($(AUDIO), YES)
    CPPDEFS += -DAUDIO
    CPPSRC += stock/audio.cpp
  else
    CPPSRC += beeper.cpp 
  endif
  
  ifeq ($(HAPTIC), YES)
   CPPDEFS += -DHAPTIC
   CPPSRC += haptic.cpp
  endif
  
endif

### Global Build-Option Directives ###

ifeq ($(PPM_CENTER_ADJUSTABLE), YES)
  CPPDEFS += -DPPM_CENTER_ADJUSTABLE
endif

ifeq ($(PPM_LIMITS_UNIT), US)
  CPPDEFS += -DPPM_LIMITS_UNIT_US
endif

ifeq ($(DECIMALS), YES)
  CPPDEFS += -DDECIMALS_DISPLAYED
endif

ifeq ($(HELI), YES)
  MODS:=${MODS}H
  CPPDEFS += -DHELI
endif

ifeq ($(TEMPLATES), YES)
  CPPDEFS += -DTEMPLATES
  CPPSRC += templates.cpp
endif

ifeq ($(DISPLAY_USER_DATA), YES)
  CPPDEFS += -DDISPLAY_USER_DATA
endif

ifeq ($(PXX), YES)
  CPPDEFS += -DPXX
endif

ifeq ($(DSM2), SERIAL)
  CPPDEFS += -DDSM2 -DDSM2_SERIAL
endif

ifeq ($(DSM2), PPM)
  CPPDEFS += -DDSM2 -DDSM2_PPM
endif

ifeq ($(IRPROTOS), YES)
  CPPDEFS += -DIRPROTOS
endif

#---------------- Compiler Options C++ ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CPPFLAGS = -g$(DBGFMT)
CPPFLAGS += $(CPPDEFS)
CPPFLAGS += -O$(OPT)
CPPFLAGS += -Wall
CPPFLAGS += -fno-exceptions
CPPFLAGS += -Wno-strict-aliasing
CPPFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

MATH_LIB = -lm

#---------------- External Memory Options ----------------

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# used for variables (.data/.bss) and heap (malloc()).
#EXTMEMOPTS = -Wl,-Tdata=0x801100,--defsym=__heap_end=0x80ffff

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# only used for heap (malloc()).
#EXTMEMOPTS = -Wl,--defsym=__heap_start=0x801100,--defsym=__heap_end=0x80ffff

EXTMEMOPTS =



#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += $(EXTMEMOPTS)
LDFLAGS += $(MATH_LIB)
#LDFLAGS += -T linker_script.x

# Define Messages
# English
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before:
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_COMPILING = Compiling C++:
MSG_CLEANING = Cleaning project:

# Combine all necessary flags and optional flags.
# Add target processor to flags.

ARMCPPFLAGS = -c -mcpu=$(MCU) -mthumb -fomit-frame-pointer -fverbose-asm -Wa,-ahlms=open9x.lst -Dat91sam3s4 -DRUN_FROM_FLASH=1 $(CPPFLAGS) $(GENDEPFLAGS)
AVRCPPFLAGS = -mmcu=$(MCU) -I. -x c++ $(CPPFLAGS) $(GENDEPFLAGS) -fwhole-program

ifneq ($(PCB), ARM)
  GCCVERSIONGTE462 := $(shell expr 4.6.2 \<= `$(CC) -dumpversion`)
  ifeq ($(GCCVERSIONGTE462),1)
    CPPFLAGS += -flto
  endif

  CPPFLAGS += -fno-inline-small-functions

  GCCVERSIONGTE470 := $(shell expr 4.7.0 \<= `$(CC) -dumpversion`)
  ifeq ($(GCCVERSIONGTE470),1)
    AVRCPPFLAGS += -mstrict-X
  endif
endif

MAJ_VER = ${shell sh -c "grep \"MAJ_VERS\" open9x.h | cut -d\  -f3 | egrep -o \"[[:digit:]]\""}
MIN_VER = ${shell sh -c "grep \"MIN_VERS\" open9x.h | cut -d\  -f3"}

# Default target.
all: begin gccversion sizebefore build sizeafter end

# Change the build target to build a HEX file or a library.
ifeq ($(PCB), ARM)
  build: stamp_header font.lbm font_dblsize.lbm sticks.lbm s9xsplash.lbm asterisk.lbm allsrc.cpp elf remallsrc bin hex lss
else
  build: stamp_header font.lbm font_dblsize.lbm sticks.lbm s9xsplash.lbm asterisk.lbm allsrc.cpp elf remallsrc hex eep lss sym
endif

elf: $(TARGET).elf
bin: $(TARGET).bin
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss
sym: $(TARGET).sym

# Build stamp-file
stamp_header:
	@echo
	@echo "Generate Version-stamp:"
	@echo "//Automatically generated file (Makefile) - do not edit" > stamp-open9x.h
	@echo "#define DATE_STR \"`date +%Y-%m-%d`\"" >> stamp-open9x.h
	@echo "#define TIME_STR \"`date +%H:%M:%S`\"" >> stamp-open9x.h
	@echo "#define VERS_STR \"$(MAJ_VER).$(MIN_VER)-$(MODS)\"" >> stamp-open9x.h
	@echo "#define SVN_STR  \"open9x-r$(SVNREV)\"" >> stamp-open9x.h
	@cat stamp-open9x.h
	
stock-stamp:
	@echo "#define DATE_STR \"`date +%Y-%m-%d`\"" > stamp-open9x-stock.txt
	@echo "#define TIME_STR \"`date +%H:%M:%S`\"" >> stamp-open9x-stock.txt
	@echo "#define VERS_STR \"$(MAJ_VER).$(MIN_VER)\"" >> stamp-open9x-stock.txt
	@echo "#define SVN_VERS \"open9x-r$(SVNREV)\"" >> stamp-open9x-stock.txt
	@cat stamp-open9x-stock.txt

v4-stamp:
	@echo "#define DATE_STR \"`date +%Y-%m-%d`\"" > stamp-open9x-v4.txt
	@echo "#define TIME_STR \"`date +%H:%M:%S`\"" >> stamp-open9x-v4.txt
	@echo "#define VERS_STR \"$(MAJ_VER).$(MIN_VER)\"" >> stamp-open9x-v4.txt
	@echo "#define SVN_VERS \"open9x-r$(SVNREV)\"" >> stamp-open9x-v4.txt
	@cat stamp-open9x-v4.txt
	
arm-stamp:
	@echo "#define DATE_STR \"`date +%Y-%m-%d`\"" > stamp-open9x-arm.txt
	@echo "#define TIME_STR \"`date +%H:%M:%S`\"" >> stamp-open9x-arm.txt
	@echo "#define VERS_STR \"$(MAJ_VER).$(MIN_VER)\"" >> stamp-open9x-arm.txt
	@echo "#define SVN_VERS \"open9x-r$(SVNREV)\"" >> stamp-open9x-arm.txt
	@cat stamp-open9x-arm.txt

font.lbm: font_6x1.xbm
	@echo
	@echo "Convert font from xbm to lbm:"
	$(IMG2LBM) $< $@ char
	$(IMG2LBM) translations/font_se.xbm translations/font_se.lbm char
	$(IMG2LBM) translations/font_de.xbm translations/font_de.lbm char
	$(IMG2LBM) translations/font_it.xbm translations/font_it.lbm char
	$(IMG2LBM) translations/font_cz.xbm translations/font_cz.lbm char
	$(IMG2LBM) translations/font_fr.xbm translations/font_fr.lbm char

font_dblsize.lbm: font_dblsize.xbm
	@echo
	@echo "Convert font from xbm to lbm:"
	$(IMG2LBM) $< $@ dblsize
	$(IMG2LBM) translations/font_dblsize_se.xbm translations/font_dblsize_se.lbm dblsize
	$(IMG2LBM) translations/font_dblsize_de.xbm translations/font_dblsize_de.lbm dblsize
	$(IMG2LBM) translations/font_dblsize_it.xbm translations/font_dblsize_it.lbm dblsize
	$(IMG2LBM) translations/font_dblsize_fr.xbm translations/font_dblsize_fr.lbm dblsize

sticks.lbm: sticks_4x1.xbm
	@echo
	@echo "Convert sticks from xbm to lbm:"
	$(IMG2LBM) $< $@ img 4

s9xsplash.lbm: s9xsplash.xbm
	@echo
	@echo "Convert splash from xbm to lbm:"
	$(IMG2LBM) $< $@ img

asterisk.lbm: asterisk.xbm
	@echo
	@echo "Convert asterisk from xbm to lbm:"
	$(IMG2LBM) $< $@ img
	 
# Eye candy.
# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.

begin:
	@echo
	@echo $(MSG_BEGIN)

end:
	@echo $(MSG_END)
	@echo

# Display size of file.
ifeq ($(PCB), ARM)
  ELFSIZE = $(SIZE) $(TARGET).elf
else
  ELFSIZE = $(SIZE) --mcu=$(MCU) --format=avr $(TARGET).elf
endif

sizebefore:
	@if test -f $(TARGET).elf; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); fi

sizeafter:
	@if test -f $(TARGET).elf; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); fi

# Display compiler version information.
gccversion :
	@$(CC) --version

# gruvin: added extra include and lib paths to get simu working on my Mac
FOXINC=-I/usr/local/include/fox-1.6 -I/usr/include/fox-1.6 \
       -I$(FOXPATH)/include \
       -I/opt/local/include/fox-1.6
FOXLIB=-L/usr/local/lib \
       -L$(FOXPATH)/src/.libs \
       -L/opt/local/lib \
       -lFOX-1.6 \
       -Wl,-rpath,$(FOXPATH)/src/.libs

simu: stamp_header $(BOARDSRC) $(CPPSRC) Makefile simu.cpp simpgmspace.cpp *.h *.lbm eeprom.bin
	g++ $(CPPFLAGS) simu.cpp $(BOARDSRC) $(CPPSRC) simpgmspace.cpp $(ARCH) -MD -DSIMU -o simu $(FOXINC) $(FOXLIB) -pthread

eeprom.bin:
	dd if=/dev/zero of=$@ bs=1 count=2048


# Create final output files (.hex, .eep) from ELF output file.
ifeq ($(PCB), ARM)
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	 $(OBJCOPY) -O ihex open9x.elf open9x.hex
else
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@
endif

%.bin: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O binary  $< $@

%.eep: %.elf
	@echo
	@echo $(MSG_EEPROM) $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S $< > $@

# Concatenate all sources files in one big file to optimize size
allsrc.cpp: Makefile $(BOARDSRC) $(CPPSRC) $(EXTRABOARDSRC)
	@rm -f allsrc.cpp 
	for f in $(BOARDSRC) $(CPPSRC) $(EXTRABOARDSRC) ; do echo "# 1 \"$$f\"" >> allsrc.cpp; cat "$$f" >> allsrc.cpp; done
	
remallsrc:
	$(REMOVE) allsrc.cpp
    
# Link: create ELF output file from object files.
ifeq ($(PCB), ARM)
%.o: %.c
	$(CC) $(ARMCPPFLAGS) $< -o $@
OBJS = $(SRC:.c=.o)
%.elf: allsrc.cpp $(OBJS)
	@echo
	@echo $(MSG_COMPILING) $@
	$(CC) $(ARMCPPFLAGS) $< -o allsrc.o
	$(CC) allsrc.o $(OBJS) -mcpu=cortex-m3 -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(TARGET).map,--cref,--no-warn-mismatch -o $@
else
%.elf: allsrc.cpp
	@echo
	@echo $(MSG_COMPILING) $@
	$(CC) $(AVRCPPFLAGS) $< --output $@ $(LDFLAGS)
endif
	
# Target: clean project.
clean: begin clean_list end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) simu
	$(REMOVE) gtests
	$(REMOVE) gtest.a
	$(REMOVE) gtest_main.a
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).eep
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(TARGET).lst
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) *.o
	$(REMOVE) */*.o
	$(REMOVE) */*/*.o
	$(REMOVE) *.d
	$(REMOVE) *.lst
	$(REMOVE) allsrc.cpp
	$(REMOVE) stamp-open9x.h

#### GOOGLE TESTS 

GTEST_DIR = ../gtest-1.6.0

# Where to find user code.
USER_DIR = ./

# Flags passed to the preprocessor.
CPPFLAGS += -I$(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

gtests: $(CPPSRC) gtests.cpp simpgmspace.cpp *.h gtest_main.a
	g++ $(CPPSRC) gtests.cpp simpgmspace.cpp $(CPPFLAGS) -I$(GTEST_DIR) $(ARCH) -o gtests -lpthread -MD -DSIMU gtest_main.a 
	
