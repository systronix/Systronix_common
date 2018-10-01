#ifndef Systronix_common_H_
#define Systronix_common_H_


//---------------------------< I N C L U D E S >--------------------------------------------------------------
//
// name all 'common' or 'utility' header files stored in Systronix_common here so that other source files need only #include <Systronix_common.h>
//

#include "Systronix_utilities.h"


// files that are not in the common folder:

#include <Systronix_i2c_common.h>


//---------------------------< B I T   D E F I N I T I O N S >------------------------------------------------
//
// bit defines for setting and clearing specific bits of a variable.  Remember that #defines are SIGNED so
// BIT31 when shifted right as a 32-bit value will sign extend unless cast to uint32_t.  The same applies to
// BIT15 and BIT7; cast to uint16_t or uint8_t if shifting right in those types.
//

#define	BIT0	1
#define	BIT1	(1<<1)
#define	BIT2	(1<<2)
#define	BIT3	(1<<3)
#define	BIT4	(1<<4)
#define	BIT5	(1<<5)
#define	BIT6	(1<<6)
#define	BIT7	(1<<7)
#define	BIT8	(1<<8)
#define	BIT9	(1<<9)
#define	BIT10	(1<<10)
#define	BIT11	(1<<11)
#define	BIT12	(1<<12)
#define	BIT13	(1<<13)
#define	BIT14	(1<<14)
#define	BIT15	(1<<15)
#define	BIT16	(1<<16)
#define	BIT17	(1<<17)
#define	BIT18	(1<<18)
#define	BIT19	(1<<19)
#define	BIT20	(1<<20)
#define	BIT21	(1<<21)
#define	BIT22	(1<<22)
#define	BIT23	(1<<23)
#define	BIT24	(1<<24)
#define	BIT25	(1<<25)
#define	BIT26	(1<<26)
#define	BIT27	(1<<27)
#define	BIT28	(1<<28)
#define	BIT29	(1<<29)
#define	BIT30	(1<<30)
#define	BIT31	(1<<31)


//---------------------------< R E T U R N   V A L U E   D E F I N I T I O N S >------------------------------
//
// list of uint8_t return values
//

#define	SUCCESS			0			// standard return value for successful operation
#define	FAIL			(~SUCCESS)	// standard return value for unsuccessful operations that don't require detail


//---------------------------< M I S C E L L A N E O U S >----------------------------------------------------

#define	SW_RESET_KEY_A		0x05000000
#define	SW_RESET_KEY_2		0x00FA0000
#define	SYSRESETREQ			0x00000004


//---------------------------< P O R T   P I N S >------------------------------------------------------------
//
// Teensy port pin defines
//
// TODO: #if teensy 3.0, 3.1, 3.2, 3.5, 3.6 ...
//

enum {
	_P0 = 0, _P1, _P2, _P3, _P4, _P5, _P6, _P7, _P8, _P9,			// teensy 3.2
	_P10, _P11, _P12, _P13, _P14, _P15, _P16, _P17, _P18, _P19,
	_P20, _P21, _P22, _P23, _P24, _P25, _P26, _P27, _P28, _P29,
	_P30, _P31, _P32, _P33,											// end of teensy 3.2
	_P34, _P35, _P36, _P37, _P38, _P39,								// teensy 3.5
	_P40, _P41, _P42, _P43, _P44, _P45, _P46, _P47, _P48, _P49, 
	_P50, _P51, _P52, _P53, _P54, _P55, _P56, _P57,					// end of teensy 3.5
	};




/* 
#define	TOUCH			_P2		// display touch input; active low
#define TE				_P5		// tear enable input; active high
#define	WAIT			_P5		// display wait input; active low
#define	DDS_CS			_P6		// dds chip select output; active low
#define	DATA_H_CMD_L	_P21		// display control
#define	PERIPH_RST		_P22		// peripheral reset; active low
#define	DDS_RST			_P23		// dds reset; active high
 */


//---------------------------< P O R T   P I N   T E S T I N G >----------------------------------------------
//
// TODO: move and rename the por testing defines into Systronix_utilities.h?
//

#define RST_P			1		// NOT pin numbers; these are used by fw_reset() in utilities.cpp to identify a reset pin
#define RST_E			2

#define	SCL_PIN			_P19		// used for POR testing
#define	SDA_PIN			_P18

#define	SCL1_PIN		_P29		// used for POR testing
#define	SDA1_PIN		_P30

#define	FAIL_HIGH		0xF7	// these are used for POR testing results
#define	FAIL_LOW		0xF3






#endif	// Systronix_common_H_