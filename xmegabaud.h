#ifndef __XMEGA_BAUD_H__
#define __XMEGA_BAUD_H__

//(beware the line continuations!) Basically, the BSCALE macro steps through the +-7 range hunting for the highest legal (12-bit) BSEL value, and the BSEL macro uses that to generate the right divider.  A typical usage would be something like this:
// 
//#include "xmegabaud.h"
//#define F_CPU 32000000
//#define BAUDRATE 115200

//USARTC0.BAUDCTRLA = BSEL(F_CPU,BAUDRATE) & 0xff;
//USARTC0.BAUDCTRLB = (BSCALE(F_CPU,BAUDRATE) << USART_BSCALE0_bp) | (BSEL(F_CPU,BAUDRATE) >>8)

/*
// For CLK2X=1
#define _BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,bscale) (                \
((bscale) < 0) ?                                                      \
  (int)((((float)(f_cpu)/(8*(float)(baud)))-1)*(1<<-(bscale)))        \
: (int)((float)(f_cpu)/((1<<(bscale))*8*(float)(baud)))-1 )
*/

// For CLK2X=0
#define _BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,bscale) (                \
((bscale) < 0) ?                                                      \
  (int)((((float)(f_cpu)/(16*(float)(baud)))-1)*(1<<-(bscale)))        \
: (int)((float)(f_cpu)/((1<<(bscale))*16*(float)(baud)))-1 )


#define _BSCALE(f_cpu,baud) (                                         \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-7) < 4096) ? -7 :              \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-6) < 4096) ? -6 :              \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-5) < 4096) ? -5 :              \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-4) < 4096) ? -4 :              \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-3) < 4096) ? -3 :              \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-2) < 4096) ? -2 :              \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,-1) < 4096) ? -1 :              \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,0) < 4096) ? 0 :                \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,1) < 4096) ? 1 :                \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,2) < 4096) ? 2 :                \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,3) < 4096) ? 3 :                \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,4) < 4096) ? 4 :                \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,5) < 4096) ? 5 :                \
(_BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,6) < 4096) ? 6 :                \
7 )

#define BSEL(f_cpu,baud)                                              \
  _BAUD_BSEL_FROM_BAUDSCALE(f_cpu,baud,_BSCALE(f_cpu,baud))

#define BSCALE(f_cpu,baud) ((_BSCALE(f_cpu,baud)<0) ? (16+_BSCALE(f_cpu,baud)) : _BSCALE(f_cpu,baud))

#endif /* __XMEGA_BAUD_H__ */

