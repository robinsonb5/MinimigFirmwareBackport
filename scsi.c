#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "hardware.h"
#include "mmc.h"
#include "fat.h"
#include "scsi.h"

/* SCSI Backbus interface */
#define BB_REG_LINES	0
#define BB_LINES_BSY	7
#define BB_LINES_SEL	6	
#define BB_LINES_ACK	5
#define BB_LINES_ATN	4
#define BB_LINES_RST	3

#define BB_REG_ASSERT	1
#define BB_ASSERT_BSY	7
#define BB_ASSERT_SEL	6
#define BB_ASSERT_CD	5
#define BB_ASSERT_IO	4
#define BB_ASSERT_MSG	3
#define BB_ASSERT_REQ	2
#define BB_ASSERT_RST	1
#define BB_ASSERT_AUTO	0	/* Automatic mode */

#define BB_REG_ODATA	2
#define BB_REG_IDATA	3
#define BB_REG_ACNT_HI	4
#define BB_REG_ACNT_LO	5

#define MASK(bit)	(1u << (bit))

static unsigned char scsi_cmd[12];
static unsigned char scsi_assert;
static unsigned long scsi_size;

#define scsi_dbg(fmt...)

void scsi_open(void)
{
	long size;

	size = fat_open("MACHD   IMG");
	if (size < 0) {
		printf("File MACHD.IMG not found !\n");
		return;
	}
	scsi_size = size;
	FileSeek(0);
}

static unsigned char scsi_get_lines(void)
{
	unsigned char lines;

	EnableFpga();
	SPI(BB_REG_SCSI_BASE | BB_REG_LINES);
	SPI(0xff);
	lines = SPI(0xff);
	DisableFpga();

	return lines;
}

static void scsi_set_assert(void)
{
	EnableFpga();
	SPI(BB_WRITE | BB_REG_SCSI_BASE | BB_REG_ASSERT);


	SPI(scsi_assert);
	DisableFpga();
}

static unsigned char scsi_get_data(void)
{
	unsigned char lines;

	EnableFpga();
	SPI(BB_REG_SCSI_BASE | BB_REG_ODATA);
	SPI(0xff);
	lines = SPI(0xff);
	DisableFpga();

	return lines;
}

static void scsi_set_data(unsigned char val)
{
	EnableFpga();
	SPI(BB_WRITE | BB_REG_SCSI_BASE | BB_REG_IDATA);
	SPI(val);
	DisableFpga();
}

static void scsi_set_autocnt(unsigned short cnt)
{
	EnableFpga();
	SPI(BB_WRITE | BB_AUTOINC | BB_REG_SCSI_BASE | BB_REG_ACNT_HI);
	SPI(cnt >> 8);
	SPI(cnt & 0xff);
	DisableFpga();
}

static unsigned char scsi_do_data_in(unsigned short len)
{
	unsigned short i = 0;

	scsi_assert |= MASK(BB_ASSERT_AUTO) | MASK(BB_ASSERT_IO);
	scsi_set_assert();
	scsi_set_autocnt(len);

	EnableFpga();
	SPI(BB_WRITE | BB_REG_SCSI_BASE | BB_REG_IDATA);
	while(len--) {
		SPI(secbuf[i++]);
		/* Handhake goes down approx. 160ns after the above
		 * and we then need to wait for it to go back up,
		 * the PIC executes an instruction in about 200ns so
		 * we shouldn't need any delay here before we test it
		 */
		// while(!SCSI_HSHAKE);
		while(!FPGA1); //FPGA1
	}
	scsi_assert &= ~MASK(BB_ASSERT_AUTO);
	scsi_set_assert();
	return 0;
}

