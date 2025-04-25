# Bluetooth Audio Mod
The iPod Classic does not have any built-in radio. The only way to listen to music is by physically connecting to the 3.5mm audio jack or the dock connector.

For the Bluetooth mod, people usually use off-the-shelf Bluetooth transmitter modules connected to the headphone output. This method works, but the audio quality isn't the best, and you can't control playback via Bluetooth headphones or speakers.

Instead of using an off-the-shelf Bluetooth transmitter module, I made my own Bluetooth board for this purpose. The main controller is an ESP32.

## Version 1 - Music via I2S
![](/images/bluetooth-1.png)

When the iPod produces sound, it sends a digital audio signal from the CPU to the DAC via the I2S protocol. The digital signal is then converted into an analog signal, which is the sound we hear through headphones. To preserve the original sound quality, tapping into the I2S signal is the way to go.

I didn’t coin the idea of hijacking the I2S signal from the iPod. I found [this really cool project](https://github.com/lemonjesus/iPodBluetooth) by @lemonjesus, where he successfully did this with his iPod Nano 3rd gen. I wanted to apply this concept to my iPod Classic 6th gen as well.

From my research, the iPod Classic 6th gen uses a DAC from Cirrus Logic — allegedly the CS42L55. I checked the pinout from the datasheet, and it matches.

![](/images/IPOD_WORD.jpg)

Long story short, this version of the iPod uses a not-so-standard I2S protocol. While the signal is technically I2S, the BCLK line runs at a very high speed (12 MHz) and also acts as MCLK. The word length is also not 16-bit. After months of trying, I wasn’t able to get it working properly with the ESP32. I gave up using I2S as the audio source from the iPod Classic.

However, I bought a Nano 3rd gen, and my Bluetooth board works as expected with it.

## Version 2 - Music via Line Out
![](/images/esp-2.jpg)

### Design 
Instead of using I2S, I used the line-out signal from the dock connector. The analog audio signal from the line-out is converted back to digital I2S using an ADC—specifically the PCM1860.

For playback control, external accessories can control the iPod via the dock connector using a serial protocol. There are a bunch of [examples online](https://github.com/finsprings/arduinaap). I’ve already prepared breakout pads for both the line-out and serial connections from the USB-C mod.

### Sound Quality
Definitely not as good as the original source since the audio goes through double conversion. I would say the sound quality is neither great nor terrible — it's acceptable. When listening through high-quality headphones or speakers, I can hear very subtle digital noise from interference. I even tried connecting the line-out directly from the DAC IC, but the noise was still there. So, I applied a simple filter: if the signal amplitude is very low, I set it to zero. I think this is good enough. The noise is mostly inaudible when music is playing.

### Power
I soldered the VBATT line after it passes through the power management IC. As a result, it's powered off along with the iPod. However, it doesn’t sleep when the iPod sleeps. So I implemented a check on the audio signal — if it's silent for five minutes, the Bluetooth board goes into deep sleep. There is still a small current draw, which drains the iPod’s battery in a few days. Initially, I used the 1.8V bus to enable the ESP32's power regulator, but that method stopped working for some reason, and I’ve been too lazy to fix it.

The original battery gives around 2 hours of Bluetooth music playback. This is just a proof of concept, and the battery life isn’t great.

### Bluetooth usage
I intended to not modify any sofware on the iPod, so how I could pair bluetooth device from the iPod, here's the step.
1. Restart the iPod by holding `SELECT` + `MENU`. This cuts off the power to the iPod, as well as the bluetooth board.
2. Once connected to the power, the bluetooth board starts discovering bluetooth audio device, then connected to the nearest (strongest signal) one.
3. Enjoy the music via bluetooth.
4. If there's no audio for 5 minutes, the Bluetooth board (ESP32) goes into deep sleep—until power is cut and reconnected.

### Signal
The iPod Classic has a metal front and back plate, which basically makes it a Faraday cage — signal can’t pass through easily. I removed the metal support underneath the display, which allows the signal to pass through a bit, but the range is still very limited (<1m). This could be easily fixed by 3D printing a plastic back cover—but that’s for later.

***TBC***

### Images
![](/images/nanog3-i2s.jpg)
![](/images/bluetooth-4.png)