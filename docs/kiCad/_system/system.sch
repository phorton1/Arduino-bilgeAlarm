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
L Device:Battery BT?
U 1 1 63B978B0
P 4500 6650
F 0 "BT?" H 4608 6696 50  0000 L CNN
F 1 "Battery" H 4608 6605 50  0000 L CNN
F 2 "" V 4500 6710 50  0001 C CNN
F 3 "~" V 4500 6710 50  0001 C CNN
	1    4500 6650
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPST SW?
U 1 1 63B99995
P 1650 6200
F 0 "SW?" H 1650 6100 50  0000 C CNN
F 1 "Float Switch 1" H 1650 6000 50  0000 C CNN
F 2 "" H 1650 6200 50  0001 C CNN
F 3 "~" H 1650 6200 50  0001 C CNN
	1    1650 6200
	1    0    0    -1  
$EndComp
$Comp
L Motor:Motor_DC M?
U 1 1 63B9A386
P 1200 6550
F 0 "M?" H 850 6550 50  0000 L CNN
F 1 "Pump 1" H 650 6450 50  0000 L CNN
F 2 "" H 1200 6460 50  0001 C CNN
F 3 "~" H 1200 6460 50  0001 C CNN
	1    1200 6550
	1    0    0    -1  
$EndComp
$Comp
L Motor:Motor_DC M?
U 1 1 63BA8494
P 2350 6550
F 0 "M?" H 2000 6550 50  0000 L CNN
F 1 "Pump 2" H 1850 6450 50  0000 L CNN
F 2 "" H 2350 6460 50  0001 C CNN
F 3 "~" H 2350 6460 50  0001 C CNN
	1    2350 6550
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPST SW?
U 1 1 63BABAB9
P 2850 6200
F 0 "SW?" H 2850 6100 50  0000 C CNN
F 1 "Float Switch 2" H 2850 6000 50  0000 C CNN
F 2 "" H 2850 6200 50  0001 C CNN
F 3 "~" H 2850 6200 50  0001 C CNN
	1    2850 6200
	1    0    0    -1  
$EndComp
$Comp
L Device:Fuse F?
U 1 1 63B9EE56
P 4350 5200
F 0 "F?" V 4350 5450 50  0000 C CNN
F 1 "1 - Alarm Power" V 4350 5600 50  0000 L CNN
F 2 "" V 4280 5200 50  0001 C CNN
F 3 "~" H 4350 5200 50  0001 C CNN
	1    4350 5200
	0    1    1    0   
$EndComp
$Comp
L Device:Fuse F?
U 1 1 63BACE86
P 4350 5350
F 0 "F?" V 4350 5600 50  0000 C CNN
F 1 "2 - Pump1 Power" V 4350 5750 50  0000 L CNN
F 2 "" V 4280 5350 50  0001 C CNN
F 3 "~" H 4350 5350 50  0001 C CNN
	1    4350 5350
	0    1    1    0   
$EndComp
$Comp
L Device:Fuse F?
U 1 1 63BAD1AC
P 4350 5500
F 0 "F?" V 4350 5750 50  0000 C CNN
F 1 "3 - Pump2 Power" V 4350 5900 50  0000 L CNN
F 2 "" V 4280 5500 50  0001 C CNN
F 3 "~" H 4350 5500 50  0001 C CNN
	1    4350 5500
	0    1    1    0   
$EndComp
$Comp
L Switch:SW_SPST SW?
U 1 1 63BAE2A4
P 6850 2200
F 0 "SW?" H 6850 2000 50  0000 C CNN
F 1 "Manual Override" H 6850 2100 50  0000 C CNN
F 2 "" H 6850 2200 50  0001 C CNN
F 3 "~" H 6850 2200 50  0001 C CNN
	1    6850 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 6450 4500 5500
Connection ~ 4500 5350
Wire Wire Line
	4500 5350 4500 5200
Connection ~ 4500 5500
Wire Wire Line
	4500 5500 4500 5350
$Comp
L power:GND #PWR?
U 1 1 63BB7635
P 4500 7100
F 0 "#PWR?" H 4500 6850 50  0001 C CNN
F 1 "GND" H 4505 6927 50  0000 C CNN
F 2 "" H 4500 7100 50  0001 C CNN
F 3 "" H 4500 7100 50  0001 C CNN
	1    4500 7100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 63BBA475
