EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Device:R R11
U 1 1 61B1B320
P 7500 2450
F 0 "R11" V 7400 2400 50  0000 L CNN
F 1 "10K" V 7500 2450 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7430 2450 50  0001 C CNN
F 3 "~" H 7500 2450 50  0001 C CNN
	1    7500 2450
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R10
U 1 1 61B1BB6A
P 7500 2200
F 0 "R10" V 7400 2150 50  0000 L CNN
F 1 "1K" V 7500 2200 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7430 2200 50  0001 C CNN
F 3 "~" H 7500 2200 50  0001 C CNN
	1    7500 2200
	0    -1   -1   0   
$EndComp
$Comp
L cnc3018_Library:BUCK01 M1
U 1 1 61B26C43
P 2700 3250
F 0 "M1" H 2850 3250 50  0000 C CNN
F 1 "BUCK01" H 2600 3250 50  0000 C CNN
F 2 "0_my_footprints:myMini360BuckConverter" H 2650 3550 50  0001 C CNN
F 3 "" H 2650 3550 50  0001 C CNN
	1    2700 3250
	0    1    1    0   
$EndComp
$Comp
L Device:R R9
U 1 1 61B2FB1D
P 7500 1950
F 0 "R9" V 7400 1900 50  0000 L CNN
F 1 "1K" V 7500 1950 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7430 1950 50  0001 C CNN
F 3 "~" H 7500 1950 50  0001 C CNN
	1    7500 1950
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R8
U 1 1 61B3D87B
P 7500 1700
F 0 "R8" V 7400 1650 50  0000 L CNN
F 1 "1K" V 7500 1700 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 7430 1700 50  0001 C CNN
F 3 "~" H 7500 1700 50  0001 C CNN
	1    7500 1700
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7650 1700 7650 1950
Connection ~ 7650 1950
Wire Wire Line
	7650 1950 7650 2200
Text Notes 850  8450 0    50   ~ 0
LEDS
Text Notes 950  8600 0    50   ~ 0
12V POWER INDICATOR - CYAN>13V, GREEN>12V, YELLOW>11V,  RED if possible  
Text Notes 950  8800 0    50   ~ 0
BUCK POWER INDICATOR - MAGENTA>8V,  GREEN>6.4V, YELLOW>5V,  RED if possible  
Text Notes 950  9000 0    50   ~ 0
PUMP INDICATOR - BLUE if ON
Text Notes 950  9200 0    50   ~ 0
PUMP2 INDICATOR - RED if ON
Text Notes 950  9400 0    50   ~ 0
ALARM INDICATOR - FLASHING RED
$Comp
L power:GND #PWR03
U 1 1 61C56583
P 2150 1350
F 0 "#PWR03" H 2150 1100 50  0001 C CNN
F 1 "GND" H 2155 1177 50  0000 C CNN
F 2 "" H 2150 1350 50  0001 C CNN
F 3 "" H 2150 1350 50  0001 C CNN
	1    2150 1350
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_DIP_x01 SW3
U 1 1 61DE95D4
P 7000 4150
F 0 "SW3" H 7000 4400 50  0000 C CNN
F 1 "BUTTON2" H 7000 4300 50  0000 C CNN
F 2 "0_my_footprints:myButton" H 7000 4150 50  0001 C CNN
F 3 "~" H 7000 4150 50  0001 C CNN
	1    7000 4150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_DIP_x01 SW2
U 1 1 61D638ED
P 7000 3750
F 0 "SW2" H 7000 4000 50  0000 C CNN
F 1 "BUTTON1" H 7000 3900 50  0000 C CNN
F 2 "0_my_footprints:myButton" H 7000 3750 50  0001 C CNN
F 3 "~" H 7000 3750 50  0001 C CNN
	1    7000 3750
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR02
U 1 1 61D92C26
P 2550 3600
F 0 "#PWR02" H 2550 3450 50  0001 C CNN
F 1 "+5V" H 2565 3773 50  0000 C CNN
F 2 "" H 2550 3600 50  0001 C CNN
F 3 "" H 2550 3600 50  0001 C CNN
	1    2550 3600
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR05
U 1 1 61D9E980
P 2850 3600
F 0 "#PWR05" H 2850 3350 50  0001 C CNN
F 1 "GND" H 2855 3427 50  0000 C CNN
F 2 "" H 2850 3600 50  0001 C CNN
F 3 "" H 2850 3600 50  0001 C CNN
	1    2850 3600
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR06
U 1 1 61DD0190
P 2700 4450
F 0 "#PWR06" H 2700 4300 50  0001 C CNN
F 1 "+5V" V 2700 4650 50  0000 C CNN
F 2 "" H 2700 4450 50  0001 C CNN
F 3 "" H 2700 4450 50  0001 C CNN
	1    2700 4450
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR07
U 1 1 61DD1643
P 2700 4650
F 0 "#PWR07" H 2700 4400 50  0001 C CNN
F 1 "GND" V 2700 4450 50  0000 C CNN
F 2 "" H 2700 4650 50  0001 C CNN
F 3 "" H 2700 4650 50  0001 C CNN
	1    2700 4650
	0    -1   -1   0   
