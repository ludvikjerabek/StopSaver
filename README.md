# Stop Saver

Statically compiled Windows VC binary that stops screensaver, sleep, and inactivity.  

### Requirements:

* Windows x86 or x64

### Standalong Installation

Download the correct version of the application for your system:

[64 Bit Windows](./Standalone/x64/StopSaver.exe)  
[32 Bit Windows](./Standalone/Win32)

Note: Standalone install will not auto launch when you login. This configuration will need to be done manually.

### Installation by MSI Installers

[64 Bit Windows](./Installers/x64)  
[32 Bit Windows](./Installers/Win32)

### Usage

Once Stop Saver is launched you will have a task tray icon as shown below.

![image](https://github.com/user-attachments/assets/3cb5e3b9-0ddb-4cd6-8710-63eea2c86eef)

Click the icon and select the desired state. 

![image](https://github.com/user-attachments/assets/254c01fb-a4ca-4505-a72c-2f67447fef70)

Once you select "Start" you will see the icon change as shown below.

![image](https://github.com/user-attachments/assets/cc75e46f-3e50-4412-accc-987131e3d1e6)

### Safety Features

If the the user session is locked Stop Saver will disable. This is to prevent battery drain as well as preventing sleep when an explicit lock was requested. 

### Performance

Stop Saver is very low on resouce consumption.

![image](https://github.com/user-attachments/assets/13173b5e-dee3-4863-95da-4dcf1c9a439f)

