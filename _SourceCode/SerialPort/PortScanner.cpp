#include "PortScanner.h"

#include <string>
#include <locale>
#include <codecvt>

PortScanner::PortScanner(LogManager& log) : _log(log) { }

void PortScanner::ForceStop()
{
	_forceStop = true;
}

void PortScanner::EraseList()
{
	_deviceCount = 0;
	_deviceTypeList.clear();
	_deviceNameList.clear();
	_deviceIDList.clear();
}

std::vector<std::wstring> PortScanner::ScanDevices(GUID guid)
{
	std::vector<std::wstring> deviceIDAndName;

	// Bluetooth 혹은 COM port GUID 기반으로 hDevInfo 생성
	// hDevInfo 생성 실패 시 함수 동작 종료
	HDEVINFO hDevInfo;
	hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		_log.Developer(L"Failed to create HDEVINFO from GUID");
		return deviceIDAndName;
	}

	// SP_DEVINFO_DATA 클래스 생성
	SP_DEVINFO_DATA DeviceInfoData;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	// hDevInfo에서 조회 가능한 SP_DEVINFO_DATA 클래스 탐색
	for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
		// 디바이스의 이름 조회
		TCHAR deviceName[MAX_PATH] = { 0 };
		DWORD propertyType;
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, &propertyType, (PBYTE)deviceName, sizeof(deviceName), NULL)) continue;
		if (propertyType != REG_SZ) continue;

		// 디바이스의 ID 조회
		TCHAR deviceID[MAX_DEVICE_ID_LEN] = { 0 };
		CM_Get_Device_ID(DeviceInfoData.DevInst, deviceID, MAX_DEVICE_ID_LEN, 0);

		// 디바이스의 ID 및 이름 wstring 형으로 변환하여 벡터에 저장
		std::wstring deviceIDStr(deviceID);
		std::wstring deviceNameStr(deviceName);
		deviceIDAndName.push_back(deviceID);
		deviceIDAndName.push_back(deviceName);

		//wprintf(L"\n");
		//wprintf(L"Device found (ID) : %s\n", deviceIDStr.c_str());
		//wprintf(L"Device found (Name) : %s\n", deviceNameStr.c_str());
	}

	// SP_DEVINFO_DATA 정보 삭제
	// 메모리 누수 방지를 위해 필수
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return deviceIDAndName;
}

void PortScanner::ScanDevices()
{
	EraseList();

	// PC에서 조회 가능한 COM, BT 디바이스 ID 및 이름 조회
	std::vector<std::wstring> COMDevInfo = ScanDevices(GUID_DEVCLASS_PORTS);
	std::vector<std::wstring> BTDevInfo  = ScanDevices(GUID_DEVCLASS_BLUETOOTH);

	// COM Device ID refining 및 List로 저장
	for (size_t i = 0; i < COMDevInfo.size()/2; i++) {
		if (COMDevInfo[2*i + 1].find(L"USB") == std::wstring::npos) continue;

		size_t start = COMDevInfo[2*i + 1].find(L"(");
		size_t end   = COMDevInfo[2*i + 1].find(L")");
		if (start == std::string::npos || end == std::string::npos) continue;
		if (start > end) continue;
		std::wstring deviceNameStr = COMDevInfo[2*i + 1].substr(start + 1, end - start - 1);

		_deviceTypeList[_deviceCount] = USB_COM;
		_deviceIDList[_deviceCount]   = SplitWstring(COMDevInfo[2*i], L"\\")[1];
		_deviceNameList[_deviceCount] = deviceNameStr;
		_deviceCount++;
	}

	// BT Device ID refining 및 List로 저장
	for (size_t i = 0; i < BTDevInfo.size()/2; i++) {
		std::wstring checkType = SplitWstring(BTDevInfo[2*i], L"\\")[0];
		std::wstring checkDev  = SplitWstring(BTDevInfo[2*i], L"\\")[1];

		if (checkDev.find(L"DEV") == std::wstring::npos) continue;

		if      (checkType == L"BTHLE")   _deviceTypeList[_deviceCount] = BTClassic;
		else if (checkType == L"BTHENUM") _deviceTypeList[_deviceCount] = BLE;

		std::wstring deviceIDStr = SplitWstring(checkDev, L"_")[1];

		_deviceIDList[_deviceCount]   = deviceIDStr;
		_deviceNameList[_deviceCount] = BTDevInfo[2*i + 1];
		_deviceCount++;
	}

	_log.Developer(L"PortScanner::ScanDevices - scan finished [_deviceCount]" + std::to_wstring(_deviceCount));
	if (onScanEnded) onScanEnded();
}

