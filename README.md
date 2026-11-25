# SECUREBANK-TERMINAL
# SecureBank ATM Terminal 🏦
### RFID and PIN-Based ATM Transaction Simulator with PC Database Integration

A dual-platform embedded ATM transaction simulation featuring the NXP LPC2148 (ARM7TDMI-S) microcontroller and a Linux-based C application for banking database management.

The system uses RFID-based card identification and PIN authentication to provide secure ATM operations. The LPC2148 handles user interaction through a 4x4 matrix keypad and 16x2 LCD, while communicating with a PC-based banking application through UART serial communication.

This project demonstrates embedded C programming, microcontroller interfacing, UART communication, Linux system programming, data structures, and file handling.

## Table of Contents
- Project Overview
- System Workflow & Block Diagram
- Key Features
- Tech Stack & Hardware
- Pin Configuration
- System Architecture & Communication Protocol
- Repository Structure
- Build & Execution Instructions
- Output Gallery
- Future Enhancements

## Project Overview

SecureBank ATM Terminal connects a physical embedded microcontroller with a PC-based banking database.

The user authenticates using an RFID card and a 4-digit PIN entered through a matrix keypad.

The LPC2148 microcontroller manages the ATM operations and displays information on a 16x2 LCD. It communicates with the Linux-based C application through UART.

The PC-side application validates user credentials, manages account balances, processes transactions, updates user records, and maintains transaction history using file handling and data structures.

### Block Diagram & System Workflow
![Uploading image.png…]()


**1. Block Diagram**

The system consists of the following major components:

- 4x4 Matrix Keypad
- RFID Reader and RFID Cards
- LPC2148 Microcontroller
- 16x2 LCD Display
- Buzzer
- MAX232 UART Level Converter
- PC running Linux

**2. System Workflow**

1. System initializes and waits for an RFID card.
2. RFID reader reads the card number.
3. Card number is transmitted to the PC through UART.
4. PC validates the RFID card.
5. User enters the 4-digit PIN using the keypad.
6. PC verifies the PIN.
7. After successful authentication, the ATM menu is displayed.
8. User selects the required banking operation.
9. Transaction request is sent to the PC.
10. PC processes the request and updates the database.
11. Response is transmitted to the microcontroller.
12. Result is displayed on the LCD.

## Key Features

**RFID-Based Authentication:** Identifies users through their registered RFID cards.

**PIN Authentication:** Provides an additional authentication step using a 4-digit PIN.

**Balance Enquiry:** Retrieves and displays the current account balance.

**Cash Deposit:** Adds the entered amount to the user's account balance.

**Cash Withdrawal:** Checks available balance before processing withdrawals.

**PIN Change:** Allows authenticated users to update their ATM PIN.

**Mini Statement:** Retrieves and displays the three most recent transactions.

**Transaction Logging:** Maintains transaction records using linked lists and file handling.

**UART Communication:** Enables serial data exchange between the LPC2148 and Linux PC.

**Keypad-Based Navigation:** Provides menu selection and numeric input through a 4x4 matrix keypad.

**Timeout Handling:** Returns the system to its initial state if the user does not select an option within 30 seconds.

## Tech Stack & Hardware

### Software & Tools

- Programming Language: Embedded C and C
- Microcontroller IDE: Keil µVision
- Compiler: GCC
- Flashing Tool: Flash Magic
- Operating System: Linux
- Communication Protocol: UART
- Linux Serial Interface: POSIX termios
- Data Management: Structures, Linked Lists and File Handling

### Hardware Components

- NXP LPC2148 Microcontroller (ARM7TDMI-S)
- RFID Reader Module
- RFID Cards
- 16x2 Character LCD Display
- 4x4 Matrix Keypad
- MAX232 Level Converter
- USB-to-UART Converter / DB-9 Cable
- Buzzer

## Pin Configuration

### Hardware Interface Mapping

The project uses the following hardware interfaces:

| Component | Interface |
|---|---|
| RFID Reader | UART1 |
| PC Communication | UART0 |
| LCD Display | GPIO |
| 4x4 Keypad | GPIO |
| Buzzer | GPIO |
| MAX232 | UART level conversion |

Refer to the LPC2148 board configuration and component datasheets for the exact pin connections.

## System Architecture & Communication Protocol

### 1. Microcontroller Firmware

The LPC2148 acts as the ATM front-end controller.

Its responsibilities include:

- Initializing LCD, keypad, RFID and UART peripherals.
- Reading RFID card information through UART1.
- Capturing PIN input through the keypad.
- Displaying authentication and transaction details.
- Sending transaction requests to the PC.
- Receiving responses and displaying results.

### 2. Linux-Based PC Application

The Linux application acts as the banking database and transaction processing system.

Its responsibilities include:

- Initializing UART communication.
- Receiving RFID card numbers.
- Validating card details and PIN.
- Managing user account information.
- Processing deposits and withdrawals.
- Updating account balances.
- Maintaining transaction history.
- Saving updated information using file handling.

### 3. UART Communication Protocol

The LPC2148 and Linux PC exchange data using a custom request-response string protocol.

**MCU to PC – Request Formats**

RFID Card:
```text
#CARD:12345678$
```

PIN Authentication:
```text
#CARD:12345678#PIN:4321$
```

Balance Enquiry:
```text
#TXN:BALANCE#REQ$
```

Deposit:
```text
#TXN:DEPOSIT#AMOUNT:1000$
```

Withdrawal:
```text
#TXN:WITHDRAW#AMOUNT:500$
```

PIN Change:
```text
#PINCHANGE#NEWPIN$
```

Mini Statement:
```text
#MINISTMT#REQ$
```

**PC to MCU – Response Formats**

Successful Authentication:
```text
@OK#VALID_CARD$
@OK#LOGIN_SUCCESS$
```

Balance Response:
```text
@OK#BALANCE:5300$
```

Invalid PIN:
```text
@ERR#INVALID_PIN$
```

Insufficient Balance:
```text
@ERR#INSUFFICIENT_BALANCE$
```

PIN Change Success:
```text
@OK#PINCHANGED$
```

Mini Statement:
```text
@OK#MINISTMT:TXN1|TXN2|TXN3$
```

The `$` character is used as the message termination delimiter.

## Repository Structure

```text
SecureBank-ATM-Terminal/
│
├── firmware/
│   ├── main.c
│   ├── lcd.c
│   ├── lcd.h
│   ├── delay.c
│   ├── delay.h
│   ├── uart.c
│   ├── uart.h
│   ├── keypad.c
│   ├── keypad.h
│   └── rfid.c
│
├── server/
│   ├── main.c
│   ├── uart.c
│   ├── uart.h
│   ├── users.txt
│   └── transactions.txt
│
└── README.md
```

*Note: This is the suggested organization for the complete project. Add the corresponding source files as they are implemented.*

## Build & Execution Instructions

### Phase 1: Linux UART Testing

**Step 1:** Extract the UART loopback ZIP file.

**Step 2:** Open the Linux terminal and navigate to the extracted directory.

**Step 3:** Compile the UART program:

```bash
gcc main.c uart.c -o uart_test
```

**Step 4:** Connect the serial port in loopback configuration.

**Step 5:** Execute the program:

```bash
sudo ./uart_test
```

The uploaded UART test program uses `/dev/ttyS0` with a baud rate of 57600.

It transmits characters from A to Z and reads back the received characters to verify serial communication.

### Phase 2: Microcontroller Firmware Setup

1. Open Keil µVision IDE.
2. Create a new project targeting the LPC2148.
3. Add the required Embedded C source files.
4. Configure the target settings.
5. Compile the project using F7.
6. Generate the HEX output file.
7. Connect the LPC2148 development board.
8. Use Flash Magic to program the HEX file.
9. Reset the board and verify the LCD and peripheral operations.

### Phase 3: PC-Side Banking Application

1. Open the Linux environment.
2. Create the PC-side C application.
3. Configure UART communication using the POSIX termios interface.
4. Prepare the user database file (`users.txt`).
5. Implement RFID and PIN validation.
6. Implement balance enquiry, deposit, withdrawal, PIN change and mini statement operations.
7. Compile the application using GCC.
8. Connect the LPC2148 and PC through the UART interface.
9. Run both applications and verify communication and transaction operations.

## Output Gallery

**1. RFID Authentication:** Reads the RFID card and validates the registered user.

**2. PIN Authentication:** Accepts the 4-digit PIN through the matrix keypad.

**3. Main Menu:** Displays the available banking operations on the LCD.

**4. Balance Enquiry:** Displays the current account balance.

**5. Deposit:** Displays the updated balance after a successful deposit.

**6. Withdrawal:** Displays withdrawal status and remaining balance.

**7. PIN Change:** Displays the result of PIN modification.

**8. Mini Statement:** Displays the latest three transactions.

*Add actual project photographs and LCD screenshots in this section.*

## Future Enhancements

- Implement AES encryption for secure UART communication.
- Replace flat-file storage with an SQLite database.
- Integrate EEPROM for local data storage.
- Add improved error handling and communication timeout detection.
- Implement additional authentication mechanisms.
- Enhance transaction history and reporting features.

---

**Project Title:** SecureBank Terminal: RFID and PIN-Based ATM Transaction Simulator with PC Database Integration

**Microcontroller:** LPC2148 (ARM7TDMI-S)

**Programming Languages:** Embedded C, C

**Communication:** UART

**Development Tools:** Keil µVision, GCC, Flash Magic and Linux
