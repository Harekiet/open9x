// NON ZERO TERMINATED STRINGS
#define LEN_OFFON        "\003"
#define TR_OFFON         "VYP""ZAP"

#define LEN_MMMINV       "\003"
#define TR_MMMINV        "---""INV"

#define LEN_NCHANNELS    "\004"
#define TR_NCHANNELS     "\0014CH\0016CH\0018CH10CH12CH14CH16CH"

#define LEN_VBEEPMODE    "\005"
#define TR_VBEEPMODE     "Tich\212""Alarm""BezKL""V\207e  "

#define LEN_VBEEPLEN     "\005"
#define TR_VBEEPLEN      "O====""=O===""==O==""===O=""====O"

#define LEN_VRENAVIG     "\003"
#define TR_VRENAVIG      "Ne REaREb"

#define LEN_VFILTERADC   "\004"
#define TR_VFILTERADC    "SING""OSMP""FILT"

#define LEN_TRNMODE      "\003"
#define TR_TRNMODE       " X "" +="" :="

#define LEN_TRNCHN       "\003"
#define TR_TRNCHN        "CH1CH2CH3CH4"

#define LEN_DATETIME     "\006"
#define TR_DATETIME      "DATUM:""\201AS:  "

#define LEN_VTRIMINC     "\007"
#define TR_VTRIMINC      "Exp    ""ExJemn\212""Jemn\212  ""St\206edn\204""Hrub\212  "

#define LEN_RETA123      "\001"
#if defined(PCBV4)
#if defined(EXTRA_ROTARY_ENCODERS)
#define TR_RETA123       "SVPK123abcd"
#else //EXTRA_ROTARY_ENCODERS
#define TR_RETA123       "SVPK123ab"
#endif //EXTRA_ROTARY_ENCODERS
#else
#define TR_RETA123       "SVPK123"
#endif

#define LEN_VPROTOS      "\006"
#ifdef PXX
#define TR_PXX  "PXX\0  "
#else
#define TR_PXX  "[PXX]\0"
#endif
#ifdef DSM2
#define TR_DSM2 "DSM2\0 "
#else
#define TR_DSM2 "[DSM2]"
#endif
#ifdef IRPROTOS
#define TR_IRPROTOS "SILV  TRAC09PICZ  SWIFT\0"
#else
#define TR_IRPROTOS
#endif
#define TR_VPROTOS       "PPM\0  ""PPM16\0""PPMsim" TR_PXX TR_DSM2 TR_IRPROTOS

#define LEN_POSNEG       "\003"
#define TR_POSNEG        "POS""NEG"

#define LEN_VCURVEFUNC   "\003"
#define TR_VCURVEFUNC    "---""x>0""x<0""|x|""f>0""f<0""|f|"

#define LEN_CURVMODES    "\005"
#define TR_CURVMODES     "EDIT ""PRSET"

#define LEN_EXPLABELS    "\006"
#ifdef PCBARM
#define TR_EXPLABEL_NAME "Name  "
#else
#define TR_EXPLABEL_NAME
#endif
#ifdef FLIGHT_PHASES
#define TR_EXPLABEL_FP   "F\200ze  "
#else
#define TR_EXPLABEL_FP
#endif
#define TR_EXPLABELS     TR_EXPLABEL_NAME "V\200ha  ""Expo  ""K\206ivka" TR_EXPLABEL_FP "Sp\204na\201""Strana""\0"

#define LEN_VMLTPX       "\010"
#define TR_VMLTPX        "Se\201\204st  ""N\200sobit ""Zam\203nit  "

#define LEN_VMLTPX2      "\002"
#define TR_VMLTPX2       "+=""*="":="

#define LEN_VMIXTRIMS    "\004"
#define TR_VMIXTRIMS     "VYP\0""ZAP\0""Sm\203r""V\212\207k""Plyn""K\206id"

#define LEN_VCSWFUNC     "\007"
#define TR_VCSWFUNC      "----\0  ""v>ofs  ""v<ofs  ""|v|>ofs""|v|<ofs""AND    ""OR     ""XOR    ""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 "

