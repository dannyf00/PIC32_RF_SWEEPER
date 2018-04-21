//using REFCO as RF signal sweep generator
//
//MCU: PIC32MX1/2 family of chips. Tested on a PIC32MX250F128B, running at 48Mhz, off an internal RC oscillator
//
//connection / two output pins:
//1. RF out: RA4
//2. Ramp out: RB4
//
//RF output pin, defined by REFCO_OUT() in refco.h, can be any of RPA2()/RPB6()/RPA4()/RPB13()/RPB2()/RPC6()/RPC1()/RPC3() - not all pins are available on a given package
//Voltage ramp pin, defined by PWM2_TO_RP() in pwm2.h, can be any of RPA1()/RPB5()/RPB1()/RPB8()/RPA8()/RPC8()/RPA9() - not all pins are available on a given package
//
//wiring:
//
//  3.3v
//   _
//   |
//   |
//  [ ] 10k
//   |
//   |   |----------------|
//   ----|_RESET          |         110R
//       |            RA4 |---------[ ]------< RF sweep output < 
//       |                |
//       |                |
//       |                |
//       |                |
//       |                |
//       |                |
//       |                |
//       |                |
//       |                |
//       |                |
//       |                |      10 - 100K
//       |            RB5 |---------[ ]-------< Ramp   output <
//       |                |               |
//       |----------------|               |
//                                        = 4.7 - 11u
//                                        |
//                                        |
//                                        -
//                                       GND
//
//additional parts
// 1. use 0.1uf decoupling on PIC32.
// 2. use 0.1uf ceramic from Vcap pin to ground
//
//
//Compiler: tested on PIC32 and should work for XC32
//
//

#include "config.h"						//configuration words
#include "gpio.h"
#include "delay.h"						//we use software delays
#include "pwm2.h"						//use pwm generator
#include "refco.h"						//use reference clock generator

//hardware configuration
#define LED_PORT			LATB
#define LED_DDR				TRISB
#define LED					(1<<7)
#define LED_DLY				100ul		//cycles to be wasted -> to manage main loop update frequency

//various options for bands to be sweeped
//for audio band (1-20Khz)
#define F0					(1)			//output frequency, in Khz
#define F1					(20)		//output frequency, in Khz

//for AM IF 455Khz alignment
//#define F0					(455-30)	//output frequency, in Khz
//#define F1					(455+30)	//output frequency, in Khz

//for FM IF 10.7Mhz alignment
//#define F0					(10700-100)	//output frequency, in Khz
//#define F1					(10700+100)	//output frequency, in Khz

//for FM IF 10.7Mhz alignment
//#define F0					(10700-100)	//output frequency, in Khz
//#define F1					(10700+100)	//output frequency, in Khz

//for Broadcast AM band sweep (535 - 1605Khz)
//#define F0					(535)		//output frequency, in Khz
//#define F1					(1605)		//output frequency, in Khz

#define F_STEP				100			//number of steps between F0 and F1
//end hardware configuration

//global defines
//convert Khz to NM512
#define Khz2NM512(khz)		((F_CPU) / 1000 * 256 / (khz))
#define Khz2NM512f(khz)		((double)(F_CPU) / 1000 * 256 / (khz) + 0.5)

//global variables

int main(void) {
	double freq;					//current frequency, in Khz
	int i;							//frequency step index
	uint32_t nm512;					//n=15bit, m=9bit
	
	mcu_init();						//reset the mcu
	//reset the refco
	refco_init(REFCO_SYSCLK);		//use sysclk
	nm512 = Khz2NM512f(F0);			//get NM512
	refco_setnm(nm512);				//start sweeping at F0
	refco_en();						//enable the output
	
	//generate a ramp
	pwm2_init(TMRPS_8x, F_STEP);
	
	IO_OUT(LED_DDR, LED);			//led as output

	ei();							//enable interrupts
	while (1) {
		//generating frequency sweep, in F_STEP
		for (i = 0; i < F_STEP; i++) {
			freq = F0 + (F1 - F0) * (double) i / F_STEP;	//calculate target frequency
			nm512 = Khz2NM512f(freq);	//calculate REFCO dividers
			refco_setnm(nm512);			//set REFCO dividers
			
			//generate the ramp
			pwm2_setdc(i);
			
			//blink an led - debug only
			IO_FLP(LED_PORT, LED);		//flip the led
			delay_ms(LED_DLY);			//waste sometime
		}	
	}
}