P 2350 7100
F 0 "#PWR?" H 2350 6850 50  0001 C CNN
F 1 "GND" H 2355 6927 50  0000 C CNN
F 2 "" H 2350 7100 50  0001 C CNN
F 3 "" H 2350 7100 50  0001 C CNN
	1    2350 7100
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 63BBA94B
P 1200 7100
F 0 "#PWR?" H 1200 6850 50  0001 C CNN
F 1 "GND" H 1205 6927 50  0000 C CNN
F 2 "" H 1200 7100 50  0001 C CNN
F 3 "" H 1200 7100 50  0001 C CNN
	1    1200 7100
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 6850 4500 7100
Wire Wire Line
	2350 6850 2350 7100
Wire Wire Line
	1200 6850 1200 7100
Wire Wire Line
	4200 5500 3050 5500
Wire Wire Line
	3050 5500 3050 6200
Wire Wire Line
	2650 6200 2350 6200
Wire Wire Line
	2350 6200 2350 6350
Wire Wire Line
	4200 5350 1850 5350
Wire Wire Line
	1850 5350 1850 6200
Wire Wire Line
	1450 6200 1200 6200
Wire Wire Line
	2350 6200 1900 6200
Connection ~ 2350 6200
Wire Wire Line
	1200 5350 1800 5350
Wire Wire Line
	1200 5350 1200 6200
Connection ~ 1200 6200
Wire Wire Line
	1200 6200 1200 6350
Wire Wire Line
	4200 5200 1950 5200
$Comp
L power:GND #PWR?
U 1 1 63BD1691
P 2000 4700
F 0 "#PWR?" H 2000 4450 50  0001 C CNN
F 1 "GND" V 2005 4527 50  0000 C CNN
F 2 "" H 2000 4700 50  0001 C CNN
F 3 "" H 2000 4700 50  0001 C CNN
	1    2000 4700
	1    0    0    -1  
$EndComp
Connection ~ 1850 5350
$Comp
L power:GND #PWR?
U 1 1 63BE1D92
P 2350 2300
F 0 "#PWR?" H 2350 2050 50  0001 C CNN
F 1 "GND" V 2350 2100 50  0000 C CNN
F 2 "" H 2350 2300 50  0001 C CNN
F 3 "" H 2350 2300 50  0001 C CNN
	1    2350 2300
	0    -1   -1   0   
$EndComp
Text GLabel 2350 2200 2    50   Input ~ 0
BA_POWER
Text GLabel 2350 2400 2    50   Input ~ 0
PUMP2_ON
Text GLabel 2350 2500 2    50   Input ~ 0
PUMP1_ON
Text GLabel 2350 2600 2    50   Input ~ 0
PUMP1_POWER
$Comp
L power:GND #PWR?
U 1 1 63BE90C2
P 3750 2300
F 0 "#PWR?" H 3750 2050 50  0001 C CNN
F 1 "GND" V 3750 2100 50  0000 C CNN
F 2 "" H 3750 2300 50  0001 C CNN
F 3 "" H 3750 2300 50  0001 C CNN
	1    3750 2300
	0    1    -1   0   
$EndComp
Text GLabel 3750 2200 0    50   Input ~ 0
BA_POWER
Text GLabel 3750 2400 0    50   Input ~ 0
PUMP2_ON
Text GLabel 3750 2500 0    50   Input ~ 0
PUMP1_ON
Text GLabel 3750 2600 0    50   Input ~ 0
PUMP1_POWER
$Comp
L Relay:G5LE-1 K?
U 1 1 63BEBAA9
P 6850 3100
F 0 "K?" V 7350 3300 50  0000 C CNN
F 1 "Relay" V 7350 3100 50  0000 C CNN
F 2 "Relay_THT:Relay_SPDT_Omron-G5LE-1" H 7300 3050 50  0001 L CNN
F 3 "http://www.omron.com/ecb/products/pdf/en-g5le.pdf" H 6850 3100 50  0001 C CNN
	1    6850 3100
	0    -1   -1   0   