#define LEN_VFSWFUNC     "\015"
#if defined(VARIO)
#define TR_VVARIO         "Vario        "
#else
#define TR_VVARIO         "[Vario]      "
#endif
#if defined(AUDIO)
#define TR_SOUND         "P\206ehr\200t\0     "
#else
#define TR_SOUND         "P\204pnout\0     "
#endif
#if defined(HAPTIC)
#define TR_HAPTIC        "Vibrovat\0    "
#else
#define TR_HAPTIC        "[Vibrovat]\0  "
#endif
#if defined(PCBARM)
#if defined(SDCARD)
#define TR_SDCLOGS       "[SDCARD Logs]"
#define TR_PLAY_TRACK    "Play Track\0  "
#define TR_PLAY_VALUE    "[Play Value]\0"
#else
#define TR_SDCLOGS       "[SDCARD Logs]"
#define TR_PLAY_TRACK    "[Play Track]\0"
#define TR_PLAY_VALUE    "[Play Value]\0"
#endif
#define TR_FSW_VOLUME    "Volume\0      "
#elif defined(PCBV4)
#if defined(SDCARD)
#define TR_SDCLOGS       "SDCARD Logs  "
#else
#define TR_SDCLOGS       "[SDCARD Logs]"
#endif
#if defined(SOMO)
#define TR_PLAY_TRACK    "Hr\200t Stopu\0  "
#define TR_PLAY_VALUE    "Hr\200t Hodnotu\0"
#else
#define TR_PLAY_TRACK    "[Hr\200t Stopu]\0"
#define TR_PLAY_VALUE    "Hr\200t Hodnotu\0"
#endif
#define TR_FSW_VOLUME
#else
#define TR_SDCLOGS
#define TR_PLAY_TRACK
#define TR_PLAY_VALUE
#define TR_FSW_VOLUME
#endif
#ifdef DEBUG
#define TR_TEST          "Test\0        "
#else
#define TR_TEST
#endif
#define TR_VFSWFUNC      "Bezpe\201\0      ""Tren\202r\0      ""Instant Trim " TR_SOUND TR_HAPTIC "Reset\0       " TR_VVARIO TR_PLAY_TRACK TR_PLAY_VALUE TR_SDCLOGS TR_FSW_VOLUME TR_TEST

#define LEN_VFSWRESET    "\006"
#define TR_VFSWRESET     "Timer1""Timer2""Vse   ""Telem."

#define LEN_FUNCSOUNDS   "\006"
#define TR_FUNCSOUNDS    "Warn1 ""Warn2 ""Cheep ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  "

#define LEN_VTELEMCHNS   "\004"
#define TR_VTELEMCHNS    "---\0""Tmr1""Tmr2""A1\0 ""A2\0 ""Tx\0 ""Rx\0 ""Alt\0""Rpm\0""Fuel""T1\0 ""T2\0 ""Spd\0""Dist""GAlt""Cell""Volt""Curr""Cnsp""Powr""AccX""AccY""AccZ""Hdg\0""VSpd""A1-\0""A2-\0""Alt-""Alt+""Rpm+""T1+\0""T2+\0""Spd+""Dst+""Cur+""Acc\0""Time"

#ifdef IMPERIAL_UNITS
#define LENGTH_UNIT "ft\0"
#define SPEED_UNIT  "kts"
#else
#define LENGTH_UNIT "m\0 "
#define SPEED_UNIT  "kmh"
#endif

#define LEN_VTELEMUNIT   "\003"
#define TR_VTELEMUNIT    "v\0 ""A\0 ""m/s""-\0 " SPEED_UNIT LENGTH_UNIT "@\0 ""%\0 ""mA\0""mAh""W\0 "
#define STR_V            (STR_VTELEMUNIT+1)
#define STR_A            (STR_VTELEMUNIT+4)

#define LEN_VALARM       "\004"
#define TR_VALARM        "----""\217lut""oran""\201erv"

#define LEN_VALARMFN     "\001"
#define TR_VALARMFN      "<>"

#define LEN_VTELPROTO    "\004"
#if defined(WS_HOW_HIGH)
#define TR_VTELPROTO     "----Hub WSHH"
#elif defined(FRSKY_HUB)
#define TR_VTELPROTO     "----Hub "
#else
#define TR_VTELPROTO     ""
#endif

#define LEN_CURRENTSRC   "\003"
#define TR_CURRENTSRC    "---""Hub""A1\0""A2\0"

#define LEN_VARIOSRC     "\006"
#define TR_VARIOSRC      "BaroV1""BaroV2""A1\0   ""A2\0"

#define LEN_GPSFORMAT    "\004"
#define TR_GPSFORMAT     "HMS NMEA"

#define LEN2_VTEMPLATES  13
#define LEN_VTEMPLATES   "\015"
#define TR_VTEMPLATES    "Smazat Mixy\0\0""Z\200kl. 4kan\200l\0""T-Cut       \0""V-Ocas      \0""Elevon\\Delta\0""eCCPM       \0""Heli Setup  \0""Servo Test  \0"

