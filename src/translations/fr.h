// NON ZERO TERMINATED STRINGS
#define LEN_OFFON        "\003"
#define TR_OFFON         "OFF""ON "

#define LEN_MMMINV       "\003"
#define TR_MMMINV        "---""INV"

#define LEN_NCHANNELS    "\004"
#define TR_NCHANNELS     "4CH 6CH 8CH 10CH12CH14CH16CH"

#define LEN_VBEEPMODE    "\005"
#define TR_VBEEPMODE     "Aucun""Alarm""NoKey""Tout "

#define LEN_VBEEPLEN     "\005"
#define TR_VBEEPLEN      "0====""=0===""==0==""===0=""====0"

#define LEN_VRENAVIG     "\003"
#define TR_VRENAVIG      "NonREaREb"

#define LEN_VFILTERADC   "\004"
#define TR_VFILTERADC    "SIMP""OSMP""FILT"

#define LEN_TRNMODE      "\003"
#define TR_TRNMODE       "OFF"" +="" :="

#define LEN_TRNCHN       "\003"
#define TR_TRNCHN        "CH1CH2CH3CH4"

#define LEN_DATETIME     "\005"
#define TR_DATETIME      "DATE:""HEURE"

#define LEN_VTRIMINC     "\006"
#define TR_VTRIMINC      "Exp   ""ExFin ""Fin   ""Moyen ""Gros  "

#define LEN_RETA123      "\001"
#if defined(PCBV4)
#if defined(EXTRA_ROTARY_ENCODERS)
#define TR_RETA123       "DPGA123abcd"
#else //EXTRA_ROTARY_ENCODERS
#define TR_RETA123       "DPGA123ab"
#endif //EXTRA_ROTARY_ENCODERS
#else
#define TR_RETA123       "DPGA123"
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
#define TR_VPROTOS       "PPM\0  ""PPM16\0""PPMsim" TR_PXX TR_DSM2

#define LEN_POSNEG       "\003"
#define TR_POSNEG        "POS""NEG"

#define LEN_VCURVEFUNC   "\003"
#define TR_VCURVEFUNC    "---""x>0""x<0""|x|""f>0""f<0""|f|"

#define LEN_CURVMODES    "\005"
#define TR_CURVMODES     "EDIT ""PRSET""A.GAZ"

#define LEN_EXPLABELS    "\006"
#ifdef FLIGHT_PHASES
#define TR_EXPLABEL_FP   "Phase "
#else
#define TR_EXPLABEL_FP
#endif
#define TR_EXPLABELS     "Ratio ""Expo  ""Courbe" TR_EXPLABEL_FP "Inter ""Mode  ""      "

#define LEN_VMLTPX       "\010"
#define TR_VMLTPX        "Ajoute  ""Multipl.""Remplace"

#define LEN_VMLTPX2      "\002"
#define TR_VMLTPX2       "+=""*="":="

#define LEN_VMIXTRIMS    "\003"
#define TR_VMIXTRIMS     "OFF""ON ""Dir""Prf""Gaz""Ail"

#define LEN_VCSWFUNC     "\007"
#define TR_VCSWFUNC      "----\0  ""v>ofs  ""v<ofs  ""|v|>ofs""|v|<ofs""ET     ""OU     ""OU Excl""v1==v2 ""v1!=v2 ""v1>v2  ""v1<v2  ""v1>=v2 ""v1<=v2 "

#define LEN_VFSWFUNC     "\015"
#if defined(VARIO)
#define TR_VVARIO         "Vario        "
#else
#define TR_VVARIO         "[Vario]      "
#endif
#if defined(AUDIO)
#define TR_SOUND         "Jouer son\0   "
#else
#define TR_SOUND         "Bip\0         "
#endif
#if defined(HAPTIC)
#define TR_HAPTIC        "Vibreur\0     "
#else
#define TR_HAPTIC        "[Vibreur]\0   "
#endif
#if defined(PCBV4)
#if defined(SDCARD)
#define TR_SDCLOGS       "Logs carte SD "
#else
#define TR_SDCLOGS       "[Logs carteSD]"
#endif
#if defined(SOMO)
#define TR_SOMO          "Jouer fichier"
#else
#define TR_SOMO          "[Jouer fich.]"
#endif
#else
#define TR_SDCLOGS
#define TR_SOMO
#endif
#ifdef DEBUG
#define TR_TEST          "Test\0        "
#else
#define TR_TEST
#endif
#define TR_VFSWFUNC      "S\200cur.\0      ""Ecolage \0    ""Trim instant." TR_SOUND TR_HAPTIC TR_SOMO "Reset\0       " TR_VVARIO TR_SDCLOGS TR_TEST

