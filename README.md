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

### read_register()
This function sends a request to the MFRC522 to read the value stored in one of its internal registers. The register address is shifted left and combined with a special bit to tell the chip we want to read. After pulling the chip select (CS) pin low to start communication, we send the request and then read the response byte back from the chip. Once we’re done, we pull CS high to end the communication.

### write_register()
This function writes a value to one of the MFRC522’s internal registers. Like with reading, we format the register address first — this time with a “write” indicator — and send it along with the data we want to store. Pulling CS low starts the SPI communication, and pulling it back high finishes it.

--- 

# Outputs
There will be only two outputs, either it can read the card or it doesnt read the card

### No Card Detected

![IMG_0396](https://github.com/user-attachments/assets/789639bd-510d-4074-aeac-e6aaa3a6e031)

As shown in the output, the RFID reader isn't detecting any nearby cards, so the program continuously prints a message indicating that no card is in range.

---

### Card Detected

![IMG_0398](https://github.com/user-attachments/assets/bc372885-a730-41f1-8c07-a9c0aab46c6f)

As shown in the output, when an RFID card is brought near the reader, the MFRC522 successfully detects it and responds with the ATQA (Answer to Request A) bytes. This confirms that communication with the card is working. The program prints the card's presence and the exact ATQA response over serial. However, since passive RFID tags don’t always stay in an idle state after being detected, the reader may occasionally report “No card detected” between reads. This behavior is normal unless the card is explicitly halted or removed from range.

---

# Credits

This project builds on information from the MFRC522 datasheet and open-source libraries, especially the [MFRC522 Arduino Library by Miguel Balboa](https://github.com/miguelbalboa/rfid).

The low-level RFID logic, initialization routines, and card detection flow were adapted from these sources and rewritten to work with the Raspberry Pi Pico’s C SDK. Debugging and adjustments were guided with assistance.

All SPI communication and register access logic was implemented manually to better understand how the Pico interacts with peripherals at a lower level.
