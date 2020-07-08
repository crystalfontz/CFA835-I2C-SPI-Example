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

#ifndef FIFO_H_
#define FIFO_H_

//////////////////////////////////////////////////

#include <Arduino.h>

#define FIFO_INLINE
#define FIFO_OPTS
#define FIFO_VOLATILE
//#define FIFO_NOCHECKS

#define FIFO_DATA_TYPE		uint8_t
#define FIFO_LENGTH_TYPE	uint16_t
#define FIFO_BOOL_TYPE		bool
#define FIFO_BOOL_TRUE		true
#define FIFO_BOOL_FALSE		false

typedef struct
{
    FIFO_VOLATILE FIFO_LENGTH_TYPE		Head;		// first byte of data
    FIFO_VOLATILE FIFO_LENGTH_TYPE		Tail;		// last byte of data
    FIFO_VOLATILE FIFO_DATA_TYPE		*Buffer;	// block of memory or array of data
    FIFO_LENGTH_TYPE					Length;     // length of the data
} FIFO_t;

FIFO_t *FIFO_Init(FIFO_t *FIFO, FIFO_DATA_TYPE *Buffer, FIFO_LENGTH_TYPE Size);

void FIFO_Release(FIFO_t *FIFO);
FIFO_BOOL_TYPE FIFO_Push(FIFO_t *FIFO, FIFO_DATA_TYPE Data);
void FIFO_PushCircular(FIFO_t *FIFO, FIFO_DATA_TYPE Data);
FIFO_BOOL_TYPE FIFO_Pop(FIFO_t *FIFO, FIFO_DATA_TYPE *Data);
FIFO_BOOL_TYPE FIFO_Peek(FIFO_t *FIFO, FIFO_LENGTH_TYPE Position, FIFO_DATA_TYPE *Data);
FIFO_LENGTH_TYPE FIFO_Count(FIFO_t const *FIFO);
FIFO_LENGTH_TYPE FIFO_Remaining(FIFO_t const *FIFO);
void FIFO_Flush(FIFO_t *FIFO);
FIFO_BOOL_TYPE FIFO_Empty(FIFO_t const *FIFO);
FIFO_BOOL_TYPE FIFO_Full(FIFO_t const *FIFO);

#endif
