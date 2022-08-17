# AD5932
C and H file to handle the AD5932 Programmable Frequency Scan Waveform Generator

To use this code in your project, do these:<br/>
-change AD5932_SetSPI() and AD5932_SendSPICommand() functions to your system's SPI commands<br/>
-replace SPARE0_on() ... SPARE3_off() GPIO pin on/off macros to your system's<br/>
-implement your delay_us() usec delay function<br/>
-call AD5932_Init() first, then call AD5932_SetSPI() to set the SPI port<br/>
-test your HW with this self-contained command: AD5932_TestSetup();<br/>

Used types:<br/>
typedef unsigned char bool;<br/>
typedef unsigned char u08;<br/>
typedef char s08;
typedef unsigned short u16;<br/>
typedef short s16;<br/>
typedef unsigned long u32;<br/>
typedef long s32;<br/>
typedef unsigned long long u64;<br/>
typedef long long s64;<br/>

Datasheet:
https://www.analog.com/en/products/ad5932.html

AN-1044:
https://www.analog.com/media/en/technical-documentation/application-notes/an-1044.pdf
