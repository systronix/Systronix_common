// a handful of things that are handy but don't have a particularly better place to live

#include <Systronix_utilities.h>				// own .h so don't #include it through Systronix_common.h
//#include <Systronix_TMP102.h>
//#include <Systronix_LCM300.h>
//#include <SALT_JX.h>
//#include <SALT_FETs.h>
//#include <SALT_power_FRU.h>

//#include <Systronix_ili9341_helper.h>			// experiment

// Systronix_ili9341_helper display;

//extern mbox_com mailbox_common;		// in SALT_reptile.h


//---------------------------< F W _ R E S E T >--------------------------------------------------------------
//
// firmware reset.
//
// Call this function with a value of 0, 1, 2, or 3:
//		0 - release resets
//		1 - assert PERIF_RST
//		2 - assert ETHER_RST
//		3 - assert both
//
// Duration of the asserted reset signal(s) is determined by the calling function
//
// NOTE: hardware reset of the 9557s puts their I/O pins in input mode.  These devices should be initialized
// as soon as practicable following the release of PERIPH_RST.
//

void Systronix_utilities::fw_reset (uint8_t rst)
	{
	if (rst & RST_P)
		digitalWrite(PERIPH_RST, LOW);		// resets asserted
	if (rst & RST_E)
		digitalWrite(ETHER_RST, LOW);

	if (0 == rst)
		{
		digitalWrite(PERIPH_RST, HIGH);		// resets released
		digitalWrite(ETHER_RST, HIGH);
		}
	}


//---------------------------< S P I _ P I N _ M O B I L I T Y _ T E S T >------------------------------------
//
// Tests the mobility of a spi pin and leaves it pulled up so that all of the chip selects are inactive.
// This function is not to be used with the SCK pin on Teensy P13.  That pin is also the Teensy module's LED
// pin.  The controller's internal pullup is not strong enough to drag P13 above 1.7V so the test will fail.
// A separate test is required for P13/SCK.
//

uint8_t Systronix_utilities::spi_pin_mobility_test (uint8_t pin)
	{
	uint8_t ret_val = SUCCESS;

	pinMode (pin, INPUT_PULLUP);			// set the pin to input with pullup mode
	delayMicroseconds (5);					// allow time for the pullup to drag the line high
	if (!digitalRead (pin))					// read it; should be high because of pullup
		ret_val = FAIL_LOW;

	pinMode(pin, OUTPUT);					// set pin to be an output
	digitalWrite (pin, LOW);				// set the pin low
	if (digitalRead (pin))					// read it; should be low
		ret_val = FAIL_HIGH;
	pinMode (pin, INPUT_PULLUP);			// leave the pin in input with pullup mode

	return ret_val;
	}


//---------------------------< I 2 C _ P O R T _ P I N S _ T E S T >------------------------------------------
//
// This function tests for stuck i2c pins.
// TODO: log failure to uSD card? Is there any sense in logging this to uSD?  At the point in the startup when
// this test is performed, there is no time in the common mailbox so standard logging path and file name will
// be:
//		/logs.0000/1970/nil/log_1970-00-00.txt		// last octet of ip left off because at reset that string is empty
// and that file would hold a log entry that looks something like:
//		1970-00-00T00:00:00, <fatal error message text goes here>
// A new entry will be added to that file every time a fatal error occurs so at best a service person would know
// that at some point in the past, the port pins test failed but not when that happened: today, yesterday, six
// months ago, 2 years ago, ...  Also, if this failure is i2c net 0, that is a fatal error so, in an attempt to
// unstick stuck i2c pins, SALT forces a reset approximately 30 seconds after this test fails which brings us
// back here so we could enter an endless loop of test, fail, reset, test, fail, reset, ...  Not much need to
// log each one of those failures.
//

uint8_t Systronix_utilities::i2c_port_pins_test (uint8_t port)
	{
	uint8_t ret_val = SUCCESS;

	uint8_t		scl_pin;
	uint8_t		sda_pin;

	if (0 == port)
		{
		scl_pin = I2C_SCL_PIN;
		sda_pin = I2C_SDA_PIN;
		}
	else
		{
		scl_pin = I2C_SCL1_PIN;
		sda_pin = I2C_SDA1_PIN;
		}

	pinMode(scl_pin, INPUT);				// set SCL to be an input
	if (!digitalRead (scl_pin))				// read it; should be high because of external pullup
		{
		ret_val = TEST_I2C_SCL;
		Serial.printf ("\nI2C SCL%d pin (P%d) low", port, I2C_SCL_PIN);	// if not, say so
//		Serial.printf ("\nport %d: SCL pin low", port);	// if not, say so
		}

	pinMode(scl_pin, OUTPUT);				// set SCL to be an output
	digitalWrite (scl_pin, LOW);			// set SCL low; do this pin before SDA to avoid start/stop conditions
	if (digitalRead (scl_pin))				// read it; should be low
		{
		ret_val = TEST_I2C_SCL | TEST_I2C_SCL_HL;
		Serial.printf ("\nI2C SCL%d pin (P%d) high", port, I2C_SCL_PIN);
//		Serial.printf ("\nport %d: SCL pin high", port);
		}

	for (uint8_t i=0; i<10; i++)			// send a bunch of clocks in case an i2c operation was interrupted by a reset
		{									// leaving SDA asserted
		digitalWrite (scl_pin, HIGH);
		digitalWrite (scl_pin, LOW);
		}

	pinMode(sda_pin, INPUT);				// set SDA to be an input
	if (!digitalRead (sda_pin))				// read it; should be high because of external pullup
		{
		ret_val = TEST_I2C_SDA;
		Serial.printf ("\nI2C SDA%d pin (P%d) low", port, I2C_SDA_PIN);	// if not, say so
//		Serial.printf ("\r\nport : I2C SDA%d pin low", port);
		}

	pinMode(sda_pin, OUTPUT);				// set SDA to be an output
	digitalWrite (sda_pin, LOW);			// set SDA low
	if (digitalRead (sda_pin))				// read it; should be low
		{
		ret_val = TEST_I2C_SDA | TEST_I2C_SDA_HL;
		Serial.printf ("\nI2C SDA%d pin (P%d) high", port, I2C_SDA_PIN);
		}

	digitalWrite (sda_pin, HIGH);			// this in order to avoid a stop condition
	delayMicroseconds(5);
	digitalWrite (scl_pin, HIGH);

	pinMode(scl_pin, INPUT);				// set SCL to be an input
	pinMode(sda_pin, INPUT);				// set SDA to be an input

	return ret_val;
	}


