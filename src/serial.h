#ifndef __SERIAL_H_INCLUDED__
#define __SERIAL_H_INCLUDED__

#include <Windows.h>
#include<iostream>

using namespace std; 

HANDLE init_serial7() {
	HANDLE h; 
	h = CreateFileA("COM7", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL); 

	if (h == INVALID_HANDLE_VALUE) {
		cout << "error initializing handle"; 
	}
	else {
		cout << "success initializing handle"; 
	}

	DCB params = {};  
	params.DCBlength = sizeof(params);
	BOOL status;
	params.BaudRate = CBR_115200;
	params.ByteSize = 8;
	params.Parity = NOPARITY;
	params.StopBits = ONESTOPBIT;

	status = GetCommState(h, &params);
	if (status == FALSE) {
		cout << "get comm state error" << endl;
		CloseHandle(h);
	}
	else {
		cout << "get comm state success" << endl;
	}

	status = SetCommState(h, &params);

	if (status == FALSE) {
		cout << "set comm state error" << endl;
		CloseHandle(h);
	}

	else {
		cout << "set comm state success" << endl;
	}

	PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
	cout << "microcontroller rebooting."; 
	Sleep(1000); 
	cout << '.'; 
	Sleep(1000); 
	cout << ".\n";
	Sleep(1000); 
	cout << "reboot successful" << endl; 
	return h; 
}


BOOL serial_write(HANDLE h, BYTE *input, u32 size) {
	DWORD bytesWritten = 0; 
	return (WriteFile(h, input, size, &bytesWritten, NULL)); 

}

void free_serial(HANDLE h) {
	CloseHandle(h); 
	cout << "connection closed" << endl; 
}



#endif
