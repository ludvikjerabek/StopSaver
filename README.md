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

<img width="264" height="192" alt="image" src="https://github.com/user-attachments/assets/7276c0c8-41a4-4091-926d-c1a579b69de9" />

Once you select "Start," the icon will change as shown below.

![Start Icon](https://github.com/user-attachments/assets/cc75e46f-3e50-4412-accc-987131e3d1e6)

## Safety Features

If the user session is locked, Stop Saver will disable to prevent battery drain (e.g., closing the laptop lid to travel).

## Tray Configuration Options

<img width="254" height="133" alt="image" src="https://github.com/user-attachments/assets/6209b9c0-80aa-436d-9c6d-56d9c4e57a83" />
  
* Start automatically on launch - Once the applicaiton lanches, it will automatically be in the started state.
* Restore active state on unlock - If you lock your system, walk away, and unlock. The prior running state will resume.

## User Registry Options (Manual Overrides)

Under the `HKEY_CURRENT_USER\Software\StopSaver` registry

* LogFile - REG_EXPAND_SZ value can be used to change the logfile path. Default is `%USERPOFILE%/stopsaver.log`
* LogLevel - REG_SZ string values trace, debug, info, warn, error, critical, off. Default is `error`
* MaxSizeLogSize - REG_DWORD value max size of logfile in bytes. Default is `10485760` 10MB
* MouseIntervalMs - REG_DWORD value from 1000 to 60000 (in miliseconds). Default is `30000` (30s).
* AutoStartOnLaunch - REG_DWORD value 0 or 1 are associated with the Tray Configuration Option `Start automatically on launch`
* RestoreOnUnlock - REG_DWORD value 0 or 1 are associated with theTray Configuration Option `Restore active state on unlock`

## Performance

Stop Saver is very low on resource consumption. 

<img width="812" height="26" alt="image" src="https://github.com/user-attachments/assets/6bafb8f5-f1af-4ca8-b691-20338b14b05a" />  
  
Note: When the application starts task manager will show higher memory, but it falls after running for a while.   