int PortScanner::RequestPairing(std::wstring deviceName)
{
	BLUETOOTH_FIND_RADIO_PARAMS btfrp = { sizeof(BLUETOOTH_FIND_RADIO_PARAMS) };
    HANDLE hRadio = INVALID_HANDLE_VALUE;
    HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&btfrp, &hRadio);
	int pairingResult = -1;

    if (hFind == NULL) {
		_log.Developer(L"Failed to find Bluetooth radio");
		_log.Error(L"Please check if the Bluetooth setting is turned on");
        return -2;
    }

    do {
        BLUETOOTH_RADIO_INFO bri = { sizeof(BLUETOOTH_RADIO_INFO), 0 };
        if (BluetoothGetRadioInfo(hRadio, &bri) == ERROR_SUCCESS) {
			_log.Developer((std::wstring(L"Found Bluetooth radio : ") + std::wstring(bri.szName)).c_str());
        }

        BLUETOOTH_DEVICE_SEARCH_PARAMS bdsp = {
            sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
            1,
            0,
            1,
            1,
            1,
            10,
            hRadio
        };

        BLUETOOTH_DEVICE_INFO bdi = { sizeof(BLUETOOTH_DEVICE_INFO), 0 };
        HBLUETOOTH_DEVICE_FIND hDeviceFind = BluetoothFindFirstDevice(&bdsp, &bdi);

        if (hDeviceFind == NULL) {
			_log.Error(L"Failed to find Bluetooth device");
			return -1;
        } 
		else {
            do {
				if (_forceStop) break;

				std::wstring foundDeviceName(bdi.szName);
				_log.Normal((std::wstring(L"Found Bluetooth device : ") + foundDeviceName).c_str());
				
				if (foundDeviceName == deviceName) {
					_log.Normal((std::wstring(L"Request pairing to device : ") + foundDeviceName).c_str());

					BLUETOOTH_OOB_DATA_INFO oobData;
					ZeroMemory(&oobData, sizeof(BLUETOOTH_OOB_DATA_INFO));

					DWORD result = BluetoothAuthenticateDeviceEx(NULL, hRadio, &bdi, &oobData, MITMProtectionNotRequired);
					if (result == ERROR_SUCCESS) {
						_log.Normal(L"Device paired successfully");
						pairingResult = 1;
					} 
					else {
						_log.Error((std::wstring(L"Device pairing failed with error : ") + std::to_wstring(result)).c_str());
						switch (result) {
							case 1244: _log.Error(L"Pairing request was canceled"); break;
							case 170:  _log.Error(L"Previous request has not been completed"); break;
						}
					}
					break;
                }
            } while (BluetoothFindNextDevice(hDeviceFind, &bdi));
            BluetoothFindDeviceClose(hDeviceFind);
        }
        CloseHandle(hRadio);
    } while (BluetoothFindNextRadio(hFind, &hRadio));
    BluetoothFindRadioClose(hFind);

	return pairingResult;
}

int PortScanner::CheckDeviceName(std::wstring deviceName, bool isBTDevice)
{
	ScanDevices();

	// Output : -2   -> 진행 불가능한 오류 (connection failed)
	// Output : -1   -> 페어링 request 진행
	// Output : >= 0 -> 검색된 index로 연결 절차 수행

	if (_deviceCount == 0) {
		_log.Error(L"Failed to find scanned device");
		return -1;
	}
	if (deviceName == L"") {
		_log.Error(L"Given device name is empty");
		return -1;
	}

	int targetDeviceCount = 0;
	int targetDeviceIndex = 0;
	for (int i = 0; i < _deviceCount; i++) {
		if (_deviceNameList[i] == deviceName) {
			targetDeviceCount++;
			targetDeviceIndex = i;
		}
	}

	if (targetDeviceCount > 1) {
		_log.Normal(L"Multiple devices found : " + deviceName);
		_log.Developer(L"PortScanner::CheckDeviceName - [targetDeviceCount]" + std::to_wstring(targetDeviceCount));
		return targetDeviceIndex;
	}
	else if (targetDeviceCount == 1) {
		_log.Normal((std::wstring(L"Device found : ") + deviceName).c_str());
		return targetDeviceIndex;
	}
	else if (targetDeviceCount == 0) {
		if (!isBTDevice) {
			_log.Error((std::wstring(L"Failed to find device : ") + deviceName).c_str());
			return -1;
		}

		_log.Normal(L"Failed to find device in scanned device list : request pairing");

		size_t pairingTryCount = 0;
		int    isPaired = 0;

		do {
			if (pairingTryCount == requestCountLimit) {
				_log.Error(L"Failed to pair device");
				return -1;
			}
			_log.Normal((std::wstring(L"Try to pair device : count ") + std::to_wstring(pairingTryCount + 1)).c_str());

			isPaired = RequestPairing(deviceName);
			if (isPaired == -2) return -1;
			pairingTryCount++;

			if (_forceStop) return -1;

		} while (isPaired != 1);

		Sleep(5000);

		_log.Normal(L"Rescan ports");
		return CheckDeviceName(deviceName, isBTDevice);
	}
}

