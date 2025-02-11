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

FIFO_t *FIFO_Init(FIFO_t *FIFO, FIFO_DATA_TYPE *Buffer, FIFO_LENGTH_TYPE Size) {
  FIFO->Buffer = Buffer;
  FIFO->Length = Size;
  FIFO->Head = 0;
  FIFO->Tail = 0;
  return FIFO;
}

FIFO_OPTS void FIFO_Release(FIFO_t *FIFO) {
  FIFO->Length = 0;
  FIFO->Head = 0;
  FIFO->Tail = 0;
}

FIFO_OPTS FIFO_INLINE FIFO_LENGTH_TYPE FIFO_Count(FIFO_t const *FIFO) {
  return FIFO->Head - FIFO->Tail;
}

FIFO_OPTS FIFO_INLINE FIFO_LENGTH_TYPE FIFO_Remaining(FIFO_t const *FIFO) {
  return FIFO->Length - FIFO_Count(FIFO);
}

FIFO_OPTS FIFO_INLINE FIFO_BOOL_TYPE FIFO_Full(FIFO_t const *FIFO) {
  return (FIFO_Count(FIFO) == FIFO->Length) ? FIFO_BOOL_TRUE : FIFO_BOOL_FALSE;
}

FIFO_OPTS FIFO_INLINE FIFO_BOOL_TYPE FIFO_Empty(FIFO_t const *FIFO) {
  return (FIFO->Head == FIFO->Tail) ? FIFO_BOOL_TRUE : FIFO_BOOL_FALSE;
}

FIFO_OPTS FIFO_INLINE void FIFO_PushCircular(FIFO_t *FIFO, FIFO_DATA_TYPE Data) {
  if (FIFO_Full(FIFO))
    //full, remove last byte
    FIFO->Tail++;
  FIFO->Buffer[FIFO->Head % FIFO->Length] = Data;
  FIFO->Head++;
}

FIFO_OPTS FIFO_INLINE FIFO_BOOL_TYPE FIFO_Push(FIFO_t *FIFO, FIFO_DATA_TYPE Data) {
#ifndef FIFO_NOCHECKS
  if (FIFO_Full(FIFO))
    return FIFO_BOOL_FALSE;
#endif
  FIFO->Buffer[FIFO->Head % FIFO->Length] = Data;
  FIFO->Head++;
  return FIFO_BOOL_TRUE;
}

FIFO_OPTS FIFO_INLINE FIFO_BOOL_TYPE FIFO_Pop(FIFO_t *FIFO, FIFO_DATA_TYPE *Data) {
#ifndef FIFO_NOCHECKS
  if (FIFO_Empty(FIFO))
    return FIFO_BOOL_FALSE;
#endif
  *Data = FIFO->Buffer[FIFO->Tail % FIFO->Length];
  FIFO->Tail++;
  return FIFO_BOOL_TRUE;
}

FIFO_OPTS FIFO_INLINE FIFO_BOOL_TYPE FIFO_Peek(FIFO_t *FIFO, FIFO_LENGTH_TYPE Position, FIFO_DATA_TYPE *Data) {
#ifndef FIFO_NOCHECKS
  if (Position >= FIFO_Count(FIFO))
    return FIFO_BOOL_FALSE;
#endif
  *Data = FIFO->Buffer[(FIFO->Tail + Position) % FIFO->Length];
  return FIFO_BOOL_TRUE;
}

FIFO_OPTS FIFO_INLINE void FIFO_Flush(FIFO_t *FIFO) {
  FIFO->Tail = FIFO->Head;
}
