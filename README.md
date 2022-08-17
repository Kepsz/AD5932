# AD5932
C and H file to handle the AD5932 Programmable Frequency Scan Waveform Generator

To use this code in your project, do these:
-change AD5932_SetSPI() and AD5932_SendSPICommand() functions to your system's SPI commands
-replace SPARE0_on() ... SPARE3_off() GPIO pin on/off macros to your system's
-implement your delay_us() usec delay function
-call AD5932_Init() first, then call AD5932_SetSPI() to set the SPI port
-test your HW with this self-contained command: AD5932_TestSetup();

Used types:
typedef unsigned char bool;
typedef unsigned char u08;
typedef char s08;
typedef unsigned short u16;
typedef short s16;
typedef unsigned long u32;
typedef long s32;
typedef unsigned long long u64;
typedef long long s64;

Datasheet:
https://www.analog.com/en/products/ad5932.html

AN-1044:
https://www.analog.com/media/en/technical-documentation/application-notes/an-1044.pdf
