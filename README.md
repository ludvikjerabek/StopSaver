# Stop Saver
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/ludvikjerabek/stopsaver/total)

Stop Saver is a statically compiled Windows VC binary designed to prevent screensaver activation, sleep, and inactivity.

## Why I Created This Tool

I built Stop Saver to solve a common frustration: system lockouts and sleep interruptions while working from home. Many systems automatically disconnect VPN sessions after periods of inactivity, which can disrupt critical tasks like long-running SSH sessions, monitoring, or downloads.

With Stop Saver, your computer stays active and responsive—no manual intervention needed.

### Why I Chose to Build It Myself
* I didn’t trust other online solutions.
* I wanted a simple, lightweight approach.
* I needed a native Win32 C/C++ single-binary solution.

## Requirements

* Windows x86 or x64

## Standalone Installation

Download the appropriate standalone version of the application for your system from the [releases area](https://github.com/ludvikjerabek/StopSaver/releases).

**Note:** The standalone installation will not auto-launch upon login. You will need to configure this manually.

## Installation via MSI Installers

Download the appropriate version of the MSI package for your system from the [releases area](https://github.com/ludvikjerabek/StopSaver/releases).

Once the installation is complete, you can launch Stop Saver from the application menu.

![Application Menu](https://github.com/user-attachments/assets/cbad578c-3362-4b47-8b44-fbecc9f5b112)

## Usage

After launching Stop Saver, you will see a task tray icon as shown below.

![Task Tray Icon](https://github.com/user-attachments/assets/3cb5e3b9-0ddb-4cd6-8710-63eea2c86eef)

Click the icon and select the desired state.

![Select State](https://github.com/user-attachments/assets/254c01fb-a4ca-4505-a72c-2f67447fef70)

Once you select "Start," the icon will change as shown below.

![Start Icon](https://github.com/user-attachments/assets/cc75e46f-3e50-4412-accc-987131e3d1e6)

## Safety Features

If the user session is locked, Stop Saver will disable to prevent battery drain (e.g., closing the laptop lid to travel). There will be an option to enable/disable this behavior in future updates.

## Performance

Stop Saver is very low on resource consumption.

![Performance](https://github.com/user-attachments/assets/13173b5e-dee3-4863-95da-4dcf1c9a439f)
