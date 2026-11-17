## FT-001 – Vehicle 12 V Battery Voltage During Engine Cranking

**Status:** ⚠️ Observed

### Test Environment

| Parameter | Value |
| ---------- | ----- |
| Vehicle | Volvo P2 |
| Raspberry Pi | Raspberry Pi 5 |
| Power Supply | DC/DC Converter |
| Vehicle Battery | 7 years old |
| Test Scenario | Ignition ON for ~5 minutes before engine start |

### Summary

During testing, the Raspberry Pi unexpectedly lost power while the engine was being started.

### Cause

After leaving the ignition ON for approximately five minutes, the aging 12 V battery experienced a significant voltage drop during engine cranking.

The input voltage fell below the minimum operating voltage of the DC/DC converter, causing its undervoltage protection to disconnect the output. As a result, the Raspberry Pi lost power without completing a graceful shutdown.

### Impact

- Unexpected Raspberry Pi power loss
- Unclean operating system shutdown
- Possible filesystem corruption after repeated occurrences

### Recommendation

Use a healthy vehicle battery capable of maintaining sufficient voltage during engine cranking.

If the battery is old or has reduced capacity, consider replacing it before installing this retrofit.

> [!NOTE]
> This observation is related to the condition of the vehicle's electrical system and **is not** considered a hardware or software issue with this project.

---


## FT-002 – Raspberry Pi 5 Overheating in Summer Without Additional Cooling

**Status:** ⚠️ Observed

### Test Environment

| Parameter          | Value                                                         |
| ------------------ | ------------------------------------------------------------- |
| Vehicle            | Volvo P2                                                      |
| Raspberry Pi       | Raspberry Pi 5                                                |
| Installation       | Phone module compartment, retrofit bracket                    |
| Power Supply       | DC/DC Converter                                               |
| Additional Cooling | Noctua NF-A4x20 5V 40 mm fan in printed fan cover             |
| Fan Power          | Directly from DC/DC converter (runs whenever the Pi is on)    |
| Monitoring         | Hudiy dashboard (CPU temperature, fan RPM)                    |
| Test Scenario      | Normal daily use in summer, no logged measurements            |

### Summary

In summer, the Raspberry Pi 5 in the phone module compartment required additional cooling.

### Cause

The compartment is small with little ventilation, and cabin temperatures rise significantly in summer, especially in direct sunlight. Passive cooling alone is not sufficient.

### Solution

A fan cover with an integrated 40 mm fan mount adds active airflow:

[Fan Cover for Volvo P2 Phone Module Compartment (Raspberry Pi 5 Retrofit Add-on)](https://www.printables.com/model/1915566-fan-cover-for-volvo-p2-phone-module-compartment-ra)

- Print in ABS or PETG, 50 % infill (PLA softens in a hot car)
- Fan is powered directly from the DC/DC converter, so it runs continuously while the Pi is on and is not temperature-controlled
- The Hudiy dashboard shows CPU temperature and fan RPM for maintenance, to confirm the fan is spinning and catch a failing fan early

### Impact

Without additional cooling, summer heat may cause thermal throttling, reduced stability, and shorter component lifetime.

### Recommendation

Install the fan cover with an active fan in warm climates, and check CPU temperature and fan RPM in Hudiy periodically, especially at the start of summer.

<a href="#"><img src="pics/RPi_cooling_cfd.png" alt="CFD simulation of temperature distribution and heat dissipation in the Raspberry Pi 5 enclosure." width="750"></a><br><sub>CFD simulation of temperature distribution and heat dissipation in the Raspberry Pi 5 enclosure.</sub>

> [!NOTE]
> Qualitative observation only. No temperature logging was performed, so no specific temperature reduction is claimed.

---