#define LEN_VFSWRESET    "\006"
#define TR_VFSWRESET     "Timer1""Timer2""Tout  ""T\200l\200m."

#define LEN_FUNCSOUNDS   "\006"
#define TR_FUNCSOUNDS    "Warn1 ""Warn2 ""Cheep ""Ring  ""SciFi ""Robot ""Chirp ""Tada  ""Crickt""Siren ""AlmClk""Ratata""Tick  "

#define LEN_VTELEMCHNS   "\004"
#define TR_VTELEMCHNS    "---\0""Chr1""Chr2""A1\0 ""A2\0 ""Tx\0 ""Rx\0 ""Alt\0""Rpm\0""Carb""T1\0 ""T2\0 ""Vit\0""Dist""AltG""Elem""AccX""AccY""AccZ""Cap\0""VitV""A1-\0""A2-\0""Alt-""Alt+""Rpm+""T1+\0""T2+\0""Vit+""Dst+""Acc\0""Time"

#ifdef IMPERIAL_UNITS
#define LENGTH_UNIT "ft\0"
#define SPEED_UNIT  "kts"
#else
#define LENGTH_UNIT "m\0 "
#define SPEED_UNIT  "kmh"
#endif

#define LEN_VTELEMUNIT   "\003"
#define TR_VTELEMUNIT    "v\0 ""A\0 ""m/s""-\0 " SPEED_UNIT LENGTH_UNIT "@\0 ""%\0 ""mA\0"
#define STR_V            (STR_VTELEMUNIT+1)
#define STR_A            (STR_VTELEMUNIT+4)

#define LEN_VALARM       "\004"
#define TR_VALARM        "----""Jaun""Oran""Roug"

#define LEN_VALARMFN     "\001"
#define TR_VALARMFN      "<>"

#define LEN_VTELPROTO    "\004"
#if defined(WS_HOW_HIGH)
#define TR_VTELPROTO     "Non Hub WSHH"
#elif defined(FRSKY_HUB)
#define TR_VTELPROTO     "Non Hub "
#else
#define TR_VTELPROTO     ""
#endif

#define LEN_VARIOSRC     "\006"
#define TR_VARIOSRC      "BaroV1""BaroV2""A1    ""A2    "

#define LEN_GPSFORMAT    "\004"
#define TR_GPSFORMAT     "HMS NMEA"

#define LEN_VUNITSFORMAT "\004"
#define TR_VUNITSFORMAT  "M\200trImpr"

#define LEN2_VTEMPLATES  15
#define LEN_VTEMPLATES   "\017"
#define TR_VTEMPLATES    "Suppr Mixages\0\0""4 Voies simple\0""Coupure Gaz   \0""Empennage V   \0""Elevon\\Delta  \0""eCCPM         \0""Conf. H\200lico  \0""Servo Test    \0"

#define LEN_VSWASHTYPE   "\004"
#define TR_VSWASHTYPE    "--- ""120 ""120X""140 ""90  "

#define LEN_VKEYS        "\005"
#define TR_VKEYS         " Menu"" Exit""  Bas"" Haut""Droit""Gauch"

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
#define TR_VSRCRAW       "Dir ""Prf ""Gaz ""Ail ""P1  ""P2  ""P3  " TR_ROTARY_ENCODERS_VSRCRAW "TrmD" "TrmP" "TrmG" "TrmA" "MAX ""3POS" TR_CYC_VSRCRAW

#define LEN_VTMRMODES    "\003"
#define TR_VTMRMODES     "OFF""ABS""GZs""GZ%""GZt"

#define LEN_DSM2MODE     "\007"
#define TR_DSM2MODE      "LP4/LP5DSMonlyDSMX   "

