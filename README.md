# Outdated-Volvo-P2-Nav-Retrofit-Guide-for-Android-Auto-Multimedia
> [!IMPORTANT]
> **Project Status: Work in Progress (WIP)**
...


Designed specifically for Volvo P2 models lacking the factory-installed RTI navigation system. Steering wheel button events are absent from the CAN gateway. Solution: direct LIN bus sniffing between SWM and CEM.
<img src="pics/LIN_bus_sniffing.png" alt="LIN bus sniffing Wiring Diagram" width="1500">
## Decoded Signal Table

| Signal Name   | LIN ID | Data Bytes | Description        | Example Decode |
|--------------|--------|-----------|--------------------|----------------|
| NO_PRESS     | `0x01` | `3F 00`   | No button pressed  | <img src="pics/no_press_LIN.png" width="500"> |
| BTN_NEXT     | `0x01` | `3D 00`   | Next track         | <img src="pics/BTN_NEXT_LIN.png" width="140"> |
| BTN_PREV     | `0x01` | `3E 00`   | Previous track     | <img src="./assets/lin_decode.png" width="140"> |
| BTN_VOL_UP   | `0x01` | `37 00`   | Volume up          | <img src="./assets/lin_decode.png" width="140"> |
| BTN_VOL_DOWN | `0x01` | `3B 00`   | Volume down        | <img src="./assets/lin_decode.png" width="140"> |
| BTN_BACK     | `0x01` | `3F 10`   | Back               | <img src="./assets/lin_decode.png" width="140"> |
| BTN_ENTER    | `0x01` | `3F 20`   | Enter / Select     | <img src="./assets/lin_decode.png" width="140"> |
| BTN_UP       | `0x01` | `3F 08`   | Navigate up        | <img src="./assets/lin_decode.png" width="140"> |
| BTN_DOWN     | `0x01` | `3F 04`   | Navigate down      | <img src="./assets/lin_decode.png" width="140"> |
| BTN_LEFT     | `0x01` | `3F 02`   | Navigate left      | <img src="./assets/lin_decode.png" width="140"> |
| BTN_RIGHT    | `0x01` | `3F 01`   | Navigate right     | <img src="./assets/lin_decode.png" width="140"> |
| CLL_ANSWR    | `0x01` | `2F 00`   | Answer call        | <img src="./assets/lin_decode.png" width="140"> |
| CLL_HNG_UP   | `0x01` | `1F 00`   | Hang up call       | <img src="./assets/lin_decode.png" width="140"> |

---
