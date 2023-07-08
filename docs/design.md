# bilgeAlarm - Design

**[Home](readme.md)** --
**[History](history.md)** --
**[Previous](previous.md)** --
**Design** --
**[Hardware](hardware.md)** --
**[Software](software.md)** --
**[UI](user_interface.md)**

<table>
<tr><td valign='top'><b>REBOOT</b></td><td valign='top'>COMMAND</td><td valign='top'>Reboots the device.</td></tr>
<tr><td valign='top'><b>FACTORY_RESET</b></td><td valign='top'>COMMAND</td><td valign='top'>Performs a Factory Reset of the device, restoring all of the <i>parameters</i> to their initial values and rebooting</td></tr>
<tr><td valign='top'><b>VALUES</b></td><td valign='top'>COMMAND</td><td valign='top'>From Serial or Telnet monitors, shows a list of all of the current <i>parameter</i> <b>values</b></td></tr>
<tr><td valign='top'><b>PARAMS</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>JSON</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>DEVICE_NAME</b></td><td valign='top'>STRING</td><td valign='top'><i>User Modifiable</i> <b>name</b> of the device that will be shown in the <i>WebUI</i>, as the <i>Access Point</i> name, and in </i>SSDP</i> (Service Search and Discovery)
   <br><b>Required</b> (must not be blank)
   <br><i>default</i> : bilgeAlarm</td></tr>
<tr><td valign='top'><b>DEVICE_TYPE</b></td><td valign='top'>STRING</td><td valign='top'>The <b>type</b> of the device as determined by the implementor
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_VERSION</b></td><td valign='top'>STRING</td><td valign='top'>The <b>version number</b> of the device as determined by the implementor
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_UUID</b></td><td valign='top'>STRING</td><td valign='top'>A <b>unique identifier</b> for this device.  The last 12 characters of this are the <i>MAC Address</i> of the device
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_URL</b></td><td valign='top'>STRING</td><td valign='top'>The <b>url</b> of a webpage for this device.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_IP</b></td><td valign='top'>STRING</td><td valign='top'>The most recent Wifi <b>IP address</b> of the device. Assigned by the WiFi router in <i>Station mode</i> or hard-wired in <i>Access Point</i> mode.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEVICE_BOOTING</b></td><td valign='top'>BOOL</td><td valign='top'>A value that indicates that the device is in the process of <b>rebooting</b>
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DEBUG_LEVEL</b></td><td valign='top'>ENUM</td><td valign='top'>Sets the amount of detail that will be shown in the <i>Serial</i> and <i>Telnet</i> output.
   <br><i>allowed</i> : <b>0</b>=NONE, <b>1</b>=USER, <b>2</b>=ERROR, <b>3</b>=WARNING, <b>4</b>=INFO, <b>5</b>=DEBUG, <b>6</b>=VERBOSE
   <br><i>default</i> : <b>5</b>=DEBUG</td></tr>
<tr><td valign='top'><b>LOG_LEVEL</b></td><td valign='top'>ENUM</td><td valign='top'>Sets the amount of detail that will be shown in the <i>Logfile</i> output. <b>Note</b> that a logfile is only created if the device is built with an <b>SD Card</b> on which to store it!!
   <br><i>allowed</i> : <b>0</b>=NONE, <b>1</b>=USER, <b>2</b>=ERROR, <b>3</b>=WARNING, <b>4</b>=INFO, <b>5</b>=DEBUG, <b>6</b>=VERBOSE
   <br><i>default</i> : <b>0</b>=NONE</td></tr>
<tr><td valign='top'><b>LOG_COLORS</b></td><td valign='top'>BOOL</td><td valign='top'>Sends standard <b>ansi color codes</b> to the <i>Serial and Telnet</i> output to highlight <i>errors, warnings,</i> etc
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>LOG_DATE</b></td><td valign='top'>BOOL</td><td valign='top'>Shows the <b>date</b> in Logfile and Serial output
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>LOG_TIME</b></td><td valign='top'>BOOL</td><td valign='top'>Shows the current <b>time</b>, including <i>milliseconds</i> in Logfile and Serial output
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>LOG_MEM</b></td><td valign='top'>BOOL</td><td valign='top'>Shows the <i>current</i> and <i>least</i> <b>memory available</b>, in <i>KB</i>, on the ESP32, in Logfile and Serial output
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>WIFI</b></td><td valign='top'>BOOL</td><td valign='top'>Turns the device's <b>Wifi</b> on and off
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>AP_PASS</b></td><td valign='top'>STRING</td><td valign='top'>The <i>encrypted</i> <b>Password</b> for the <i>Access Point</i> when in AP mode
   <br><i>default</i> : 11111111</td></tr>