//---------------------------< S P I _ P O R T _ P I N S _ T E S T >------------------------------------------
//
// This function tests for stuck spi pins.  TODO: what should we do, should this test ever fail?
// TODO: log failure to uSD card (might not be possible) / fram.  This isn't a fatal error but does mean that
// SALT may not be able to access ethernet, uSD, debug display, or flash.  If a chip select is stuck low,
// spi is dead.  If any of miso, mosi, or sck is stuck high or low, spi is dead.  spi does not control any
// environmental hardware so a failure of spi should not harm any animals.  Could log to fram but that log is
// transient so the event will eventually be overwritten.
//

uint8_t Systronix_utilities::spi_port_pins_test (void)
	{
	uint8_t ret_val = SUCCESS;
	uint8_t result;
	spi_port_pins_result = 0;

	result = spi_pin_mobility_test (SPI_CS1_PIN);		// test the flash chip select pin
	if (result)
		{
		spi_port_pins_result |= TEST_SPI_CS1 | (FAIL_LOW == result) ? 0 : TEST_SPI_CS1_HL;
		Serial.printf ("SPI CS1 pin (P%d) %s\n", SPI_CS1_PIN, FAIL_LOW == result ? "low" : "high");
//		Serial.printf ("%s\n", FAIL_LOW == result ? "low" : "high");
		}

	result = spi_pin_mobility_test (SPI_CS2_PIN);		// test the touch panel chip select pin
	if (result)
		{
		spi_port_pins_result |= TEST_SPI_CS2 | (FAIL_LOW == result) ? 0 : TEST_SPI_CS2_HL;
		Serial.printf ("SPI CS2 pin (P%d) %s\n", SPI_CS2_PIN, FAIL_LOW == result ? "low" : "high");
//		Serial.printf ("%s\n", FAIL_LOW == result ? "low" : "high");
		}

	result = spi_pin_mobility_test (SPI_CS3_PIN);		// test the ethernet chip select pin
	if (result)
		{
		spi_port_pins_result |= TEST_SPI_CS3 | (FAIL_LOW == result) ? 0 : TEST_SPI_CS3_HL;
		Serial.printf ("SPI CS3 pin (P%d) %s\n", SPI_CS3_PIN, FAIL_LOW == result ? "low" : "high");
//		Serial.printf ("%s\n", FAIL_LOW == result ? "low" : "high");
		}

	result = spi_pin_mobility_test (SPI_CS4_PIN);		// test the uSD card chip select pin
	if (result)
		{
		spi_port_pins_result |= TEST_SPI_CS4 | (FAIL_LOW == result) ? 0 : TEST_SPI_CS4_HL;
		Serial.printf ("SPI CS4 pin (P%d) %s\n", SPI_CS4_PIN, FAIL_LOW == result ? "low" : "high");
//		Serial.printf ("%s\n", FAIL_LOW == result ? "low" : "high");
		}

	result = spi_pin_mobility_test (SPI_CS5_PIN);		// test the debug display chip select pin
	if (result)
		{
		spi_port_pins_result |= TEST_SPI_CS5 | (FAIL_LOW == result) ? 0 : TEST_SPI_CS5_HL;
		Serial.printf ("SPI CS5 pin (P%d) %s\n", SPI_CS5_PIN, FAIL_LOW == result ? "low" : "high");
//		Serial.printf ("%s\n", FAIL_LOW == result ? "low" : "high");
		}

	result = spi_pin_mobility_test (SPI_MOSI_PIN);			// test the MOSI pin
	if (result)
		{
		spi_port_pins_result |= TEST_SPI_MOSI | (FAIL_LOW == result) ? 0 : TEST_SPI_MOSI_HL;
		Serial.printf ("SPI MOSI pin (P%d) %s\n", SPI_MOSI_PIN, FAIL_LOW == result ? "low" : "high");
//		Serial.println (FAIL_LOW == result ? "low" : "high");
		}

	result = spi_pin_mobility_test (SPI_MISO_PIN);			// test the MISO pin
	if (result)
		{
		spi_port_pins_result |= TEST_SPI_MISO | (FAIL_LOW == result) ? 0 : TEST_SPI_MISO_HL;
		Serial.printf ("SPI MISO pin (P%d) %s\n", SPI_MISO_PIN, FAIL_LOW == result ? "low" : "high");
//		Serial.println (FAIL_LOW == result? "low" : "high");
		}

											// this pin has the Teensy LED on it; in pullup mode, LED pulls the line
											// down to about 1.7V; not enough for a HIGH so it must be driven HIGH
	pinMode(SPI_SCK_PIN, OUTPUT);				// set the pin to be an output
	digitalWrite (SPI_SCK_PIN, HIGH);			// drive the pin high
	if (!digitalRead (SPI_SCK_PIN))				// read it; should be high
		{
		ret_val = FAIL_LOW;
		spi_port_pins_result |= TEST_SPI_SCK;
		Serial.printf ("SPI_SCK pin (P%d) low", SPI_SCK_PIN);		// if not, say so
		}
	digitalWrite (SPI_SCK_PIN, LOW);			// set the pin low
	if (digitalRead (SPI_SCK_PIN))				// read it; should be low
		{
		ret_val = FAIL_HIGH;
		spi_port_pins_result |= TEST_SPI_SCK | TEST_SPI_SCK_HL;
		Serial.printf ("SPI_SCK pin (P%d) high", SPI_SCK_PIN);
		}
	pinMode (SPI_SCK_PIN, INPUT);				// leave the pin in input mode (LED off)

	return ret_val;
	}


