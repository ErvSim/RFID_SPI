#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdint.h>
#include <stdbool.h>

// SPI and MFRC522 register definitions
#define SPI_PORT spi0
#define SPI_FREQ 1000000 // 1 MHz for initial testing

#define PIN_MISO 16
#define PIN_CS 17
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_RST 21 // RST signal must be low for at least 100ns

// MFRC522 registers (unshifted addresses)
#define CommandReg 0x01
#define ComIrqReg 0x04
#define ErrorReg 0x06
#define FIFODataReg 0x09
#define FIFOLevelReg 0x0A
#define BitFramingReg 0x0D
#define TxControlReg 0x14
#define TxASKReg 0x15
#define ModeReg 0x11

// Timer registers
#define TModeReg 0x2A
#define TPrescalerReg 0x2B
#define TReloadRegH 0x2C
#define TReloadRegL 0x2D

// PICC commands
#define PICC_CMD_REQA 0x26 // REQA command (7-bit frame)

// Function prototypes
uint8_t read_register(uint8_t reg);
void write_register(uint8_t reg, uint8_t data);
void PCD_Reset(void);
void PCD_Init(void);
void PCD_AntennaOn(void);
bool detect_card(void);


int main(void)
{
    stdio_init_all();
    sleep_ms(1000);
    printf("MFRC522 RFID Reader Demo on Pico\n");

    // Initialize SPI.
    spi_init(SPI_PORT, SPI_FREQ);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Initialize CS pin.
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // Initialize RST pin.
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);

    // Reset pulse: pull RST low briefly then high.
    gpio_put(PIN_RST, 0);
    sleep_ms(1);
    gpio_put(PIN_RST, 1);
    sleep_ms(50);

    // Initialize the MFRC522.
    PCD_Init();

    // Check that antenna is on.
    uint8_t txCtrl = read_register(TxControlReg);
    printf("TxControlReg after init: 0x%02X\n", txCtrl);

    while (true)
    {
        if (detect_card())
        {
            printf("Card is in range!\n");
        }
        else
        {
            printf("No card detected.\n");
        }
        sleep_ms(1000);
    }

    return 0;
}
// Reads a register from the MFRC522 via SPI.
// The register address is shifted left 1 bit; the read command sets bit 7.
uint8_t read_register(uint8_t reg)
{
    uint8_t cmd = (reg << 1) | 0x80;
    uint8_t rx;
    uint8_t dummy = 0x00;

    gpio_put(PIN_CS, 0);
    sleep_ms(1);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    // For some versions of the Pico SDK, spi_read_blocking's second parameter is an 8-bit value.
    spi_read_blocking(SPI_PORT, dummy, &rx, 1);
    gpio_put(PIN_CS, 1);
    sleep_ms(1);

    return rx;
}

// Writes a byte to a MFRC522 register.
// The write address is the register shifted left by one with bit7 cleared.
void write_register(uint8_t reg, uint8_t data)
{
    uint8_t cmd = (reg << 1) & 0x7E;
    uint8_t buffer[2] = {cmd, data};

    gpio_put(PIN_CS, 0);
    sleep_ms(1);
    spi_write_blocking(SPI_PORT, buffer, 2);
    gpio_put(PIN_CS, 1);
    sleep_ms(1);
}

// Performs a soft reset by issuing command 0x0F.
void PCD_Reset(void)
{
    write_register(CommandReg, 0x0F);
    sleep_ms(50);
}

// Initializes MFRC522 registers necessary for proper operation.
void PCD_Init(void)
{
    PCD_Reset();

    // Timer: auto-start timer settings for communication timeouts.
    write_register(TModeReg, 0x80);
    write_register(TPrescalerReg, 0xA9);
    write_register(TReloadRegH, 0x03);
    write_register(TReloadRegL, 0xE8);

    // 100% ASK modulation
    write_register(TxASKReg, 0x40);
    // Set to ISO14443A mode.
    write_register(ModeReg, 0x3D);

    // Turn on the antenna.
    PCD_AntennaOn();
}

// Turns the antenna on by enabling TX1 and TX2 if not already enabled.
void PCD_AntennaOn(void)
{
    uint8_t value = read_register(TxControlReg);
    if ((value & 0x03) != 0x03)
    {
        write_register(TxControlReg, value | 0x03);
    }
}

// detect_card performs a REQA sequence and returns true if an ATQA response (2 bytes) is obtained.
bool detect_card(void)
{
    // Step 1: Flush FIFO by setting the FlushBuffer bit (bit 7) of FIFOLevelReg.
    write_register(FIFOLevelReg, 0x80);

    // Step 2: Write REQA command (0x26) into FIFODataReg.
    write_register(FIFODataReg, PICC_CMD_REQA);

    // Step 3: Set BitFramingReg to transmit only 7 bits (TxLastBits = 7).
    write_register(BitFramingReg, 0x07);

    // Step 4: Issue Transceive command (0x0C) by writing to CommandReg.
    write_register(CommandReg, 0x0C);

    // Step 5: Start transmission by setting StartSend bit (bit 7) plus the 7-bit frame.
    write_register(BitFramingReg, 0x87);

    // Wait for a response (polling ComIrqReg for RxIRq or ErrIRq).
    for (int i = 0; i < 100; i++)
    {
        uint8_t irq = read_register(ComIrqReg);
        if (irq & 0x30)
            break;
        sleep_ms(1);
    }

    // Check for errors.
    uint8_t error = read_register(ErrorReg);
    if (error & 0x1B)
        return false;

    // Check FIFO level; for REQA we expect 2 bytes (ATQA).
    uint8_t len = read_register(FIFOLevelReg) & 0x7F;
    if (len != 2)
        return false;

    // Read ATQA from FIFO.
    uint8_t atqa1 = read_register(FIFODataReg);
    uint8_t atqa2 = read_register(FIFODataReg);
    printf("Card detected! ATQA = 0x%02X 0x%02X\n", atqa1, atqa2);

    return true;
}
