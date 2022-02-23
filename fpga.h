#ifndef FPGA_H
#define FPGA_H

#include "rafile.h"

unsigned char ConfigureFpga(void) RAMFUNC;
void SendFile(RAFile *file);
void SendFileEncrypted(RAFile *file,unsigned char *key,int keysize);
char BootDraw(char *data, unsigned short len, unsigned short offset);
char BootPrint(const char *text);
//char PrepareBootUpload(unsigned char base, unsigned char size);
char PrepareBootUpload(unsigned char baseup, unsigned char baselo, unsigned char sizeup, unsigned char sizelo);
void BootExit(void);
void ClearMemory(unsigned long base, unsigned long size);
unsigned char GetFPGAStatus(void);

#endif

