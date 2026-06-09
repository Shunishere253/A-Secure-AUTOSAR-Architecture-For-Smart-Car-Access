# S32K144 Keyless Access System

## Overview

This project implements a keyless vehicle access prototype on the NXP S32K144 platform. It combines BLE communication through the JDY-23 module, AES-based challenge-response authentication, and CAN message transmission to coordinate lock and unlock events with an external vehicle node.

The software is organized in an AUTOSAR-oriented structure, separating platform initialization, complex device drivers, software components, application logic, and CAN transmission services.

## System Flow

The current access flow is:

```text
Vehicle locked
-> BLE connection detected
-> CAN periodically transmits 0x5A
-> Android app starts AES authentication
-> MCU sends a random challenge
-> Android app returns the encrypted challenge
-> MCU verifies the response
-> CAN transmits one-shot 0xA5 to unlock
-> Android app sends USER_IN_CAR or the auto-lock timeout expires
-> CAN transmits one-shot 0xAA to lock
```

## Software Architecture

```text
BSW/
  Platform and low-level initialization

CDD/
  JDY-23 BLE UART driver
  Crypto and AES services

SWC/
  BLE link state management
  Security access authentication

src/app/
  Keyless access application state machine

src/services/
  CAN keyless transmit service

include/
  Public application and service headers
```

## BLE Authentication

The Android app starts authentication by sending:

```text
00 01 02 03
```

The MCU detects the start sequence:

```text
01 02 03
```

Authentication sequence:

```text
MCU -> App: 16-byte challenge
App -> MCU: AES-128 encrypted challenge
MCU -> App: PASS! or FAIL!
```

After successful authentication, the app may send:

```text
USER_IN_CAR
```

ASCII payload:

```text
55 53 45 52 5F 49 4E 5F 43 41 52
```

## CAN Protocol

CAN payload length is 8 bytes.

```text
Byte 0: Command
Byte 1-7: Reserved for diagnostics or future features
```

Supported command bytes:

```text
0x5A: BLE connected, vehicle locked, authentication pending
0xA5: Authentication passed, unlock request
0xAA: Lock request
```

Current behavior:

```text
0x5A is transmitted periodically while BLE activity is detected and authentication is not completed.
0xA5 is transmitted once after successful authentication.
0xAA is transmitted once when USER_IN_CAR is received or when the auto-lock timeout expires.
```

## Auto-Lock Behavior

After successful authentication, the system enters the entry-wait state.

If `USER_IN_CAR` is not received before the configured timeout, the MCU sends a one-shot `0xAA` CAN frame to lock the vehicle.

The current timeout is loop-counter based. For production use, this should be migrated to a hardware timer or OS tick source.

## Security Notes

The current AES challenge-response mechanism provides basic replay protection by using a new challenge for each authentication session.

Current limitations:

```text
Random challenge generation currently depends on software entropy.
Replay protection depends on challenge uniqueness.
Relay attacks are not fully mitigated by AES challenge-response alone.
Runtime secret key update is not currently implemented.
```

Recommended security improvements:

```text
Use a hardware RNG or stronger challenge generation.
Add session counters or nonces.
Authenticate post-PASS commands with AES-CMAC.
Store user keys in non-volatile memory with versioning and rollback protection.
Use UWB or distance-bounding hardware for relay attack mitigation.
```

## Build Notes

This project is intended to be opened and built with S32 Design Studio.

Required source folders:

```text
BSW/src
CDD/src
SWC/src
src
src/app
src/services
```

If linker errors such as `undefined reference` occur, verify that all custom source folders are included in the active build configuration.

Recommended build steps:

```text
Project > Clean
Build Project
```

## Coding Standard

Self-written code follows an embedded AUTOSAR-oriented convention:

```text
No tab characters
No trailing spaces
Maximum line width: 250 characters
Static file-scope symbols use s_ prefix
Pointer parameters are validated where applicable
Code comments use /* ... */
Header files use guarded include macros
C files are organized into Definitions, Prototypes, Variables, and Code sections
Header files are organized into Definitions and API sections
```

## Current Status

Implemented features:

```text
BLE UART communication
AES-128 challenge-response authentication
CAN keyless access frame transmission
One-shot unlock and lock CAN messages
Auto-lock timeout after successful authentication
USER_IN_CAR command handling
AUTOSAR-style source organization
Basic defensive parameter validation
```

## Notes

The project is currently a prototype-oriented embedded implementation. Before production use, the security model, random number generation, key storage, relay attack resistance, and timing supervision should be hardened.