$EndComp
$Comp
L power:+5V #PWR08
U 1 1 61DD26D1
P 2700 5300
F 0 "#PWR08" H 2700 5150 50  0001 C CNN
F 1 "+5V" V 2700 5500 50  0000 C CNN
F 2 "" H 2700 5300 50  0001 C CNN
F 3 "" H 2700 5300 50  0001 C CNN
	1    2700 5300
	0    1    1    0   
$EndComp
$Comp
L power:+12V #PWR018
U 1 1 61DE37E9
P 4900 1700
F 0 "#PWR018" H 4900 1550 50  0001 C CNN
F 1 "+12V" H 4915 1873 50  0000 C CNN
F 2 "" H 4900 1700 50  0001 C CNN
F 3 "" H 4900 1700 50  0001 C CNN
	1    4900 1700
	0    1    1    0   
$EndComp
$Comp
L power:+12V #PWR01
U 1 1 61DEF38E
P 2550 2850
F 0 "#PWR01" H 2550 2700 50  0001 C CNN
F 1 "+12V" H 2565 3023 50  0000 C CNN
F 2 "" H 2550 2850 50  0001 C CNN
F 3 "" H 2550 2850 50  0001 C CNN
	1    2550 2850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 61DFC811
P 2850 2850
F 0 "#PWR04" H 2850 2600 50  0001 C CNN
F 1 "GND" H 2855 2677 50  0000 C CNN
F 2 "" H 2850 2850 50  0001 C CNN
F 3 "" H 2850 2850 50  0001 C CNN
	1    2850 2850
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR011
U 1 1 61E1944A
P 3150 3350
F 0 "#PWR011" H 3150 3100 50  0001 C CNN
F 1 "GND" H 3155 3177 50  0000 C CNN
F 2 "" H 3150 3350 50  0001 C CNN
F 3 "" H 3150 3350 50  0001 C CNN
	1    3150 3350
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR010
U 1 1 61E19030
P 3150 3050
F 0 "#PWR010" H 3150 2900 50  0001 C CNN
F 1 "+5V" H 3165 3223 50  0000 C CNN
F 2 "" H 3150 3050 50  0001 C CNN
F 3 "" H 3150 3050 50  0001 C CNN
	1    3150 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C1
U 1 1 61F8466C
P 3150 3200
F 0 "C1" H 3268 3246 50  0000 L CNN
F 1 "47uf" H 3268 3155 50  0000 L CNN
F 2 "0_my_footprints:CP_my100uf" H 3188 3050 50  0001 C CNN
F 3 "~" H 3150 3200 50  0001 C CNN
	1    3150 3200
	1    0    0    -1  
$EndComp
Text GLabel 2150 1700 2    50   Input ~ 0
PUMP2_IN
Text GLabel 7300 3750 2    50   Input ~ 0
BUTTON1
Text GLabel 2700 4550 2    50   Input ~ 0
LED_DATA
Text GLabel 2700 5200 2    50   Input ~ 0
SDA
Text GLabel 2700 5100 2    50   Input ~ 0
SCL
$Comp
L power:+3.3V #PWR017
U 1 1 61F4124A
P 5450 5100
F 0 "#PWR017" H 5450 4950 50  0001 C CNN
F 1 "+3.3V" H 5465 5273 50  0000 C CNN
F 2 "" H 5450 5100 50  0001 C CNN
F 3 "" H 5450 5100 50  0001 C CNN
	1    5450 5100
	-1   0    0    1   
$EndComp
$Comp
L Device:R R6
U 1 1 61B1A43C
P 6600 2450
F 0 "R6" V 6500 2400 50  0000 L CNN
F 1 "10K" V 6600 2450 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6530 2450 50  0001 C CNN
F 3 "~" H 6600 2450 50  0001 C CNN
	1    6600 2450
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R5
U 1 1 61B1B71B
P 6600 2200
F 0 "R5" V 6500 2150 50  0000 L CNN
F 1 "4.7K" V 6600 2200 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6530 2200 50  0001 C CNN
F 3 "~" H 6600 2200 50  0001 C CNN
	1    6600 2200
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R3
U 1 1 61B3D875
P 6600 1700
F 0 "R3" V 6500 1650 50  0000 L CNN
F 1 "4.7K" V 6600 1700 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6530 1700 50  0001 C CNN
F 3 "~" H 6600 1700 50  0001 C CNN
	1    6600 1700
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R4
U 1 1 61B2FB17
P 6600 1950
F 0 "R4" V 6500 1900 50  0000 L CNN
F 1 "4.7K" V 6600 1950 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 6530 1950 50  0001 C CNN
F 3 "~" H 6600 1950 50  0001 C CNN
	1    6600 1950
	0    -1   -1   0   
