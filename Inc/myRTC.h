#ifndef __myRtc_H
#include <stdbool.h>

extern void myRtcInit(void);
//extern void myRtcLcd(void);
extern void myRtcGetTime(uint8_t *buff);
extern void myRtcSaveActualTime(void);
extern bool myRtcIsNewTime(void);

#endif // __myRtc_H