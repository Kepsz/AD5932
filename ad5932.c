
// ********************************************************************************************************************
// @file        ad5932.c
// @brief:      Programmable frequency scan generator with SPI
// @version     1.0
// @date        2022.08.17
// @author      Tamas Kovacs, Tamas Besenyi
// ********************************************************************************************************************

// --------------------------------------------------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------------------------------------------------

#include "main.h"
#include "config.h"
#include "rio.h"
#include "delay.h"
#if USE_AD5932

#include "ad5932.h"

// --------------------------------------------------------------------------------------------------------------------
// Defines
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// Variables
// --------------------------------------------------------------------------------------------------------------------
LPC_SSP_TypeDef* SSPPort;
u16 ad5932CMD;
u32 ad5932MCLK;

// --------------------------------------------------------------------------------------------------------------------
// Macros
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// Notes
// --------------------------------------------------------------------------------------------------------------------

//When the AD5932 is powered up, the part is in an undefined
//state and, therefore, must be reset before use. The seven registers
//(control and frequency) contain invalid data and need to be set
//to a known value by the user. The control register should be the
//first register to be programmed, as this sets up the part. Note
//that a write to the control register automatically resets the internal
//state machines and provides an analog output of midscale, because
//it performs the same function as the INTERRUPT pin. Typically,
//this is followed by a serial loading of all the required scan
//parameters. The DAC output remains at midscale until a
//frequency scan is started using the CTRL pin.

//SPI communication with AD5932 in short:
//There are a bunch of registers to be set with 16bit long commands.
//-FSYNC needs to be held low while the 16bit is sent out, but high otherwise
//-Set CTRL pin high only after the last command, for like 100us. (low->high->low)
//-SPI mode should be CHPA: first clock edge, and CPOL: Low", but the communications is worked at all possible SPI modes in my board. o.O

//To use this code in your project, do these:
//-change AD5932_SetSPI() and AD5932_SendSPICommand() functions to your SPI commands
//-change SPARE0_on() ... SPARE3_off() GPIO pin on/off functions
//-implement your delay_us() usec delay function
//-call AD5932_Init() first, then call AD5932_SetSPI() to set the SPI port
//-test your HW with this self-contained command: AD5932_TestSetup();

// --------------------------------------------------------------------------------------------------------------------
// Functions
// --------------------------------------------------------------------------------------------------------------------

// ....................................................................................................................
// @brief:      Sets the used SSP (spi) peripheral
// @param[in]:  LPC_SSP0 or LPC_SSP1
// @return:     none
// ....................................................................................................................
void AD5932_SetSPI(LPC_SSP_TypeDef* SSPx)
{
	SSPPort = SSPx;
}

// ....................................................................................................................
// @brief:      Send out one 16Bit long command over SSP (spi) bus
// @param[in]:  none
// @return:     0 if OK. Negative if there was an SPI error, Positive if SPI is busy.
// ....................................................................................................................
s32 AD5932_SendSPICommand(u16 commandWord)
{
	s32 ret;
	//check if port is free
	ret = SSP_GetTransferStatus(SSPPort);
	if (ret == SSP_STATUS_CLEAR)
	{
		AD5932_SetFSYNCPin(false);
		ret = SSP_Transfer(SSPPort, NULL, &commandWord, NULL, 1, SSP_XFER_POLL);
		AD5932_SetFSYNCPin(true);
		if (ret > 0)
			return 0;
		return ret;
	}
	else
		return AD5932_PORT_BUSY;
}

// ....................................................................................................................
// @brief:      Set / Clear AD5932 FSYNC pin.
// @param[in]:  none
// @return:     none
// ....................................................................................................................
void AD5932_SetFSYNCPin(bool state)
{
	if (state)
		SPARE0_on();
	else
		SPARE0_off();
}

// ....................................................................................................................
// @brief:      Set / Clear AD5932 CONTROL pin.
// @param[in]:  none
// @return:     none
// ....................................................................................................................
void AD5932_SetCTRLPin(bool state)
{
	if (state)
		SPARE2_on();
	else
		SPARE2_off();
}