static unsigned char scsi_do_data_out(unsigned short len)
{
	unsigned short i = 0;

	scsi_assert |= MASK(BB_ASSERT_AUTO);
	scsi_assert &= ~MASK(BB_ASSERT_IO);
	scsi_set_assert();
	scsi_set_autocnt(len);

	EnableFpga();
	SPI(BB_REG_SCSI_BASE | BB_REG_ODATA);
	/* See handshake timing comment in scsi_do_data_in() */
//	while(!SCSI_HSHAKE);
	while(!FPGA1); //FPGA1
	SPI(0xff);
//	while(!SCSI_HSHAKE);
	while(!FPGA1); //FPGA1
	SPI(0xff);
//	while(!SCSI_HSHAKE);
	while(!FPGA1); //FPGA1
	SPI(0xff);
	while(len--) {
//		while(!SCSI_HSHAKE);
		while(!FPGA1); //FPGA1
		secbuf[i++] = SPI(0xff);
	}
	scsi_assert &= ~MASK(BB_ASSERT_AUTO);
	scsi_set_assert();
	return 0;
}

static unsigned char scsi_do_read(unsigned long lba,
				  unsigned short cnt)
{
//	unsigned char rc = true;
	unsigned char rc = 1;

	if (scsi_size == 0)
		return 1;
	DISKLED_ON;
	FileSeek(lba,0,0);
	while(cnt--) {
		if (rc)
			rc = FileRead();
		if (!rc) {
//			diskled_off();
			DISKLED_OFF;
			return 1;
		}
		scsi_do_data_in(512);
		if (cnt)
			rc = FileNextSector();
	}
    DISKLED_OFF;
	return 0;
}

static unsigned char scsi_do_write(unsigned long lba,
				   unsigned short cnt)
{
	unsigned char rc;

	if (scsi_size == 0)
		return 1;
	DISKLED_ON;
	FileSeek(lba);
	while(cnt--) {
		scsi_do_data_out(512);
		rc = fat_file_write();
		if (rc && cnt)
			rc = fat_file_next_sector();
		if (!rc) {
            DISKLED_OFF;
			return 1;
		}
	}
    DISKLED_OFF;
	return 0;
}

static unsigned char scsi_do_cmd(void)
{
	unsigned long lba;
	unsigned short cnt;

	switch(scsi_cmd[0]) {
	case 0x00:
		printf("scsi: TEST_UNIT_READY !\n");
		return 0;
	case 0x03:
		printf("scsi: REQUEST_SENSE !\n");
		fat_inval_cache();
		memset(secbuf, 0, 13);
		secbuf[0] = 0xf0;
		return scsi_do_data_in(13);
	case 0x04:
		printf("scsi: FORMAT_UNIT !\n");
		return 1;
	case 0x08:
		lba = scsi_cmd[1] & 0x1f;
		lba = (lba << 8) | scsi_cmd[2];
		lba = (lba << 8) | scsi_cmd[3];
		cnt = scsi_cmd[4];
		if (!cnt)
			cnt = 256;
		printf("scsi: READ6 (lba=%lx cnt=%x) !\n", lba, cnt);
		return scsi_do_read(lba, cnt);
	case 0x0a:
		lba = scsi_cmd[1] & 0x1f;
		lba = (lba << 8) | scsi_cmd[2];
		lba = (lba << 8) | scsi_cmd[3];
		cnt = scsi_cmd[4];
		if (!cnt)
			cnt = 256;
		printf("scsi: WRITE6 (lba=%lx cnt=%x) !\n", lba, cnt);
		return scsi_do_write(lba, cnt);
	case 0x12:
		printf("scsi: INQUIRY !\n");
		fat_inval_cache();
		memset(secbuf, 0, 36);
		memcpy(secbuf + 8, " SEAGATE", 8);
		memcpy(secbuf + 16, "          ST225N", 16);
		secbuf[4] = 32;
		cnt = scsi_cmd[4];
		if (cnt > 36)
			cnt = 36;
		return scsi_do_data_in(cnt);
	case 0x15:
		printf("scsi: MODE_SELECT !\n");
		return 1;
	case 0x1a:
		printf("scsi: MODE_SENSE !\n");
		return 1;
	case 0x1b:
		printf("scsi: START_STOP !\n");
		return 1;
	case 0x25:
		printf("scsi: READ_CAPACITY !\n");
		return 1;
	case 0x28:
		printf("scsi: READ10 !\n");
		lba = scsi_cmd[2];
		lba = (lba << 8) | scsi_cmd[3];
		lba = (lba << 8) | scsi_cmd[4];
		lba = (lba << 8) | scsi_cmd[5];
		cnt = scsi_cmd[7];
		cnt = (cnt << 8) | scsi_cmd[8];
		return scsi_do_read(lba, cnt);
	case 0x2a:
		printf("scsi: WRITE10 !\n");
		lba = scsi_cmd[2];
		lba = (lba << 8) | scsi_cmd[3];
		lba = (lba << 8) | scsi_cmd[4];
		lba = (lba << 8) | scsi_cmd[5];
		cnt = scsi_cmd[7];
		cnt = (cnt << 8) | scsi_cmd[8];
		return scsi_do_write(lba, cnt);
	case 0x2f:
		printf("scsi: VERIFY10 !\n");
		return 10;
	case 0x3c:
		printf("scsi: READ_BUFFER !\n");
		return 6;
	}
	return 1;
}

