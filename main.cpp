// Windows Service for device detection and tracking
// CMD(ADMIN):
// Start sevice "sc create DeviceTracking binpath= C:\Users\root\Desktop\DEVICE_DETECTED\dev_detected\Debug\dev_detected.exe"
// Remove service - "sc delete DeviceTracking"
// Output files programm - "C:\Windows\System32\MySevice\"
#include <iostream>
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <string>
#include <dbt.h>
#include <tchar.h>
#include <fstream>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// GLOBAL VARIABLE SERVICE
WCHAR SERVICE_NAME[] = { 'D','e','v','i','c','e','T','r','a','c','k','i','n','g','\0'}; // Name service
SERVICE_STATUS STATUS_SERVICE = {0}; //Status of service information
SERVICE_STATUS_HANDLE STATUS_HANDLE = NULL; // Current status of the service
DEV_BROADCAST_DEVICEINTERFACE NOTIFICATION_FILTER = {0};

// CASTOM FUNCTIONS FOR THE SERVICE
void LogMessage(const char* text); // Recording logs to a file
void GetListGUIDClasses(); // Get list GUID classes fro register

// NECESSARY FUNCTIONS FOR THE SERVICE
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* srgv); // Entry point to the service
DWORD __stdcall ServiceCtrlHandlerEx(DWORD ctlcode, DWORD evtype, PVOID evdata, DWORD request); // Service event handler

// Entry point into the program
int _tmain(int argc, TCHAR *argv[]) {

	
	// Information about service
	SERVICE_TABLE_ENTRY service_table[] = {
		{SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	LogMessage("SERVICE START");
	if (StartServiceCtrlDispatcher(service_table) == false) {
		LogMessage("ERROR IN FUNCTION '_tmain->StartServiceCtrlDispatcher'");
	}
	return 0;
}

// Entry point to the service
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* srgv) {
	
	// Register in Service Manager and Register an event handler
	STATUS_HANDLE = RegisterServiceCtrlHandlerEx(SERVICE_NAME, (LPHANDLER_FUNCTION_EX)ServiceCtrlHandlerEx, 0);
	if (STATUS_HANDLE == NULL) {
		LogMessage("ERROR IN FUNCTION 'ServiceMain->RegisterServiceCtrlHandlerEx'");
		return;
	}

	// Initial description of service status
	STATUS_SERVICE.dwServiceType = SERVICE_WIN32_OWN_PROCESS; // The service works in its own process.
	STATUS_SERVICE.dwCurrentState = SERVICE_START_PENDING; // Service pending
	// Control codes that the service receives and processes in its handler function
	STATUS_SERVICE.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	// COMMENTS AND REVIEW NEEDED
	STATUS_SERVICE.dwWin32ExitCode = 0;
	STATUS_SERVICE.dwServiceSpecificExitCode = 0;
	STATUS_SERVICE.dwCheckPoint = 0;
	STATUS_SERVICE.dwWaitHint = 0;
	
	// Updates service status information
	if (SetServiceStatus(STATUS_HANDLE, &STATUS_SERVICE) == false) {
		LogMessage("ERROR IN FUNCTION 'ServiceMain->SetServiceStatus 1'");
		return;
	}

	GetListGUIDClasses();

	// Signing the service to receive notifications of changes in the current equipment configuration 
	//NOTIFICATION_FILTER.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	//NOTIFICATION_FILTER.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	//NOTIFICATION_FILTER.dbcc_reserved = 0;
	//HDEVNOTIFY device_notify = RegisterDeviceNotification(STATUS_HANDLE, );
	
	// Start service
	STATUS_SERVICE.dwCurrentState = SERVICE_RUNNING;
	if (SetServiceStatus(STATUS_HANDLE, &STATUS_SERVICE) == false) {
		LogMessage("ERROR IN FUNCTION 'ServiceMain->SetServiceStatus 2'");
		return;
	}

	while (STATUS_SERVICE.dwCurrentState == SERVICE_RUNNING) {
		// The service works until you turn it off
	}
}

// Service event handler
DWORD __stdcall ServiceCtrlHandlerEx(DWORD ctlcode, DWORD evtype, PVOID evdata, DWORD request) {
	switch (ctlcode) {
	case SERVICE_ACCEPT_STOP: // Service stop command
		STATUS_SERVICE.dwWin32ExitCode = 0;
		STATUS_SERVICE.dwCurrentState = SERVICE_CONTROL_STOP;
		if (SetServiceStatus(STATUS_HANDLE, &STATUS_SERVICE) == false) {
			LogMessage("ERROR IN FUNCTION 'ServiceCtrlHandlerEx->SERVICE_ACCEPT_STOP->SetServiceStatus'");
			return ERROR;
		}
		LogMessage("SERVICE STOP");
		return NO_ERROR;
	}
	return NO_ERROR;
}

// Recording logs to a file
void LogMessage(const char * text) {
	std::fstream file; 
	// Opening a file for log entries
	// C:/Windows/System32/MySevice/Log.txt
	file.open("D:/Log/Log.txt", std::fstream::app);
	if (text != NULL) {
		file << '\n' << "TIME: HH:MM" << " | " << text;
	}
	else {
		file << '\n' << "TEXT == NULL";
	}
	file.close();
}


// Get list GUID classes fro register
void GetListGUIDClasses() {
	HKEY hKey;
	// Opening a register key with GUID classes
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\"),
		0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		
		DWORD retCode;
		TCHAR achClass[MAX_PATH] = TEXT("");  // buffer for class name 
		DWORD cchClassName = MAX_PATH;	 // size of class string 
		DWORD cSubKeys = 0;	 // number of subkeys
		DWORD cbMaxSubKey;	// longest subkey size 
		DWORD cchMaxClass;	// longest class string 
		DWORD cValues;	// number of values for key 
		DWORD cchMaxValue;	// longest value name
		DWORD cbMaxValueData;	// longest value data
		DWORD cbSecurityDescriptor;	// size of security descriptor 
		FILETIME ftLastWriteTime;	// last write time 


		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(hKey,
			achClass, &cchClassName,
			NULL, &cSubKeys, &cbMaxSubKey,
			&cchMaxClass, &cValues, &cchMaxValue,
			&cbMaxValueData, &cbSecurityDescriptor,
			&ftLastWriteTime);

		// Enumerate the subkeys, until RegEnumKeyEx fails.
		if (cSubKeys) {

			LPSTR achKey = new char[MAX_KEY_LENGTH];
			LogMessage("GUID CLASSES");
			DWORD cbName; // size of name string 

			for (DWORD i = 0; i < cSubKeys; ++i) {

				cbName = MAX_KEY_LENGTH;

				retCode = RegEnumKeyExA(hKey, i,
					achKey, &cbName, NULL, NULL, NULL,
					&ftLastWriteTime);

				if (retCode == ERROR_SUCCESS) {
					//LogMessage(achKey);


				}


			}
			delete[] achKey;
		}
		RegCloseKey(hKey); // Close a register
	}
	else {
		LogMessage("ERROR OPEN REGISTER");
	}
	//RegQueryInfoKey(HKEY_LOCAL_MACHINE, );
}