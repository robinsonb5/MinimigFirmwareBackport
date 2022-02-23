#include "rafile.h"
//
#ifdef __GNUC__
#define __noinline __attribute__ ((noinline)) 
#endif

//#ifdef __GNUC__
//#define __noinline
//#endif

typedef struct
{
    unsigned long flags;
    unsigned long base;
    unsigned long size;
    unsigned long crc;
} romTYPE;

typedef struct
{
    unsigned char id[8];
    unsigned char version[8];
//    unsigned char version[16];
    romTYPE       rom;
    unsigned long padding[119];
    unsigned long crc;
} UPGRADE;

#define true -1
#define false 0

#define NVCONFIG 0x3FFFC
#define NVCONFIG_SPIMODE 0x00000001
#define SPIMODE_NORMAL 0
#define SPIMODE_FAST 1

unsigned long CalculateCRC32(unsigned long crc, unsigned char *pBuffer, unsigned long nSize);
unsigned char CheckFirmware(fileTYPE *file, char *name);
__noinline unsigned long WriteFirmware(fileTYPE *file) RAMFUNC;
char *GetFirmwareVersion(fileTYPE *file, char *name);
__noinline void SetSPIMode(unsigned long mode) RAMFUNC;
unsigned long GetSPIMode(void);