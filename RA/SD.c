#include "SD.h"
#include "stm32f1xx_hal.h"

extern SPI_HandleTypeDef hspi1;

static int low_capacity = 0;

#define SS_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
#define SS_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)

static uint8_t spi_txrx(uint8_t data) {
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi1, &data, &rx, 1, HAL_MAX_DELAY);
    return rx;
}

uint8_t SD_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
    SS_LOW();

    // Wait until SD card is ready (returns 0xFF)
    for (uint8_t i = 0; i < 10; i++) {
        if (spi_txrx(0xFF) == 0xFF) break;
    }

    // Send command
    spi_txrx(cmd);
    spi_txrx((arg >> 24) & 0xFF);
    spi_txrx((arg >> 16) & 0xFF);
    spi_txrx((arg >> 8) & 0xFF);
    spi_txrx(arg & 0xFF);
    spi_txrx(crc);

    // Wait for response (MSB = 0)
    for (uint8_t i = 0; i < 10; i++) {
        uint8_t res = spi_txrx(0xFF);
        if (!(res & 0x80)) {
            SS_HIGH();
            return res;
        }
    }

    SS_HIGH();
    return 0xFF;
}

int SD_begin() {
    uint8_t dummy = 0xFF;

    // Give SD card time to power up
    SS_HIGH();
    for (int i = 0; i < 10; i++) {
        spi_txrx(0xFF);
    }

    // Send CMD0 to reset the card
    for (int i = 0; i < 255; i++) {
        if (SD_command(CMD0, 0, CRC0) == 0x01) break;
        if (i == 254) return 0;
    }

    // CMD8: Check voltage range
    uint8_t r = SD_command(CMD8, 0x000001AA, 0x87);
    if (r != 0x01 && r != 0x05) return 0;

    // CMD58: Read OCR
    if (SD_command(CMD58, 0, CRC58) != 0x01) return 0;

    // Try to initialize with ACMD41 or CMD1
    for (int i = 0; i < 255; i++) {
        if (SD_command(CMD55, 0, CRC55) <= 0x01) {
            if (SD_command(CMD41, 0x40000000, 0x77) == 0x00) break;
        } else {
            if (SD_command(CMD1, 0, CRC1) == 0x00) break;
        }
        if (i == 254) return 0;
    }

    // Set block size to 512 bytes
    if (SD_command(CMD16, 512, 0xFF) != 0x00) return 0;

    // Disable CRC
    SD_command(CMD59, 0, 0xFF);

    return 1;
}

void SD_end() {
    spi_txrx(0xFF); // 8 clock cycles
    SD_command(CMD0, 0, CRC0);
    SS_HIGH();
}

int SD_read(uint32_t address, uint8_t *buffer) {
    if (low_capacity) {
        address <<= 9;
    }

    if (SD_command(CMD17, address, 0xFF) != 0x00) return 0;

    SS_LOW();

    // Wait for data token (0xFE)
    for (int i = 0; i < 255; i++) {
        if (spi_txrx(0xFF) == 0xFE) break;
        if (i == 254) {
            SS_HIGH();
            return 0;
        }
    }

    for (uint16_t i = 0; i < 512; i++) {
        buffer[i] = spi_txrx(0xFF);
    }

    // Discard CRC
    spi_txrx(0xFF);
    spi_txrx(0xFF);

    SS_HIGH();
    return 1;
}

int SD_write(uint32_t address, uint8_t *buffer) {
    if (low_capacity) {
        address <<= 9;
    }

    if (SD_command(CMD24, address, 0xFF) != 0x00) return 0;

    SS_LOW();
    spi_txrx(0xFE); // Start data token

    for (uint16_t i = 0; i < 512; i++) {
        spi_txrx(buffer[i]);
    }

    // Dummy CRC
    spi_txrx(0xFF);
    spi_txrx(0xFF);

    // Check response
    uint8_t response = spi_txrx(0xFF);
    if ((response & 0x1F) != 0x05) {
        SS_HIGH();
        return 0;
    }

    // Wait until not busy
    while (spi_txrx(0xFF) == 0x00);

    SS_HIGH();
    return 1;
}