//---------------------------< F R A M _ F I L L >------------------------------------------------------------
//
// fill fram with n copies of 'c' beginning at 'address' and continuing for 'n' number of bytes.
// writes in groups of 256 (default buffer size in i2c_t3 library).
//
/*
uint8_t Systronix_utilities::fram_fill (uint8_t c, uint16_t address, size_t n)
	{
	uint8_t	buf[256];							// max length natively supported by the i2c_t3 library
	size_t	i = 0;								// the iterator

	if (FRAM_SIZE < (address + (n-1)))			// if too many bytes fram internal address will wrap
		return FAIL;							// don't do the fill

	memset (buf, c, 256);						// fill buffer with 'c' characters to write to fram

	fram.set_addr16 (address);					// set the address
	fram.control.rd_wr_len = 256;				// number of bytes to write
	fram.control.wr_buf_ptr = buf;				// point to the buffer

	while (n > (i+256))							// if possible, write blocks of 256 bytes
		{
		fram.page_write();						// write 256 bytes
		i+=256;									// tally the number of bytes written
		}

	if (n - i)									// if not zero, there fewer than 256 bytes left to write
		{
		fram.control.rd_wr_len = n-i;			// determine how many and then
		fram.page_write();						// write the remaining bytes
		}
	return SUCCESS;
	}
*/

//---------------------------< S O F T W A R E _ R E S E T _ B >----------------------------------------------
//
// DO NOT CALL THIS FUNCTION UNLESS YOU WANT TO RESET THE CONTROLLER.
//
// This is the second of two functions that are required when a software reset is necessary.  The two functions
// in sequence set the vector key in the ARM Application Interrupt and Reset Control Register.  Call this
// function with SW_RESET_KEY_2 as the argument.  The public control variable aircr MUST already have the
// SW_RESET_KEY_A value when this function is called; any other value will cause the function to abort and set
// aircr to a non-zero value.  Similarly, SW_RESET_KEY_2 must be correct or the function will abort and set
// aircr to a non-zero value.  To recover, aircr must be set to zero externally.
//
/*
void Systronix_utilities::software_reset_b (uint32_t key_2)
	{
	if ((SW_RESET_KEY_A != (aircr & SW_RESET_KEY_A)) ||		// must be 0x05000000
		(SW_RESET_KEY_2 != key_2))							// must be 0x00FA0000
			{
			logs.log_event ((char*)"failed sw reset attempt: reset b");	// log the event
			aircr = 0xFFFFFFFF;								// reset
			return;											// and quit
			}

	aircr |= key_2;											// set the rest of the unlock key
	aircr |= SYSRESETREQ;									// set the SYSRESETREQ bit
	SCB_AIRCR = aircr;										// write to the real register
	while (1);												// hang here until reset
	}
 */

//---------------------------< S O F T W A R E _ R E S E T _ 1 >----------------------------------------------
//
// DO NOT CALL THIS FUNCTION UNLESS YOU WANT TO RESET THE CONTROLLER.
//
// This is the first of two functions that are required when a software reset is necessary.  The two functions
// in sequence set the vector key in the ARM Application Interrupt and Rest Control Register.  Call this
// function with SW_RESET_KEY_A as the argument.  The public control variable aircr MUST be zero when this
// function is called; any other value will cause the function to abort and set aircr to a non-zero value.  To
// recover, aircr must be set to zero externally.
//
/*
void Systronix_utilities::software_reset_1 (uint32_t key_A)
	{
	if (aircr)											// must be zero on entry
		{
		logs.log_event ((char*)"failed sw reset attempt: reset 1");	// log the event
		aircr = 0xFFFFFFFF;								// reset
		return;											// and quit
		}

	aircr = SCB_AIRCR;									// get the register content
	aircr &= 0x0000FFFF;								// blank the vector key
	aircr |= key_A;										// set the upper byte of the unlock key
	}
 */

