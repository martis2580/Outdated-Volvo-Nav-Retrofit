# Raspberry Pi performance dashboard for HUDIY

This feature adds a Raspberry Pi performance dashboard to the Volvo HUDIY
retrofit. It displays four live readings in a custom HUDIY web widget:

- CPU utilization
- RAM utilization and used/total memory
- Raspberry Pi SoC temperature
- Cooling fan speed in RPM

The dashboard also keeps HUDIY's built-in date, now playing, and phone widgets.
The performance widget uses the `medium_narrow` size and the dashboard action
is `rpi_performance_dashboard`.

## How it works

The implementation has three parts:

1. **HUDIY dashboard configuration** — `hudiy/config/dashboards.json` sets the
   performance dashboard as the default. It places the date, now playing,
   performance, and phone widgets in that order. The custom performance widget
   is a `web_static` widget that loads `http://127.0.0.1:8765/`.
2. **Local metrics service** — `hudiy/dashboard/metrics_server.py` serves the
   dashboard page and a JSON endpoint at `/api/metrics`. It binds to
   `127.0.0.1:8765`, so the service is available to HUDIY on the Pi and is not
   exposed on the vehicle hotspot.
3. **Dashboard page** — `hudiy/dashboard/rpi_performance.html` requests the
   metrics endpoint every two seconds and updates the four values and bars.
   Its inline SVG symbols and CSS are bundled in the file; it needs no remote
   fonts, JavaScript libraries, or images.

The applications menu configuration adds a **Pi Performance** item that opens
the dashboard action.

## Reading the Raspberry Pi metrics

The service uses Linux system data and the Raspberry Pi utility where
available:

| Reading | Source | Calculation |
| --- | --- | --- |
| CPU | `/proc/stat` | Busy-time percentage calculated from the difference between consecutive aggregate CPU samples. The first sample is `null`. |
| RAM | `/proc/meminfo` | Used memory is `MemTotal - MemAvailable`; the widget displays used percentage and MiB used/total. |
| SoC temperature | `vcgencmd measure_temp`, falling back to `/sys/class/thermal/thermal_zone0/temp` | Displayed in °C. The sysfs value is in millidegrees and is divided by 1,000. |
| Cooler speed | Linux hwmon `fan1_input` | Displayed in RPM when the cooling fan tachometer is exposed by the kernel. If unavailable, the widget says so. |

The service reads but does not change fan settings or fan speed. Raspberry Pi's
fan driver controls the fan according to its configured temperature curve.
Temperature and tachometer availability can vary with the Raspberry Pi OS
image, kernel, and attached cooler. Raspberry Pi documents `vcgencmd` for SoC
temperature readings and tachometer detection for Raspberry Pi 5 fans in its
[frequency and thermal management guide](https://github.com/raspberrypi/documentation/blob/master/documentation/asciidoc/computers/raspberry-pi/frequency-management.adoc).

## API response

`GET http://127.0.0.1:8765/api/metrics` returns JSON similar to:

```json
{
  "cpu_percent": 8.4,
  "memory": {
    "used_percent": 31.2,
    "used_mib": 2470,
    "total_mib": 7936
  },
  "cpu_temp_c": 48.7,
  "fan_rpm": 0,
  "updated_at": 0
}
```

Values may be `null` if a reading is not available. A zero fan reading means
the tachometer is available and the fan is stopped; `null` means no supported
tachometer value could be read.

## Install on the Pi

The dashboard is installed as a per-user systemd service and can be deployed
without a desktop using SSH. The complete copy, configuration merge, service
setup, and verification steps are in the [SSH installation guide](../hudiy/dashboard/README.md).

At a high level, installation:

1. Copies `metrics_server.py` and `rpi_performance.html` to
   `$HOME/.local/share/volvo-hudiy-dashboard/`.
2. Installs `volvo-dashboard.service` under
   `$HOME/.config/systemd/user/` and enables it at boot.
3. Merges the `rpi_performance_dashboard` entry into HUDIY's
   `$HOME/.hudiy/share/config/dashboards.json`, removing the old
   `hudiy_dashboard` entry and preserving any other dashboard actions.
4. Adds or updates the **Pi Performance** item in
   `$HOME/.hudiy/share/config/applications_menu.json`.

The systemd service runs as the HUDIY user, starts Python directly, and has no
third-party Python dependencies. After updating the page or service script on
the Pi, restart it with:

```sh
systemctl --user restart volvo-dashboard.service
```

## Troubleshooting

Check service state and the JSON endpoint over SSH:

```sh
systemctl --user status volvo-dashboard.service --no-pager
curl http://127.0.0.1:8765/api/metrics
```

If the endpoint is unavailable, inspect the service log:

```sh
journalctl --user -u volvo-dashboard.service -n 50 --no-pager
```

If `fan_rpm` is `null`, check whether the active cooler tachometer is present:

```sh
cat /sys/devices/platform/cooling_fan/hwmon/hwmon*/fan1_input
```

If that path does not exist, the kernel may not have detected or exposed the
fan tachometer. The dashboard will still display CPU, RAM, and temperature
readings.
