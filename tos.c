#include "stdio.h"
#include "string.h"
#include "hardware.h"

#include "tos.h"
#include "mist.h"
#include "fat.h"
#include "fpga.h"

#define TOS_BASE_ADDRESS    0xfc0000
#define VIDEO_BASE_ADDRESS  0x010000

static unsigned char font[4096];  // buffer for 8x16 atari font

static void mist_memory_set_address(unsigned long a) {
  a >>= 1;   // make word address

  EnableFpga();
  SPI(MIST_SET_ADDRESS);
  SPI((a >> 24) & 0xff);
  SPI((a >> 16) & 0xff);
  SPI((a >>  8) & 0xff);
  SPI((a >>  0) & 0xff);
  DisableFpga();
}

static void mist_set_control(unsigned short ctrl) {
  EnableFpga();
  SPI(MIST_SET_CONTROL);
  SPI((ctrl >>  8) & 0xff);
  SPI((ctrl >>  0) & 0xff);
  DisableFpga();
}

static void hexdump(void *data, unsigned long size, unsigned long offset) {
  int i, b2c;
  unsigned long n=0;
  char *ptr = data;

  if(!size) return;

  while(size>0) {
    iprintf("%08x: ", n + offset);

    b2c = (size>16)?16:size;
    for(i=0;i<b2c;i++)      iprintf("%02x ", 0xff&ptr[i]);
    iprintf("  ");
    for(i=0;i<(16-b2c);i++) iprintf("   ");
    for(i=0;i<b2c;i++)      iprintf("%c", isprint(ptr[i])?ptr[i]:'.');
    iprintf("\n");
    ptr  += b2c;
    size -= b2c;
    n    += b2c;
  }
}

static void mist_memory_read(char *data, unsigned long words) {
  EnableFpga();
  SPI(MIST_READ_MEMORY);

  // transmitted bytes must be multiple of 2 (-> words)
  while(words--) {
    *data++ = SPI(0);
    *data++ = SPI(0);
  }

  DisableFpga();
}

static void mist_memory_write(char *data, unsigned long words) {
  EnableFpga();
  SPI(MIST_WRITE_MEMORY);

  while(words--) {
    SPI(*data++);
    SPI(*data++);
  }

  DisableFpga();
}

void mist_memory_set(char data, unsigned long words) {
  EnableFpga();
  SPI(MIST_WRITE_MEMORY);

  while(words--) {
    SPI(data);
    SPI(data);
  }

  DisableFpga();
}

static void tos_clr() {
  mist_memory_set_address(VIDEO_BASE_ADDRESS);
  mist_memory_set(0, 16000);
}

static void tos_write(char *str) {
  static int y = 0;
  int l;
  int c = strlen(str);

  {
    char buffer[c];

    // 16 pixel lines
    for(l=0;l<16;l++) {
      char *p = str, *f=buffer;
      while(*p)
	*f++ = font[16 * *p++ + l];
      
      mist_memory_set_address(VIDEO_BASE_ADDRESS + 80*(y+l));
      mist_memory_write(buffer, c/2);
    }
  }
  y+=16;
}

static void tos_font_load() {
  fileTYPE file;
  if(FileOpen(&file,"SYSTEM  FNT")) {
    if(file.size == 4096) {
      int i;
      for(i=0;i<8;i++) {
	FileRead(&file, font+i*512);
	FileNextSector(&file);
      }

      tos_clr();
      tos_write("\016\017 MIST core \016\017 ");

    } else
      iprintf("SYSTEM.FNT has wrong size\n");      
  } else
    iprintf("SYSTEM.FNT not found\n");      
}

