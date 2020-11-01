/*
 * WMXZ Teensy uSDFS library
 * Copyright (c) 2019 Walter Zimmer.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "core_pins.h"  // includes calls to kinetis.h or imxrt.h

#include "../diskio.h"
#include "sd_msc.h"
#include "sd_config.h"

#if defined __MK66FX1M0__ || defined __MK64FX512__ || defined __IMXRT1052__ || defined __IMXRT1062__ 
	#define HAVE_MSC USE_MSC	//USE_MSC (0,1) is defined in sd_config
#else
	#define HAVE_MSC 0
#endif

#if HAVE_MSC == 1
	#include <stdlib.h>
	#include <string.h>
	#include "usb_serial.h"
	#include "msc.h"
	#include "MassStorage.h"
// needs msc from  https://github.com/wwatson4506/MSC	
#ifndef USE_EXTERNAL_INIT
	USBHost myusb;
	USBHub hub1(myusb);
	USBHub hub2(myusb);
	USBHub hub3(myusb);
	USBHub hub4(myusb);
	msHost mscHost;
	msController msDrive1(myusb);
	msController msDrive2(myusb);
#endif


	int MSC_disk_status(BYTE pDrv) 
	{	
		int stat = 0;
		switch(pDrv) {
			case MSD1:
				if(msDrive1.checkConnectedInitialized() != MS_CBW_PASS)
					stat = STA_NODISK;
				break;
			case MSD2:
				if(msDrive2.checkConnectedInitialized() != MS_CBW_PASS)
					stat = STA_NODISK;
				break;
			default:
				stat = STA_NODISK;
		}
//		if(!deviceAvailable()) stat = STA_NODISK; 	// No USB Mass Storage Device Connected
//		if(!deviceInitialized()) stat = STA_NOINIT; // USB Mass Storage Device Un-Initialized
		return stat;
	}

	int MSC_disk_initialize(BYTE pDrv) 
	{	
		int stat = 0;

#ifndef USE_EXTERNAL_INIT
		myusb.begin();
		mscHost.mscHostInit();
#endif
		switch(pDrv) {
			case MSD1:
				if(msDrive1.mscInit() != MS_CBW_PASS)
					stat = STA_NOINIT;
				break;
			case MSD2:
				if(msDrive2.mscInit() != MS_CBW_PASS)
					stat = STA_NOINIT;
				break;
			default:
				stat = STA_NOINIT;
		}
		return stat;
	}

	int MSC_disk_read(BYTE pDrv, BYTE *buff, DWORD sector, UINT count) 
	{
		switch(pDrv) {
			case MSD1:
				return mscHost.readSectors(&msDrive1, (BYTE *)buff, sector, count);
			case MSD2:
				return mscHost.readSectors(&msDrive2, (BYTE *)buff, sector, count);
		}
	}

	int MSC_disk_write(BYTE pDrv, const BYTE *buff, DWORD sector, UINT count) 
	{
		switch(pDrv) {
			case MSD1:
				return mscHost.writeSectors(&msDrive1, (BYTE *)buff, sector, count);
			case MSD2:
				return mscHost.writeSectors(&msDrive2, (BYTE *)buff, sector, count);
		}
	}

	int asyncMSC_disk_read(BYTE pDrv, BYTE *buff, DWORD sector, UINT count) 
	{
		switch(pDrv) {
			case MSD1:
				return mscHost.asyncReadSectors(&msDrive1, (BYTE *)buff, sector, count);
			case MSD2:
				return mscHost.asyncReadSectors(&msDrive2, (BYTE *)buff, sector, count);
		}
	}

	int asyncMSC_disk_write(BYTE pDrv, const BYTE *buff, DWORD sector, UINT count) 
	{
		switch(pDrv) {
			case MSD1:
				return mscHost.asyncWriteSectors(&msDrive1, (BYTE *)buff, sector, count);
			case MSD2:
				return mscHost.asyncWriteSectors(&msDrive2, (BYTE *)buff, sector, count);
		}
	}
	int MSC_ioctl(BYTE pDrv, BYTE cmd, BYTE *buff) {return 0;}
#else
	int MSC_disk_status() {return STA_NOINIT;}
	int MSC_disk_initialize() {return STA_NOINIT;}
	int MSC_disk_read(BYTE *buff, DWORD sector, UINT count) {return STA_NOINIT;}
	int MSC_disk_write(const BYTE *buff, DWORD sector, UINT count) {return STA_NOINIT;}
	int MSC_ioctl(BYTE cmd, BYTE *buff) {return STA_NOINIT;}
#endif
