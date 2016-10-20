//Copyright 2016 by Walter Zimmer
// Version 09-09-16
//
#include "ff.h"

#define USE_USB_SERIAL
#ifdef USE_USB_SERIAL
	#define SERIALX Serial
#else
	#define SERIALX Serial1
#endif

FRESULT rc;        /* Result code */
FATFS fatfs;      /* File system object */
FIL fil;        /* File object */

#define MXFN 100 // maximal number of files 
#if defined(__MK20DX256__)
  #define BUFFSIZE (8*1024) // size of buffer to be written
#elif defined(__MK66FX1M0__)
  #define BUFFSIZE (32*1024) // size of buffer to be written
#endif

uint8_t buffer[BUFFSIZE] __attribute__( ( aligned ( 16 ) ) );
UINT wr;

/* Stop with dying message */
void die(char *str, FRESULT rc);
void setup();
void loop();

struct tm seconds2tm(uint32_t tt);

void die(char *str, FRESULT rc) 
{ SERIALX.printf("%s: Failed with rc=%u.\n", str, rc); for (;;) delay(100); }

TCHAR * char2tchar( char * charString, size_t nn, TCHAR * tcharString)
{ int ii;
  for(ii = 0; ii<nn; ii++) tcharString[ii] = (TCHAR) charString[ii];
  return tcharString;
}

char * tchar2char(  TCHAR * tcharString, size_t nn, char * charString)
{ int ii;
  for(ii = 0; ii<nn; ii++) charString[ii] = (char) tcharString[ii];
  return charString;
}

//=========================================================================
uint32_t count=0;
uint32_t ifn=2; // to test corrupted file (should be 0)
uint32_t isFileOpen=0;
char filename[80];
TCHAR wfilename[80];
uint32_t t0=0;
uint32_t t1=0;


void setup()
{
  pinMode(13,OUTPUT);
  pinMode(13,LOW);
  while(!SERIALX);
  pinMode(13,HIGH);
  
  #ifndef USB_SERIAL
	SERIALX.begin(115200,SERIAL_8N1_RXINV_TXINV);
  #endif
  SERIALX.println("\nLogger_test");

  f_mount (&fatfs, (TCHAR *)_T("/"), 0);      /* Mount/Unmount a logical drive */
}

void loop()
{
  if(ifn>MXFN) 
  { pinMode(13,OUTPUT);
    digitalWrite(13,HIGH); 
    delay(100); 
    digitalWrite(13,LOW); 
    delay(100); 
    return;
  }
  
  if(!count)
  {
    // close file
    if(isFileOpen)
    {
      //close file
      rc = f_close(&fil);
      if (rc) die("close", rc);
      //
      isFileOpen=0;
      t1=micros();
      float MBs = (1000.0f*BUFFSIZE)/(1.0*(t1-t0));
      SERIALX.printf(" (%d - %f MB/s)\n\r",t1-t0,MBs);
    }
  }
    
  //
  if(!isFileOpen)
  {
    // open new file
    ifn++;
    if(ifn>MXFN) return;

    sprintf(filename,"X_%05d.dat",ifn);
    SERIALX.println(filename);
    char2tchar(filename,80,wfilename);
    //
    // check status of file
    rc =f_stat(wfilename,0);
    Serial.printf("stat %d %x\n",rc,fil.obj.sclust);
    
    rc = f_open(&fil, wfilename, FA_WRITE | FA_CREATE_ALWAYS);
    Serial.printf(" opened %d %x\n\r",rc,fil.obj.sclust);
    // check if file is Good
    if(rc == FR_INT_ERR)
    { // only option is to close file
        rc = f_close(&fil);
        if(rc == FR_INVALID_OBJECT)
        { Serial.println("unlinking file");
          rc = f_unlink(wfilename);
          if (rc) die("unlink", rc);
        }
        else
          die("close", rc);
        
    }
    // retry open file
    rc = f_open(&fil, wfilename, FA_WRITE | FA_CREATE_ALWAYS);
    if(rc) die("open", rc);
    
    isFileOpen=1;
    t0=micros();
  }
  
  if(isFileOpen)
  {
     // fill buffer
     for(int ii=0;ii<BUFFSIZE;ii++) buffer[ii]='0'+(count%10);
     count++;
     //write data to file 
     if(!(count%10))SERIALX.printf(".");
     if(!(count%640)) SERIALX.println(); SERIALX.flush();
     //
     rc = f_write(&fil, buffer, BUFFSIZE, &wr);
     if (rc== FR_DISK_ERR) // IO error
     {  Serial.println(" write FR_DISK_ERR");
        // only option is to close file
        // force closing file
        count=1000;
     }
     else if(rc) die("write",rc);
    //    
     count %= 1000;
  }    
}