//---------------------------< G E T _ R E S E T _ S O U R C E _ T E X T >------------------------------------
//
// Provides a pointer to a string indicating the reset source and the number of that kind of reset in the args
// of the call.  For now, only the big five: POR, reset pin, watchdog, low voltage, and software are supported.
// Returns 'unknown' for any other reset source.
//
// source_text_ptr is the address of a pointer in the calling source so it is a pointer to a pointer.
//
// This function maintains a count of certain reset types in fram: external, watchdog, and other (power on,
// low voltage, and the unknown or undefined.
//
// Reset sources not listed here are:
//	loss of lock - multipurpose clock generator pll loss of lock
//	loss of clock - external clock missing
//	wakeup - SALT never sleeps
//	stop mode acknowledge - SALT does not stop
//	ExPort - SALT does not use ExPort
//	MDM-AP system reset request - SALT does not support a system debugger
//	lockup - ARM core lockup
//	JTAG - SALT does not use JTAG
//
/*
void Systronix_utilities::get_reset_source_text (const char** source_text_ptr, uint16_t* count_ptr)
	{
	uint16_t	reset_counter_addr;						// fram reset counter address

	if (RCM_SRS1 & RCM_SRS1_SW)
		{
		reset_counter_addr = OTHER_RESET_COUNTER;
		*source_text_ptr = reset_source_string[0];		// software reset
		}
	else if (RCM_SRS0 & RCM_SRS0_POR)
		{
		reset_counter_addr = OTHER_RESET_COUNTER;
		*source_text_ptr = reset_source_string[1];		// power on
		}
	else if (RCM_SRS0 & RCM_SRS0_PIN)
		{
		reset_counter_addr = EXT_RESET_COUNTER;
		*source_text_ptr = reset_source_string[2];		// external reset
		}
	else if (RCM_SRS0 & RCM_SRS0_WDOG)
		{
		reset_counter_addr = WDOG_RESET_COUNTER;
		*source_text_ptr = reset_source_string[3];		// watchdog
		}
	else if (RCM_SRS0 & RCM_SRS0_LVD)
		{
		reset_counter_addr = OTHER_RESET_COUNTER;
		*source_text_ptr = reset_source_string[4];		// low voltage
		}
	else
		{
		reset_counter_addr = OTHER_RESET_COUNTER;
		*source_text_ptr = reset_source_string[5];		// unknown or known but not yet defined here
		}

	fram.set_addr16 (reset_counter_addr);				// point to counter in fram
	fram.int16_read();									// get counter value
	if (0xFFFF > fram.control.rd_int16)					// limit count to 65535
		fram.control.wr_int16 = fram.control.rd_int16 + 1;	// bump the count
	fram.set_addr16 (reset_counter_addr);				// reset the address
	fram.int16_write();									// write the new value
//	Serial.printf ("Reset count: %s: %d\n", *source_text_ptr, fram.control.wr_int16);
	*count_ptr = fram.control.wr_int16;
	}
 */

//---------------------------< G E T _ U S E R _ Y E S _ N O >------------------------------------------------
//
// FOR DUBUG PURPOSES.  solicits yes/no input from a user at the monitor
//

boolean Systronix_utilities::get_user_yes_no (char* prompt, char* query, boolean yesno)
	{
	char c;
	Serial.print ("\r\nSALT/");
	Serial.print (prompt);
	Serial.print ("> ");
	Serial.print (query);
	if (yesno)
		Serial.print (" ([y]/n) ");
	else
		Serial.print (" (y/[n]) ");

	while (1)
		{
		if (Serial.available())
			{
			c = Serial.read();
			if ((('\x0d' == c) && !yesno) || ('n' == c) || ('N' == c))
				{
				Serial.println ("no");
				return false;
				}
			else if ((('\x0d' == c) && yesno) || ('y' == c) || ('Y' == c))
				{
				Serial.println ("yes");
				return true;
				}
			}
		}
	}


//---------------------------< F R A M _ G E T _ N _ B Y T E S >----------------------------------------------
//
// call this after setting the appropriate values in the fram control struct.  This function reads n number
// of characters into a buffer.
//
// THIS FUNCTION DOES NOT LIMIT THE NUMBER OF BYTES READ.  It is possible to set n to a value larger than the
// fram size so that this internal address counter wraps to zero.  Setting the size of n is the obligation of
// the calling function.
//
/*
void Systronix_utilities::fram_get_n_bytes (uint8_t* buf_ptr, size_t n)
	{
	size_t i;

	fram.byte_read ();						// get the first byte
	*buf_ptr++ = fram.control.rd_byte;		// save the byte we read
	for (i=0; i<(n-1); i++)					// loop getting the rest of the bytes
		{
		fram.current_address_read ();		// get next byte
		*buf_ptr++ = fram.control.rd_byte;	// save the byte we read
		};
	}
 */

