***Volvo LIN Bridge Setup Instructions
This is the full step-by-step setup we built from the chat.

1. Connect to the Raspberry Pi via SSH
From your computer:
```markdown
ssh volvopi@192.168.x.x
```
2. Confirm the Pi sees the USBlini
Plug in the adapter, then run:
```markdown
lsusb
```
You should see USBlini, for example:
```markdown
04d8:e870 Microchip Technology, Inc. USBlini
```
You can also check:
```markdown
dmesg | tail -n 30
```
3. Install required packages
On the Pi:
```markdown
sudo apt update
sudo apt install -y python3-full python3-venv python3-evdev
```
If python3-evdev is not available from apt, install it later inside the venv with pip.

4. Create the virtual environment
```markdown
python3 -m venv ~/volvo-lin-venv
source ~/volvo-lin-venv/bin/activate
```
You should now see (volvo-lin-venv) in the prompt.

5. Install the Python libraries
Inside the virtual environment:
```markdown
python -m pip install --upgrade pip
python -m pip install git+https://github.com/EmbedME/pyUSBlini
python -m pip install evdev
```
Test the imports:
```markdown
python -c "from usblini import USBlini; print('usblini import ok')"
python -c "from evdev import UInput, ecodes; print('evdev ok')"
```
If pyUSBlini shows as not found but from usblini import USBlini works, that is fine. The installed import name is usblini.

6. Copy the bridge files to the Pi
   
Create the folder:
```markdown
mkdir -p ~/volvo_linbus_pi_bridge
```
Copy the bridge project into it from your computer.
The folder should contain:
```markdown
bridge.py
README.md
requirements.txt
```

7. Run the bridge manually first
This is the command that worked in your session:
```markdown
sudo /home/volvopi/volvo-lin-venv/bin/python /home/volvopi/volvo_linbus_pi_bridge/bridge.py --frame-id 0x01 --baudrate 9600 --verbose
```

8. Fix uinput if needed
If the bridge throws:
"/dev/uinput" cannot be opened for writing

then load the kernel module:
```markdown
sudo modprobe uinput
```
To load it automatically on boot:
```markdown
echo uinput | sudo tee /etc/modules-load.d/uinput.conf
```
Then rerun the bridge with sudo.

9. Create the systemd service
Create the service file:
```markdown
sudo nano /etc/systemd/system/volvo-lin-bridge.service
```

Paste this:
```markdown
[Unit]
Description=Volvo LIN to Hudiy bridge
After=multi-user.target

[Service]
Type=simple
User=root
WorkingDirectory=/home/volvopi/volvo_linbus_pi_bridge
ExecStart=/home/volvopi/volvo-lin-venv/bin/python /home/volvopi/volvo_linbus_pi_bridge/bridge.py --frame-id 0x01 --baudrate 9600 --verbose
Restart=always
RestartSec=2

[Install]
WantedBy=multi-user.target
```
Save and exit.

10. Enable the service
Reload systemd:
```markdown
sudo systemctl daemon-reload
```

Enable auto-start:
```markdown
sudo systemctl enable volvo-lin-bridge.service
```

Start it now:
```markdown
sudo systemctl start volvo-lin-bridge.service
```

Check status:
```markdown
systemctl status volvo-lin-bridge.service
```

12. Reboot test
```markdown
sudo reboot
```
After reboot, SSH back in and confirm:

```markdown
systemctl status volvo-lin-bridge.service
```
