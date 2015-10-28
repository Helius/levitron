#pragma once

#define FALSE         0
#define TRUE          1

#define TGLBIT(REG, BIT)   (REG ^= (1 << BIT))
#define CLRBIT(REG, BIT)   (REG &= ~(1 << BIT))
#define SETBIT(REG, BIT)   (REG |= (1 << BIT))
#define TSTBIT(REG, BIT)   (REG & (1 << BIT))

#define OPAMP_CHAN 3
#define CALIB_CHAN 4
#define S_UP     2
#define S_DOWN   1

#define _BUF_SIZE 32
typedef struct {
	int buf[_BUF_SIZE];
	unsigned char curr;
} Result_Buffer;
