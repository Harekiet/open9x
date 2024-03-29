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


/*--------------------------------------------------------------------------*/
/*  RTC controls                                                            */

#include "../open9x.h"
#include "rtc.h"

#define SCL_LOW()	DDRD |=	0x01		/* SCL = LOW */
#define SCL_HIGH()	DDRD &=	~0x01		/* SCL = High-Z */
#define	SCL_VAL		((PIND & 0x01) ? 1 : 0)	/* SCL input level */
#define SDA_LOW()	DDRD |=	0x02		/* SDA = LOW */
#define SDA_HIGH()	DDRD &=	~0x02   	/* SDA = High-Z */
#define	SDA_VAL		((PIND & 0x02) ? 1 : 0)	/* SDA input level */

/*-------------------------------------------------*/
/* I2C bus protocol                                */

static
void iic_delay (void)
{
#ifndef SIMU
	for (int n = 4; n; n--) PINB;
#endif
}


/* Generate start condition on the IIC bus */
static
void iic_start (void)
{
	SDA_HIGH();
	iic_delay();
	SCL_HIGH();
	iic_delay();
	SDA_LOW();
	iic_delay();
	SCL_LOW();
	iic_delay();
}


/* Generate stop condition on the IIC bus */
static
void iic_stop (void)
{
	SDA_LOW();
	iic_delay();
	SCL_HIGH();
	iic_delay();
	SDA_HIGH();
	iic_delay();
}


/* Send a byte to the IIC bus */
static
int iic_send (BYTE dat)
{
	BYTE b = 0x80;
	int ack;


	do {
		if (dat & b)	 {	/* SDA = Z/L */
			SDA_HIGH();
		} else {
			SDA_LOW();
		}
		iic_delay();
		SCL_HIGH();
		iic_delay();
		SCL_LOW();
		iic_delay();
	} while (b >>= 1);
	SDA_HIGH();
	iic_delay();
	SCL_HIGH();
	ack = SDA_VAL ? 0 : 1;	/* Sample ACK */
	iic_delay();
	SCL_LOW();
	iic_delay();
	return ack;
}


/* Receive a byte from the IIC bus */
static
BYTE iic_rcvr (int ack)
{
	UINT d = 1;


	do {
		d <<= 1;
		SCL_HIGH();
		if (SDA_VAL) d++;
		iic_delay();
		SCL_LOW();
		iic_delay();
	} while (d < 0x100);
	if (ack) {		/* SDA = ACK */
		SDA_LOW();
	} else {
		SDA_HIGH();
	}
	iic_delay();
	SCL_HIGH();
	iic_delay();
	SCL_LOW();
	SDA_HIGH();
	iic_delay();

	return (BYTE)d;
}



/*-------------------------------------------------*/
/* I2C block read/write controls                   */


int iic_read (
	BYTE dev,		/* Device address */
	UINT adr,		/* Read start address */
	UINT cnt,		/* Read byte count */
	BYTE *buff		/* Read data buffer */
)
{
	BYTE *rbuff = buff;
	int n;


	if (!cnt) return 0;

	n = 10;
	do {							/* Select device */
		iic_start();
	} while (!iic_send(dev) && --n);
	if (n) {
		if (iic_send((BYTE)adr)) {		/* Set start address */
			iic_start();				/* Reselect device in read mode */
			if (iic_send(dev | 1)) {
				do {					/* Receive data */
					cnt--;
					*rbuff++ = iic_rcvr(cnt ? 1 : 0);
				} while (cnt);
			}
		}
	}

	iic_stop();						/* Deselect device */

	return cnt ? 0 : 1;
}



int iic_write (
	BYTE dev,			/* Device address */
	UINT adr,			/* Write start address */
	UINT cnt,			/* Write byte count */
	const BYTE *buff	/* Data to be written */
)
{
	const BYTE *wbuff = buff;
	int n;


	if (!cnt) return 0;

	n = 10;
	do {							/* Select device */
		iic_start();
	} while (!iic_send(dev) && --n);
	if (n) {
		if (iic_send((BYTE)adr)) {		/* Set start address */
			do {						/* Send data */
				if (!iic_send(*wbuff++)) break;
			} while (--cnt);
		}
	}

	iic_stop();						/* Deselect device */

	return cnt ? 0 : 1;
}



/*-------------------------------------------------*/
/* RTC functions                                   */


int rtc_gettime (RTC *rtc)
{
	BYTE buf[8];


	if (!iic_read(0xD0, 0, 7, buf)) return 0;

	rtc->sec = (buf[0] & 0x0F) + ((buf[0] >> 4) & 7) * 10;
	rtc->min = (buf[1] & 0x0F) + (buf[1] >> 4) * 10;
	rtc->hour = (buf[2] & 0x0F) + ((buf[2] >> 4) & 3) * 10;
	rtc->wday = (buf[2] & 0x07);
	rtc->mday = (buf[4] & 0x0F) + ((buf[4] >> 4) & 3) * 10;
	rtc->month = (buf[5] & 0x0F) + ((buf[5] >> 4) & 1) * 10;
	rtc->year = 2000 + (buf[6] & 0x0F) + (buf[6] >> 4) * 10;

	return 1;
}




int rtc_settime (const RTC *rtc)
{

	BYTE buf[8];


	buf[0] = rtc->sec / 10 * 16 + rtc->sec % 10;
	buf[1] = rtc->min / 10 * 16 + rtc->min % 10;
	buf[2] = rtc->hour / 10 * 16 + rtc->hour % 10;
	buf[3] = rtc->wday & 7;
	buf[4] = rtc->mday / 10 * 16 + rtc->mday % 10;
	buf[5] = rtc->month / 10 * 16 + rtc->month % 10;
	buf[6] = (rtc->year - 2000) / 10 * 16 + (rtc->year - 2000) % 10;
	return iic_write(0xD0, 0, 7, buf);
}




int rtc_init (void)
{
	BYTE buf[8];	/* RTC R/W buffer */
	UINT adr;


	/* Read RTC registers */
	if (!iic_read(0xD0, 0, 8, buf)) return 0;	/* IIC error */

	if (buf[7] & 0x20) {	/* When data has been volatiled, set default time */
		/* Clear nv-ram. Reg[8..63] */
		memset(buf, 0, 8);
		for (adr = 8; adr < 64; adr += 8)
			iic_write(0x0D, adr, 8, buf);
		/* Reset time to Jan 1, '08. Reg[0..7] */
		buf[4] = 1; buf[5] = 1; buf[6] = 8;
		iic_write(0x0D, 0, 8, buf);
	}
	return 1;
}