<tr><td valign='top'><b>STA_SSID</b></td><td valign='top'>STRING</td><td valign='top'>The <b>SSID</b> (name) of the WiFi network the device will attempt to connect to as a <i>Station</i>.  Setting this to <b>blank</b> force the device into <i>AP</i> (Access Point) mode</td></tr>
<tr><td valign='top'><b>STA_PASS</b></td><td valign='top'>STRING</td><td valign='top'>The <i>encrypted</i> <b>Password</b> for connecting in <i>STA</i> (Station) mode</td></tr>
<tr><td valign='top'><b>SSDP</b></td><td valign='top'>BOOL</td><td valign='top'>Turns <b>SSDP</b> (Service Search and Discovery Protocol) on and off.  SSDP allows a device attached to Wifi in <i>Station mode</i> to be found by other devices on the LAN (Local Area Network). Examples include the <b>Network tab</b> in <i>Windows Explorer</i> on a <b>Windows</b>
   <br><i>default</i> : <b>1</b>=on</td></tr>
<tr><td valign='top'><b>TIMEZONE</b></td><td valign='top'>ENUM</td><td valign='top'>Sets the <b>timezone</b> for the RTC (Real Time Clock) when connected to WiFi in <i>Station mode</i>. There is a very limited set of timezones currently implemented.
   <br><i>allowed</i> : <b>0</b>=EST - Panama, <b>1</b>=EDT - New York, <b>2</b>=CDT - Chicago, <b>3</b>=MST - Phoenix, <b>4</b>=MDT - Denver, <b>5</b>=PDT - Los Angeles
   <br><i>default</i> : <b>0</b>=EST - Panama</td></tr>
<tr><td valign='top'><b>NTP_SERVER</b></td><td valign='top'>STRING</td><td valign='top'>Specifies the NTP (Network Time Protocol) <b>Server</b> that will be used when connected to Wifi as a <i>Station</i>
   <br><i>default</i> : pool.ntp.org</td></tr>
<tr><td valign='top'><b>LAST_BOOT</b></td><td valign='top'>TIME</td><td valign='top'>The <b>time</b> at which the device was last rebooted.
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>UPTIME</b></td><td valign='top'>INT</td><td valign='top'>LAST_BOOT value as integer seconds since Jan 1, 1970.  Displayed as he number of <i>hours, minutes, and seconds</i> since the device was last rebooted in the WebUI
   <br><i>Readonly</i></td></tr>
<tr><td valign='top'><b>RESET_COUNT</b></td><td valign='top'>INT</td><td valign='top'>The number of times the <b>Factory Reset</b> command has been issued on this device
   <br><i>default</i> : <b>0</b></td></tr>
<tr><td valign='top'><b>AUTO_REBOOT</b></td><td valign='top'>INT</td><td valign='top'>How often, in <b>seconds</b> to automatically <b>reboot the device</b>.
   <br><i>default</i> : <b>0</b>=off&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 1000</td></tr>
