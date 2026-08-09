# Raspberry Pi Configuration

Configuration and setup notes for the Raspberry Pi used in this project.

---

## RTC Battery Charging

The Raspberry Pi 5 RTC works without battery charging enabled. If a rechargeable RTC battery is installed, charging can be enabled using the following configuration.

### Configuration

Edit the Raspberry Pi configuration file:

```bash
sudo nano /boot/firmware/config.txt
```

Add the following:

```ini
# Enable RTC battery charging
dtparam=rtc_bbat_vchg=3000000
```

Save the file and reboot:

```bash
sudo reboot
```

> **Note:** Use this setting only with a rechargeable RTC battery.