$EndComp
$Comp
L cnc3018_Library:BUCK01 M?
U 1 1 63BF004F
P 4900 4350
F 0 "M?" H 4875 4725 50  0000 C CNN
F 1 "BUCK01" H 4875 4634 50  0000 C CNN
F 2 "" H 4850 4650 50  0001 C CNN
F 3 "" H 4850 4650 50  0001 C CNN
	1    4900 4350
	1    0    0    -1  
$EndComp
$Comp
L Device:CP C?
U 1 1 63BF14B1
P 5700 4350
F 0 "C?" H 5582 4304 50  0000 R CNN
F 1 "100uf" H 5582 4395 50  0000 R CNN
F 2 "" H 5738 4200 50  0001 C CNN
F 3 "~" H 5700 4350 50  0001 C CNN
	1    5700 4350
	-1   0    0    1   
$EndComp
$Comp
L Switch:SW_SPST SW?
U 1 1 63BF312C
P 4050 4500
F 0 "SW?" H 4050 4750 50  0000 C CNN
F 1 "Power Switch" H 4050 4650 50  0000 C CNN
F 2 "" H 4050 4500 50  0001 C CNN
F 3 "~" H 4050 4500 50  0001 C CNN
	1    4050 4500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 63BF3728
P 5200 1950
F 0 "R?" H 5270 1996 50  0001 L CNN
F 1 "4.7K" V 5200 1850 50  0000 L CNN
F 2 "" V 5130 1950 50  0001 C CNN
F 3 "~" H 5200 1950 50  0001 C CNN
	1    5200 1950
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63BF45BA
P 5200 950
F 0 "R?" H 5270 996 50  0001 L CNN
F 1 "1K" V 5200 900 50  0000 L CNN
F 2 "" V 5130 950 50  0001 C CNN
F 3 "~" H 5200 950 50  0001 C CNN
	1    5200 950 
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63BF5CF7
P 5350 1950
F 0 "R?" H 5420 1996 50  0001 L CNN
F 1 "10K" V 5350 1850 50  0000 L CNN
F 2 "" V 5280 1950 50  0001 C CNN
F 3 "~" H 5350 1950 50  0001 C CNN
	1    5350 1950
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63BF5CFD
P 5350 950
F 0 "R?" H 5420 996 50  0001 L CNN
F 1 "10K" V 5350 900 50  0000 L CNN
F 2 "" V 5280 950 50  0001 C CNN
F 3 "~" H 5350 950 50  0001 C CNN
	1    5350 950 
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63C11D70
P 5050 1950
F 0 "R?" H 5120 1996 50  0001 L CNN
F 1 "4.7K" V 5050 1850 50  0000 L CNN
F 2 "" V 4980 1950 50  0001 C CNN
F 3 "~" H 5050 1950 50  0001 C CNN
	1    5050 1950
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63C11D76
P 5050 950
F 0 "R?" H 5120 996 50  0001 L CNN
F 1 "1K" V 5050 900 50  0000 L CNN
F 2 "" V 4980 950 50  0001 C CNN
F 3 "~" H 5050 950 50  0001 C CNN
	1    5050 950 
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63C124F6
P 4900 1950
F 0 "R?" H 4970 1996 50  0001 L CNN
F 1 "4.7K" V 4900 1850 50  0000 L CNN
F 2 "" V 4830 1950 50  0001 C CNN
F 3 "~" H 4900 1950 50  0001 C CNN
	1    4900 1950
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63C124FC
P 4900 950
F 0 "R?" H 4970 996 50  0001 L CNN
F 1 "1K" V 4900 900 50  0000 L CNN
F 2 "" V 4830 950 50  0001 C CNN
F 3 "~" H 4900 950 50  0001 C CNN
	1    4900 950 
	-1   0    0    1   
$EndComp
Wire Wire Line
	3750 2200 3850 2200
Wire Wire Line
	4250 4500 4500 4500
$Comp
L power:GND #PWR?
U 1 1 63C22275
P 4500 4200
F 0 "#PWR?" H 4500 3950 50  0001 C CNN
F 1 "GND" V 4500 4000 50  0000 C CNN
F 2 "" H 4500 4200 50  0001 C CNN
F 3 "" H 4500 4200 50  0001 C CNN
	1    4500 4200
	0    1    -1   0   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 63C24226