#define LEN_VSWASHTYPE   "\004"
#define TR_VSWASHTYPE    "--- ""120 ""120X""140 ""90\0"

#define LEN_VKEYS        "\005"
#define TR_VKEYS         " Menu"" Exit"" Dol\211""Nhoru""Vprvo""Vlevo"

#define LEN_RE1RE2       "\003"
#define TR_RE1RE2        "RE1""RE2"

#define LEN_VSWITCHES    "\003"
#if defined(PCBARM)
#define TR_VSWITCHES     "THR""RUD""ELE""ID0""ID1""ID2""AIL""GEA""TRN""SW1""SW2""SW3""SW4""SW5""SW6""SW7""SW8""SW9""SWA""SWB""SWC""SWD""SWE""SWF""SWG""SWH""SWI""SWJ""SWK""SWL""SWM""SWN""SWO""SWP""SWQ""SWR""SWS""SWT""SWU""SWV""SWW"
#else
#define TR_VSWITCHES     "THR""RUD""ELE""ID0""ID1""ID2""AIL""GEA""TRN""SW1""SW2""SW3""SW4""SW5""SW6""SW7""SW8""SW9""SWA""SWB""SWC"
#endif

#define LEN_VSRCRAW      "\004"
#if defined(PCBV4)
#if defined(EXTRA_ROTARY_ENCODERS)
#define TR_ROTARY_ENCODERS_VSRCRAW "REa ""REb ""REc ""REd "
#else
#define TR_ROTARY_ENCODERS_VSRCRAW "REa ""REb "
#endif
#else
#define TR_ROTARY_ENCODERS_VSRCRAW
#endif
#if defined(HELI)
#define TR_CYC_VSRCRAW "CYC1""CYC2""CYC3"
#else
#define TR_CYC_VSRCRAW "[C1]""[C2]""[C3]"
#endif
#define TR_VSRCRAW       "Sm\203r""V\212\207k""Plyn""K\206id""Pot1""Pot2""Pot3" TR_ROTARY_ENCODERS_VSRCRAW "TrmS" "TrmV" "TrmP" "TrmK" "MAX ""3POS" TR_CYC_VSRCRAW

#define LEN_VTMRMODES    "\003"
#define TR_VTMRMODES     "VYP""ABS""THs""TH%""THt"

#define LEN_DSM2MODE     "\007"
#define TR_DSM2MODE      "LP4/LP5DSMonlyDSMX   "

