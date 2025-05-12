05/12/2025:

- fixed issue with sleep timer not working
- legacy systray only supported, disabled systray when unavailable

Todo:

- build release version
- open to suggestions or pull request
- also if anyone wants to add different icons for systray, I will embed them in release version(as long as they are my bad ones I won't bother)

# HyerpxAlpha

Linux software for the Hyperx Alpha Wireless.

This is Extremely Alpha, it is hap hazardly being maintained.

Not my main git, Sorry if I don't respond in a timely fashion.

![HyerpxAlpha](assets/Hyperx.png)

Settings persist while not running so can be edited and then closed.

## Description

A simple application to provite the missing features on linux

- Battery Monitoring
- Sleep Timer
- Mic Monitor
- Voice Control

## Dependencies

- wxWidgets
- hidapi

## Installation

To install HyerpxAlpha, follow these steps:

1. Clone the repository:
   ```
   git clone https://github.com/IllicitFrog/HyerpxAlpha.git
   ```
2. Navigate to the project directory:
   ```
   cd HyerpxAlpha
   ```
3. Build the project using CMake:

   ```
   cmake -S . -B build
   cmake --build build
   ```

## Usage

**Permissions are required to access the Hyperx device.**

Editing /etc/udev/rules.d/50-hidraw.rules with:

```
SUBSYSTEMS=="usb", ATTRS{idVendor}=="03f0", MODE="0660", TAG+="uaccess"
```

After installation, you can run the HyerpxAlpha software with the following command:

```
bin/Hyerpx
```

or add --systray to start in tray(only works with legacy systray)

```
bin/Hyperx --systray
```

## Contributing

Contributions are welcome! Please fork this repository and submit pull requests.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
