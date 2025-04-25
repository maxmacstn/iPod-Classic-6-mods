# USB-C Mod
![image info](/images/usb-c-6.jpg)

## Why
As we know, the 30-pin dock connector is an outdated interface that's no longer commonly used. This mod replaces the dock connector with a USB-C port. Keep in mind that you'll lose support for other dock connector features and won't be able to use 30-pin accessories anymore. For me, the convenience of USB-C makes it a no-brainer.

## Design

The dock connector is ancient. How it works is each pin has a specific purpose, e.g. USB, FireWire, line out, serial, etc. Therefore, it has many pins. What I did was tap into the USB D+, D−, VCC, and GND from the dock connector and connect them to a USB-C port. Seems simple, doesn't it?

However, the depth of the USB-C port is longer than the 30-pin dock connector. The PCB part that holds the USB-C port sits over the dock connector solder pads. What I did was create two parts: a solid PCB and a flexible PCB. The solid PCB holds the USB-C connector as well as attaches the PCB itself to the iPod's circuit board. The flexible PCB, aka FPC, connects the corresponding pins from the dock connector solder pads to the solid PCB. The FPC bends 180° to connect to the PCB using a Molex mezzanine connector. Don't forget to put some tape on the bottom of the PCB to avoid shorting to the dock connector pads.

The PCB includes three 0603 resistors—two for enabling charging with USB-C PD adapters, and one (optional) for iPod accessory identification. Additionally, there are four soldering pads connected to the dock connector: Line_L, Line_R (for audio line out), and TX, RX (for serial communication). These are used for other mods, like the Bluetooth audio mod, which I used it.

Finally, the port bezel, attached to the iPod’s back cover, had to be replaced. I designed a new bezel for the USB-C port and 3D printed it using an FDM printer. After some sanding and painting, it now has a nice glossy finish.


## How hard if I want one?
I am not going to sell it. I have provided all the files, including the KiCad project, Gerber files, bill of materials, as well as the 3D file in this repository. If you dare to solder these tiny parts, you’ll need to fabricate them yourself through a PCB manufacturer of your choice.

I'm not a microsoldering expert, but I would say I have intermediate soldering skills. It's still quite challenging. For me, the hardest part was soldering the mezzanine connectors.

## Conclusion
USB-C works perfectly fine. It connects to the computer as well as the car's infotainment system—no issues so far. The charging function and speed are the same as the original. The Apple original charger is still needed for the iPod to display the charging icon, or you can charge via a PC.

## Images

![](/images/usb-c-1.jpg)
![](/images/usb-c-3.jpg)
![](/images/usb-c-7.jpg)
![](/images/usb-c-4.jpg)
