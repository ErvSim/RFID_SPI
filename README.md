# MFRC522 RFID Reader on Raspberry Pi Pico (C SDK)
This project connects an MFRC522 RFID reader to a Raspberry Pi Pico using SPI. When an RFID card is placed near the reader, the program sends a REQA command and prints the card's ATQA response over serial.

Everything is written in C using the official Pico SDK. No Arduino libraries or wrapper code — just direct register-level communication.

I wrote the low-level SPI functions (read_register() and write_register()) to talk to the MFRC522. The rest of the logic (initialization, timing, REQA/ATQA command flow, etc.) was adapted from existing Arduino examples and ported to work with the Pico’s SDK and timing model.

This was mostly a learning project for understanding SPI and hardware-level device communication. It’s not a full RFID stack — just the basics of getting the card detected and confirming communication is working.

--- 

The module is wired to SPI0 on the Pico and powered with 3.3V. Output is printed over USB serial.

### Wiring
	
	
| MFRC522 Pin  | Pico GPIO |
|--------------|-----------|
| SDA (CS)     | GPIO 17   |
| SCK          | GPIO 18   |
| MOSI         | GPIO 19   |
| MISO         | GPIO 16   |
| RST          | GPIO 21   |
| VCC          | 3.3V      |
| GND          | GND       |

--- 

![image](https://github.com/user-attachments/assets/19fa6276-0f14-4e63-a0c3-d073e7b1ee92)
