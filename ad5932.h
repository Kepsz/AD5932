
// ********************************************************************************************************************
// @file        ad5932.h
// @brief:      Programmable frequency scan generator with SPI
// @version     1.2
// @date        2022.09.03
// @author      Tamas Kovacs, Tamas Besenyi
// ********************************************************************************************************************

#ifndef __AD5932_H
#define __AD5932_H

#include "defs.h"

#if (MCU_FAMILY == LPC175X6X) || (MCU_FAMILY == LPC177X8X_LPC407X8X)
	#include "lpc17xx_ssp.h"
	#include "lpc17xx_gpio.h"
#elif (MCU_FAMILY == LPC55XX) || (MCU_FAMILY == LPC54XXX)
	#include "LPC5x_spi.h"
	#include "LPC5x_gpio.h"
#endif

#define AD5932_PORT_BUSY		0xFFFF
#define AD5932_PARAM_ERROR		0xFFF0
#define AD5932_ACCU_RESOLUTION	0x1000000

//parameter structure for external use
typedef struct
{
	u32 startF;
	u32 deltaF;
	u32 increment;
	u32 intervall;
	bool incrementBase;
	bool sweepType;
} AD5932Params_t;

//config bits
typedef enum _RegBits_t
{
	DAC_EN					= 1,
	DAC_DISABLE				= 0,
	SINE_OUT				= 1,
	TRIANGLE_OUT			= 0,
	MSBOUT_EN				= 1,
	MSBOUT_DISABLE			= 0,
	AUTOMATIC_TRIGGER		= 0,
	EXTERNAL_TRIGGER		= 1,
	SYNCSEL_END				= 1,
	SYNCSEL_SUBSEQVENT		= 0,
	SYNCOUT_EN				= 1,
	SYNCOUT_DISABLE			= 0
} RegBits_t;

// SPI control bits D15-12
typedef enum _AD5932_ControlRegs_t
{
	AD5932_CREG				= 0x0000,	//Control bits
	AD5932_NINCR			= 0x1000,	//Number of increments
	AD5932_DFREQ_LO			= 0x2000,	//Lower 12 bits of delta frequency
	AD5932_DFREQ_HI			= 0x3000,	//Upper 12 bits of delta frequency
	AD5932_TINT_WCYCLES		= 0x4000,	//Increment interval based on fixed number of output waveform cycles
	AD5932_TINT_MCLKCYCLES	= 0x6000,	//Increment interval based on fixed number of clock periods
	AD5932_FSTART_LO		= 0xC000,	//Lower 12 bits of start frequency
	AD5932_FSTART_HI		= 0xD000	//Upper 12 bits of start frequency
} AD5932_ControlRegs_t;

// Sweep type control
typedef enum _AD5932_SweepType_t
{
	INCREMENTAL_SWEEP		= true,		//Low to high frequency sweep
	DECREMENTAL_SWEEP		= false		//High to low frequency sweep
} AD5932_SweepType_t;

// Increment interval control
typedef enum _AD5932_IncIntervall_t
{
	WAVE_OUT_BASED			= true,		//Increment interval based on fixed number of output waveform cycles
	MCLK_INP_BASED			= false		//Increment interval based on fixed number of clock periods
} AD5932_IncIntervall_t;

void AD5932_SetSPI(LPC_SSP_TypeDef* SSPx);
void AD5932_Init(u32 MCLK);
void AD5932_TriggerCTRLPin(void);
void AD5932_TriggerINTPin(void);
s32 AD5932_SingleFrequencyGenerator(u32 frequency, RegBits_t WAVE_TYPE, RegBits_t MSBOUT, RegBits_t TRIGGER);
s32 AD5932_SweepGenerator(u32 startFreq, u32 deltaFrerq, u32 increment, AD5932_IncIntervall_t INCRTYPE, u32 incIntervall, RegBits_t SWEEPTYPE, RegBits_t WAVE_TYPE, RegBits_t MSBOUT, RegBits_t TRIGGER, RegBits_t SYNCSEL, RegBits_t SYNCOUT);
s32 AD5932_TestSetup(void);

#endif