$EndComp
Text GLabel 6300 1700 0    50   Input ~ 0
PUMP_OUT
Text GLabel 6300 1950 0    50   Input ~ 0
PUMP2_IN
$Comp
L power:+5V #PWR020
U 1 1 620A0133
P 6350 2450
F 0 "#PWR020" H 6350 2300 50  0001 C CNN
F 1 "+5V" V 6365 2578 50  0000 L CNN
F 2 "" H 6350 2450 50  0001 C CNN
F 3 "" H 6350 2450 50  0001 C CNN
	1    6350 2450
	0    -1   -1   0   
$EndComp
Text GLabel 6950 2700 3    50   Input ~ 0
S_POUT
Text GLabel 7050 2700 3    50   Input ~ 0
S_PIN2
Text GLabel 7150 2700 3    50   Input ~ 0
S_12V
Text GLabel 7250 2700 3    50   Input ~ 0
S_5V
Wire Wire Line
	6750 1700 6950 1700
Wire Wire Line
	6750 1950 7050 1950
Wire Wire Line
	6750 2200 7150 2200
Wire Wire Line
	6750 2450 7250 2450
Wire Wire Line
	7650 2200 7650 2450
Connection ~ 7650 2200
Wire Wire Line
	6300 1700 6450 1700
Wire Wire Line
	6300 1950 6450 1950
Wire Wire Line
	6300 2200 6450 2200
Wire Wire Line
	6350 2450 6450 2450
$Comp
L power:GND #PWR022
U 1 1 620D7AD9
P 7650 2700
F 0 "#PWR022" H 7650 2450 50  0001 C CNN
F 1 "GND" H 7655 2527 50  0000 C CNN
F 2 "" H 7650 2700 50  0001 C CNN
F 3 "" H 7650 2700 50  0001 C CNN
	1    7650 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	7650 2450 7650 2700
Connection ~ 7650 2450
Wire Wire Line
	6950 2700 6950 1700
Connection ~ 6950 1700
Wire Wire Line
	6950 1700 7350 1700
Wire Wire Line
	7050 2700 7050 1950
Connection ~ 7050 1950
Wire Wire Line
	7050 1950 7350 1950
Wire Wire Line
	7150 2700 7150 2200
Connection ~ 7150 2200
Wire Wire Line
	7150 2200 7350 2200
Wire Wire Line
	7250 2700 7250 2450
Connection ~ 7250 2450
Wire Wire Line
	7250 2450 7350 2450
Text GLabel 7300 4150 2    50   Input ~ 0
BUTTON2
Text GLabel 7300 4550 2    50   Input ~ 0
BUTTON3
$Comp
L power:GND #PWR026
U 1 1 62146A82
P 9400 2900
F 0 "#PWR026" H 9400 2650 50  0001 C CNN
F 1 "GND" H 9405 2727 50  0000 C CNN
F 2 "" H 9400 2900 50  0001 C CNN
F 3 "" H 9400 2900 50  0001 C CNN
	1    9400 2900
	1    0    0    -1  
$EndComp
Text GLabel 8700 2500 0    50   Input ~ 0
RELAY
Wire Wire Line
	9000 2300 8700 2300
Text GLabel 4050 3600 3    50   Input ~ 0
RELAY
Text GLabel 4850 2600 1    50   Input ~ 0
S_5V
Text GLabel 4950 2600 1    50   Input ~ 0
S_12V
Text GLabel 5050 2600 1    50   Input ~ 0
S_PIN2
Text GLabel 4550 3600 3    50   Input ~ 0
SD_CS
Text GLabel 4650 3600 3    50   Input ~ 0
SCLK
Text GLabel 4750 3600 3    50   Input ~ 0
MISO
Text GLabel 4850 3600 3    50   Input ~ 0
SDA
Text GLabel 5150 3600 3    50   Input ~ 0
SCL
Text GLabel 5250 3600 3    50   Input ~ 0
MOSI
Text GLabel 4250 3600 3    50   Input ~ 0
LED_DATA
$Comp
L power:+3.3V #PWR013
U 1 1 61D8580D
P 3850 3600
F 0 "#PWR013" H 3850 3450 50  0001 C CNN
F 1 "+3.3V" H 3865 3773 50  0000 C CNN
F 2 "" H 3850 3600 50  0001 C CNN
F 3 "" H 3850 3600 50  0001 C CNN
	1    3850 3600
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR015
U 1 1 61D31B08
P 3950 3600
F 0 "#PWR015" H 3950 3350 50  0001 C CNN
F 1 "GND" H 3950 3250 50  0001 C CNN
F 2 "" H 3950 3600 50  0001 C CNN
F 3 "" H 3950 3600 50  0001 C CNN
	1    3950 3600
	1    0    0    -1  
