/*
 * SD.h
 *
 *  Created on: May 16, 2025
 *      Author: dbran
 */

#ifndef SDCORE_H
#define SDCORE_H

// SD Commands Definition
#define CMD0 0x40
#define CMD1 0x41
#define CMD8 0x48
#define CMD9 0x49
#define CMD10 0x4A
#define CMD12 0x4C
#define CMD16 0x50
#define CMD17 0x51
#define CMD18 0x52
#define CMD23 0x57
#define CMD24 0x58
#define CMD25 0x59
#define CMD41 0x69
#define CMD55 0x77
#define CMD58 0x7A
#define CMD59 0x7B

// SD Commands CRC
// TODO: Complete CRC list
#define CRC0 0x95
#define CRC1 0xF9
#define CRC55 0x65
#define CRC58 0x95



int SD_begin();
void SD_end();
int SD_read(unsigned long address, uint8_t *buffer);
int SD_write(unsigned long address, uint8_t *buffer);
uint8_t SD_command(uint8_t command, unsigned long param, uint8_t crc);

#endif
