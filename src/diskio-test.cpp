/*
 I put this in place of the code in the FRSKY first menu page, just as a quick test
*/
  // DISK_IO DEBUG -- XXX DELETE ME

  static uint8_t onceonly = 0;
  static FRESULT f_err_code;
  static FATFS FATFS_Obj;
  static uint8_t result = 0;
  static TCHAR sBuffer[100] = {0};
  static TCHAR *myStr = sBuffer;

  if (!onceonly)
  {
    // First, let's try to set the RTC date/time
    RTC rtc;

    rtc.year = 2011;
    rtc.month = 6;
    rtc.mday = 23;
    rtc.wday = 5;
    rtc.hour = 21;
    rtc.min = 53;
    rtc.sec = 0;

//    rtc_settime(&rtc);
// IT WORKED! And battery back-up for the RTC chip is working also.
    //////////////////////

    f_err_code = f_mount(0, &FATFS_Obj);
    FIL fil_obj;

    // atempt creating and writing to a new file
    result = f_open(&fil_obj, "/foo.txt", FA_CREATE_ALWAYS | FA_WRITE);
    f_printf(&fil_obj, "gruvin9x created this file! Yay!\n");
    f_close(&fil_obj);

    // That worked! Now test reading a line from another file.
    result = f_open(&fil_obj, "/foo.txt", FA_READ);
    myStr = f_gets(sBuffer, 100, &fil_obj);

    f_close(&fil_obj);

    onceonly = 1;
  }

  lcd_outdezAtt(5*FW, 2*FH, f_err_code, 0);
  lcd_outdezAtt(5*FW, 3*FH, result, 0);
  lcd_outdezAtt(5*FW, 4*FH, strlen(myStr), 0);
  lcd_outdezAtt(5*FW, 5*FH, myStr[0], 0);

  // can't use lcd_puts... becasue it specifies prog_char mem space, not SRAM
  uint8_t x=0;
  uint8_t j=6;
  for (uint8_t i=0; i<strlen(myStr); i++)
  {
    if (myStr[i]==0) break;
    if (myStr[i]!='\n') lcd_putc(x, j*FH, myStr[i]);
    x+=FW;
  }

  return;