$EndComp
$Comp
L cnc3018_Library:ESP32_DEV_0 U1
U 1 1 61B0E9CA
P 4650 3150
F 0 "U1" H 4500 3350 50  0000 L CNN
F 1 "ESP32_DEV_0" H 4300 3250 50  0000 L CNN
F 2 "0_my_footprints:myESP32DEV0" H 4100 3250 50  0001 C CNN
F 3 "" H 4100 3250 50  0001 C CNN
	1    4650 3150
	1    0    0    -1  
$EndComp
Text GLabel 4750 2600 1    50   Input ~ 0
BUTTON3
Wire Wire Line
	5150 4550 5450 4550
Connection ~ 5150 4550
Wire Wire Line
	5150 4950 5150 4550
Wire Wire Line
	5000 4850 5450 4850
Wire Wire Line
	4950 4750 5450 4750
Wire Wire Line
	4950 4650 5450 4650
Wire Wire Line
	4950 4550 5150 4550
Connection ~ 5450 4950
Wire Wire Line
	5450 5100 5450 4950
Wire Wire Line
	5450 4450 5450 4300
$Comp
L power:GND #PWR016
U 1 1 61F3CAE7
P 5450 4300
F 0 "#PWR016" H 5450 4050 50  0001 C CNN
F 1 "GND" H 5450 3950 50  0001 C CNN
F 2 "" H 5450 4300 50  0001 C CNN
F 3 "" H 5450 4300 50  0001 C CNN
	1    5450 4300
	-1   0    0    1   
$EndComp
Text GLabel 5000 4850 0    50   Input ~ 0
SD_CS
Text GLabel 4950 4750 0    50   Input ~ 0
MOSI
Text GLabel 4950 4650 0    50   Input ~ 0
SCLK
Text GLabel 4950 4550 0    50   Input ~ 0
MISO
$Comp
L Device:R R1
U 1 1 61F2825C
P 5300 4950
F 0 "R1" V 5200 4900 50  0000 L CNN
F 1 "10K" V 5300 4950 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 5230 4950 50  0001 C CNN
F 3 "~" H 5300 4950 50  0001 C CNN
	1    5300 4950
	0    -1   -1   0   
$EndComp
$Comp
L cnc3018_Library:SD_TYP0 M2
U 1 1 61EAA2DF
P 5750 4650
F 0 "M2" V 5754 4928 50  0000 L CNN
F 1 "SD_TYP0" V 5845 4928 50  0000 L CNN
F 2 "0_my_footprints:SDCardReader" H 5550 4800 50  0001 C CNN
F 3 "" H 5550 4800 50  0001 C CNN
	1    5750 4650
	0    1    1    0   
$EndComp
Text GLabel 8500 4550 0    50   Input ~ 0
ALARM
Text GLabel 2150 1800 2    50   Input ~ 0
PUMP_OUT
Text GLabel 2150 1900 2    50   Input ~ 0
PUMP_IN
$Comp
L Relay:SANYOU_SRD_Form_A K1
U 1 1 62368573
P 9200 2000
F 0 "K1" H 9530 2046 50  0000 L CNN
F 1 "RELAY" H 9530 1955 50  0000 L CNN
F 2 "0_my_footprints:myRelay" H 9550 1950 50  0001 L CNN
F 3 "http://www.sanyourelay.ca/public/products/pdf/SRD.pdf" H 9200 2000 50  0001 C CNN
	1    9200 2000
	-1   0    0    1   
$EndComp
Wire Wire Line
	9800 2150 9800 2300
Wire Wire Line
	9800 1850 9800 1700
$Comp
L Diode:1N4001 D2
U 1 1 61BD214F
P 9800 2000
F 0 "D2" V 9750 2150 50  0000 L CNN
F 1 "1N4005" V 9850 2150 50  0000 L CNN
F 2 "0_my_footprints:myDiodeSchotsky" H 9800 1825 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 9800 2000 50  0001 C CNN
	1    9800 2000
	0    1    1    0   
$EndComp
Wire Wire Line
	9000 1700 8700 1700
$Comp
L power:+5V #PWR025
U 1 1 62375244
P 9400 1600
F 0 "#PWR025" H 9400 1450 50  0001 C CNN
F 1 "+5V" H 9415 1773 50  0000 C CNN
F 2 "" H 9400 1600 50  0001 C CNN
F 3 "" H 9400 1600 50  0001 C CNN
	1    9400 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	9400 1600 9400 1700
Wire Wire Line
	9400 1700 9800 1700
Connection ~ 9400 1700
Connection ~ 4200 1700
Wire Wire Line
	4200 1700 4300 1700