<tr><td valign='top'><b>SELF_TEST</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>SUPPRESS_ALARM</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>CLEAR_ERROR</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>CLEAR_HISTORY</b></td><td valign='top'>COMMAND</td><td valign='top'></td></tr>
<tr><td valign='top'><b>STATE</b></td><td valign='top'>BENUM</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>ALARM_STATE</b></td><td valign='top'>BENUM</td><td valign='top'><i>allowed</i> : <b>0</b>=ERROR, <b>1</b>=CRITICAL, <b>2</b>=EMERGENCY, <b>3</b>=SUPPRESSED
   <br><i>default</i> : <b>0</b>
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>TIME_LAST_RUN</b></td><td valign='top'>TIME</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>SINCE_LAST_RUN</b></td><td valign='top'>INT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>DUR_LAST_RUN</b></td><td valign='top'>INT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>NUM_LAST_HOUR</b></td><td valign='top'>INT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>NUM_LAST_DAY</b></td><td valign='top'>INT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>NUM_LAST_WEEK</b></td><td valign='top'>INT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>ALARMS</b></td><td valign='top'>ENUM</td><td valign='top'><i>allowed</i> : <b>0</b>=enabled, <b>1</b>=silent, <b>2</b>=disabled
   <br><i>default</i> : <b>0</b>=enabled</td></tr>
<tr><td valign='top'><b>ERR_RUN_TIME</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>10</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3600</td></tr>
<tr><td valign='top'><b>CRIT_RUN_TIME</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>30</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3600</td></tr>
<tr><td valign='top'><b>ERR_PER_HOUR</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>4</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>ERR_PER_DAY</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>20</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>RUN_EMERGENCY</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>30</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3600</td></tr>
<tr><td valign='top'><b>EXTRA_RUN_TIME</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>5</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 3600</td></tr>
<tr><td valign='top'><b>EXTRA_RUN_MODE</b></td><td valign='top'>ENUM</td><td valign='top'><i>allowed</i> : <b>0</b>=from_start, <b>1</b>=after_end
   <br><i>default</i> : <b>0</b>=from_start</td></tr>
<tr><td valign='top'><b>EXTRA_RUN_DELAY</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>1000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 5&nbsp;&nbsp;&nbsp;<i>max</i> : 30000</td></tr>
<tr><td valign='top'><b>SENSE_MILLIS</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>10</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 5&nbsp;&nbsp;&nbsp;<i>max</i> : 30000</td></tr>
<tr><td valign='top'><b>PUMP_DEBOUNCE</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>500</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 5&nbsp;&nbsp;&nbsp;<i>max</i> : 30000</td></tr>
<tr><td valign='top'><b>RELAY_DEBOUNCE</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>2000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 5&nbsp;&nbsp;&nbsp;<i>max</i> : 30000</td></tr>
<tr><td valign='top'><b>SW_THRESHOLD</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>900</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 5&nbsp;&nbsp;&nbsp;<i>max</i> : 4095</td></tr>
<tr><td valign='top'><b>BACKLIGHT_SECS</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>0</b>=off&nbsp;&nbsp;&nbsp;<i>min</i> : 30&nbsp;&nbsp;&nbsp;<i>max</i> : 3600</td></tr>
<tr><td valign='top'><b>MENU_SECS</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>15</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 15&nbsp;&nbsp;&nbsp;<i>max</i> : 3600</td></tr>
<tr><td valign='top'><b>LED_BRIGHT</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>10</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>EXT_LED_BRIGHT</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>10</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 0=off&nbsp;&nbsp;&nbsp;<i>max</i> : 255</td></tr>
<tr><td valign='top'><b>FORCE_RELAY</b></td><td valign='top'>BOOL</td><td valign='top'><i>default</i> : <b>0</b>=off
   <br><i>Memory Only</i></td></tr>
<tr><td valign='top'><b>POWER_12V</b></td><td valign='top'>FLOAT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>POWER_5V</b></td><td valign='top'>FLOAT</td><td valign='top'><i>Readonly</i></td></tr>
<tr><td valign='top'><b>CALIB_12V</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>1000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 500&nbsp;&nbsp;&nbsp;<i>max</i> : 1500</td></tr>
<tr><td valign='top'><b>CALIB_5V</b></td><td valign='top'>INT</td><td valign='top'><i>default</i> : <b>1000</b>&nbsp;&nbsp;&nbsp;<i>min</i> : 500&nbsp;&nbsp;&nbsp;<i>max</i> : 1500</td></tr>
<tr><td valign='top'><b>HISTORY_LINK</b></td><td valign='top'>STRING</td><td valign='top'><i>Readonly</i></td></tr>
</table>


Next: The **[hardware](hardware.md)** schematics and electronics for the **bilgeAlarm** ...