// ....................................................................................................................
// @brief:      Set / Clear AD5932 INTERRUPT pin.
// @param[in]:  none
// @return:     none
// ....................................................................................................................
void AD5932_SetINTPin(bool state)
{
	if (state)
		SPARE3_on();
	else
		SPARE3_off();
}

// ....................................................................................................................
// @brief:      Set / Clear AD5932 STANDBY pin.
// @param[in]:  none
// @return:     none
// ....................................................................................................................
void AD5932_SetSTDBYPin(bool state)
{
	if (state)
		SPARE1_on();
	else
		SPARE1_off();
}

// ....................................................................................................................
// @brief:      Initial AD5932 pin config after startup
// @param[in]:  External MCLK frequency in HZ
// @return:     none
// ....................................................................................................................
void AD5932_Init(u32 MCLK)
{
	AD5932_SetCTRLPin(false);
	AD5932_SetINTPin(false);
	AD5932_SetFSYNCPin(true);
	AD5932_SetSTDBYPin(false);
	ad5932MCLK = MCLK;
}

// ....................................................................................................................
// @brief:      Sets the Control register of AD5932
// @param[in]:  DAC_EN / DAC_DAC_DISABLE - enables or disables the DAC
// @param[in]:  SINE_OUT / TRIANGLE_OUT - output waveform
// @param[in]:  MSBOUT_EN / MSBOUT_DISABLE - MSB Out functionality
// @param[in]:  AUTOMATIC_TRIGGER / EXTERNAL_TRIGGER - sweep start trigger type
// @param[in]:  SYNCSEL_END / SYNCSEL_SUBSEQVENT - pulse at end of scan (EOS) or at each frequency increment.
// @param[in]:  SYNCOUT_EN / SYNCOUT_DISABLE - use of SYNCOUT pin
// @return:     Return 0 if all is OK. Negative if error, 0xFFFF if SPI port is busy.
// ....................................................................................................................
s32 AD5932_SetControlRegister(RegBits_t DAC_STATE, RegBits_t WAVE_TYPE, RegBits_t MBSOUT_STATE, RegBits_t TRIGGER_TYPE, RegBits_t SYNCSEL_STATE, RegBits_t SYNCOUT_STATE)
{
	u16 temp = 1;					//reserved, B0 must be '1'

	temp |= 1 << 1;					//reserved, B1 must be '1'
	temp |= SYNCOUT_STATE << 2;
	temp |= SYNCSEL_STATE << 3;
	temp |= 1 << 4;					//reserved, B4 must be '1'
	temp |= TRIGGER_TYPE << 5;
	temp |= 1 << 6;					//reserved, B6 must be '1'
	temp |= 1 << 7;					//reserved, B7 must be '1'
	temp |= MBSOUT_STATE << 8;
	temp |= WAVE_TYPE << 9;
	temp |= DAC_STATE << 10;
	temp |= 1 << 11;				//B11 '1' means 24 bit long command mode. The other mode is stupid. Yes. Stupid.

	ad5932CMD = AD5932_CREG | temp;

	return AD5932_SendSPICommand(ad5932CMD);
}

// ....................................................................................................................
// @brief:      Set the frequency increment
// @param[in]:  2..4095 frequency increments is multiplied with delta frequency during a frequency step.
// @return:     Return 0 if all is OK. Negative if error, 0xFFFF if SPI port is busy. 0xFFF0 if range error.
// ....................................................................................................................
s32 AD5932_SetIncrement(u16 value)
{
	if ((value > 4095) || (value < 2))
		return AD5932_PARAM_ERROR;

	ad5932CMD = AD5932_NINCR | value;

	return AD5932_SendSPICommand(ad5932CMD);
}

// ....................................................................................................................
// @brief:      Set the time between frequency steps
// @param[in]:  Number of cycles required to jump the frequency to the next value.
// @param[in]:  Type of frequency increment base
// @return:     Return 0 if all is OK. Negative if error, 0xFFFF if SPI port is busy. 0xFFF0 if range error.
// ....................................................................................................................
s32 AD5932_SetIncrementIntervall(u16 value, AD5932_IncIntervall_t incrementBase)
{
	if ((value > 2047) || (value < 2))
		return AD5932_PARAM_ERROR;

	if (incrementBase == WAVE_OUT_BASED)
		ad5932CMD = AD5932_TINT_WCYCLES | value;
	else
		ad5932CMD = MCLK_INP_BASED | value;

	return AD5932_SendSPICommand(ad5932CMD);
}

