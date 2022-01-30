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
L LED:WS2812B D4
U 1 1 61F5AC05
P 2750 4650
F 0 "D4" V 2796 4306 50  0000 R CNN
F 1 "WS2812B" V 2705 4306 50  0000 R CNN
F 2 "cnc3018-PCB:myWS2812" H 2800 4350 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 2850 4275 50  0001 L TNN
	1    2750 4650
	0    -1   -1   0   
$EndComp
$Comp
L LED:WS2812B D3
U 1 1 61F5C5E0
P 2750 4050
F 0 "D3" V 2796 3706 50  0000 R CNN
F 1 "WS2812B" V 2705 3706 50  0000 R CNN
F 2 "cnc3018-PCB:myWS2812" H 2800 3750 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 2850 3675 50  0001 L TNN
	1    2750 4050
	0    -1   -1   0   
$EndComp
$Comp
L LED:WS2812B D2
U 1 1 61F5DD56
P 2750 3450
F 0 "D2" V 2796 3106 50  0000 R CNN
F 1 "WS2812B" V 2705 3106 50  0000 R CNN
F 2 "cnc3018-PCB:myWS2812" H 2800 3150 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 2850 3075 50  0001 L TNN
	1    2750 3450
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J1
U 1 1 61F61BA9
P 2750 1800
F 0 "J1" V 2714 1612 50  0000 R CNN
F 1 "Conn_01x03" V 2800 1600 50  0000 R CNN
F 2 "cnc3018-PCB:my3pin" H 2750 1800 50  0001 C CNN
F 3 "~" H 2750 1800 50  0001 C CNN
	1    2750 1800
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x03 J2
U 1 1 61F62D99
P 2750 5350
F 0 "J2" V 2622 5530 50  0000 L CNN
F 1 "Conn_01x03" V 2713 5530 50  0000 L CNN
F 2 "cnc3018-PCB:my3pin" H 2750 5350 50  0001 C CNN
F 3 "~" H 2750 5350 50  0001 C CNN
	1    2750 5350
	0    1    1    0   
$EndComp
Wire Wire Line
	2750 4950 2750 5150
Connection ~ 3050 3450
Connection ~ 3050 4050
Wire Wire Line
	3050 4050 3050 3450
Connection ~ 3050 4650
Wire Wire Line
	3050 4650 3050 4050
Wire Wire Line
	2450 5050 2450 4650
Connection ~ 2450 3450
Connection ~ 2450 4050
Wire Wire Line
	2450 4050 2450 3450
Connection ~ 2450 4650
Wire Wire Line
	2450 4650 2450 4050
$Comp
L Connector_Generic:Conn_01x01 J4
U 1 1 61F66634
P 3400 2250
F 0 "J4" H 3480 2292 50  0000 L CNN
F 1 "Conn_01x01" H 3480 2201 50  0000 L CNN
F 2 "cnc3018-PCB:my1pin" H 3400 2250 50  0001 C CNN
F 3 "~" H 3400 2250 50  0001 C CNN
	1    3400 2250
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x01 J3
U 1 1 61F68801
P 3400 2150
F 0 "J3" H 3480 2192 50  0000 L CNN
F 1 "Conn_01x01" H 3480 2101 50  0000 L CNN
F 2 "cnc3018-PCB:my1pin" H 3400 2150 50  0001 C CNN
F 3 "~" H 3400 2150 50  0001 C CNN
	1    3400 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 3450 3050 2850
Connection ~ 3050 2850
Wire Wire Line
	2450 3450 2450 2850
Connection ~ 2450 2850
$Comp
L LED:WS2812B D1
U 1 1 61F5E592
P 2750 2850
F 0 "D1" V 2796 2506 50  0000 R CNN
F 1 "WS2812B" V 2705 2506 50  0000 R CNN
F 2 "cnc3018-PCB:myWS2812" H 2800 2550 50  0001 L TNN
F 3 "https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf" H 2850 2475 50  0001 L TNN
	1    2750 2850
	0    -1   -1   0   
$EndComp
Wire Wire Line
	2750 2000 2750 2550
Wire Wire Line
	3050 5000 2650 5000
Wire Wire Line
	2650 5000 2650 5150
Wire Wire Line
	3050 5000 3050 4650
Wire Wire Line
	2450 5050 2850 5050
Wire Wire Line
	2850 5050 2850 5150
Wire Wire Line
	2450 2150 2850 2150
Wire Wire Line
	2850 2150 2850 2000
Wire Wire Line
	2450 2150 2450 2850
Wire Wire Line
	3050 2250 2650 2250
Wire Wire Line
	2650 2250 2650 2000
Wire Wire Line
	3050 2250 3050 2850
Wire Wire Line
	3050 2250 3200 2250
Connection ~ 3050 2250
Wire Wire Line
	2850 2150 3200 2150
Connection ~ 2850 2150
$EndSCHEMATC