static void scsi_req_ack(void)
{
	unsigned char l;

	scsi_assert |= MASK(BB_ASSERT_REQ);
	scsi_set_assert();
	do
		l = scsi_get_lines();
	while (!(l & MASK(BB_LINES_ACK)));
}

static void scsi_nreq_nack(void)
{
	unsigned char l;

	scsi_assert &= ~MASK(BB_ASSERT_REQ);
	scsi_set_assert();
	do
		l = scsi_get_lines();
	while (l & MASK(BB_LINES_ACK));
}

void do_scsi(void)
{
	unsigned char lines, val, i, len, stat;

	lines = scsi_get_lines();
	if ((lines & MASK(BB_LINES_SEL)) /*&& !(lines & MASK(BB_LINES_BSY)) */) {
		val = scsi_get_data();
		if (!(val & MASK(6))) {
			//printf("scsi: Wrong ID 0x%x\n", val);
			return;
		}
		scsi_assert = MASK(BB_ASSERT_BSY);
		scsi_set_assert();
	} else if (lines & MASK(BB_LINES_RST)) {
		printf("scsi: RESET !\n");
		return;
	} else {
		printf("scsi: Glitch (reset ?) lines=%x!\n", lines);
		return;
	}
	while(lines & MASK(BB_LINES_SEL))
		lines = scsi_get_lines();
	/* XXX Check for ATN for MESSAGE_OUT phase, Mac ROM doesn't do it */
	scsi_dbg("scsi: Selected, command phase...\n");
	i = 0;
	stat = 0;
	len = 1;
	scsi_assert |= MASK(BB_ASSERT_CD);
	scsi_set_assert();
	while(len) {
		scsi_req_ack();
		val = scsi_get_data();
		scsi_cmd[i++] = scsi_get_data();
		scsi_dbg("scsi: CMD = 0x%x\n", val);
		if (i == 1) {
			if (val == 0x00 || /* TEST_UNIT_READY */
			    val == 0x03 || /* REQUEST_SENSE */
			    val == 0x04 || /* FORMAT_UNIT */
			    val == 0x08 || /* READ6 */
			    val == 0x0a || /* WRITE6 */
			    val == 0x12 || /* INQUIRY */
			    val == 0x15 || /* MODE_SELECT */
			    val == 0x1a || /* MODE_SENSE */
			    val == 0x1b || /* START_STOP */
			    val == 0x3c || /* READ_BUFFER */
			    0)
				len = 6;
			else if (val == 0x25 ||	/* READ_CAPACITY */
				 val == 0x28 || /* READ10 */
				 val == 0x2a || /* WRITE10 */
				 val == 0x2f || /* VERIFY10 */
				 0)
				len = 10;
			else {
				stat = 1;
				goto fail;
			}
		}
		len--;
	fail:
		scsi_nreq_nack();
	}
	scsi_assert &= ~MASK(BB_ASSERT_CD);
	stat = scsi_do_cmd();
	scsi_dbg("Status phase (stat=%x)\n", stat);
	scsi_assert |= MASK(BB_ASSERT_IO) | MASK(BB_ASSERT_CD);
	scsi_set_data(stat);
	scsi_req_ack();
	scsi_nreq_nack();
	scsi_dbg("Message IN phase (stat=%x)\n", stat);
	scsi_assert |= MASK(BB_ASSERT_MSG);
	scsi_set_data(0);
	scsi_req_ack();
	scsi_nreq_nack();
	scsi_assert = 0;
	scsi_set_assert();
}