HANDLE PortScanner::GetBTHandle(std::wstring deviceName, std::wstring uuidService, bool isBLEDevice)
{
	_forceStop = false;

	HANDLE handle = INVALID_HANDLE_VALUE;

	int targetDeviceIndex = CheckDeviceName(deviceName, !isBLEDevice);
	if (targetDeviceIndex == -1) return handle;

	HDEVINFO				 hDevInfo;
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    SP_DEVINFO_DATA			 deviceInfoData;
	GUID					 guid;

	// String으로 전달 받은 uuid service 정보를 GUID 클래스에 입력
	if(IIDFromString(uuidService.c_str(), &guid) != S_OK) {
		_log.Developer(L"PortScanner::GetBTHandle - wrong Bluetooth classic service UUID");
		_log.Error(L"Failed to connect device");
		return handle;
	}

	hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	if (hDevInfo == INVALID_HANDLE_VALUE) {
		_log.Developer(L"PortScanner::GetBTHandle - failed to get bluetooth device info");
		_log.Error(L"Failed to connect device");
		return handle;
	}

	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	deviceInfoData.cbSize      = sizeof(SP_DEVINFO_DATA);

	int scanCount    = 0;
	int IdMatchedCount = 0;
	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, i, &deviceInterfaceData); i++)
	{
		SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;
		DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		DWORD size = 0;
		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &deviceInterfaceData, NULL, 0, &size, 0))
		{
			int err = GetLastError();
			if (err == ERROR_NO_MORE_ITEMS) break;

			PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);
			pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &deviceInterfaceData, pInterfaceDetailData, size, &size, &deviceInfoData))
				continue;

			wchar_t currDeviceID[MAX_DEVICE_ID_LEN];
			CM_Get_Device_ID(deviceInfoData.DevInst, currDeviceID, MAX_DEVICE_ID_LEN, 0);
			std::wstring currDeviceIDStr = currDeviceID;
			//wprintf(L"current device ID : %s\n", currDeviceIDStr.c_str());
			//wprintf(L"target device ID : %s\n", _deviceIDList[targetDeviceIndex].c_str());

			if (currDeviceIDStr.find(_deviceIDList[targetDeviceIndex]) != std::wstring::npos) {
				handle = CreateFile(
					pInterfaceDetailData->DevicePath,
					GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED,
					NULL);

				if (handle == INVALID_HANDLE_VALUE) {
					DWORD errorCode = GetLastError();
					_log.Developer(L"PortScanner::GetBTHandle - failed to create handle [errorCode]"+ std::to_wstring(errorCode));
					_log.Error(L"The BT device failed to connect. Please unpair the device and try reconnecting");
					return handle;
				}

				_log.Developer(L"PortScanner::GetBTHandle - success to create handle [deviceName]"+ _deviceNameList[targetDeviceIndex]);

				//if (_deviceTypeList[targetDeviceIndex] == BTClassic) {
				if(!isBLEDevice) {
					handle = SetTimeout(handle, _handleTimeout);
				}

				return handle;
			}
			scanCount++;
			GlobalFree(pInterfaceDetailData);
		}
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	if (scanCount == 0) {
		_log.Developer(L"Failed find interface using given UUID");
		_log.Error(L"Failed to connect device");
	}
	else {
		_log.Developer(L"Interface found but ID was not matched");
		_log.Error(L"Failed to connect device");
	}

	return handle;
}