void tos_upload() {

  // put cpu into reset
  mist_set_control(MIST_CONTROL_CPU_RESET);

  tos_font_load();

  // do the MiST core handling
  tos_write("Uploading TOS ... ");
  iprintf("Uploading TOS ...\n");
  
  // upload and verify tos image
  fileTYPE file;
  if(FileOpen(&file,"TOS     IMG")) {
    int i;
    char buffer[512];
    unsigned long time;
	
    iprintf("TOS.IMG:\n  size = %d\n", file.size);

    int blocks = file.size / 512;
    iprintf("  blocks = %d\n", blocks);

    // clear first 16k
    mist_memory_set_address(0);
    mist_memory_set(0x00, 8192);

#if 0
    iprintf("Erasing:   ");
    
    // clear memory to increase chances of catching write problems
    mist_memory_set_address(TOS_BASE_ADDRESS);
    mist_memory_set(0x00, file.size/2);
    iprintf("done\n");
#endif
	
    time = GetTimer(0);
    iprintf("Uploading: [");
    
    for(i=0;i<blocks;i++) {
      FileRead(&file, buffer);

      // copy first 8 bytes to address 0 as well
      if(i == 0) {
	mist_memory_set_address(0);

	// write first 4 words
	mist_memory_write(buffer, 4);

	// set real tos base address
	mist_memory_set_address(TOS_BASE_ADDRESS);
      }
      
      mist_memory_write(buffer, 256);
      
      if(!(i & 7)) iprintf(".");
      
      if(i != blocks-1)
	FileNextSector(&file);
    }
    iprintf("]\n");
    
    time = GetTimer(0) - time;
    printf("TOS.IMG uploaded in %lu ms\r", time >> 20);
    
  } else
    iprintf("Unable to find tos.img\n");
  
#if 0
  {
    char rx[512], buffer[512];
    int i,j;
    int blocks = file.size / 512;
    
    FileSeek(&file, 0, SEEK_SET);
    
    mist_memory_set_address(TOS_BASE_ADDRESS);
    
    iprintf("Verifying: [");
    for(i=0;i<blocks;i++) {
      FileRead(&file, buffer);

      mist_memory_read(rx, 256);
      
      if(!(i & 7)) iprintf("+");
      
      for(j=0;j<512;j++) {
	if(buffer[j] != rx[j]) {
	  iprintf("Verify error block %d, byte %x\n", i, j);

	  iprintf("should be:\n");
	  hexdump(buffer, 512, 0);

	  iprintf("is:\n");
	  hexdump(rx, 512, 0);

	  // try to re-read to check whether read or write failed
	  mist_memory_set_address(TOS_BASE_ADDRESS+i*512);
	  mist_memory_read(rx, 256);

	  iprintf("re-read: %s\n", (buffer[j] != rx[j])?"failed":"ok");
	  hexdump(rx, 512, 0);


	  while(1);
	}
      }
      
      if(i != blocks-1)
	FileNextSector(&file);
    }
    iprintf("]\n");
  }
#endif

  tos_write("Booting ... ");

  // let cpu run (release reset)
  mist_set_control(0);
}

static unsigned long get_long(char *buffer, int offset) {
  unsigned long retval = 0;
  int i;
  
  for(i=0;i<4;i++)
    retval = (retval << 8) + *(unsigned char*)(buffer+offset+i);

  return retval;
}

void tos_show_state() {
  static unsigned long cnt = 0;

  if(cnt == 1) {    
    int i;
    char buffer[1024];

#if 0
    for(i=0;i<4;i++) {
      mist_memory_set_address(1024*i);
      mist_memory_read(buffer, 512);
    
      iprintf("dump %x:\n" ,i*1024);
      hexdump(buffer, 1024, i*1024);
    }
#endif

    // tos system varables are from $400 
    mist_memory_set_address(0x400);
    mist_memory_read(buffer, 512);

    iprintf("memvalid:  $%lx (should be $752019F3)\n", get_long(buffer,0x20));
    iprintf("memcntrl:  $%x  (memory controller low nibble)\n", buffer[0x24]); 
    iprintf("phystop:   $%lx (Physical RAM top)\n", get_long(buffer,0x2e));
    iprintf("memval2:   $%lx (should be $237698AA)\n", get_long(buffer,0x3a));
    iprintf("sshiftmd:  $%x  (Shadow shiftmd, LMH/012)\n", buffer[0x4c]); 
    iprintf("_v_bas_ad: $%lx (Screen memory base)\n", get_long(buffer,0x4e));
    iprintf("_vbclock:  $%lx (vbl counter)\n", get_long(buffer,0x62));
    iprintf("_frclock:  $%lx (frame counter)\n", get_long(buffer,0x66));
    iprintf("_hz_200:   $%lx (Raw 200Hz timer)\n", get_long(buffer,0xba));
    iprintf("_sysbase:  $%lx (begin of tos)\n", get_long(buffer,0xf2));

#if 0
    for(i=0;i<4;i++) {
      mist_memory_set_address(0x3f8000 + 1024*i);
      mist_memory_read(buffer, 512);
    
      iprintf("vid %x:\n" ,0x3f8000 + i*1024);
      hexdump(buffer, 1024, 0x3f8000 + i*1024);
    }
#endif
  }

  cnt++;
}