Wire Wire Line
	4750 1700 4900 1700
Wire Wire Line
	4200 1700 4200 1600
Wire Wire Line
	4200 1600 4600 1600
Wire Wire Line
	4600 1800 4750 1700
$Comp
L power:+12V #PWR023
U 1 1 61C07060
P 9350 3650
F 0 "#PWR023" H 9350 3500 50  0001 C CNN
F 1 "+12V" H 9365 3823 50  0000 C CNN
F 2 "" H 9350 3650 50  0001 C CNN
F 3 "" H 9350 3650 50  0001 C CNN
	1    9350 3650
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 3750 6700 4150
Connection ~ 6700 4150
Wire Wire Line
	6700 4150 6700 4550
Text GLabel 2150 1250 2    50   Input ~ 0
RAW_12V
Text GLabel 4100 1700 0    50   Input ~ 0
RAW_12V
Wire Wire Line
	4100 1700 4200 1700
Text GLabel 6300 2200 0    50   Input ~ 0
RAW_12V
Wire Wire Line
	9800 2300 9400 2300
Text GLabel 8700 2300 0    50   Input ~ 0
PUMP_OUT
Text GLabel 8700 1700 0    50   Input ~ 0
PUMP_IN
$Comp
L power:+5V #PWR0101
U 1 1 61EFA933
P 3950 5050
F 0 "#PWR0101" H 3950 4900 50  0001 C CNN
F 1 "+5V" H 3965 5223 50  0000 C CNN
F 2 "" H 3950 5050 50  0001 C CNN
F 3 "" H 3950 5050 50  0001 C CNN
	1    3950 5050
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 61EFA939
P 3650 5050
F 0 "#PWR0102" H 3650 4800 50  0001 C CNN
F 1 "GND" H 3655 4877 50  0000 C CNN
F 2 "" H 3650 5050 50  0001 C CNN
F 3 "" H 3650 5050 50  0001 C CNN
	1    3650 5050
	0    1    1    0   
$EndComp
$Comp
L power:+3.3V #PWR019
U 1 1 61F04C27
P 5400 2300
F 0 "#PWR019" H 5400 2150 50  0001 C CNN
F 1 "+3.3V" H 5415 2473 50  0000 C CNN
F 2 "" H 5400 2300 50  0001 C CNN
F 3 "" H 5400 2300 50  0001 C CNN
	1    5400 2300
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C2
U 1 1 61F3EFF1
P 5400 2450
F 0 "C2" H 5650 2500 50  0000 R CNN
F 1 "10uf" H 5650 2600 50  0000 R CNN
F 2 "0_my_footprints:CP_my10uf" H 5438 2300 50  0001 C CNN
F 3 "~" H 5400 2450 50  0001 C CNN
	1    5400 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5250 2600 5400 2600
$Comp
L Device:Q_NPN_CBE Q2
U 1 1 61C3B86C
P 9300 2500
F 0 "Q2" H 9490 2546 50  0000 L CNN
F 1 "BCS547" H 9490 2455 50  0000 L CNN
F 2 "cnc3018-PCB:my3pin" H 9500 2600 50  0001 C CNN
F 3 "~" H 9300 2500 50  0001 C CNN
	1    9300 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	9400 2700 9400 2900
$Comp
L Device:R R13
U 1 1 61BA8B75
P 8850 2500
F 0 "R13" V 8750 2450 50  0000 L CNN
F 1 "1K" V 8850 2500 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 8780 2500 50  0001 C CNN
F 3 "~" H 8850 2500 50  0001 C CNN
	1    8850 2500
	0    1    1    0   
$EndComp
Connection ~ 9400 2300
Wire Wire Line
	9000 2500 9100 2500
$Comp
L Device:R R7
U 1 1 61C579E3
P 9050 4850
F 0 "R7" V 8950 4800 50  0000 L CNN
F 1 "10K" V 9050 4850 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 8980 4850 50  0001 C CNN
F 3 "~" H 9050 4850 50  0001 C CNN
	1    9050 4850
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0103
U 1 1 61C605D5
P 9350 5100
F 0 "#PWR0103" H 9350 4850 50  0001 C CNN
F 1 "GND" H 9355 4927 50  0000 C CNN
F 2 "" H 9350 5100 50  0001 C CNN
F 3 "" H 9350 5100 50  0001 C CNN
	1    9350 5100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 61C62C8B
P 8800 4550
F 0 "R2" V 8700 4500 50  0000 L CNN
F 1 "220" V 8800 4550 50  0000 C CNN
F 2 "0_my_footprints:myResistor" V 8730 4550 50  0001 C CNN
F 3 "~" H 8800 4550 50  0001 C CNN
	1    8800 4550
	0    -1   -1   0   
$EndComp
Wire Wire Line
	9350 4750 9350 5000
Wire Wire Line
	9050 5000 9350 5000