// ....................................................................................................................
// @brief:      Set the delta frequency. This is the increment (or decrement) steps.
// @param[in]:  Frequency in Hz, Increment / Decrement sweep type
// @return:     Return 0 if all is OK. Negative if error, 0xFFFF if SPI port is busy. 0xFFF0 if range error.
// ....................................................................................................................
s32 AD5932_SetDeltaFrequency(u32 value, AD5932_SweepType_t SweepType)
{
	s32 ret;
	if (value > 0x7FFFFFFF)
		return AD5932_PARAM_ERROR;

	//We have to calculate the right command based on the MCLK frequency, the desired start frequency and the on-chip accumulator resolution (See AN-1044)
	u32 tmp = (u64)value * AD5932_ACCU_RESOLUTION / ad5932MCLK;

	ad5932CMD = AD5932_DFREQ_LO | (tmp & 0x00000FFF);
	ret = AD5932_SendSPICommand(ad5932CMD);
	if (ret == AD5932_PORT_BUSY)
		return ret;

	ad5932CMD = AD5932_DFREQ_HI | ((tmp >> 12) & 0x00000FFF);
	if (SweepType == DECREMENTAL_SWEEP)
		ad5932CMD |= 1 << 11;	//negative sweep indicator bit

	ret = AD5932_SendSPICommand(ad5932CMD);
	if (ret == AD5932_PORT_BUSY)
		return ret;

	return 0;
}

// ....................................................................................................................
// @brief:      Set the start frequency.
// @param[in]:  Frequency in Hz
// @return:     Return 0 if all is OK. Negative if error, 0xFFFF if SPI port is busy. 0xFFF0 if range error.
// ....................................................................................................................
s32 AD5932_SetStartFrequency(u32 value)
{
	s32 ret;
	if ((value > 0x7FFFFFFF) || (value < 1))
		return AD5932_PARAM_ERROR;

	//We have to calculate the right command based on the MCLK frequency, the desired start frequency and the on-chip accumulator resolution (See AN-1044)
	u32 tmp = (u64)value * AD5932_ACCU_RESOLUTION / ad5932MCLK;

	ad5932CMD = AD5932_FSTART_LO | (tmp & 0x00000FFF);
	ret = AD5932_SendSPICommand(ad5932CMD);
	if (ret == AD5932_PORT_BUSY)
		return ret;

	ad5932CMD = AD5932_FSTART_HI | ((tmp >> 12) & 0x00000FFF);
	ret = AD5932_SendSPICommand(ad5932CMD);
	if (ret == AD5932_PORT_BUSY)
		return ret;

	return 0;
}

// ....................................................................................................................
// @brief:      Triggers the CTRL pin that starts the sweep after programming.
// @param[in]:  none
// @return:     none
// ....................................................................................................................
void AD5932_TriggerCTRLPin(void)
{
	AD5932_SetCTRLPin(true);
	delay_us(100);
	AD5932_SetCTRLPin(false);
}

// ....................................................................................................................
// @brief:      The AD5932 will function as a simple DDS, outputting the same frequency non-stop.
// @param[in]:  Frequency in Hz
// @param[in]:  Wave type SINE_OUT / TRIANGLE_OUT
// @return:     0 if all is OK, negative value if not.
// ....................................................................................................................
s32 AD5932_SingleFrequencyGenerator(u32 frequency, RegBits_t WAVE_TYPE, RegBits_t MSBOUT, RegBits_t TRIGGER)
{
	s32 ret;
	AD5932_SetCTRLPin(false);

	ret = AD5932_SetControlRegister(DAC_EN, WAVE_TYPE, MSBOUT, EXTERNAL_TRIGGER, SYNCSEL_END, SYNCOUT_EN);
	if (ret < 0)
		return -1;

	ret = AD5932_SetStartFrequency(frequency);
	if (ret < 0)
		return -2;

	if (TRIGGER == AUTOMATIC_TRIGGER)
		AD5932_TriggerCTRLPin();
	return 0;
}

