# bilgeAlarm - User Interface

**[Home](readme.md)** --
**[History](history.md)** --
**[Previous](previous.md)** --
**[Design](design.md)** --
**[Hardware](hardware.md)** --
**[Software](software.md)** --
**UI**

The bilgeAlarm is designed so that it can be completely configured
and controlled via three **pushbuttons** and the **lcd screen** built
into the device.  If your boat has a **wifi network** it can additionally
be connected to that wifi network to present a **WebUI** so that you
can monitor and control it from any web browser, as well as allowing
you to connect to it via **Telnet**.

It can be built with **MQTT** capabilities, so that you can control
and monitor it via **HomeAssistant** or **Mosquito/NodeRed** or any other
**IOT** (Internet of Things) architecture (if you use those on your boat)
for additional UI capabilities, and you can even attach it to things like
*Siri* or *Alexa* for voice control.

Finally there is an **as-yet-unpublished** project that uses a *Raspberry Pi*
to connect it, and any other [**myIOT**](https://github.com/phorton1/Arduino-libraries-myIOT)
device to to the **internet** so that you can monitor and control it
from anywhere in the world, over the internet.

In any case, regardless of how you monitor and control it, the **bilgeAlarm**
presents a **User Interface** that consists of a number of **parameters** that
you can see and set, as well as a variety of **indicators and alarms** that
will inform you if something untoward happens in your bilge.


## External Indicators - Alarm and LED

The bilgeAlarm provides two primary ways of notifying you that there is a problem.

The first is a very loud **car alarm** that will go off in an emergency, as well
as *chirp* for less serious situations.

[![car_alarm.jpg](images/car_alarm.jpg)](https://www.ebay.com/itm/185515812222)

There is also an **external LED** that
you mount someplace that is easily visible that will *light up and flash* different
colors depending on the situation.

The **car alarm** is connected directly to the bilgeAlarm during construction.
I have chosen to mount the **LED** in the salon of my boat where it can also
be seen from the helm.

## Menu/Screen Tree

Any errors/alarms take over the whole screen and all buttons (ERROR_MODE).
Pressing any key will suppress (turn of) the (audible) alarm.
You then have a chance to see the Alarm Message, which cycles through 3 screens.
Pressing any key a second time will clear the alarm condition

Otherwise, by default device is in MAIN_MODE and shows the MAIN_SCREEN
which shows number of times the pump has run in the last HOUR, DAY, and
WEEK, respectively, along with a TIMER (in hhh::mm::ss) since the last run.


### Modes

The UI can be in MAIN_MODE, CONFIG_MODE, DEVICE_MODE, and HISTORY_MODE.
When in MAIN_MODE:

- long pressing the **left** putton will go to config mode
- long pressing the **middle** putton will go to device mode
- long pressing the **right** button will go to history mode

When **in any mode, long pressing the left button will return you to MAIN_MODE**
and the MAIN_SCREEN.

Thereafter, in general, short pressing the **left button increments the screen**
and the other two buttons are used to decrement/increment values and/or execute
commands.

## Main Mode Screens

In Main Mode, pressing the **left button increments the screen**, and,
for commands, the **right button executes the command**.

```
#define SCREEN_MAIN             // readonly - the HOUR, DAY, WEEK, TIMER window
#define SCREEN_WIFI             // readonly - shows the IP address and Wifi Info
#define SCREEN_POWER            // readonly - shows the power to the device
#define SCREEN_RELAY            // command - turn the relay on and off
#define SCREEN_SELFTEST         // command - run through the LEDs and chirp
#define SCREEN_CLEAR_HISTORY    // command - clear the history with confirm
#define SCREEN_REBOOT           // command - reboot the device with confirm
#define SCREEN_FACTORY_RESET    // command - factory reset with confirm
```

Some commands will ask you to confirm the decision.
Pressing the right button will confirm and execute the command.
Pressing any other button will return to the previous screen.

## Config Mode Screens

Likewise config mode consists of a number of screens.  The middle
button (does nothing for readonly or command values or) allows you to
toggle or decrement a value or say "no" to a confirm message.
The right button toggles/increments values or says "yes" to a confirm message

The config mode screens are configured using the **myIOT valueIDs** for the bilgeAlarm,
and currently include the following:

```
ID_DISABLED,			// off or on means alarms are disabled
ID_ERR_RUN_TIME,		// off, or pump1 running seconds for a warning chirp every 10 seconds; default(10)
ID_CRIT_RUN_TIME,		// off, or pump1 running seconds for 5 error chirps every 10 seconds; default(30)
ID_ERR_PER_HOUR,		// off, or number of runs in an hour for a warning chirp; default(4)
ID_ERR_PER_DAY,			// off, or number of runs in the last 24 hours for an error chrip; default(20)
ID_RUN_EMERGENCY,		// off, or how many seconds to run pump1 if pump2 goes on; default(30)
ID_EXTRA_RUN_TIME,		// off, or how many seconds extra to run pump1 anytime it goes on; defualt(5)
ID_EXTRA_RUN_MODE,		// whether to add pump1's extra time AT_START (coincident with it starting) or AFTER_END (after it goes off); default(AT_START)
ID_EXTRA_RUN_DELAY,		// and if AFTER_END, off, or how many milliseconds after pump1 stops should we turn pump1 back on; default(1000)

ID_BACKLIGHT_SECS,      // off, or turn off the backlight after this many seconds of inactivity; min(30) default(off)
ID_MENU_SECS,           // off, or automatically return to the MAIN_MODE and MAIN_SCREEN after no button preesses for this many seconds; default(15)
ID_LED_BRIGHT,			// adjust the brightness of the LEDs on the box; default(10)
ID_EXT_LED_BRIGHT,		// adjust the brighneess of the external LED; default(60)

ID_DEBUG_LEVEL,			// ??? set the Serial Debugging Level
ID_LOG_LEVEL,			// ??? set the Serial Logging level

// These probably should be in DEVICE_MODE

ID_DEVICE_NAME,         // strings are forced to read-only
ID_DEVICE_TYPE,
ID_DEVICE_VERSION,
ID_DEVICE_UUID,

ID_CALIB_12V,			// calibrate the 12V sensor; range(0..2000) default(1000)
ID_CALIB_5V,			// calibrate the 5V sensor; range(0..2000) default(1000)
ID_SENSE_MILLIS,		// how often to check pump state (millis, delay for stateTask); default(10); min(10) due to use of vTaskDelay
ID_PUMP_DEBOUNCE,		// debounce milliseconds for float switches; default(500)
ID_RELAY_DEBOUNCE,		// debounce milliseconds for the relay; default(2000)
ID_SW_THRESHOLD,		// threshold for float switch on/off; range(0..4096) default(9000)
```

The voltage calibration integer values are used as floating point numbers by dividing them by 1000,
and then multiplying the sampled voltage by that ratio.  The voltage dividers are not perfect,
and so these let you calibrate the power supplies (using a voltmeter).





Done! **[Back](readme.md)** to the beginning ...
