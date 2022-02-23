#include "AT91SAM7S256.h"

#ifndef HARDWARE_H
#define HARDWARE_H

#define MCLK 48000000
#define FWS 1 // Flash wait states

#define RAMFUNC __attribute__ ((long_call, section (".ramsection")))

#if defined(MINIMIG_V1_0)
#define DISKLED       AT91C_PIO_PA10
#define DISKLED_ON    *AT91C_PIOA_SODR = DISKLED;
#define DISKLED_OFF   *AT91C_PIOA_CODR = DISKLED;
#define MMC_CLKEN     AT91C_PIO_PA24
#define MMC_SEL       AT91C_PIO_PA27
#define BUTTON        AT91C_PIO_PA28

// xilinx programming interface
#define XILINX_DIN    AT91C_PIO_PA20
#define XILINX_CCLK   AT91C_PIO_PA4
#define XILINX_PROG_B AT91C_PIO_PA9
#define XILINX_INIT_B AT91C_PIO_PA7
#define XILINX_DONE   AT91C_PIO_PA8

#define FPGA0 AT91C_PIO_PA26
#define FPGA1 AT91C_PIO_PA25
#define FPGA2 AT91C_PIO_PA15

/* Backbus interface defines */
#define BB_WRITE		0x80
#define BB_AUTOINC		0x40

#define BB_REG_CTRL_BASE    0x00
#define BB_REG_CPU_BASE		0x10
#define BB_REG_MEM_BASE		0x20
#define BB_REG_IWM_BASE		0x28
#define BB_REG_SCSI_BASE	0x30
#define BB_REG_SCOPE_BASE	0x38

#elif defined(SAM7_P256)
#define DISKLED       AT91C_PIO_PA18
#define DISKLED_ON    *AT91C_PIOA_CODR = DISKLED;
#define DISKLED_OFF   *AT91C_PIOA_SODR = DISKLED;
#define MMC_SEL       AT91C_PIO_PA11
#define BUTTON        AT91C_PIO_PA19
#define USB_SEL       AT91C_PIO_PA31
#define USB_PUP       (AT91C_PIO_PA16 || AT91C_PIO_PA8)

// altera programming interface
#define ALTERA_DONE    AT91C_PIO_PA2
#define ALTERA_DATA0   AT91C_PIO_PA3
#define ALTERA_NCONFIG AT91C_PIO_PA1
#define ALTERA_NSTATUS AT91C_PIO_PA0
#define ALTERA_DCLK    AT91C_PIO_PA4

#elif defined(MIST)
#define DISKLED       AT91C_PIO_PA29
#define DISKLED_ON    *AT91C_PIOA_CODR = DISKLED;
#define DISKLED_OFF   *AT91C_PIOA_SODR = DISKLED;
#define MMC_SEL       AT91C_PIO_PA31
#define USB_SEL       AT91C_PIO_PA11
#define USB_PUP       AT91C_PIO_PA16

// altera programming interface
#define ALTERA_DONE    AT91C_PIO_PA4
#define ALTERA_DATA0   AT91C_PIO_PA9
#define ALTERA_NCONFIG AT91C_PIO_PA8
#define ALTERA_NSTATUS AT91C_PIO_PA7
#define ALTERA_DCLK    AT91C_PIO_PA15

// db9 joystick ports
#define JOY1_UP        AT91C_PIO_PA28
#define JOY1_DOWN      AT91C_PIO_PA27
#define JOY1_LEFT      AT91C_PIO_PA26
#define JOY1_RIGHT     AT91C_PIO_PA25
#define JOY1_BTN1      AT91C_PIO_PA24
#define JOY1_BTN2      AT91C_PIO_PA23
#define JOY1  (JOY1_UP|JOY1_DOWN|JOY1_LEFT|JOY1_RIGHT|JOY1_BTN1|JOY1_BTN2)

#define JOY0_UP        AT91C_PIO_PA22
#define JOY0_DOWN      AT91C_PIO_PA21
#define JOY0_LEFT      AT91C_PIO_PA20
#define JOY0_RIGHT     AT91C_PIO_PA19
#define JOY0_BTN1      AT91C_PIO_PA18
#define JOY0_BTN2      AT91C_PIO_PA17
#define JOY0  (JOY0_UP|JOY0_DOWN|JOY0_LEFT|JOY0_RIGHT|JOY0_BTN1|JOY0_BTN2)

// chip selects for FPGA communication
#define FPGA0 AT91C_PIO_PA10
#define FPGA1 AT91C_PIO_PA3
#define FPGA2 AT91C_PIO_PA2

#define FPGA3         AT91C_PIO_PA9   // same as ALTERA_DATA0
#else

#error "Undefined setup!"
#endif

#define VBL           AT91C_PIO_PA7

void USART_Init(unsigned long baudrate);
void USART_Write(unsigned char c) RAMFUNC;

static inline unsigned char SPI(unsigned char outByte) {
  while (!(*AT91C_SPI_SR & AT91C_SPI_TDRE));
  *AT91C_SPI_TDR = outByte;
  while (!(*AT91C_SPI_SR & AT91C_SPI_RDRF));
  return((unsigned char)*AT91C_SPI_RDR);
}

#define SPI_SDC_CLK_VALUE 2     // 24 Mhz
#define SPI_MMC_CLK_VALUE 3     // 16 Mhz
#define SPI_SLOW_CLK_VALUE 120  // 400kHz

static inline void SPI_Wait4XferEnd() {
  while (!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY));
  
  /* Clear any data left in the receiver */
  (void)*AT91C_SPI_RDR;
  (void)*AT91C_SPI_RDR;
}

static inline void EnableCard() {
    *AT91C_PIOA_CODR = MMC_SEL;  // clear output (MMC chip select enabled)
}


static inline void PullUpCard() {
    *AT91C_PIOA_PPUSR = AT91C_PA14_SPCK | AT91C_PA13_MOSI | AT91C_PA12_MISO;  // Pull-up Status Register
}

static inline void DisableCard() {
    SPI_Wait4XferEnd();
    *AT91C_PIOA_SODR = MMC_SEL;  // set output (MMC chip select disabled)
    SPI(0xFF);
    SPI_Wait4XferEnd();
}

void SPI_Init(void);
//unsigned char SPI(unsigned char outByte);
//void SPI_Wait4XferEnd(void);
void EnableFpga(void);
void DisableFpga(void);
void EnableOsd(void);
void DisableOsd(void);
unsigned long CheckButton(void);
void Timer_Init(void);
unsigned long GetTimer(unsigned long offset);
unsigned long CheckTimer(unsigned long t);
void WaitTimer(unsigned long time);

void SPI_slow();
void SPI_fast();
void SPI_fast_mmc();
void TIMER_wait(unsigned long ms);
void EnableDMode();
void DisableDMode();
void SPI_block(unsigned short num);

#define SPI_BLOCK_READ
void SPI_block_read(unsigned char *pReadBuffer) RAMFUNC;

#ifdef FPGA3
// the MiST has the user inout on the arm controller
void EnableIO(void);
void DisableIO(void);
#endif

#define DEBUG_FUNC_IN() 

#endif // HARDWARE_H