// ZERO TERMINATED STRINGS
#define TR_POPUPS       "[MENU]\004[EXIT]"
#define OFS_EXIT        7
#define TR_MENUWHENDONE "[MENU] > DOKON\201IT"
#define TR_FREE         "voln\202:"
#define TR_DELETEMODEL  "SMAZAT MODEL"
#define TR_COPYINGMODEL "Kop\204ruji model.."
#define TR_MOVINGMODEL  "P\206esouv\200m model."
#define TR_LOADINGMODEL "Aktivuji model.."
#define TR_NAME         "Jm\202no"
#define TR_TIMER        "Stopky "
#define TR_ELIMITS      "Limit+25%"
#define TR_ETRIMS       "\207ir\207\204Trim"
#define TR_TRIMINC      "KrokTrimu"
#define TR_TTRACE       "StopaPlyn"
#define TR_TTRIM        "TrimPlynu"
#define TR_BEEPCTR      "P\204pSt\206ed"
#define TR_PROTO        "Proto."
#define TR_PPMFRAME     "PPM frame"
#define TR_MS           "ms"
#define TR_SWITCH       "Sp\204na\201"
#define TR_TRIMS        "Trimy"
#define TR_FADEIN       "P\206echodZap"
#define TR_FADEOUT      "P\206echodVyp"
#define TR_DEFAULT      "(v\212choz\204)"
#define TR_CHECKTRIMS   "Kont. \002  Trimy"
#define TR_SWASHTYPE    "Swash Type"
#define TR_COLLECTIVE   "Collective"
#define TR_SWASHRING    "Swash Ring"
#define TR_ELEDIRECTION "ELE Direction"
#define TR_AILDIRECTION "AIL Direction"
#define TR_COLDIRECTION "COL Direction"
#define TR_MODE         "M\205d"
#define TR_NOFREEEXPO   "Nen\204 voln\202 expo!"
#define TR_NOFREEMIXER  "Nen\204 voln\212 mix!"
#define TR_INSERTMIX    "VLO\217IT MIX "
#define TR_EDITMIX      "UPRAVIT MIX "
#define TR_SOURCE       "Zdroj"
#define TR_WEIGHT       "V\200ha"
#define TR_MIXERWEIGHT  "V\200ha Mixu"
#define TR_DIFFERENTIAL "DifV\212ch"
#define TR_OFFSET       "Ofset"
#define TR_MIXEROFFSET  "Offset Mixu"
#define TR_DRWEIGHT     "V\200ha DV"
#define TR_DREXPO       "Expo DV"
#define TR_TRIM         "Trim"
#define TR_CURVES       "K\206ivka"
#define TR_FPHASE       "F\200ze"
#define TR_MIXWARNING   "Varov\200n\204"
#define TR_OFF          "VYP"
#define TR_MULTPX       "Multpx"
#define TR_DELAYDOWN    "Zpo\217d\203n\204 Vyp"
#define TR_DELAYUP      "Zpo\217d\203n\204 Zap"
#define TR_SLOWDOWN     "Zpomal. Vyp"
#define TR_SLOWUP       "Zpomal. Zap"
#define TR_MIXER        "MIXER"
#define TR_CV           "CV"
#define TR_ACHANNEL     "A\002Kan\200l"
#define TR_RANGE        "Rozsah"
#define TR_BAR          "Bar"
#define TR_ALARM        "Alarm"
#define TR_USRDATA      "U\217ivData"
#define TR_BLADES       "Lopatky"
#define TR_BARS         "Ukazatele"
#define TR_DISPLAY      "Zobrazen\204"
#ifdef AUDIO
#define TR_BEEPERMODE   "Repro: M\205d"
#define TR_BEEPERLEN    " += D\202lka"
#define TR_SPKRPITCH    " += T\205n"
#else
#define TR_BEEPERMODE   "Zvuk: M\205d"
#define TR_BEEPERLEN    " += D\202lka"
#endif
#define TR_HAPTICMODE   "Vibrace: M\205d"
#define TR_HAPTICSTRENGTH " += S\204la"
#define TR_HAPTICLENGTH " += D\202lka "
#define TR_CONTRAST     "Kontrast"
#define TR_BATTERYWARNING "Alarm Baterie"
#define TR_INACTIVITYALARM "UpozNaNe\201innost"
#define TR_RENAVIG      "Navig RotEnc"
#define TR_FILTERADC    "Filtr ADC"
#define TR_THROTTLEREVERSE "Revers Plynu"
#define TR_MINUTEBEEP   "P\204pat: Minuta"
#define TR_BEEPCOUNTDOWN " += Odpo\201et"
#define TR_FLASHONBEEP  "Podsv\203tl.:Alarm"
#define TR_LIGHTSWITCH  " += Sp\204na\201"
#define TR_LIGHTOFFAFTER " += Zhasnout po"
#define TR_SPLASHSCREEN  "\210vodn\204Logo"
#define TR_THROTTLEWARNING "Upoz:Plyn"
#define TR_SWITCHWARNING ":=Sp\204na\201e"
#define TR_MEMORYWARNING "UpozNaPlnPam\203t'"
#define TR_ALARMWARNING "UpozNaVypZvuk"
#define TR_TIMEZONE     "\201asov\202 p\200smo"
#define TR_RXCHANNELORD "Po\206ad\204 Kan\200l\211"
#define TR_SLAVE        "Pod\206\204zen\212"
#define TR_MODESRC      " M\205d\003% Zdroj"
#define TR_MULTIPLIER   "N\200sobi\201"
#define TR_CAL          "Kal."
#define TR_EEPROMV      "EEPROM v"
#define TR_VTRIM        "Trim- +"
#define TR_BG           "BG:"
#define TR_MENUTOSTART  " [MENU] = START"
#define TR_SETMIDPOINT  "NASTAV ST\215EDY"
#define TR_MOVESTICKSPOTS "H\216BEJ P\213KY/POTY"
#define TR_RXBATT       "Rx Batt:"
#define TR_TXnRX        "Tx:\0Rx:"
#define OFS_RX          4
#define TR_ACCEL        "Acc:"
#define TR_NODATA       "NO DATA"
#define TR_TM1TM2       "TM1\015TM2"
#define TR_THRTHP       "THR\015TH%"
#define TR_TOT          "TOT"
#define TR_TMR1LATMAXUS "Tmr1Lat max\003us"
#define STR_US (STR_TMR1LATMAXUS+12)
#define TR_TMR1LATMINUS "Tmr1Lat min\003us"
#define TR_TMR1JITTERUS "Tmr1 Jitter\003us"
#define TR_TMAINMAXMS   "Tmain max\005ms"
#define TR_T10MSUS      "T10ms\007us"
#define TR_FREESTACKMINB "Free Stack\004b"
#define TR_MENUTORESET  "[MENU] >> Reset"
#define TR_PPM          "PPM"
#define TR_CH           "CH"
#define TR_MODEL        "MODEL"
#define TR_FP           "FP"
#define TR_EEPROMLOWMEM "Doch\200z\204 EEPROM"
#define TR_ALERT        "\003(!)POZOR"
#define TR_PRESSANYKEYTOSKIP "Kl\200vesa >>> p\206esko\201it"
#define TR_THROTTLENOTIDLE "P\200ka plynu je pohnut\200."
#define TR_ALARMSDISABLED "Alarmy jsou zak\200z\200ny"
#define TR_PRESSANYKEY  "\003Stiskni Kl\200vesu"
#define TR_BADEEPROMDATA "\003Chyba dat EEprom"
#define TR_EEPROMFORMATTING " Formatov\200n\204 EEPROM"
#define TR_EEPROMOVERFLOW "P\206etekla EEPROM"
#define TR_MENURADIOSETUP "NASTAVEN\214 R\213DIA"
#define TR_MENUDATEANDTIME "DATUM A \201AS"
#define TR_MENUTRAINER  "TRENER"
#define TR_MENUVERSION  "VERZE"
#define TR_MENUDIAG     "DIAG"
#define TR_MENUANA      "ANALOGY"
#define TR_MENUCALIBRATION "KALIBRACE"
#define TR_TRIMS2OFFSETS "Trimy => Offsety"
#define TR_MENUMODELSEL "MODEL"
#define TR_MENUSETUP    "NASTAVEN\214"
#define TR_MENUFLIGHTPHASE "F\213ZE LETU"
#define TR_MENUFLIGHTPHASES "F\213ZE LETU"
#define TR_MENUHELISETUP "HELI SETUP"
#define TR_MENUDREXPO   "DV/EXPO" // TODO flash saving this string is 2 times here
#define TR_MENULIMITS   "LIMITY"
#define TR_MENUCURVES   "K\215IVKY"
#define TR_MENUCURVE    "\003CV"
#define TR_MENUCUSTOMSWITCHES "VLASTN\214 SP\214NA\201E"
#define TR_MENUFUNCSWITCHES "FUNK\201.SP\214NA\201E"
#define TR_MENUTELEMETRY "TELEMETRIE"
#define TR_MENUTEMPLATES "\207ABLONY"
#define TR_MENUSTAT      "STATISTIKA"
#define TR_MENUDEBUG     "DIAG"
#define TR_RXNUM         "RxNum"
#define TR_SYNCMENU      "Sync [MENU]"
#define TR_BACK          "Back"
#define TR_MINLIMIT      "Min Limit"
#define STR_LIMIT        (STR_MINLIMIT+4)
#define TR_MAXLIMIT      "Max Limit"
#define TR_MINRSSI       "Min Rssi"
#define TR_LATITUDE      "Latitude"
#define TR_LONGITUDE     "Longitude"
#define TR_GPSCOORD      "Gps Sou\206adnice"
#define TR_VARIO         "Vario"
#define TR_SHUTDOWN      "VYP\214N\213\N\204.."
#define TR_BATT_CALIB    "Kalibrace Bat"
#define TR_CURRENT_CALIB "Kalib.Proudu"
#define TR_VOLTAGE       "Voltage"
#define TR_CURRENT       "Proud"
#define TR_SELECT_MODEL  "Vyber Model"
#define TR_CREATE_MODEL  "Vytvo\206 Model"
#define TR_BACKUP_MODEL  "Z\200lohuj Model"
#define TR_DELETE_MODEL  "Sma\217 Model" // TODO merged into DELETEMODEL?
#define TR_RESTORE_MODEL "Obnov Model"
#define TR_SDCARD_ERROR  "Chyba SDkarty"
#define TR_NO_SDCARD     "Nen\204 SDkarta"
#define TR_INCOMPATIBLE  "Incompatible"
#define TR_WARNING       "KONTROLA"
#define TR_EEPROMWARN    "EEPROM"
#define TR_THROTTLEWARN  "PLYNU"
#define TR_ALARMSWARN    "ALARMU"
#define TR_SWITCHWARN    "POZICE"
#define TR_INVERT_THR    "Invertovat Plyn?"
