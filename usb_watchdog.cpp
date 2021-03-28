#include "hidapi/hidapi.h"
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
using namespace std;

#define SEND_SIZE 2		//command size in bytes to send/recieve from the USB device
#define RECIEVE_SIZE 2  //these SHOULD be equal, may cause undefined behavior if not

const char *DEVICE_PATH = "/dev/hidraw0";	//connect to literally the only HID device connected

bool reconnectToDev(hid_device *handle) {
    //wait for device to connect
    if (handle == NULL) {
        while (handle == NULL) {
            cout << "Device did not open on initial contact. Attempting to connect again..." << endl;
            handle = hid_open_path(DEVICE_PATH);
            sleep(1);
        }
    }
    else {
        handle = hid_open_path(DEVICE_PATH);
        while (handle == NULL) {
            cout << "Device disconnected. Attempting to reconnect..." << endl;
            handle = hid_open_path(DEVICE_PATH);
            sleep(1);
        }
    }

    return true;
}

int main(int argc, char **argv) {
    int heartbeatInterval;
    int refreshRate;

    //catch argument issues
    bool error = false;

    //if too many or no arguments are given
    if (argc == 1) {
        error = true;
        cout << "No arguments provided. "; 
    }
    else if (argc == 2) {
        error = true;
        cout << "Too few arguments provided. ";
    }
    else if (argc > 4) {
        error = true;
        cout << "Too many arguments provided. ";
    }
    //if either provided argument is not a number
    else {
        try {
            heartbeatInterval = stoi(argv[1]) / 10;
            refreshRate = stoi(argv[2]);
        }
        catch (invalid_argument) {
            error = true;
            cout << "An argument provided is not a number. ";
        }
    }
    //if the provided heartbeat interval is not in the allowable range of seconds
    if (heartbeatInterval > UINT8_MAX) {
        error = true;
        cout << "Heartbeat interval greater than max allowed value of " << UINT8_MAX*10 << ".";
    }
    else if (heartbeatInterval < 0 && !error) {
        error = true;
        cout << "Heartbeat interval is less than the min allowed value of 0. ";
    }
    else if (refreshRate <= 0 && !error) {
        error = true;
        cout << "Invalid refresh rate. ";
    }
    else if (refreshRate > heartbeatInterval*10 && !error) {
        error = true;
        cout << "Refresh rate higher than heartbeat interval! The watchdog would reboot before a heartbeat could be sent. ";
    }

    //then if there is a problem with the arguments, exit the program
    if (error) {
        cout << "USAGE: hidusbwatchdog heartbeat_interval refresh_rate\n" << endl;

        cout << "heartbeat_interval: a number from 0-2550 seconds specifying how long the watchdog should reboot after losing heartbeat from the host" << endl;
        cout << "refresh_rate: a number in seconds specifying how often to ping the watchdog" << endl;
        cout << "For heartbeat_interval, the watchdog only accepts multiples of 10 seconds, so your input's ones place will be truncated if you do not follow this" << endl;
        cout << "Examples of acceptable delays: 0, 10, 20, 30,...,2540, 2550" << endl;
        exit(-1);
    }

    //no argument errors, so connect to the watchdog
    hid_device *handle;
	int result;

	result = hid_init();
	handle = hid_open_path(DEVICE_PATH);

    //reconnect if device did not respond
    if (handle == NULL) {
        cout << "Are you running as sudo?" << endl;
	    reconnectToDev(handle);
    }

    unsigned char data[SEND_SIZE];
    memset(data, 0, SEND_SIZE);
    data[0] = heartbeatInterval;

    unsigned char recievedData[RECIEVE_SIZE];
	memset(recievedData, 0, RECIEVE_SIZE);

    //send heartbeat to watchdog forever
    while (1) {
        //write the data to the usb device
        result = hid_write(handle, data, SEND_SIZE);

        
        //if the write failed, check if the watchdog needs to be reconnected to
        if (result == -1) {
            //this will reconnect if needed
            reconnectToDev(handle);
        }

        cout << "Ping!" << endl;

        //check that the watchdog is responding
        result = hid_read_timeout(handle, recievedData, RECIEVE_SIZE, 5000);

        //if the read fails or the watchdog incorrectly confirms the byte sent
        //check if the watchdog needs to be reconnected to
        if (result == -1 || recievedData[0] != heartbeatInterval) {
            //this will reconnect if needed
            reconnectToDev(handle);
        }

        cout << "Pong!" << endl;

        sleep(refreshRate);
    }

    //if the while loop exited somehow, close everything properly
	hid_close(handle);
	hid_exit();

	//when you
	return 0;
}
