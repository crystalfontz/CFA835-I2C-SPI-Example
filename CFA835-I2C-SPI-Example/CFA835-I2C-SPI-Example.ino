
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

#include <Arduino.h>
#include "fifo.h"
#include "cfa_commands.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////

//PROJECT SETTINGS

//operation mode (select one)
#define MODE_STANDALONE
//#define MODE_HOST_BRIDGE

// MODE_STANDALONE = example data packets are sent/recieved by the Seeeduino
//   debug text is set to the host PC via the USB/serial connection.

// MODE_HOST_BRIDGE = data is relayed to/from the host PC using the Seeeduinio USB connection
//   in this mode, upload the project to the Seeeduino/Arduino, then cfTest or other software on the host PC
//   to communicate with the module.

//interface selection (select one)
#define IFACE_I2C
//#define IFACE_SPI

//I2C Interface connections
// SEEEDUNIO <-> CFA835
// SCL <-> H1-P4
// SDA <-> H1-P3
// PIN9 <-> H1-P13 (data ready)

//SPI Interface connections
// SEEEDUNIO <-> CFA835
// MOSI/PIN11 <-> H1-P8
// MISO/PIN12 <-> H1-P7
// SCK/PIN13 <-> H1-P10
// PIN10 <-> H1-P9 (slave select)
// PIN9 <-> H1-P12 (data ready)

//////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MODE_STANDALONE
# ifndef MODE_HOST_BRIDGE
#  error YOU MUST SELECT AN OPERATION MODE ABOVE
# endif
#endif

#ifndef IFACE_I2C
# ifndef IFACE_SPI
#  error YOU MUST SELECT A MODULE INTERFACE ABOVE
# endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////

//interface settings
#ifdef IFACE_I2C
#include <Wire.h>
# define IFACE_I2C_ADDR		0x44
# define IFACE_I2C_SPEED	100000
# define IFACE_I2C_READY	9
# define IFACE_I2C_REQ_LEN	16
#endif
#ifdef IFACE_SPI
# include <SPI.h>
# define IFACE_SPI_SPEED	1000000
# define IFACE_SPI_BITFIRST	MSBFIRST
# define IFACE_SPI_MODE		SPI_MODE0
# define IFACE_SPI_SS		10
# define IFACE_SPI_READY	9
# define IFACE_SPI_REQ_LEN	16
#endif

#define IFACE_READY_WAIT	100 /*mS*/

//////////////////////////////////////////////////////////////////////////////////////////////////////

//maxium CFA835 packet length is 128 bytes, but we set a lower number here
//so we dont use up all the Arduino/Seeduino's RAM.
//this will need to be set higher if you do expect to send/recv packets larger than
//the 64 bytes set here.
#define PACKET_MAX_DATA_LENGTH	64

//make sure we can buffer up a few packets worth of data.
//this must be PACKET_MAX_DATA_LENGTH*2 in size at a minimum, so
//getPacketFromFIFO() can deal with any garbage data
//also, the smaller the buffer, the more often you need to call getPacketFromFIFO() to make sure
//the incoming FIFO buffer does not overflow.
#define FIFO_IN_LENGTH			(PACKET_MAX_DATA_LENGTH*4)

typedef union
{
	uint8_t		b[2];
	uint16_t	w;
} wordUnion_t;

typedef struct
{
	uint8_t		command;
	uint8_t		length;
	uint8_t		data[PACKET_MAX_DATA_LENGTH];
	wordUnion_t	CRC;
} CFAPacket_t;

//////////////////////////////////////////////////////////////////////////////////////////////////////

FIFO_DATA_TYPE moduleInFIFOBuffer[FIFO_IN_LENGTH];
FIFO_t moduleInFIFO;

uint16_t recieveData(uint16_t readyWaitmS);
bool sendPacket(CFAPacket_t *packet);
bool getPacketFromFIFO(FIFO_t *FIFO, CFAPacket_t *packet);
uint16_t getCRC(uint8_t *data, uint8_t length);
void debugPrintPacket(const char *append, CFAPacket_t *packet);

#ifdef IFACE_SPI
SPISettings spiSettings = SPISettings(IFACE_SPI_SPEED, IFACE_SPI_BITFIRST, IFACE_SPI_MODE);
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
	//run at power-on/reset of the Seeeduino/Arduino
	//setup interfaces / variables
	
	//debug out
	Serial.begin(115200);