HANDLE PortScanner::GetCOMHandle(std::wstring portName, int baudRate, char dataBits, char parity, char stopBits, eFlowControl flowControl)
{
	HANDLE handle = INVALID_HANDLE_VALUE;

	if (CheckDeviceName(portName, false) == -1) return handle;

	handle = CreateFile((L"\\\\.\\" + portName).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	if (handle == INVALID_HANDLE_VALUE) {
		_log.Developer(L"Failed to get COM handle");
		_log.Error(L"Failed to connect device");
		return handle;
	}

	SetupComm(handle, 8192, 8192);
	PurgeComm(handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	DCB  dcb;
	//dcb의 기본값을 받는다.
	if (!GetCommState(handle, &dcb)) {
		DWORD error = GetLastError();
		_log.Developer(L"Failed to get COM state : " + std::to_wstring(error));
		_log.Error(L"Failed to connect device");
		return handle;
	}

	// Set the new data
	dcb.BaudRate = baudRate;		// Baudrate at which running       
	dcb.ByteSize = dataBits;		// Number of bits/byte, 4-8     
	dcb.Parity   = parity;			// 0-4=None,Odd,Even,Mark,Space    
	dcb.StopBits = stopBits;		// 0,1,2 = 1, 1.5, 2               
	dcb.fParity  = parity ? true : false;	// Enable parity checking          
  
	// 일단 기본 flow control 옵션을 설정한다.
	// 뒤에서 flow control 옵션에 따라 필요한 항목을 Enable 한다.
	dcb.fOutxCtsFlow = false;		// CTS(Clear To Send) handshaking on output       
	dcb.fOutxDsrFlow = false;		// DSR(Data Set Ready) handshaking on output       
	dcb.fDtrControl  = DTR_CONTROL_ENABLE;	// DTR(Data Terminal Ready) Flow control                
	dcb.fRtsControl  = RTS_CONTROL_ENABLE;	// RTS(Ready To Send) Flow control                
	// Xon, Xoff flow control 시 아래 두 항목을 Enable 한다.
	dcb.fOutX        = false;		// Enable output X-ON/X-OFF        
	dcb.fInX         = false;		// Enable input X-ON/X-OFF         

	//Setup the flow control 
	dcb.fDsrSensitivity = false;
	switch (flowControl) {
	case FC_XONXOFF:	// Software Handshaking 
		dcb.fOutX    = true;
		dcb.fInX     = true;
		dcb.XonChar  = 0x11;		// Tx and Rx X-ON character      
		dcb.XoffChar = 0x13;		// Tx and Rx X-OFF character   
		dcb.XoffLim  = 100;			// Transmit X-ON threshold 
		dcb.XonLim   = 100;			// Transmit X-OFF threshold    
		break;
	case FC_RTSCTS:
		dcb.fOutxCtsFlow = true;
		dcb.fRtsControl  = RTS_CONTROL_HANDSHAKE;
		break;
	case FC_DTRDSR:
		dcb.fOutxDsrFlow = true;
		dcb.fDtrControl  = DTR_CONTROL_HANDSHAKE;
		break;
	case FC_FULLHARDWARE: // Hardware Handshaking
		dcb.fOutxCtsFlow = true;
		dcb.fOutxDsrFlow = true;
		dcb.fDtrControl  = DTR_CONTROL_HANDSHAKE;
		dcb.fRtsControl  = RTS_CONTROL_HANDSHAKE;
		break;
	}

	dcb.fBinary           = true;		// Binary Mode (skip EOF check)    
	dcb.fNull             = false;		// Enable Null stripping           
	dcb.fAbortOnError     = false;		// Abort all reads and writes on Error 
	dcb.fDsrSensitivity   = false;		// DSR Sensitivity              
	dcb.fTXContinueOnXoff = true;		// Continue TX when Xoff sent 
	dcb.fErrorChar        = false;		// Enable Err Replacement          
	dcb.ErrorChar         = 0;			// Error replacement char          
	dcb.EofChar           = 0;			// End of Input character          
	dcb.EvtChar           = 0;			// Received Event character        


	if (!SetCommState(handle, &dcb)) {
		_log.Developer(L"Failed to set COM state");
		_log.Error(L"Failed to connect device");
		return handle;
	}

	handle = SetTimeout(handle, _handleTimeout);
	return handle;
}

std::vector<std::wstring> PortScanner::SplitWstring(std::wstring& input, const std::wstring& delimiter)
{
	std::vector<std::wstring> parts;
    size_t start = 0;
    size_t end = 0;

    while ((end = input.find(delimiter, start)) != std::wstring::npos) {
        parts.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
    }
    parts.push_back(input.substr(start));

    return parts;
}


HANDLE PortScanner::SetTimeout(HANDLE handle, int timeout)
{
	COMMTIMEOUTS commTimeout;
	GetCommTimeouts(handle, &commTimeout);

	int readIntervalTimeout = 1;
	commTimeout.ReadIntervalTimeout = readIntervalTimeout;
	commTimeout.ReadTotalTimeoutMultiplier = 0;
	commTimeout.ReadTotalTimeoutConstant = timeout;
	commTimeout.WriteTotalTimeoutMultiplier = 0;
	commTimeout.WriteTotalTimeoutConstant = timeout;

	if (SetCommTimeouts(handle, &commTimeout)) {
		return handle;
	}
	else {
		_log.Developer(L"Failed to set COM handle timeout");
		_log.Error(L"Failed to connect device");
		return INVALID_HANDLE_VALUE;
	}
}