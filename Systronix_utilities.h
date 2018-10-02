#ifndef Systronix_utilities_H_
#define Systronix_utilities_H_


//---------------------------< I N C L U D E S >--------------------------------------------------------------

#include <Arduino.h>
#include <Systronix_common.h>			// general purpose defines

//#include <SALT_logging.h>
//#include "../../SALT_reptile/SALT_reptile.h"	// for mailbox struct definition


//---------------------------< D E F I N E S >----------------------------------------------------------------

#define ETHER_RST			_P9			// TODO: find a better way to support 'anonymous' fw controlled reset pins
#define	PERIPH_RST			_P22		// peripheral reset; active low


#define	TEST_SPI_MISO_HL	BIT0		// bit position definitions for spi_port_pins_result
#define	TEST_SPI_MOSI_HL	BIT1		// these bits indicate that the matching pin in the upper byte is
#define	TEST_SPI_SCK_HL		BIT2		// stuck high (1) or stuck low (0)
#define	TEST_SPI_CS1_HL		BIT3
#define	TEST_SPI_CS2_HL		BIT4
#define	TEST_SPI_CS3_HL		BIT5
#define	TEST_SPI_CS4_HL		BIT6
#define	TEST_SPI_CS5_HL		BIT7

#define	TEST_SPI_MISO		BIT8		// these bits identify the failing signal; 0 if ok, 1 if failed
#define	TEST_SPI_MOSI		BIT9		// for any of miso, mosi, sck stuck high or low: no spi
#define	TEST_SPI_SCK		BIT10
#define	TEST_SPI_CS1		BIT11		// any cs stuck low then no spi; for cs stuck high the others should be functional
#define	TEST_SPI_CS2		BIT12
#define	TEST_SPI_CS3		BIT13
#define	TEST_SPI_CS4		BIT14
#define	TEST_SPI_CS5		BIT15

#define	SPI_MISO_PIN		_P12		// port pin defines used only for the SPI pin mobility tests
#define	SPI_MOSI_PIN		_P11
#define	SPI_SCK_PIN			_P13
#define	SPI_CS1_PIN			_P6
#define	SPI_CS2_PIN			_P8
#define	SPI_CS3_PIN			_P10
#define	SPI_CS4_PIN			_P15
#define	SPI_CS5_PIN			_P20


										// bit position definitions for i2c mobility test return values
#define	TEST_I2C_SDA_HL		BIT2		// these bits indicate that the matching pin in the upper nibble is
#define	TEST_I2C_SCL_HL		BIT3		// stuck high (1) or stuck low (0)

#define	TEST_I2C_SDA		BIT6		// these bits identify the failing signal; 0 if ok, 1 if failed
#define	TEST_I2C_SCL		BIT7

#define	I2C_SCL_PIN			_P19		// used for POR testing
#define	I2C_SDA_PIN			_P18

#define	I2C_SCL1_PIN		_P29		// used for POR testing
#define	I2C_SDA1_PIN		_P30

#define	FAIL_HIGH			0xF7		// these are used for POR testing results
#define	FAIL_LOW			0xF3


//---------------------------< C L A S S >--------------------------------------------------------------------

