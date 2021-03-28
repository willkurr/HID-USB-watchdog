a.out: 	usb_watchdog.cpp
	g++ -c usb_watchdog.cpp -o watchdog.o
	g++ -Llinux/.libs watchdog.o -lhidapi-hidraw -o hidusbwatchdog
