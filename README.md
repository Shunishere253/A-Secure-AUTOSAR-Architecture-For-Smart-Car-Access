# A-Secure-AUTOSAR-Architecture-For-Smart-Car-Access
# S32K144 Keyless Access CAN/BLE Project

## Overview

This project implements a keyless access prototype on S32K144 using BLE UART, AES-based challenge-response authentication, and CAN message transmission.

The system flow is:

```
Vehicle locked
-> BLE connected
-> CAN sends periodic 0x5A
-> AES authentication PASS
-> CAN sends one-shot 0xA5 to unlock
-> App sends USER_IN_CAR or auto-lock timeout expires
-> CAN sends one-shot 0xAA to lock
```

Architecture

The project is split into AUTOSAR-style software layers:

```
BSW/
  Platform initialization

CDD/
  JDY-23 BLE UART driver
  Crypto/AES driver

SWC/
  BLE link state tracking
  Security access authentication

src/app/
  Keyless access application state machine

src/services/
  CAN keyless transmit service

include/
  Public headers for application and services
```

BLE Authentication Flow

The Android app starts authentication by sending:
```
00 01 02 03
```
The MCU detects the start sequence:
```
01 02 03
```
Then:
```
MCU -> App: 16-byte random challenge
App -> MCU: AES-128 encrypted challenge
MCU -> App: PASS! or FAIL!
```
After PASS!, the app may send:
```
USER_IN_CAR
```
ASCII bytes:
```
55 53 45 52 5F 49 4E 5F 43 41 52
```

CAN Frames

CAN payload is 8 bytes.
```
Byte 0: command
Byte 1-7: reserved / diagnostic / future data
```
Current command bytes:
```
0x5A: BLE connected, vehicle still locked, authentication not completed
0xA5: Authentication passed, unlock request
0xAA: Lock request
```
Current CAN behavior:
```
Before authentication:
  0x5A is sent periodically only while BLE activity is detected.

After PASS:
  0xA5 is sent once.

After USER_IN_CAR:
  0xAA is sent once.

If USER_IN_CAR is not received after timeout:
  0xAA is sent once.
```

Auto Lock Timeout

After successful authentication, the application enters WAIT_ENTRY state.

If ```USER_IN_CAR``` is not received before timeout, the MCU sends 0xAA once to lock the vehicle.

The timeout is currently loop-counter based, not hardware-timer based.

Security Notes

The current AES challenge-response helps reduce replay attacks because each session uses a new challenge.

Limitations:
```
Replay protection depends on challenge randomness.
Relay attack is not fully solved by AES challenge-response.
rand() is not a cryptographic random generator.
Secret key update is not currently implemented.
```

Recommended future improvements:
```
Use hardware RNG or stronger challenge generation.
Add session counter / nonce.
Use AES-CMAC for authenticated commands.
Add bounded retry for critical CAN one-shot frames.
Use UWB or distance-bounding hardware for relay resistance.
```
Build Notes

Open the project with S32 Design Studio.

Important source folders must be included in the build:
```
BSW/src
CDD/src
SWC/src
src
src/app
src/services
```
If linker errors such as undefined reference appear, check that the corresponding source folder is not excluded from build.

Recommended build steps:
```
Project > Clean
Build Project
```

Coding Convention

Self-written source follows embedded/AUTOSAR-oriented conventions:

```Coding Convention
No tabs
No trailing spaces
Line length <= 250 characters
Static file-scope symbols use s_ prefix
Public APIs validate pointer parameters where applicable
Comments use /* ... */
Headers use guarded include macros
C files are split into Definitions, Prototypes, Variables, Code
Headers are split into Definitions and API
```
Main Runtime Entry

The main loop initializes BSW, CAN service, BLE link, security access, and application modules, then repeatedly calls the application main function.

The BLE/AES flow and CAN flow are designed to run without blocking each other.