Connection ~ 9350 5000
Wire Wire Line
	9350 5000 9350 5100
Wire Wire Line
	9050 4550 9050 4700
Wire Wire Line
	9050 4550 8950 4550
Connection ~ 9050 4550
$Comp
L Transistor_FET:IRLZ44N Q1
U 1 1 61C50415
P 9250 4550
F 0 "Q1" H 9454 4596 50  0000 L CNN
F 1 "IRLZ44N" H 9454 4505 50  0000 L CNN
F 2 "0_my_footprints:myTO-220-3" H 9500 4475 50  0001 L CIN
F 3 "http://www.irf.com/product-info/datasheets/data/irlz44n.pdf" H 9250 4550 50  0001 L CNN
	1    9250 4550
	1    0    0    -1  
$EndComp
$Comp
L Diode:1N4001 D1
U 1 1 61BE277E
P 9350 4000
F 0 "D1" V 9304 4080 50  0000 L CNN
F 1 "1N5818" V 9395 4080 50  0000 L CNN
F 2 "0_my_footprints:myDiodeSchotsky" H 9350 3825 50  0001 C CNN
F 3 "http://www.vishay.com/docs/88503/1n4001.pdf" H 9350 4000 50  0001 C CNN
	1    9350 4000
	0    1    1    0   
$EndComp
Wire Wire Line
	9350 3650 9350 3750
Wire Wire Line
	9150 3750 9350 3750
Connection ~ 9350 3750
Wire Wire Line
	9350 3750 9350 3850
Wire Wire Line
	9150 4250 9350 4250
Wire Wire Line
	9350 4250 9350 4150
Wire Wire Line
	9350 4250 9350 4350
Connection ~ 9350 4250
Wire Wire Line
	8500 4550 8650 4550
$Comp
L Connector_Generic:Conn_01x01 J21
U 1 1 61CFC6E3
P 4150 3800
F 0 "J21" H 4100 3800 50  0001 L CNN
F 1 "AUX" H 4400 3800 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4150 3800 50  0001 C CNN
F 3 "~" H 4150 3800 50  0001 C CNN
	1    4150 3800
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J23
U 1 1 61D1E62D
P 4450 3800
F 0 "J23" H 4400 3800 50  0001 L CNN
F 1 "AUX" H 4700 3800 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4450 3800 50  0001 C CNN
F 3 "~" H 4450 3800 50  0001 C CNN
	1    4450 3800
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J27
U 1 1 61D24402
P 4950 3800
F 0 "J27" H 4900 3800 50  0001 L CNN
F 1 "AUX" H 5200 3800 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4950 3800 50  0001 C CNN
F 3 "~" H 4950 3800 50  0001 C CNN
	1    4950 3800
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J28
U 1 1 61D256ED
P 5050 3800
F 0 "J28" H 5000 3800 50  0001 L CNN
F 1 "AUX" H 5300 3800 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 5050 3800 50  0001 C CNN
F 3 "~" H 5050 3800 50  0001 C CNN
	1    5050 3800
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J25
U 1 1 61D2A588
P 4450 2400
F 0 "J25" H 4400 2400 50  0001 L CNN
F 1 "AUX" H 4700 2400 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4450 2400 50  0001 C CNN
F 3 "~" H 4450 2400 50  0001 C CNN
	1    4450 2400
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J22
U 1 1 61D2E052
P 4350 3800
F 0 "J22" H 4300 3800 50  0001 L CNN
F 1 "AUX" H 4600 3800 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4350 3800 50  0001 C CNN
F 3 "~" H 4350 3800 50  0001 C CNN
	1    4350 3800
	0    1    1    0   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J26
U 1 1 61D2E058
P 4550 2400
F 0 "J26" H 4500 2400 50  0001 L CNN
F 1 "AUX" H 4800 2400 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4550 2400 50  0001 C CNN
F 3 "~" H 4550 2400 50  0001 C CNN
	1    4550 2400
	0    -1   -1   0   
$EndComp
Text GLabel 4650 2600 1    50   Input ~ 0
BUTTON2
Text GLabel 4050 2600 1    50   Input ~ 0
BUTTON1
$Comp
L power:GND #PWR014
U 1 1 61E8F524
P 3950 2600
F 0 "#PWR014" H 3950 2350 50  0001 C CNN
F 1 "GND" H 3950 2250 50  0001 C CNN
F 2 "" H 3950 2600 50  0001 C CNN
F 3 "" H 3950 2600 50  0001 C CNN
	1    3950 2600
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR012
U 1 1 61E93031
P 3850 2600
F 0 "#PWR012" H 3850 2450 50  0001 C CNN
F 1 "+5V" H 3865 2773 50  0000 C CNN
F 2 "" H 3850 2600 50  0001 C CNN
F 3 "" H 3850 2600 50  0001 C CNN
	1    3850 2600
	1    0    0    -1  
