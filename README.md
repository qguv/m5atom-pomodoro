# m5atom-pomodoro

Simple pomodoro timer for the m5stack atom device.

Counts out work/break cycles of 47/13 minutes using orange/blue color. You can press the screen to immediately take a break or return to work.

What it does:

- Shows a single orange LED
- Adds orange LEDs one-by-one over 47 minutes until they're all orange
- Switches color of all LEDs to blue and blinks
- Turns each LED off one-by-one over 13 minutes until they're all dark
- Blink orange a few times
- Repeat

Compile and upload:

    make upload

Warning: the LEDs exceed their recommended brightness for a few 20ms intervals during the flashes at the end of each cycle. I found that it couldn't get my attention otherwise.

_Note: Don't mind the commit history; in a past life, it was an ESP8266 gas meter. Then it was a breathalyzer._
