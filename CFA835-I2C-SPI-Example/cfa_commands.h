/* ===========================================================================

 Crystalfontz CFA835 module I2C / SPI communications example.
 This example is designed to run on a Seeeduino v4.2 development board.
 It should also run on an Arduino Uno with minimal (or no) changes.

 This example may also work with other Crsytalfontz Intelligent packet based
 displays that use an I2C / SPI interface, but it has not been tested as yet.

 Mark Williams (2020)
 Distributed under the "The Unlicense".
 http://unlicense.org
 This is free and unencumbered software released into the public domain.
 For more details, see the website above.

=========================================================================== */

// Crystalfontz America Inc (Mark Williams)
#ifndef CFPKT_COMMANDS_H_
#define CFPKT_COMMANDS_H_

#define CFAALL

///////////////////////////////////////////////////////////////////////
//UNIVERSAL (CFTEST/CC2/FW835)

//debugging use: set PACKET_EXT_LENGTH for packet length range testing
#ifndef PACKET_EXT_LENGTH
# define PACKET_EXT_LENGTH 0
#else
# warning PACKET_EXT_LENGTH is defined!!
#endif

#define PACKET_HEADER_SIZE          4 /*command,length,crc,data*/

#define PACKET_CFA533_DATA_SIZE		(22 +PACKET_EXT_LENGTH)
#define PACKET_CFA631_DATA_SIZE		(22 +PACKET_EXT_LENGTH)
#define PACKET_CFA633_DATA_SIZE		(22 +PACKET_EXT_LENGTH)
#define PACKET_CFA635_DATA_SIZE		(22 +PACKET_EXT_LENGTH)
#define PACKET_CFA735_DATA_SIZE		(124 +PACKET_EXT_LENGTH)
#define PACKET_CFA835_DATA_SIZE		(124 +PACKET_EXT_LENGTH)
#define PACKET_MAX_DATA_SIZE        PACKET_CFA835_DATA_SIZE
#define PACKET_MAX_SIZE(d)			(PACKET_HEADER_SIZE+(d))

///////////////////////////////////////////////////////////////////////
// CFA533 through to CFA735 commands

#if defined(CFAALL) || defined(CFA533) || defined(CFA631) || defined(CFA633) || defined(CFA635) || defined(CFA735)
typedef enum
{
	PCMD1_INDEX_START							= 0,

	PCMD1_PING									= 0,
	PCMD1_GET_VER								= 1,
	PCMD1_WRITE_USER_FLASH						= 2,
	PCMD1_READ_USER_FLASH						= 3,
	PCMD1_STORE_BOOT_STATE						= 4,
	PCMD1_REBOOT								= 5,
	PCMD1_CLEAR_LCD								= 6,
#if defined(CFAALL) || defined(CFA633) || defined(CFA533)
	PCMD1_PRINT_LINE1							= 7,
	PCMD1_PRINT_LINE2							= 8,
#endif
	PCMD1_SET_LCD_SPECIAL_CHAR_DATA				= 9,
	PCMD1_READ_8_BYTES_LCD_MEMORY				= 10,
	PCMD1_SET_LCD_CURSOR_POSITION				= 11,
	PCMD1_SET_LCD_CURSOR_STYLE					= 12,
	PCMD1_SET_LCD_CONTRAST						= 13,
	PCMD1_SET_LCD_AND_KEYPAD_BACKLIGHT			= 14,
	PCMD1_READ_FBSCAB_INFORMATION				= 15,
	PCMD1_SET_FAN_REPORTING						= 16,
	PCMD1_SET_FAN_POWER							= 17,
	PCMD1_READ_DOW_DEVICE_INFORMATION			= 18,
	PCMD1_SET_TEMPERATURE_REPORTING				= 19,
#if defined(CFAALL) || defined(CFA533) || defined(CFA635) || defined(CFA633)
	PCMD1_ARBITRARY_DOW_TRANSACTION				= 20,
#endif
#if defined(CFAALL) || defined(CFA635) || defined(CFA533)
	PCMD1_LIVE_DATA_DISPLAY						= 21,
#endif
	PCMD1_SEND_COMMAND_TO_CFACONTROLLER			= 22,
	PCMD1_SET_KEY_REPORTING						= 23,
	PCMD1_READ_KEYPAD_STATE						= 24,
	PCMD1_SET_FAN_POWER_FAIL_SAFE				= 25,
	PCMD1_FAN_TACH_GLITCH_FILTER				= 26,
	PCMD1_READ_FAN_POWER_FAIL_SAFE_MASK			= 27,
	PCMD1_SET_ATX_POWER_SWITCH_FUNCTIONALITY	= 28,
	PCMD1_ENABLE_DISABLE_RESET_WATCHDOG			= 29,
	PCMD1_READ_REPORTING_AND_STATUS				= 30,
	PCMD1_LCD_PRINT								= 31,
#if defined(CFAALL) || defined(CFA631)
	PCMD1_KEY_LEGENDS							= 32,
#endif
	PCMD1_CONFIGURE_INTERFACE					= 33,
	PCMD1_CONFIGURE_GPIO_PIN					= 34,
	PCMD1_READ_GPIO_PIN_AND_CONFIGURATION		= 35,

	PCMD1_INDEX_END
} PacketCommands_t;
#endif

