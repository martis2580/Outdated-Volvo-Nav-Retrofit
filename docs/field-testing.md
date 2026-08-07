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