//---------------------------< H E X _ D U M P _ C O R E >----------------------------------------------------
//
// Render a 'page' (16 rows of 16 columns) of hex data and its character equivalent.  data_ptr MUST point to
// a buffer of at least 256 bytes.  This function always renders 256 items regardless of the size of the source.
// TODO: add support for 32-bit addressing

void Systronix_utilities::hex_dump_core (uint16_t address, uint8_t* data_ptr)
	{
	uint8_t*	text_ptr = data_ptr;					// walks in tandem with data_ptr; prints the text

	for (uint8_t i=0; i<16; i++)						// dump 'pages' that are 16 lines of 16 bytes
		{
		Serial.printf ("\n%.4X    ", address);

		for (uint8_t j=0; j<8; j++)						// first half
			Serial.printf ("%.2x ", *data_ptr++);
		Serial.print (" -  ");							// mid array separator

		for (uint8_t j=0; j<8; j++)						// second half
			Serial.printf ("%.2x ", *data_ptr++);
		Serial.print ("   ");							// space between hex and character data

		for (uint8_t j=0; j<16; j++)
			{
			if ((' ' > *text_ptr) || ('~' < *text_ptr))						// if a control character (0x00-0x1F)
				Serial.print ('.');						// print a dot
			else
				Serial.print ((char)*text_ptr);
			text_ptr++;									// bump the pointer
			}

		address += 16;
		address &= 0x7FFF;								// limit to 32k
		}
	}


//---------------------------< F R A M _ H E X _ D U M P >----------------------------------------------------
//
// Dumps 256 byte 'pages' of fram to the monitor. Used by the ini loaders but can be added as a debug function
// when necessary.
//
/*
void Systronix_utilities::fram_hex_dump (uint16_t start_address)
	{
	uint8_t		buf [256];

	fram.control.rd_buf_ptr = buf;
	fram.control.rd_wr_len = 256;

	while (1)
		{
		fram.set_addr16 (start_address);						// first address to dump
		fram.page_read ();										// get a page
		hex_dump_core (start_address, buf);

		Serial.println ("");									// insert a blank line
		if (!get_user_yes_no ((char*)"loader", (char*)"another page?", true))	// default answer yes
			return;
		start_address += 256;									// next 'page' starting address
		start_address &= 0x7FFF;								// limit to 32k
		}
	}
 */

//---------------------------< _ M A K E _ I 2 C _ L O G _ E N T R Y >----------------------------------------
//
// creates log entry string and then call log_event() to do the logging
//
/*
void Systronix_utilities::_make_i2c_log_entry (char * device, char *counter_name, uint32_t count)
	{
	char	log_msg[64];

	sprintf (log_msg, "%s: %s: %lu", device, counter_name, count);
//	Serial.printf ("\t%s\n", log_msg);
	logs.log_event (log_msg);
	}
*/

//---------------------------< _ E V A L U A T E _ I 2 C _ S T A T U S >--------------------------------------
//
// this function polls all of the error counters in an i2c library's error struct. For each non-zero error count
// calls _make_i2c_log_entry() to make a log entry.
//
/*
void Systronix_utilities::_evaluate_i2c_status (error_t* err_ptr, char* device)
	{
//	if (err_ptr->successful_count)			// for debugging so that we have something to log
//		_make_i2c_log_entry (device, (char*)"successful count", err_ptr->successful_count);


	if (err_ptr->incomplete_write_count)
		_make_i2c_log_entry (device, (char*)"incomplete write count", err_ptr->incomplete_write_count);
	if (err_ptr->data_len_error_count)
		_make_i2c_log_entry (device, (char*)"data len error count", err_ptr->data_len_error_count);
	if (err_ptr->timeout_count)
		_make_i2c_log_entry (device, (char*)"timeout count", err_ptr->timeout_count);
	if (err_ptr->rcv_addr_nack_count)
		_make_i2c_log_entry (device, (char*)"rcv addr nack count", err_ptr->rcv_addr_nack_count);
	if (err_ptr->rcv_data_nack_count)
		_make_i2c_log_entry (device, (char*)"rcv data nack count", err_ptr->rcv_data_nack_count);
	if (err_ptr->arbitration_lost_count)
		_make_i2c_log_entry (device, (char*)"arbitration lost count", err_ptr->arbitration_lost_count);
	if (err_ptr->buffer_overflow_count)
		_make_i2c_log_entry (device, (char*)"buffer overflow count", err_ptr->buffer_overflow_count);
	if (err_ptr->other_error_count)
		_make_i2c_log_entry (device, (char*)"other error count", err_ptr->other_error_count);
	if (err_ptr->unknown_error_count)
		_make_i2c_log_entry (device, (char*)"unknown error count", err_ptr->unknown_error_count);
	if (err_ptr->data_value_error_count)
		_make_i2c_log_entry (device, (char*)"data value error count", err_ptr->data_value_error_count);
	if (err_ptr->silly_programmer_error)
		_make_i2c_log_entry (device, (char*)"silly programmer error", err_ptr->silly_programmer_error);
	}
*/