P 5450 4200
F 0 "#PWR?" H 5450 3950 50  0001 C CNN
F 1 "GND" V 5450 4000 50  0000 C CNN
F 2 "" H 5450 4200 50  0001 C CNN
F 3 "" H 5450 4200 50  0001 C CNN
	1    5450 4200
	1    0    0    1   
$EndComp
Wire Wire Line
	5250 4200 5450 4200
Connection ~ 5450 4200
Wire Wire Line
	5450 4200 5700 4200
Wire Wire Line
	5250 4500 5700 4500
Wire Wire Line
	5700 4500 6300 4500
Connection ~ 5700 4500
$Comp
L Transistor_BJT:BC547 Q?
U 1 1 63C27484
P 7500 3750
F 0 "Q?" H 7691 3796 50  0000 L CNN
F 1 "BC547" H 7691 3705 50  0000 L CNN
F 2 "Package_TO_SOT_THT:TO-92_Inline" H 7700 3675 50  0001 L CIN
F 3 "http://www.fairchildsemi.com/ds/BC/BC547.pdf" H 7500 3750 50  0001 L CNN
	1    7500 3750
	-1   0    0    -1  
$EndComp
Wire Wire Line
	6550 3300 6300 3300
Wire Wire Line
	6300 3300 6300 4500
Connection ~ 6300 4500
$Comp
L Device:R R?
U 1 1 63C378EE
P 7850 3750
F 0 "R?" H 7920 3796 50  0001 L CNN
F 1 "1K" V 7850 3700 50  0000 L CNN
F 2 "" V 7780 3750 50  0001 C CNN
F 3 "~" H 7850 3750 50  0001 C CNN
	1    7850 3750
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7150 3300 7400 3300
Wire Wire Line
	7400 3300 7400 3550
Wire Wire Line
	7400 3950 7400 4050
$Comp
L power:GND #PWR?
U 1 1 63C4110F
P 7400 4050
F 0 "#PWR?" H 7400 3800 50  0001 C CNN
F 1 "GND" V 7400 3850 50  0000 C CNN
F 2 "" H 7400 4050 50  0001 C CNN
F 3 "" H 7400 4050 50  0001 C CNN
	1    7400 4050
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3750 2600 4900 2600
Wire Wire Line
	6250 2600 6250 2800
Wire Wire Line
	6250 2800 6550 2800
Wire Wire Line
	3750 2500 5050 2500
Wire Wire Line
	7350 2500 7350 2200
Wire Wire Line
	7350 2200 7050 2200
Wire Wire Line
	7150 2900 7350 2900
Wire Wire Line
	7350 2900 7350 2500
Connection ~ 7350 2500
Wire Wire Line
	6250 2600 6250 2200
Wire Wire Line
	6250 2200 6650 2200
Connection ~ 6250 2600
Connection ~ 4900 2600
Wire Wire Line
	4900 2600 6250 2600
Connection ~ 5050 2500
Wire Wire Line
	5050 2500 7350 2500
Wire Wire Line
	3750 2400 5200 2400
Wire Wire Line
	3850 2200 5350 2200
Wire Wire Line
	5350 2200 5350 2100
Connection ~ 3850 2200
Wire Wire Line
	3850 2200 3850 4500
Wire Wire Line
	5200 2100 5200 2400
Wire Wire Line
	5050 2100 5050 2500
Wire Wire Line
	4900 2100 4900 2600
Wire Wire Line
	6300 4500 8600 4500
Wire Wire Line
	8000 3750 8550 3750
Wire Wire Line
	5350 1800 8950 1800
Wire Wire Line
	5200 1800 5200 1700
Wire Wire Line
	5200 1700 8950 1700
Wire Wire Line
	5050 1800 5050 1600
Wire Wire Line
	5050 1600 8950 1600
Wire Wire Line
	8550 1900 8950 1900
Wire Wire Line
	8550 1900 8550 3750
Wire Wire Line
	8600 2000 8950 2000
Wire Wire Line
	8600 2000 8600 4500
Wire Wire Line
	5350 800  5200 800 
Wire Wire Line
	4350 800  4350 950 