#ifdef MODE_STANDALONE
	Serial.write("setup()\n");
#endif

	//packet fifo init
	FIFO_Init(&moduleInFIFO, moduleInFIFOBuffer, FIFO_IN_LENGTH);

	//cfa module interface init
#ifdef IFACE_I2C
	Wire.begin();
	pinMode(IFACE_I2C_READY, INPUT); //I2C-Ready
#endif
#ifdef IFACE_SPI
	pinMode(IFACE_SPI_SS, OUTPUT); //SPI-SS
	digitalWrite(IFACE_SPI_SS, HIGH); //SS deselect
	pinMode(IFACE_SPI_READY, INPUT); //SPI-Ready
	SPI.begin();
#endif
}

#ifdef MODE_HOST_BRIDGE
void hostBridgeMode(void)
{
	//host bridge mode
	//relay data to/from selected interface to host PC
	uint16_t n, i;
	uint8_t buf[FIFO_IN_LENGTH];

	while (1)
	{
		//check for incoming data from host pc
		n = Serial.available();
		if (n != 0)
		{
			//cap the length to buffer size
			if (n > FIFO_IN_LENGTH)
				n = FIFO_IN_LENGTH;
			//buffer up serial the data first
			Serial.readBytes(buf, n);
			//send it to the module
#ifdef IFACE_I2C
			//send over I2C to the module
			Wire.beginTransmission(IFACE_I2C_ADDR);
			Wire.write(buf, n);
			Wire.endTransmission();
#endif
#ifdef IFACE_SPI
			//send over SPI to module
			SPI.beginTransaction(spiSettings);
			digitalWrite(IFACE_SPI_SS, LOW); //slave-select
			for (i = 0; i < n; i++)
				//send the serial data, but also buffer up incoming data too
				FIFO_Push(&moduleInFIFO, SPI.transfer(buf[i]));
			digitalWrite(IFACE_SPI_SS, HIGH); //slave-deselect
			SPI.endTransaction();
#endif
		}

		//check for incoming data from cf module
		recieveData(10);
		//forward any data in module in-fifo to host
		if ((n = FIFO_Count(&moduleInFIFO)))
		{
			//forward n bytes data to host
			for (i = 0; i < n; i++)
			{
				uint8_t d;
				FIFO_Pop(&moduleInFIFO, &d);
				Serial.write(d);
			}
		}
	}
}
#endif

#ifdef MODE_STANDALONE
void standaloneMode(void)
{
	Serial.write("standaloneMode()\n");

	CFAPacket_t outPacket;
	CFAPacket_t inPacket;
	uint16_t countSent, countRecv, loopCount;
	char temps[32];

	countSent = 0;
	countRecv = 0;
	loopCount = 0;
	while (1)
	{
		//send a ping packet
		outPacket.command = PCMD2_PING;
		outPacket.length = sprintf((char*)outPacket.data, "PING-ME-%u", loopCount);
		//outPacket.length = sprintf((char*)outPacket.data, "ABCDEFGHIJKLMNOPQ");
		sendPacket(&outPacket);
		countSent++;

		//send a display packet
		outPacket.command = PCMD2_LCD_WRITE;
		outPacket.length = 0;
		outPacket.data[outPacket.length++] = 0; //lcd column
		outPacket.data[outPacket.length++] = 0; //lcd row
		outPacket.length += sprintf((char*)&outPacket.data[outPacket.length], "Loops: %u", loopCount);
		sendPacket(&outPacket);
		countSent++;

		//buffer up all raw reply data from module
		recieveData(IFACE_READY_WAIT);
		//sift through raw data for valid packets
		while (getPacketFromFIFO(&moduleInFIFO, &inPacket))
		{
			//got a packet, do something with it
			debugPrintPacket("IN", &inPacket);
			countRecv++;
		}

		//debug print sent & recieved packet counts
		sprintf(temps, "Sent:%d Recv:%d", countSent, countRecv);
		Serial.println(temps);

		//loop delay
		loopCount++;
		delay(250);
	}
}
#endif