//---------------------------< I 2 C _ L O G >----------------------------------------------------------------
//
// monitors minutes value of the mailbox tm struct.  At 5 minutes after the hour, queries the error structs in
// various i2c libraries looking for non-zero error.total_error_count; logs the non-zero counters from that
// library's error struct.
//
/*
struct i2c_error_list_t
	{
	error_t*	error_ptr;				// pointer to the i2c library's error struct
	char*		device;					// text string that names the library's device
	} i2c_error_list[] ={
		{&lcm300.error, (char*)"lcm300"},
		{&tmp102.error, (char*)"tmp102"},
		{&fram.error, (char*)"fram"},
		{NULL, (char*)"J2"},			// done this way because these values not known at compile time
		{NULL, (char*)"J3"},
		{NULL, (char*)"J4"},
		{NULL, (char*)"fet"},
		{NULL, (char*)"fru"},
		};

void Systronix_utilities::i2c_log (uint8_t minutes)
	{
	static	boolean logged=false;

	if (logged && 5 == minutes)
		return;												// already logged this hour's readings

	if (logged)
		{
		logged = false;										// reset the flag
		return;												// not on the hour so done
		}

	if (5 != minutes)
		return;												// not 5 minutes after the hour so done

	i2c_error_list[3].error_ptr = coreJ2.error_ptr;			// load the i2c_error_list array; done this way because
	i2c_error_list[4].error_ptr = coreJ3.error_ptr;			// these values not known at compile time
	i2c_error_list[5].error_ptr = coreJ4.error_ptr;
	i2c_error_list[6].error_ptr = FETs.error_ptr;
	i2c_error_list[7].error_ptr = fru.error_ptr;

	for (uint8_t i=0; i<(sizeof(i2c_error_list)/sizeof(i2c_error_list_t)); i++)
		{
		if (!i2c_error_list[i].error_ptr->total_error_count)
			_evaluate_i2c_status (i2c_error_list[i].error_ptr, i2c_error_list[i].device);
		}

	logged = true;
	}
*/

//---------------------------< I S _ T A S K _ T I M E >------------------------------------------------------
//
// This function queries the state of a task's elapsed timer and compares it to the the task's interval setting.
// For the time that a task's interval time has not yet elapsed, the function returns false.  When the interval
// time has elapsed, the function resets the timer and returns true.
//
// to call this function, pass it a pointer to the task's e_timer_struct.
//
// return true and reset the timer if interval has elapsed, e.g. task time has expired, else return false

boolean Systronix_utilities::is_task_time (e_timer *et_ptr)
	{
	if (et_ptr->elapsed_time >= et_ptr->interval)	// has the time since the last reset passed?
		{
		et_ptr->elapsed_time = 0;					// yes, reset the timer
		return true;								// and return true
		}
	return false;									// no, return false
	}


//---------------------------< T A S K _ T I M E _ R E S E T >------------------------------------------------
//
// by reset this means set the elapsed time to zero, so it will be full interval before @is_task_time() returns true
//

void Systronix_utilities::task_time_reset (e_timer *et_ptr)
	{
	et_ptr->elapsed_time = 0;					// reset the timer
	}


//---------------------------< T A S K _ T I M E _ F A S T F W D >--------------------------------------------
//
// Fast Forward task timer so that @is_task_time() will return true on next query
// Do this by forcing elapsed time = interval
// This can be any task timer including the LCD min display timer
// To be overly cautious about this, fast fwd interval - min LCD interval so we don't overrun it...
// ... never fast fwd a negative amount and and at most the interval
// Usually the interval will be much larger than min_task_time or we would not need to fast fwd
// @ TODO make sure this can't be negative or we will wait a VERY long time
//

void Systronix_utilities::task_time_fastfwd (e_timer *et_ptr, uint32_t min_task_time)
	{
	uint32_t et = et_ptr->elapsed_time;		// get current value in case it increments while we are testing it

	if (min_task_time > et_ptr->interval)
		{
		min_task_time = et_ptr->interval;
		}

	et_ptr->elapsed_time = (et > min_task_time ? et_ptr->interval : et + (et_ptr->interval - min_task_time));
	}


//---------------------------< _ D A Y _ O F _ W E E K >------------------------------------------------------
//
// This code is adapted from similar code written by Martin Minow while he was at Digital Equipment Corporation.
// His code, called Today, converted system date and time to a readable format of the date and time as
// WGBH Morning Pro Musica host Robert J Lurtsema spoke it on air.
// Year, 1978 = 1978
// Month, January = 1
// Day of month, 1 = 1
//
// Returns the day of the week on which this date falls: Sunday = 0. Note, this routine is valid only for the
// Gregorian calendar.
//

uint8_t Systronix_utilities::_day_of_week(uint16_t year, uint8_t month, uint8_t day)
	{
	uint16_t yearfactor;

	yearfactor = year + (month - 14)/12;
	return (uint8_t)(( (13 * (month + 10 - (month + 10)/13*12) - 1)/5
		+ day + 77 + 5 * (yearfactor % 100)/4
		+ yearfactor / 400
		- yearfactor / 100 * 2) % 7);
	}