// ZERO TERMINATED STRINGS
#define TR_POPUPS       "[MENU]\004[QUIT]"
#define OFS_EXIT        7
#define TR_MENUWHENDONE "[MENU]QUAND PRET"
#define TR_FREE         "libre"
#define TR_DELETEMODEL  "SUPPRIMER MODELE"
#define TR_COPYINGMODEL "Copie..."
#define TR_MOVINGMODEL  "D\200placement..."
#define TR_LOADINGMODEL "Chargement..."
#define TR_NAME         "Nom"
#define TR_TIMER        "Chrono"
#define TR_ELIMITS      "Limites Et"
#define TR_ETRIMS       "Trims Et."
#define TR_TRIMINC      "Pas Trim"
#define TR_TTRACE       "Trace Gaz"
#define TR_TTRIM        "Trim Gaz"
#define TR_THROTTLEWARNING "Alerte Gaz"
#define TR_BEEPCTR      "Bips Contr"
#define TR_PROTO        "Proto."
#define TR_PPMFRAME     "Trame PPM"
#define TR_MS           "ms"
#define TR_SWITCH       "Inter"
#define TR_TRIMS        "Trims"
#define TR_FADEIN       "Fade In"
#define TR_FADEOUT      "Fade Out"
#define TR_DEFAULT      "(d\200faut)"
#define TR_CHECKTRIMS   "V\200rif\005Trims"
#define TR_SWASHTYPE    "Type de Plat."
#define TR_COLLECTIVE   "Collectif"
#define TR_SWASHRING    "Limite Cycl."
#define TR_ELEDIRECTION "Direction PRF"
#define TR_AILDIRECTION "Direction AIL"
#define TR_COLDIRECTION "Direction COL"
#define TR_NOFREEEXPO   "Max expos atteint!"
#define TR_NOFREEMIXER  "Max mixages atteint!"
#define TR_INSERTMIX    "INSERER MIXAGE "
#define TR_EDITMIX      "EDITER MIXAGE "
#define TR_SOURCE       "Source"
#define TR_WEIGHT       "Ratio"
#define TR_MIXERWEIGHT  "Ratio Mixage"
#define TR_DIFFERENTIAL "Diff\200r."
#define TR_OFFSET       "Offset"
#define TR_MIXEROFFSET  "Offset Mixage"
#define TR_DRWEIGHT     "Ratio DR"
#define TR_DREXPO       "Expo DR"
#define TR_TRIM         "Trim"
#define TR_CURVES       "Courbe"
#define TR_FPHASE       "P.Vol"
#define TR_MIXWARNING   "Alerte"
#define TR_OFF          "OFF"
#define TR_MULTPX       "Multpx"
#define TR_DELAYDOWN    "Retard Bas"
#define TR_DELAYUP      "Retard Haut"
#define TR_SLOWDOWN     "Ralenti Haut"
#define TR_SLOWUP       "Ralenti Bas"
#define TR_MIXER        "MIXEUR"
#define TR_CV           "CV"
#define TR_ACHANNEL     "A"
#define TR_RANGE        "Plage"
#define TR_BAR          "Barre"
#define TR_ALARM        "Alarme"
#define TR_USRDATA      "UsrData"
#define TR_BLADES       "Pales"
#define TR_BARS         "Jauges"
#define TR_DISPLAY      "Affichage"
#ifdef AUDIO
#define TR_BEEPERMODE   "Choix mode sons"
#define TR_BEEPERLEN    "Dur\200e sons"
#define TR_SPKRPITCH    "Tonalit\200 sons"
#else
#define TR_BEEPERMODE   "Mode bipeur"
#define TR_BEEPERLEN    "Longueur bips"
#endif
#define TR_HAPTICMODE   "Mode vibreur"
#define TR_HAPTICSTRENGTH "Force vibreur"
#define TR_HAPTICLENGTH "Dur\200e vibreur"
#define TR_CONTRAST     "Contraste"
#define TR_BATTERYWARNING "Alarme batterie"
#define TR_INACTIVITYALARM "Alarme inactiv."
#define TR_RENAVIG      "Navig EncRot"
#define TR_FILTERADC    "Filtre ADC"
#define TR_THROTTLEREVERSE "Inversion Gaz"
#define TR_MINUTEBEEP   "Bip t.minutes"
#define TR_BEEPCOUNTDOWN "Bip \202 rebours"
#define TR_FLASHONBEEP  "Flash quand bip"
#define TR_LIGHTSWITCH  "Inter \200clairage"
#define TR_LIGHTOFFAFTER "Dur\200e \200clairage"
#define TR_SPLASHSCREEN  "Ecran d'accueil"
#define TR_SWITCHWARNING "Alerte Int"
#define TR_MEMORYWARNING "Alerte m\200moire"
#define TR_ALARMWARNING "Alerte alarmes"
#define TR_TIMEZONE     "Fuseau horaire"
#define TR_RXCHANNELORD "Ordre voies RX"
#define TR_MODE         "Mode"
#define TR_SLAVE        "Esclave"
#define TR_MODESRC      "Mode\003% Source"
#define TR_MULTIPLIER   "Multiplieur"
#define TR_CAL          "Cal"
#define TR_EEPROMV      "EEPROM v"
#define TR_VTRIM        "Trim- +"
#define TR_BG           "BG:"
#define TR_MENUTOSTART  "[MENU]POUR DEBUT"
#define TR_SETMIDPOINT  "REGLER NEUTRES"
#define TR_MOVESTICKSPOTS "BOUGER STICKS/POTS"
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
#define TR_MENUTORESET  "[MENU]pour reset"
#define TR_PPM          "PPM"
#define TR_CH           "CH"
#define TR_MODEL        "MODELE"
#define TR_FP           "PV"
#define TR_EEPROMLOWMEM "EEPROM pleine!"
#define TR_ALERT        "   ALERTE"
#define TR_PRESSANYKEYTOSKIP "Touche pour ignorer"
#define TR_THROTTLENOTIDLE "Gaz pas \202 z\200ro"
#define TR_ALARMSDISABLED   "Alarmes D\200sactiv\200es"
#define TR_PRESSANYKEY      "Touche pour continuer"
#define TR_BADEEPROMDATA    "EEPROM corrompue"
#define TR_EEPROMFORMATTING "Formatage EEPROM"
#define TR_EEPROMOVERFLOW   "D\200passement EEPROM"
#define TR_MENURADIOSETUP "CONFIG RADIO"
#define TR_MENUDATEANDTIME "DATE ET HEURE"
#define TR_MENUTRAINER  "ECOLAGE"
#define TR_MENUVERSION  "VERSION"
#define TR_MENUDIAG     "DIAG"
#define TR_MENUANA      "ANAS"
#define TR_MENUCALIBRATION "CALIBRATION"
#define TR_TRIMS2OFFSETS "Trims => Offsets"
#define TR_MENUMODELSEL "MODELES"
#define TR_MENUSETUP    "SETUP"
#define TR_MENUFLIGHTPHASE "PHASE DE VOL"
#define TR_MENUFLIGHTPHASES "PHASES DE VOL"
#define TR_MENUHELISETUP "CONF.HELI"
#define TR_MENUDREXPO   "DR/EXPO"
#define TR_MENULIMITS   "LIMITES"
#define TR_MENUCURVES   "COURBES"
#define TR_MENUCURVE    "COURBE"
#define TR_MENUCUSTOMSWITCHES "INTERS PERS"
#define TR_MENUFUNCSWITCHES   "INTERS FONCTIONS"
#define TR_MENUTELEMETRY "TELEMESURE"
#define TR_MENUTEMPLATES "GABARITS"
#define TR_MENUSTAT      "STATS"
#define TR_MENUDEBUG     "DEBUG"
#define TR_RXNUM         "NumRx"
#define TR_SYNCMENU      "Sync [MENU]"
#define TR_BACK          "Back"
#define TR_MINLIMIT      "Min Limit"
#define TR_MAXLIMIT      "Max Limit"
#define STR_LIMIT        (STR_MINLIMIT+4)
#define TR_MINRSSI       "RSSI Min."
#define TR_LATITUDE      "Latitude"
#define TR_LONGITUDE     "Longitude"
#define TR_GPSCOORD      "Coordonn\200es GPS"
#define TR_VARIO         "Vario"
#define TR_UNITSFORMAT   "Format Unit\200s"
#define TR_SHUTDOWN      "ARRET EN COURS"
#define TR_BATT_CALIB    "Calib. Batterie"
#define TR_CURRENT_CALIB "Calib. Courant"
#define TR_CURRENT       "Courant"
#define TR_SELECT_MODEL  "S\200lect Mod\201le"
#define TR_CREATE_MODEL  "Cr\200er Mod\201le"
#define TR_BACKUP_MODEL  "Archiver Mod\201le"
#define TR_DELETE_MODEL  "Supprimer Mod\201le"
#define TR_RESTORE_MODEL "R\200storer Mod\201le"
#define TR_SDCARD_ERROR  "SDCARD Error"
#define TR_NO_SDCARD     "No SDCARD"
#define TR_WARNING       "ALERTE"
#define TR_EEPROMWARN    "EEPROM"
#define TR_THROTTLEWARN  "GAZ"
#define TR_ALARMSWARN    "ALARMES"
#define TR_SWITCHWARN    "INTERS"
