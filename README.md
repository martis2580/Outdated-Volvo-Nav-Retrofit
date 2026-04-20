# Outdated-Volvo-P2-Nav-Retrofit-Guide-for-Android-Auto-Multimedia
> [!IMPORTANT]
> **Project Status: Work in Progress (WIP)**
...

Designed specifically for Volvo P2 models lacking the factory-installed RTI navigation system. 

The goal of this project is to retrofit the stock low-res RTI monitor with its simple non-touch LCD. Since there’s no touchscreen, the steering wheel buttons are essential for navigating and controlling the display. There are two ways to get those buttons in place: replace the entire wheel, or fit just the nav button module. I chose the second option for this pilot build.

<img src="pics/Steering_wheel_nav_button.jpg" alt="LIN bus sniffing Wiring Diagram" width="700">

Steering wheel button events are absent from the CAN gateway. Solution: direct LIN bus sniffing between SWM and CEM.
<img src="pics/LIN_bus_sniffing.png" alt="LIN bus sniffing Wiring Diagram" width="1500">

### Vehicle-side connector pinout specifications.

|| Location | Pin Numbering |
| :---: | :---: | :---: |
|| <img src="pics/SWM_connector.png" width="200"> | <img src="pics/SWM_connector_2.png" width="300"> |
| SWM Connector | Vehicle-side connector | Signal type |
| #1 | #1 | - |
| #2 | #2 | Ground |
| #3 | #3 | Control module communication cable CAN_L |
| #4 | #4 | Control module communication cable CAN_H |
| #5 | #5 | - |
| #6 | #6 | - |
| #7 | #7 | Communication cable LIN central electronic module (CEM) |
| #8 | #8 | Power supply |
| #9 | #9 | Windshield wipers |
| #10 | #10 | Horn |

## Decoded Steering Wheel Button Signal Table

| Signal Name   | LIN ID | Data Bytes | Description        | Example Decode |
|--------------|--------|-----------|--------------------|----------------|
| NO_PRESS     | `0x01` | `3F 00`   | No button pressed  | <img src="pics/no_press_LIN.png" width="400"> |
| BTN_NEXT     | `0x01` | `3D 00`   | Next track         | <img src="pics/BTN_NEXT_LIN.png" width="400"> |
| BTN_PREV     | `0x01` | `3E 00`   | Previous track     | <img src="pics/BTN_PREV_LIN.png" width="400"> |
| BTN_VOL_UP   | `0x01` | `37 00`   | Volume up          | <img src="pics/BTN_VOL_UP_LIN.png" width="400"> |
| BTN_VOL_DOWN | `0x01` | `3B 00`   | Volume down        | <img src="pics/BTN_VOL_DOWN_LIN.png" width="400"> |
| BTN_BACK     | `0x01` | `3F 10`   | Back               | <img src="pics/BTN_BACK_LIN.png" width="400"> |
| BTN_ENTER    | `0x01` | `3F 20`   | Enter / Select     | <img src="pics/BTN_ENTER_LIN.png" width="400"> |
| BTN_UP       | `0x01` | `3F 08`   | Navigate up        | <img src="pics/BTN_UP_LIN.png" width="400"> |
| BTN_DOWN     | `0x01` | `3F 04`   | Navigate down      | <img src="pics/BTN_DOWN_LIN.png" width="400"> |
| BTN_LEFT     | `0x01` | `3F 02`   | Navigate left      | <img src="pics/BTN_LEFT_LIN.png" width="400"> |
| BTN_RIGHT    | `0x01` | `3F 01`   | Navigate right     | <img src="pics/BTN_RIGHT_LIN.png" width="400"> |
| CLL_ANSWR    | `0x01` | `2F 00`   | Answer call        | <img src="pics/CLL_ANSWR_LIN.png" width="400"> |
| CLL_HNG_UP   | `0x01` | `1F 00`   | Hang up call       | <img src="pics/CLL_HNG_UP_LIN.png" width="400"> |

This data is reverse-engineered and may vary depending on vehicle model or manufacturer.

All LIN bus data in this project was captured using the  
**[USBlini USB-to-LIN interface](https://www.fischl.de/usblini/)**

### Software Support
- Python library: **[pyUSBlini](https://github.com/EmbedME/pyUSBlini)**