//---------------------------< I S _ D S T >----------------------------------------------------------------
//
// DST begins begins at 0200 local time on the second Sunday of March (as early as the 8th, as late as the 14th)
// DST ends at 0200 on the first Sunday of November (as early as the 1st, as late as the 7th).
//
// How to determine if we're in dst?
// Figure out the day of the week for 01 March of specified year using _day_of_week().  From that find second Sunday:
//
// 01 March falls on: then date of second Sunday is:
// 		Sunday => 8
// 		Monday => 14
// 		Tuesday => 13
// 		Wednesday => 12
// 		Thursday => 11
// 		Friday => 10
// 		Saturday => 9
//
// 01 November falls on: then date of first Sunday is:
// 		Sunday => 1
// 		Monday => 7
// 		Tuesday => 6
// 		Wednesday => 5
// 		Thursday => 4
// 		Friday => 3
// 		Saturday => 2
//
// So, November dates are always 7 days less than the March dates for the first of the month falling on
// the same day of the week.
//
// Function returns true if date and time is in DST time.
//
// In this application, this function is called by utc_to_local() (most common because it is done every second)
// and local_to_utc() (least common because it is done only when setting date/time).
//
// When called by _utc_to_local(), the time is adjusted to local standard time so this function determines if
// the local time should be further adjusted for dst.  This function compares local standard date/time against
// dst starting date at 0200 and against dst ending date at 0100 (dst ends at 0100 local standard time).  This
// function returns true when the local standard time SHOULD be adjusted to daylight saving time.
//
// When called by local_to_utc(), a user has entered a new local date/time which is assumed to be dst adjusted
// when dst is in effect.  This function then determines if it is necessary to remove the dst adjustment from
// the local time by comparing local date/time to dst start date at 0200 (02:00:00 to 02:59:59 do not exist on
// dst start date because time changes 01:59:59, 03:00:00 so any time greater than 0200 is declared to have been
// adjusted to dst).  dst ends at 0200 local daylight saving time so for this comparison, the function compares
// the local time against the end date at 0200.  This function returns true when local time MUST be adjusted to
// standard time in the conversion to utc time.

// NOTE: for the local_to_utc() conversion, any time 02:00:00 to 02:59:59 on dst start date is an error that we
// don't catch.  We presume that it is unlikely that user's will be setting the habitat clocks at that time of
// the morning.


boolean Systronix_utilities::is_dst (uint16_t year, uint8_t month, uint8_t day, int8_t hour, boolean has_adjustment)
	{
	uint8_t dst_start [] = {8, 14, 13, 12, 11, 10, 9};	// Sunday, Monday, ..., Saturday
	uint8_t date_of_dst_start, date_of_dst_end;

	date_of_dst_start = dst_start [_day_of_week (year, 3, 0x01)];	// figure dst start date
	date_of_dst_end = dst_start [_day_of_week (year, 11, 0x01)] - 7;	// figure dst end date

	if (3 == month)					// march
		{
		if (day < date_of_dst_start)
			return false;			// isn't yet date of dst start

		if (day > date_of_dst_start)
			return true;			// dst has already started

		if ((day == date_of_dst_start) && (hour >= 2))
			return true;			// dst starts today and has already started

		return false;				// dst starts today but hasn't yet started (00:00:00 -> 01:59:59)
		}

	if (11 == month)				// november
		{
		if (day < date_of_dst_end)
			return true;			// isn't yet date of dst end

		if (day > date_of_dst_end)
			return false;			// dst has already ended

		if ((day == date_of_dst_end) && (hour >= (has_adjustment ? 2 : 1)))	// time/date comparison is in local time with or without adjustment
			return false;			// dst ends today and has already ended

		return true;				// dst ends today but hasn't yet ended  (00:00:00 -> 00:59:59 standard time)
		}

	if ((month > 3) && (month < 11))
		return true;				// in DST

//	if ((month < 3) || (month > 11))
		return false;				// in standard time
	}


//----------------------------< _ I S _ L E A P _ Y E A R >---------------------------------------------------
//
// Generally years evenly divisible by 4 are leap years.  But, years evenly divisible by 100 are not leap years
// unless those years are also evenly divisible by 400.  1900 was not a leap year but 2000 was.
//

boolean Systronix_utilities::_is_leap_year (uint16_t year)
	{
	return (!(year % 4) && (year % 100 || !(year % 400))) ? true : false;
	}


//---------------------------< I S _ V A L I D _ D A T E >----------------------------------------------------
//
// return true when the combination of year, month, and day is a valid date: 30 Feb 2016 is not valid
//

boolean Systronix_utilities::is_valid_date (uint16_t year, uint8_t month, uint8_t day)
	{
	if (2 == month)
		{
		if (_is_leap_year (year))
			return (29 < day) ? false : true;
		else
			return (28 < day) ? false : true;
		}
	return (day <= days_in_month[month]) ? true : false;
	}


//---------------------------< _ A D D _ T I M E >------------------------------------------------------------
//
// adjusts date/time after addition in case hours is greater than or equal to 24 hours
//
/*
void Systronix_utilities::_add_time (tmElements_t* tm_ptr)
	{
	if (24 > tm_ptr->Hour)
		return;								// done; nothing to do

	tm_ptr->Hour -= 24;
	tm_ptr->Day++;							// bump to the next day

	if ((2 == tm_ptr->Month) && _is_leap_year (tm_ptr->Year))
		{
		if (29 < tm_ptr->Day)
			{
			tm_ptr->Day = 1;
			tm_ptr->Month = 3;
			return;										// nothing more to do
			}
		}
	else if (days_in_month[tm_ptr->Month] < tm_ptr->Day)
		{
		tm_ptr->Day = 1;
		tm_ptr->Month++;
		}
	if (12 < tm_ptr->Month)
		{
		tm_ptr->Month = 1;
		tm_ptr->Year++;
		}
	}
*/