void loop()
{
	//main arduino framework loop
#ifdef MODE_STANDALONE
	standaloneMode();
#endif
#ifdef MODE_HOST_BRIDGE
	hostBridgeMode();
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef MODE_STANDALONE
void debugPrintPacket(const char *append, CFAPacket_t *packet)
{
	//write debug packet info to host
	uint8_t i, p = 0;
	char	out[128];
	p += sprintf(out, "%s c:%d l:%d d:", append, packet->command, packet->length);
	for (i = 0; i < packet->length; i++)
		p += sprintf(&out[p], "\\%02X", packet->data[i]);
	p += sprintf(&out[p], " crc:\\%02X\\%02X\n", packet->CRC.b[0], packet->CRC.b[1]);
	Serial.write(out);
}
#else
void debugPrintPacket(const char *append, CFAPacket_t *packet)
{
	//do nothing in standalone mode
}
#endif

bool sendPacket(CFAPacket_t *packet)
{
	//send a packet
	uint8_t		i;
	//calc CRC
	packet->CRC.w = getCRC((byte*)packet, 2+packet->length);
	//
	debugPrintPacket("OUT", packet);
	//send the packet
#ifdef IFACE_I2C
	//put the packet to send into a data buffer, and send it
	uint8_t outbuf[packet->length+4];
	uint8_t c = 0;
	outbuf[c++] = packet->command;
	outbuf[c++] = packet->length;
	for (i = 0; i < packet->length; i++)
		outbuf[c++] = packet->data[i];
	outbuf[c++] = packet->CRC.b[0];
	outbuf[c++] = packet->CRC.b[1];
	//I2c.write(IFACE_I2C_ADDR, outbuf, packet->length+4);
	Wire.beginTransmission(IFACE_I2C_ADDR);
	Wire.write(outbuf, packet->length+4);
	Wire.endTransmission();
#endif
#ifdef IFACE_SPI
	//any time we write SPI data, we also buffer up incoming data
	//getPacketFromFIFO() takes care of removing any trash bytes
	//between packets
	SPI.beginTransaction(spiSettings);
	digitalWrite(IFACE_SPI_SS, LOW); //slave-select
	FIFO_Push(&moduleInFIFO, SPI.transfer(packet->command));
	FIFO_Push(&moduleInFIFO, SPI.transfer(packet->length));
	for (i = 0; i < packet->length; i++)
		FIFO_Push(&moduleInFIFO, SPI.transfer(packet->data[i]));
	FIFO_Push(&moduleInFIFO, SPI.transfer(packet->CRC.b[0]));
	FIFO_Push(&moduleInFIFO, SPI.transfer(packet->CRC.b[1]));
	digitalWrite(IFACE_SPI_SS, HIGH); //slave-deselect
	SPI.endTransaction();
#endif
	return true;
}

#ifdef IFACE_I2C
uint16_t recieveData(uint16_t readyWaitmS)
{
	//checks interface for incoming data
	//if any present, puts on moduleInFIFO
	uint16_t count = 0;

	//request data from the I2C slave (the CFA module)
	//since we dont know how much data is in the CFA modules outgoing buffer,
	//read 8 bytes at a time until the data ready line goes high
	unsigned long timeout;

	//wait for ready pin to go low indicating data is ready to read
	//to exit immediatley if ready is not low, use recieveData(0)
	timeout = millis() + readyWaitmS;
	while (digitalRead(IFACE_I2C_READY) == HIGH)
	{
		if (millis() > timeout)
			//wait timeout
			return 0;
	}

	//while the ready pin is low, there is data to read, get it
	while(digitalRead(IFACE_I2C_READY) == LOW)
	{
		//NOTE!!!
		//The Arduino I2C Wire library has no timeouts
		//If requestFrom() requests data, but doesnt get anything back it can hang.
		//Best idea would be to use an alternate library. We use the Wire lib here
		//for compatibility.

		//req I2C data
		Wire.requestFrom(IFACE_I2C_ADDR, IFACE_I2C_REQ_LEN);
		while(Wire.available())
		{
			//put it in the incoming fifo
			if (!FIFO_Push(&moduleInFIFO, Wire.read()))
				//returns false if FIFO is full
				//stop reading
				break;
			//count it for ret
			count++;
		}
	}
	//return data read qty
	return count;
}
#endif

#ifdef IFACE_SPI
uint16_t recieveData(uint16_t readyWaitmS)
{
	//checks interface for incoming data
	//if any present, puts on moduleInFIFO
	uint16_t count = 0;

	//request data from the SPI slave (the CFA module)
	//since we dont know how much data is in the CFA modules outgoing buffer,
	//read 8 bytes at a time until the data ready line goes high
	uint8_t data;
	unsigned long timeout;

	//wait for ready pin to go low indicating data is ready to read
	//to exit immediatley if ready is not low, use recieveData(0)
	timeout = millis() + readyWaitmS;
	while (digitalRead(IFACE_SPI_READY) == HIGH)
	{
		if (millis() > timeout)
			//wait timeout
			return 0;
	}

	SPI.beginTransaction(spiSettings);
	digitalWrite(IFACE_SPI_SS, LOW); //slave-select
	while (digitalRead(IFACE_SPI_READY) == LOW)
	{
		//ready data while the ready pin remains low
		//to get data we need to send data, so just send 0xFF's
		data = SPI.transfer(0xFF);
		//put it in our incoming buffer. getPacketFromFIFO() takes care of
		//removing any trash bytes between packets
		if (!FIFO_Push(&moduleInFIFO, data))
			//returns false if FIFO is full
			//stop reading
			break;
		//loop again, try to get more data
		count++;
	}
	//done
	digitalWrite(IFACE_SPI_SS, HIGH); //slave-deselect
	SPI.endTransaction();
	//return data read qty
	return count;
}
#endif

bool getPacketFromFIFO(FIFO_t *FIFO, CFAPacket_t *packet)
{
	//return first packet found on input FIFO
	FIFO_LENGTH_TYPE	positionCount;
	FIFO_LENGTH_TYPE	inQueue;
	FIFO_LENGTH_TYPE	i;

	//check for enough data in fifo
	inQueue = FIFO_Count(FIFO);
	if (inQueue < PACKET_HEADER_SIZE)
		//needs to have 4 or more bytes to contain valid packet
		return false;

	//check for packets
	for (positionCount = 0; positionCount < inQueue - (PACKET_HEADER_SIZE-1); positionCount++)
	{
		FIFO_Peek(FIFO, positionCount, &packet->command);
		FIFO_Peek(FIFO, positionCount + 1, &packet->length);
		//check Command/reply/report
		if (
				(packet->command & (PCMD_REPLY | PCMD_REPORT | PCMD_ERROR)) && //replies only
				(packet->length < PACKET_CFA835_DATA_SIZE+1) &&
				(inQueue > (positionCount + packet->length + PACKET_HEADER_SIZE - 1))
				)
		{
			//valid length, check CRC
			for (i = 0; i < packet->length; i++)
				FIFO_Peek(FIFO, positionCount + 2 + i, &packet->data[i]);
			FIFO_Peek(FIFO, positionCount + 2 + packet->length + 0, &packet->CRC.b[0]);
			FIFO_Peek(FIFO, positionCount + 2 + packet->length + 1, &packet->CRC.b[1]);
			if (getCRC((byte*)packet, 2+packet->length) == packet->CRC.w)
			{
				//valid CRC, is a packet (packet ready to be returned)
				//remove packet (and data before it) from the queue
				//quick adjust FIFO length
				FIFO->Tail += positionCount + packet->length + PACKET_HEADER_SIZE;
				//done
				return true;
			}
		}
	}

	//no packet found, pop off bad bytes (fifo-tail -> (fifo-head - PACKET_MAX_DATA_LEN))
	if (inQueue > PACKET_MAX_SIZE(PACKET_CFA835_DATA_SIZE))
	{
		//quick adjust FIFO length
		FIFO->Tail += inQueue - PACKET_MAX_SIZE(PACKET_CFA835_DATA_SIZE);
	}

	//done
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//CRC lookup table to avoid bit-shifting loops.
const uint16_t CRCLookupTable[256] =
	{0x00000,0x01189,0x02312,0x0329B,0x04624,0x057AD,0x06536,0x074BF,
	0x08C48,0x09DC1,0x0AF5A,0x0BED3,0x0CA6C,0x0DBE5,0x0E97E,0x0F8F7,
	0x01081,0x00108,0x03393,0x0221A,0x056A5,0x0472C,0x075B7,0x0643E,
	0x09CC9,0x08D40,0x0BFDB,0x0AE52,0x0DAED,0x0CB64,0x0F9FF,0x0E876,
	0x02102,0x0308B,0x00210,0x01399,0x06726,0x076AF,0x04434,0x055BD,
	0x0AD4A,0x0BCC3,0x08E58,0x09FD1,0x0EB6E,0x0FAE7,0x0C87C,0x0D9F5,
	0x03183,0x0200A,0x01291,0x00318,0x077A7,0x0662E,0x054B5,0x0453C,
	0x0BDCB,0x0AC42,0x09ED9,0x08F50,0x0FBEF,0x0EA66,0x0D8FD,0x0C974,
	0x04204,0x0538D,0x06116,0x0709F,0x00420,0x015A9,0x02732,0x036BB,
	0x0CE4C,0x0DFC5,0x0ED5E,0x0FCD7,0x08868,0x099E1,0x0AB7A,0x0BAF3,
	0x05285,0x0430C,0x07197,0x0601E,0x014A1,0x00528,0x037B3,0x0263A,
	0x0DECD,0x0CF44,0x0FDDF,0x0EC56,0x098E9,0x08960,0x0BBFB,0x0AA72,
	0x06306,0x0728F,0x04014,0x0519D,0x02522,0x034AB,0x00630,0x017B9,
	0x0EF4E,0x0FEC7,0x0CC5C,0x0DDD5,0x0A96A,0x0B8E3,0x08A78,0x09BF1,
	0x07387,0x0620E,0x05095,0x0411C,0x035A3,0x0242A,0x016B1,0x00738,
	0x0FFCF,0x0EE46,0x0DCDD,0x0CD54,0x0B9EB,0x0A862,0x09AF9,0x08B70,
	0x08408,0x09581,0x0A71A,0x0B693,0x0C22C,0x0D3A5,0x0E13E,0x0F0B7,
	0x00840,0x019C9,0x02B52,0x03ADB,0x04E64,0x05FED,0x06D76,0x07CFF,
	0x09489,0x08500,0x0B79B,0x0A612,0x0D2AD,0x0C324,0x0F1BF,0x0E036,
	0x018C1,0x00948,0x03BD3,0x02A5A,0x05EE5,0x04F6C,0x07DF7,0x06C7E,
	0x0A50A,0x0B483,0x08618,0x09791,0x0E32E,0x0F2A7,0x0C03C,0x0D1B5,
	0x02942,0x038CB,0x00A50,0x01BD9,0x06F66,0x07EEF,0x04C74,0x05DFD,
	0x0B58B,0x0A402,0x09699,0x08710,0x0F3AF,0x0E226,0x0D0BD,0x0C134,
	0x039C3,0x0284A,0x01AD1,0x00B58,0x07FE7,0x06E6E,0x05CF5,0x04D7C,
	0x0C60C,0x0D785,0x0E51E,0x0F497,0x08028,0x091A1,0x0A33A,0x0B2B3,
	0x04A44,0x05BCD,0x06956,0x078DF,0x00C60,0x01DE9,0x02F72,0x03EFB,
	0x0D68D,0x0C704,0x0F59F,0x0E416,0x090A9,0x08120,0x0B3BB,0x0A232,
	0x05AC5,0x04B4C,0x079D7,0x0685E,0x01CE1,0x00D68,0x03FF3,0x02E7A,
	0x0E70E,0x0F687,0x0C41C,0x0D595,0x0A12A,0x0B0A3,0x08238,0x093B1,
	0x06B46,0x07ACF,0x04854,0x059DD,0x02D62,0x03CEB,0x00E70,0x01FF9,
	0x0F78F,0x0E606,0x0D49D,0x0C514,0x0B1AB,0x0A022,0x092B9,0x08330,
	0x07BC7,0x06A4E,0x058D5,0x0495C,0x03DE3,0x02C6A,0x01EF1,0x00F78};

uint16_t getCRC(uint8_t *data, uint8_t length)
{
	register uint16_t newCrc = 0xFFFF;
	while (length--)
		newCrc = (newCrc >> 8) ^ CRCLookupTable[(newCrc ^ *data++) & 0xff];
	//Make this crc match the one's complement that is sent in the packet.
	return (~newCrc);
}