Connection ~ 5050 800 
Wire Wire Line
	5050 800  4900 800 
Connection ~ 5200 800 
Wire Wire Line
	5200 800  5050 800 
$Comp
L power:GND #PWR?
U 1 1 63C9B9A7
P 4350 950
F 0 "#PWR?" H 4350 700 50  0001 C CNN
F 1 "GND" V 4350 750 50  0000 C CNN
F 2 "" H 4350 950 50  0001 C CNN
F 3 "" H 4350 950 50  0001 C CNN
	1    4350 950 
	-1   0    0    -1  
$EndComp
Connection ~ 5050 1600
Connection ~ 5200 1700
Connection ~ 5350 1800
$Comp
L power:GND #PWR?
U 1 1 63CA82E1
P 8950 2100
F 0 "#PWR?" H 8950 1850 50  0001 C CNN
F 1 "GND" V 8950 1900 50  0000 C CNN
F 2 "" H 8950 2100 50  0001 C CNN
F 3 "" H 8950 2100 50  0001 C CNN
	1    8950 2100
	0    -1   -1   0   
$EndComp
Text GLabel 8950 2000 2    50   Input ~ 0
5V
Text GLabel 8950 1800 2    50   Input ~ 0
5V_SENSE
Text GLabel 8950 1700 2    50   Input ~ 0
PUMP2_SENSE
Text GLabel 8950 1900 2    50   Input ~ 0
RELAY
Text GLabel 8950 1600 2    50   Input ~ 0
PUMP1_SENSE
Connection ~ 4900 1500
Wire Wire Line
	4900 1800 4900 1500
Wire Wire Line
	4900 1500 8950 1500
Text GLabel 8950 1500 2    50   Input ~ 0
POWER1_SENSE
Wire Wire Line
	2050 5500 2050 2100
Wire Wire Line
	2050 5500 3050 5500
Connection ~ 3050 5500
Text GLabel 2350 2100 2    50   Input ~ 0
PUMP2_POWER
Text GLabel 3750 2100 0    50   Input ~ 0
PUMP2_POWER
$Comp
L Device:R R?
U 1 1 63CE53D6
P 4750 950
F 0 "R?" H 4820 996 50  0001 L CNN
F 1 "1K" V 4750 900 50  0000 L CNN
F 2 "" V 4680 950 50  0001 C CNN
F 3 "~" H 4750 950 50  0001 C CNN
	1    4750 950 
	-1   0    0    1   
$EndComp
$Comp
L Device:R R?
U 1 1 63CE8072
P 4750 1950
F 0 "R?" H 4820 1996 50  0001 L CNN
F 1 "4.7K" V 4750 1850 50  0000 L CNN
F 2 "" V 4680 1950 50  0001 C CNN
F 3 "~" H 4750 1950 50  0001 C CNN
	1    4750 1950
	-1   0    0    1   
$EndComp
Wire Wire Line
	3750 2100 4750 2100
Wire Wire Line
	4750 1750 4750 1400
Wire Wire Line
	4350 800  4750 800 
Connection ~ 4900 800 
Connection ~ 4750 800 
Wire Wire Line
	4750 800  4900 800 
Wire Wire Line
	5350 1100 5350 1800
Wire Wire Line
	5200 1100 5200 1700
Wire Wire Line
	5050 1100 5050 1600
Wire Wire Line
	4900 1100 4900 1500
Wire Wire Line
	4750 1400 8950 1400
Connection ~ 4750 1400
Wire Wire Line
	4750 1400 4750 1100
Text GLabel 8950 1400 2    50   Input ~ 0
POWER2_SENSE
Wire Wire Line
	2350 2600 1800 2600
Wire Wire Line
	1800 2600 1800 5350
Wire Wire Line
	2350 2500 1850 2500
Wire Wire Line
	1850 2500 1850 5350
Wire Wire Line
	2350 2400 1900 2400
Wire Wire Line
	1900 2400 1900 6200
Wire Wire Line
	2350 2300 1950 2300
Wire Wire Line
	1950 2300 1950 5200
Wire Wire Line
	2350 2200 2000 2200
Wire Wire Line
	2000 2200 2000 4700
Wire Wire Line
	2350 2100 2050 2100
$EndSCHEMATC