///////////////////////////////////////////////////////////////////////
// CFA835 commands

#if defined(CFAALL) || defined(CFA835)
typedef enum
{
	PCMD2_INDEX_START							= 0,

	PCMD2_PING									= 0,
	PCMD2_GET_VER								= 1,
	PCMD2_WRITE_USER_FLASH						= 2,
	PCMD2_READ_USER_FLASH						= 3,
	PCMD2_STORE_BOOT_STATE						= 4,
	PCMD2_REBOOT								= 5,
	PCMD2_CLEAR_LCD								= 6,
	PCMD2_SET_LCD_SPECIAL_CHAR_DATA				= 9,
	PCMD2_LCD_CURSOR_POSITION					= 11,
	PCMD2_LCD_CURSOR_STYLE						= 12,
	PCMD2_LCD_CONTRAST							= 13,
	PCMD2_LCD_AND_KEYPAD_BACKLIGHT				= 14,
	PCMD2_LIVE_DATA_DISPLAY						= 21,
	PCMD2_KEY_REPORTING							= 23,
	PCMD2_READ_KEYPAD_STATE						= 24,
	PCMD2_ATX_POWER_CONTROL						= 28,
	PCMD2_RESET_WATCHDOG						= 29,
	PCMD2_LCD_WRITE								= 31,
	PCMD2_LCD_READ								= 32,
	PCMD2_CONFIGURE_INTERFACE					= 33,
	PCMD2_GPIO_PIN								= 34,
	PCMD2_INTERFACE_BRIDGE						= 36,
	PCMD2_FBSCAB_PORTAL							= 37,
	PCMD2_FONT_PORTAL							= 38,
	PCMD2_FILE_PORTAL							= 39,
	PCMD2_GFX_PORTAL							= 40,
	PCMD2_VIDEO_PORTAL							= 41,

	PCMD2_DEBUG									= 0x3E, /* 62 */

	PCMD2_INDEX_END
} PacketCommands2_t;

typedef enum
{
	SCMD_FBSCAB_QUERY			= 0,
	SCMD_FBSCAB_FAN				= 1,
	SCMD_FBSCAB_FAN_RPM			= 2,
	SCMD_FBSCAB_DOW_QUERY		= 3,
	SCMD_FBSCAB_TEMP_VALUE		= 4,
	SCMD_FBSCAB_GPIO			= 5,
	SCMD_FBSCAB_RESET			= 6,
	SCMD_FBSCAB_LIVE_DISPLAY	= 7,
	SCMD_FBSCAB_AUTO_FAN_CONT	= 8
} SubCommand_FBSCAB_t;

typedef enum
{
	SCMD_FONT_LOAD		= 0,
	SCMD_FONT_PRINT		= 1
} SubCommand_Font_t;

typedef enum
{
	SCMD_FILE_FILE		= 0,
	SCMD_FILE_SEEK		= 1,
	SCMD_FILE_READ		= 2,
	SCMD_FILE_WRITE		= 3,
	SCMD_FILE_DELETE	= 4
} SubCommand_File_t;

typedef enum
{
	SCMD_GFX_OPTIONS	= 0,
	SCMD_GFX_FLUSH		= 1,
	SCMD_GFX_SEND_IMG	= 2,
	SCMD_GFX_LOAD_IMG	= 3,
	SCMD_GFX_SCREENSHOT	= 4,
	SCMD_GFX_PIXEL		= 5,
	SCMD_GFX_LINE		= 6,
	SCMD_GFX_RECT		= 7,
	SCMD_GFX_CIRC		= 8,
	SCMD_GFX_RRECT		= 9,
	SCMD_GFX_CRAW_LOAD		= 10,
	SCMD_GFX_CRAW_DISPLAY	= 11,

#ifdef LCD_DEBUG_FULL_LCD_WRITE
	SCMD_GFX_TIMG		= 0xF0
#endif
} SubCommand_GFX_t;

typedef enum
{
	SCMD_VIDEO_LOAD		= 0,
	SCMD_VIDEO_CONTROL	= 1
} SubCommand_Video_t;

#endif

///////////////////////////////////////////////////////////////////////
// Common

typedef enum
{
	PCMD_REPLY									= 0x40,
	PCMD_REPORT									= 0x80,
	PCMD_ERROR									= 0xC0
} PacketTypes_t;

typedef enum
{
	PRPT_INDEX_START							= PCMD_REPORT,
	PRPT_KEY									= PCMD_REPORT + 0,
	PRPT_FAN									= PCMD_REPORT + 1,
	PRPT_TEMP									= PCMD_REPORT + 2,
#if defined(CFAALL) || defined(CFA835)
	PRPT_DEBUG									= PCMD_REPORT + PCMD2_DEBUG,
#endif
	PRPT_INDEX_END
} PacketReports_t;

///////////////////////////////////////////////////////////////////////////////

#endif