$EndComp
Text GLabel 5150 2600 1    50   Input ~ 0
S_POUT
Text GLabel 4150 2600 1    50   Input ~ 0
ALARM
$Comp
L Connector_Generic:Conn_01x01 J29
U 1 1 61EA5693
P 4350 2400
F 0 "J29" H 4300 2400 50  0001 L CNN
F 1 "AUX" H 4600 2400 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4350 2400 50  0001 C CNN
F 3 "~" H 4350 2400 50  0001 C CNN
	1    4350 2400
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J24
U 1 1 61D2A58E
P 4250 2400
F 0 "J24" H 4200 2400 50  0001 L CNN
F 1 "AUX" H 4500 2400 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4250 2400 50  0001 C CNN
F 3 "~" H 4250 2400 50  0001 C CNN
	1    4250 2400
	0    -1   -1   0   
$EndComp
$Comp
L Switch:SW_SPDT SW1
U 1 1 61BE9A56
P 4400 1700
F 0 "SW1" H 4400 1985 50  0000 C CNN
F 1 "POWER" H 4400 1894 50  0000 C CNN
F 2 "0_my_footprints:mySwitchToggleSPDT" H 4400 1700 50  0001 C CNN
F 3 "~" H 4400 1700 50  0001 C CNN
	1    4400 1700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J30
U 1 1 61F5036D
P 4500 5650
F 0 "J30" H 4450 5650 50  0001 L CNN
F 1 "AUX" H 4750 5650 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4500 5650 50  0001 C CNN
F 3 "~" H 4500 5650 50  0001 C CNN
	1    4500 5650
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J31
U 1 1 61F52116
P 4500 5750
F 0 "J31" H 4450 5750 50  0001 L CNN
F 1 "AUX" H 4750 5750 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4500 5750 50  0001 C CNN
F 3 "~" H 4500 5750 50  0001 C CNN
	1    4500 5750
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J32
U 1 1 61F5325C
P 4500 5850
F 0 "J32" H 4450 5850 50  0001 L CNN
F 1 "AUX" H 4750 5850 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4500 5850 50  0001 C CNN
F 3 "~" H 4500 5850 50  0001 C CNN
	1    4500 5850
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J33
U 1 1 61F54394
P 4500 5950
F 0 "J33" H 4450 5950 50  0001 L CNN
F 1 "AUX" H 4750 5950 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4500 5950 50  0001 C CNN
F 3 "~" H 4500 5950 50  0001 C CNN
	1    4500 5950
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J34
U 1 1 61F55455
P 4500 6050
F 0 "J34" H 4450 6050 50  0001 L CNN
F 1 "AUX" H 4750 6050 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4500 6050 50  0001 C CNN
F 3 "~" H 4500 6050 50  0001 C CNN
	1    4500 6050
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J35
U 1 1 61F5657F
P 4500 6150
F 0 "J35" H 4450 6150 50  0001 L CNN
F 1 "AUX" H 4750 6150 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4500 6150 50  0001 C CNN
F 3 "~" H 4500 6150 50  0001 C CNN
	1    4500 6150
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J36
U 1 1 61F577F6
P 4900 5650
F 0 "J36" H 4850 5650 50  0001 L CNN
F 1 "AUX" H 5150 5650 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4900 5650 50  0001 C CNN
F 3 "~" H 4900 5650 50  0001 C CNN
	1    4900 5650
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J37
U 1 1 61F5AD42
P 4900 5750
F 0 "J37" H 4850 5750 50  0001 L CNN
F 1 "AUX" H 5150 5750 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4900 5750 50  0001 C CNN
F 3 "~" H 4900 5750 50  0001 C CNN
	1    4900 5750
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J38
U 1 1 61F5BE86
P 4900 5850
F 0 "J38" H 4850 5850 50  0001 L CNN
F 1 "AUX" H 5150 5850 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4900 5850 50  0001 C CNN
F 3 "~" H 4900 5850 50  0001 C CNN
	1    4900 5850
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J39
U 1 1 61F5CF27
P 4900 5950
F 0 "J39" H 4850 5950 50  0001 L CNN
F 1 "AUX" H 5150 5950 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4900 5950 50  0001 C CNN
F 3 "~" H 4900 5950 50  0001 C CNN
	1    4900 5950
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J40
U 1 1 61F5DFB5
P 4900 6050
F 0 "J40" H 4850 6050 50  0001 L CNN
F 1 "AUX" H 5150 6050 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4900 6050 50  0001 C CNN
F 3 "~" H 4900 6050 50  0001 C CNN
	1    4900 6050
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J41
U 1 1 61F5F048
P 4900 6150
F 0 "J41" H 4850 6150 50  0001 L CNN
F 1 "AUX" H 5150 6150 50  0000 R CNN
F 2 "cnc3018-PCB:my1pin" H 4900 6150 50  0001 C CNN
F 3 "~" H 4900 6150 50  0001 C CNN
	1    4900 6150
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J6
U 1 1 63C2EBA7
P 2500 4550
F 0 "J6" H 2700 4500 50  0000 L CNN
F 1 "LEDS" H 2650 4600 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x03" H 2500 4550 50  0001 C CNN
F 3 "~" H 2500 4550 50  0001 C CNN
	1    2500 4550
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J7
U 1 1 63C5E994
P 3800 4800
F 0 "J7" V 4050 4750 50  0000 L CNN
F 1 "LEDS OUT" V 3950 4600 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x03" H 3800 4800 50  0001 C CNN
F 3 "~" H 3800 4800 50  0001 C CNN
	1    3800 4800
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J8
U 1 1 63C63579
P 3800 5300
F 0 "J8" V 4050 5250 50  0000 L CNN
F 1 "LEDS IN" V 3950 5100 50  0000 L CNN
F 2 "0_my_footprints:myPinSocket_1x03" H 3800 5300 50  0001 C CNN
F 3 "~" H 3800 5300 50  0001 C CNN
	1    3800 5300
	0    -1   1    0   