// ....................................................................................................................
// @brief:      The AD5932 will perform frequency sweep(s) based on the input params.
// @param[in]:  Start frequency in HZ
// @param[in]:  Delta frequency in HZ
// @param[in]:  Increment number 2..4095
// @param[in]:  Increment interval type,
//				WAVE_OUT_BASED: Increment interval based on fixed number of output waveform cycles
//				MCLK_INP_BASED:	Increment interval based on fixed number of clock periods
// @param[in]:  Sweep type,
//				INCREMENTAL_SWEEP
//				DECREMENTAL_SWEEP
// @param[in]:  Increment interval: 2..2047 - The number of cycles required to jump the frequency to the next value.
// @param[in]:  Wave type,
//				SINE_OUT
//				TRIANGLE_OUT
// @param[in]:  Wave type,
//				MSBOUT_EN
//				MSBOUT_DISABLE
// @param[in]:  Trigger type,
//				AUTOMATIC_TRIGGER
//				EXTERNAL_TRIGGER
// @param[in]:  Syncsel,
//				SYNCSEL_END: the SYNCOUT pin outputs a high level at end of scan and returns to 0 at the start of the subsequent scan
//				SYNCSEL_SUBSEQVENT: the SYNCOUT pin outputs a pulse of 4 × T CLOCK only at each frequency increment.
// @param[in]:  Syncout,
//				SYNCOUT_EN: the SYNC output is available at the SYNCOUT pin.
//				SYNCOUT_DISABLE: the SYNCOP pin is disabled (three-state).
// @return:     0 if all is OK, negative value if not.
// ....................................................................................................................
s32 AD5932_SweepGenerator(u32 startFreq, u32 deltaFrerq, u32 increment, AD5932_IncIntervall_t INCRTYPE, u32 incIntervall, RegBits_t SWEEPTYPE, RegBits_t WAVE_TYPE, RegBits_t MSBOUT, RegBits_t TRIGGER, RegBits_t SYNCSEL, RegBits_t SYNCOUT)
{
	s32 ret;
	AD5932_SetCTRLPin(false);

	ret = AD5932_SetControlRegister(DAC_EN, WAVE_TYPE, MSBOUT, TRIGGER, SYNCSEL, SYNCOUT);
	if (ret < 0)
		return -1;

	ret = AD5932_SetStartFrequency(startFreq);
	if (ret < 0)
		return -2;

	ret = AD5932_SetDeltaFrequency(deltaFrerq, SWEEPTYPE);
	if (ret < 0)
		return -3;

	ret = AD5932_SetIncrementIntervall(incIntervall, INCRTYPE);
	if (ret < 0)
		return -4;

	ret = AD5932_SetIncrement(increment);
	if (ret < 0)
		return -5;

	if (TRIGGER == AUTOMATIC_TRIGGER)
		AD5932_TriggerCTRLPin();
	return 0;
}

// ....................................................................................................................
// @brief:      Quick debug command to check HW functionality. The AD5932 will produce continuous sine wave sweeps.
// @param[in]:  none
// @return:     0 if all is OK, negative value if not.
// ....................................................................................................................
s32 AD5932_TestSetup(void)
{
	s32 ret;
	AD5932_SetCTRLPin(false);

	ret = AD5932_SetControlRegister(DAC_EN, SINE_OUT, MSBOUT_EN, AUTOMATIC_TRIGGER, SYNCSEL_END, SYNCOUT_EN);
	if (ret < 0)
		return -1;

	ret = AD5932_SetStartFrequency(1000);
	if (ret < 0)
		return -2;

	ret = AD5932_SetDeltaFrequency(1000, INCREMENTAL_SWEEP);
	if (ret < 0)
		return -3;

	ret = AD5932_SetIncrementIntervall(2000, WAVE_OUT_BASED);
	if (ret < 0)
		return -4;

	ret = AD5932_SetIncrement(2);
	if (ret < 0)
		return -5;

	AD5932_TriggerCTRLPin();
	return 0;
}

#endif