class Systronix_utilities
	{
	private:
		uint32_t		aircr;									// used to do a software reset; leave this at zero
																// unless you really want to reset the controller
		const char*	const reset_source_string[6] =
							{
							"software reset",
							"power on",
							"external reset",
							"watchdog",
							"low voltage",
							"unknown"
							};

		uint8_t 	days_in_month [13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	protected:
		void			hex_dump_core (uint16_t, uint8_t*);

		uint8_t			spi_pin_mobility_test (uint8_t);

		boolean			_is_leap_year (uint16_t);
		uint8_t			_day_of_week (uint16_t, uint8_t, uint8_t);
//		void			_add_time (tmElements_t*);
//		void			_subtract_time (tmElements_t*);

//		void			_evaluate_i2c_status (error_t* err_ptr, char* device);
//		void			_make_i2c_log_entry (char * device, char *counter_name, uint32_t count);

	public:
//		char			display_text [35];						// text displayed on legacy LCD U/I; do not include the 'd' prefix or the '\r' suffix
		uint16_t		spi_port_pins_result;					// '1' bits in upper byte identify failing signal; matching bit positions in lower byte identify high or low

		uint8_t			fram_fill (uint8_t, uint16_t, size_t);
		void 			fram_get_n_bytes (uint8_t*, size_t);
		void			fram_hex_dump (uint16_t);

		void			fw_reset (uint8_t);
		void			get_reset_source_text (const char**, uint16_t*);
		boolean			get_user_yes_no (char*, char*, boolean);
		void 			software_reset_1 (uint32_t);
		void 			software_reset_b (uint32_t);				// per tom & ray magliozzi
		void			fatal_error_restart (void);
//		void			ui_display_update (uint8_t);

		uint8_t			i2c_port_pins_test (uint8_t);
		uint8_t			spi_port_pins_test (void);
//		void			pod_temp_log (uint8_t);

//		void			get_eip (float* e_ptr, float* i_ptr, float* p_ptr);
//		void			get_temp_fan (float* temp_ptr, float* fan_ptr, boolean cvt_c2f);
//		void 			get_status_bits (uint8_t* stat1_ptr, uint16_t* stat2_ptr, uint8_t* stat3_ptr, uint8_t* stat4_ptr, uint8_t* stat5_ptr);
//		void			pod_power_log (uint8_t);

		void			i2c_log (uint8_t minutes);

		boolean			is_dst (uint16_t, uint8_t, uint8_t, int8_t, boolean);
		boolean			is_valid_date (uint16_t, uint8_t, uint8_t);
//		void			local_to_utc (tmElements_t*);
//		void			utc_to_local (tmElements_t*);


//---------------------------< T A S K   T I M E R S >------------------------------------------------
//
// For tasks that are repeatedly accomplished every n milliseconds, use a task timer to has an entry in the task_timers struct.  The
// interval can be set at any time but is generally set just once in the main setup() function.
//
// These use the magic data type elapsedMillis which increments every millisecond as a baked-in Teensy task.
// @see https://www.pjrc.com/teensy/td_timing_elaspedMillis.html [sic]
//

		struct e_timer								// a struct to hold an individual task's timer and interval
			{
			elapsedMillis	elapsed_time;			// time since last reset
			uint32_t		interval;				// interval (in milliseconds)
			};

		struct task_timers_struct					// a struct to hold all of the task timers
			{
			e_timer		timer_10mS;					//
			e_timer		timer_500mS;				//
			e_timer		timer_1sec;					// RTC
			e_timer		timer_5sec;					//
			e_timer		min_loop_time;				// typically 5mS used to prevent false watchdog resets when WD is serviced too soon
			} task_timers =
				{{0, 10}, {0, 500}, {0, 1000}, {0, 5000}, {0, 5}};			// initialize the timer values

//		struct menu_timers_struct					// more menu-specific timers
//			{
//			e_timer		min_display_time_A;				// 1000mS minimum time that a display must persist
//			e_timer		min_display_time_B;
//			e_timer		min_display_time_between_writes_A;	// 50mS time between writes to LCD display in habitat A
//			e_timer		min_display_time_between_writes_B;	// same for habitat B
//			e_timer		menu_timeout_A;						// 30 sec to wait for user action
//			e_timer		menu_timeout_B;						//
//			} menu_timers =
//				{{0, 1000}, {0, 1000}, {0, 50}, {0, 50}, {0, 30*1000}, {0, 30*1000}};


		boolean		is_task_time (e_timer *);
		void		task_time_reset (e_timer *);
		void		task_time_fastfwd (e_timer *, uint32_t);
	};

extern Systronix_utilities utils;

#endif	// Systronix_utilities_H_