$EndComp
Wire Wire Line
	3700 5000 3700 5050
Wire Wire Line
	3800 5000 3800 5100
Wire Wire Line
	3900 5000 3900 5050
Wire Wire Line
	3950 5050 3900 5050
Connection ~ 3900 5050
Wire Wire Line
	3900 5050 3900 5100
Wire Wire Line
	3650 5050 3700 5050
Connection ~ 3700 5050
Wire Wire Line
	3700 5050 3700 5100
$Comp
L Connector_Generic:Conn_01x04 J9
U 1 1 63C83A53
P 2500 5300
F 0 "J9" H 2800 5150 50  0000 C CNN
F 1 "LCD" H 2800 5250 50  0000 C CNN
F 2 "0_my_footprints:myPinSocket_1x04" H 2500 5300 50  0001 C CNN
F 3 "~" H 2500 5300 50  0001 C CNN
	1    2500 5300
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR09
U 1 1 61DD1C37
P 2700 5400
F 0 "#PWR09" H 2700 5150 50  0001 C CNN
F 1 "GND" V 2700 5200 50  0000 C CNN
F 2 "" H 2700 5400 50  0001 C CNN
F 3 "" H 2700 5400 50  0001 C CNN
	1    2700 5400
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J1
U 1 1 63C47095
P 1950 1250
F 0 "J1" H 2600 1200 50  0000 C CNN
F 1 "BA_POWER" H 2250 1200 50  0000 C CNN
F 2 "0_my_footprints:myTerminalBlock_5.08x02" H 1950 1250 50  0001 C CNN
F 3 "~" H 1950 1250 50  0001 C CNN
	1    1950 1250
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J2
U 1 1 63C60B20
P 1950 1800
F 0 "J2" H 2150 1750 50  0000 L CNN
F 1 "PUMPS" H 2100 1850 50  0000 L CNN
F 2 "0_my_footprints:myTerminalBlock_5.08x03" H 1950 1800 50  0001 C CNN
F 3 "~" H 1950 1800 50  0001 C CNN
	1    1950 1800
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x02 J3
U 1 1 63CB6821
P 8850 4000
F 0 "J3" H 9150 3850 50  0000 C CNN
F 1 "ALARM" H 9150 3950 50  0000 C CNN
F 2 "0_my_footprints:myTerminalBlock_5.08x02" H 8850 4000 50  0001 C CNN
F 3 "~" H 8850 4000 50  0001 C CNN
	1    8850 4000
	-1   0    0    1   
$EndComp
Wire Wire Line
	9150 3750 9150 3900
Wire Wire Line
	9150 3900 9050 3900
Wire Wire Line
	9050 4000 9150 4000
Wire Wire Line
	9150 4000 9150 4250
$Comp
L Switch:SW_DIP_x01 SW4
U 1 1 61DED5F7
P 7000 4550
F 0 "SW4" H 7000 4800 50  0000 C CNN
F 1 "BUTTON3" H 7000 4700 50  0000 C CNN
F 2 "0_my_footprints:myButton" H 7000 4550 50  0001 C CNN
F 3 "~" H 7000 4550 50  0001 C CNN
	1    7000 4550
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0104
U 1 1 63C5132D
P 6700 3500
F 0 "#PWR0104" H 6700 3350 50  0001 C CNN
F 1 "+3.3V" H 6715 3673 50  0000 C CNN
F 2 "" H 6700 3500 50  0001 C CNN
F 3 "" H 6700 3500 50  0001 C CNN
	1    6700 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	6700 3500 6700 3750
Connection ~ 6700 3750
$EndSCHEMATC