//---------------------------< _ S U B T R A C T _ T I M E >--------------------------------------------------
//
// adjusts date/time after subtraction in case hours is negative
//
/*
void Systronix_utilities::_subtract_time (tmElements_t* tm_ptr)
	{
	if (0 <= (int8_t)tm_ptr->Hour)
		return;									// done; nothing else to do

	tm_ptr->Hour += 24;							// hours negative so add 24 hours
	tm_ptr->Day--;								// back to the previous day

	if (0 >= (int8_t)tm_ptr->Day)
		{
		tm_ptr->Month--;
		if (0 == tm_ptr->Month)					// from January to December
			{
			tm_ptr->Day = 31;					// back to last day
			tm_ptr->Month = 12;					// of the last month
			tm_ptr->Year--;						// of the previous year
			}
		else if ((2 == tm_ptr->Month) && _is_leap_year (tm_ptr->Year))
			tm_ptr->Day = 29;					// last day in a leap year
		else
			tm_ptr->Day = days_in_month[tm_ptr->Month];	// last day of any month
		}
	}
*/

//---------------------------< L O C A L _ T O _ U T C >------------------------------------------------------
//
// Convert local time to UTC by accounting for time zone and daylight saving time.  Propagate time though day
// month, and year as necessary.
//
/*
void Systronix_utilities::local_to_utc (tmElements_t* tm_ptr)
	{
	boolean	local_dst = is_dst (tm_ptr->Year+1970, tm_ptr->Month, tm_ptr->Day, (int8_t)tm_ptr->Hour, true);	// use local time to figure this

	if (settings.sys_settings.dst && local_dst)	// if dst observed and in effect for LOCAL specified date/time
		{
		tm_ptr->Hour--;							// dst adjustment to standard time
		_subtract_time(tm_ptr);					// in case hours went negative
		}

	tm_ptr->Hour -= settings.sys_settings.tz;	// calculate UTC hour; in this application, tz is always negative so this is an addition
	_add_time (tm_ptr);							// adjust utc date/time in case hour exceeds 24
	}
*/

//---------------------------< U T C _ T O _ L O C A L >------------------------------------------------------
//
// Convert UTC to local time by accounting for time zone and daylight saving time.  Propagate time though day
// month, and year as necessary.
//
/*
void Systronix_utilities::utc_to_local (tmElements_t* tm_ptr)
	{
	tm_ptr->Hour += settings.sys_settings.tz;	// calculate local hour; in this application, tz is always negative
	_subtract_time (tm_ptr);						// adjust date/time in case hours are negative

	if (settings.sys_settings.dst)
		{
		if (is_dst (tm_ptr->Year+1970, tm_ptr->Month, tm_ptr->Day, (int8_t)tm_ptr->Hour, false))	// if dst observed and in effect for specified date/time
			{
			tm_ptr->Hour++;						// dst adjustment
			_add_time (tm_ptr);
			}
		}

	_subtract_time (tm_ptr);						// adjust date/time in case hours are negative because tz in this application is always negative
	}
*/

//---------------------------< F A T A L _ E R R O R _ R E S T A R T >----------------------------------------
//
// Call this function to do a software restart following a fatal error.  This function waits for 30 seconds
// before it pulls the trigger to give time for a message to be displayed.  The message is the responsibility
// of the calling function.
//

void Systronix_utilities::fatal_error_restart ()
	{
	delay (30000);
	software_reset_1 (SW_RESET_KEY_A);
	software_reset_b (SW_RESET_KEY_2);
	}


//---------------------------< U I _ D I S P L A Y _ U P D A T E >--------------------------------------------
//
// Single point through which ALL lcd writes are to happen.  This function can also crudely display all
// habitat A U/I displays when a debug display module is connected to SALT P3.
//
/*
void Systronix_utilities::ui_display_update (uint8_t habitat)
	{
//	static char last_display [33];		// experiment

	if (HABITAT_A == habitat)
		{
		while (!utils.is_task_time (&utils.menu_timers.min_display_time_between_writes_A));	// wait until the timer has expired before next write (50mS)
		Serial1.printf ("d%s\r", display_text);
		Serial1.flush ();
//		if (strcmp(display_text, last_display))
//			{
//			strcpy (last_display, display_text);
//			display.display_line_write (display_text, ILI9341_CYAN);
//			}
		utils.task_time_reset (&utils.menu_timers.min_display_time_between_writes_A);		// reset just as a precaution?
		}
	else if (HABITAT_B == habitat)
		{
		while (!utils.is_task_time (&utils.menu_timers.min_display_time_between_writes_B));	// wait until the timer has expired before next write (50mS)
		Serial2.printf ("d%s\r", display_text);
		Serial2.flush ();
		utils.task_time_reset (&utils.menu_timers.min_display_time_between_writes_B);		// reset just as a precaution?
		}
	};
*/

