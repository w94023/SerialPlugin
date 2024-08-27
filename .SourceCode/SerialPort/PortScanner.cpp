#include "PortScanner.h"

#include <string>
#include <locale>
#include <codecvt>

PortScanner::PortScanner(LogManager& log): 
	_log(log) { }

void PortScanner::SetDeviceInfo(std::wstring deviceName, int deviceHandle)
{
	_deviceName = deviceName;
	_deviceHandle = deviceHandle;

	_log.Developer(L"PortScanner::SetDeviceInfo - set device : %s, %d", _deviceName.c_str(), _deviceHandle);
}

void PortScanner::OnFileAccessError(const wchar_t *errHeader)
{
	int errCode = GetLastError();
	LPSTR errString = NULL;

	int size = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0, errCode, 0, (LPWSTR)&errString, 0, 0); 

	_log.Developer(L"[%s, %d] %s [Error code] %d : %s", 
		_deviceName.c_str(), _deviceHandle, errHeader, errCode, errString);

	LocalFree (errString); 
}

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

std::vector<DeviceInfo> PortScanner::ScanDevices(GUID guid)
{
	std::vector<DeviceInfo> deviceInfoVector;

	// Bluetooth 혹은 COM port GUID 기반으로 hDevInfo 생성
	// hDevInfo 생성 실패 시 함수 동작 종료
	HDEVINFO hDevInfo;
	hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		_log.Developer(L"Failed to create HDEVINFO from GUID");
		return deviceInfoVector;
	}

	// SP_DEVINFO_DATA 클래스 생성
	SP_DEVINFO_DATA DeviceInfoData;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	// hDevInfo에서 조회 가능한 SP_DEVINFO_DATA 클래스 탐색
	for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++) {
		// SPDRP_FRIENDLYNAME : 장치의 "사용자 친화적 이름(Friendly Name)"을 가져옵니다. 이 이름은 장치를 사용자가 쉽게 식별할 수 있도록 하는 텍스트 문자열입니다.
		// (예를 들어, USB 드라이브의 경우, SPDRP_FRIENDLYNAME은 "SanDisk USB Drive"와 같은 이름을 반환할 수 있습니다.)
		// SPDRP_HARDWAREID : 장치의 특정 모델이나 제조사 정보를 포함하고 있어, 장치 종류를 정확하게 구별할 수 있는 하드웨어 ID를 가져옵니다.
		// (USB 장치의 일반적인 하드웨어 ID는 다음과 같은 형식을 따릅니다.)
		// (USB\VID_xxxx&PID_yyyy)

		//장치 이름 불러오기
		TCHAR deviceName[MAX_PATH] = { 0 };
		DWORD propertyType;
		if (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, &propertyType, (PBYTE)deviceName, sizeof(deviceName), NULL)) {
			DWORD error = GetLastError();
			_log.Developer(L"Failed to get device name, i : %d, error code : %s", (int)i, std::to_wstring(error).c_str());
			continue;
		}
		if (propertyType != REG_SZ) continue;
		_log.Developer(L"Device name : %s", deviceName);

		// 장치 ID 조회
		TCHAR deviceID[MAX_DEVICE_ID_LEN] = { 0 };
		CM_Get_Device_ID(DeviceInfoData.DevInst, deviceID, MAX_DEVICE_ID_LEN, 0);
		_log.Developer(L"Device ID : %s", deviceID);

		// 장치 하드웨어 ID 불러오기
		// 장치 하드웨어 ID로 데이터 타입 분류
		DeviceType deviceType = None;
		wchar_t hardwareID[256];
		wchar_t deviceClass[256];
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_HARDWAREID, nullptr, (BYTE*)hardwareID, sizeof(hardwareID), nullptr)) {
			_log.Developer(L"Device hardware ID : %s", hardwareID);

			// USB 장치 확인
			if (wcsstr(hardwareID, L"USB") || wcsstr(hardwareID, L"BUS")) {
				deviceType = USB_COM;
			}
			// Bluetooth Classic 장치 확인
			else if (wcsstr(hardwareID, L"BTHENUM") || wcsstr(hardwareID, L"Bluetooth")) {
				deviceType = BTClassic;
			}
			// BLE 장치 확인
			else if (wcsstr(hardwareID, L"BTHLE")) {
				deviceType = BLE;
			}
		}
		else {
			DWORD error = GetLastError();
			_log.Developer(L"Failed to get device hardware ID, i : %d, error code : %s", (int)i, std::to_wstring(error).c_str());
			continue;
		}

		// 장치 클래스 가져오기
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_CLASS, nullptr, (BYTE*)deviceClass, sizeof(deviceClass), nullptr)) {
			_log.Developer(L"Device class : %s", deviceClass);
		}
		else {
			DWORD error = GetLastError();
			_log.Developer(L"Failed to get device class, i : %d, error code : %s", (int)i, std::to_wstring(error).c_str());
			continue;
		}
		

		// 디바이스의 ID, 이름, 타입, 클래스를 벡터에 저장
		DeviceInfo deviceInfo = DeviceInfo();
		std::wstring deviceNameStr(deviceName);   deviceInfo.deviceName  = deviceNameStr;
		std::wstring deviceIDStr(deviceID);       deviceInfo.deviceID    = deviceIDStr;
		std::wstring deviceClassStr(deviceClass); deviceInfo.deviceClass = deviceClassStr;
												  deviceInfo.deviceType  = deviceType;
		deviceInfoVector.push_back(deviceInfo);
	}

	// SP_DEVINFO_DATA 정보 삭제
	// 메모리 누수 방지를 위해 필수
	SetupDiDestroyDeviceInfoList(hDevInfo);

	return deviceInfoVector;
}

void PortScanner::ScanDevices()
{
	EraseList();

	// PC에서 조회 가능한 COM, BT 디바이스 ID 및 이름 조회
	std::vector<DeviceInfo> COMDevInfo = ScanDevices(GUID_DEVCLASS_PORTS);
	std::vector<DeviceInfo> BTDevInfo  = ScanDevices(GUID_DEVCLASS_BLUETOOTH);
	// COM, BT로 조회한 디바이스 정보 벡터 합치기
	std::vector<DeviceInfo> devInfo;
	devInfo.reserve(COMDevInfo.size() + BTDevInfo.size());
    std::copy(COMDevInfo.begin(), COMDevInfo.end(), std::back_inserter(devInfo));
    std::copy(BTDevInfo.begin(),   BTDevInfo.end(), std::back_inserter(devInfo));

	//USB 장치 분류
	for (size_t i = 0; i < devInfo.size(); i++) {
		if (devInfo[i].deviceType == USB_COM) {
			// 장치 이름에서 "(", ")" 사이의 이름을 추출하여 반환
			// 일반적으로 USB 디바이스는 이름의 괄호 사이에 COM포트 번호가 등록되어 있음
			// --> COM# (#는 포트 번호)로 USB 장치의 이름을 반환
			size_t start = devInfo[i].deviceName.find(L"(");
			size_t end   = devInfo[i].deviceName.find(L")");
			if (start == std::string::npos || end == std::string::npos) continue;

			_deviceTypeList[_deviceCount] = USB_COM;
			_deviceIDList[_deviceCount]   = SplitWstring(devInfo[i].deviceID, L"\\")[1];
			_deviceNameList[_deviceCount] = devInfo[i].deviceName.substr(start + 1, end - start - 1);
			_deviceCount++;
		}
		else if (devInfo[i].deviceType == BTClassic) {
			_deviceTypeList[_deviceCount] = BTClassic;
			_deviceIDList[_deviceCount]   = SplitWstring(SplitWstring(devInfo[i].deviceID, L"\\")[1], L"_")[1];
			_deviceNameList[_deviceCount] = devInfo[i].deviceName;
			_deviceCount++;
		}
		else if (devInfo[i].deviceType == BLE) {
			_deviceTypeList[_deviceCount] = BLE;
			_deviceIDList[_deviceCount]   = SplitWstring(SplitWstring(devInfo[i].deviceID, L"\\")[1], L"_")[1];
			_deviceNameList[_deviceCount] = devInfo[i].deviceName;
			_deviceCount++;
		}
	}

	_log.Developer(L"[%s, %d] PortScanner::ScanDevices - 스캔 완료", _deviceName.c_str(), _deviceHandle);
	_log.Developer(L"[%s, %d] PortScanner::ScanDevices - deviceCount : %d", _deviceName.c_str(), _deviceHandle, _deviceCount);
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

				// 검색된 장치 이름 std::wstring 형으로 전환
				std::wstring foundDeviceName(bdi.szName);
				
				if (foundDeviceName == deviceName) {
					_log.Normal(L"장치 검색 성공 - 페어링 요청 전송 : %s", foundDeviceName.c_str());

					BLUETOOTH_OOB_DATA_INFO oobData;
					ZeroMemory(&oobData, sizeof(BLUETOOTH_OOB_DATA_INFO));

					// BT classic 장치 페어링 요청
					DWORD result = BluetoothAuthenticateDeviceEx(NULL, hRadio, &bdi, &oobData, MITMProtectionNotRequired);
					if (result == ERROR_SUCCESS) {
						_log.Normal(L"장치 페어링에 성공하였습니다");
						pairingResult = 1;
					} 
					else {
						_log.Error(L"장치 페어링에 실패하였습니다 - [오류] %s", std::to_wstring(result).c_str());
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

	//if (_deviceCount == 0) {
	//	_log.Error(L"검색된 블루투스 장치가 없습니다");
	//	return -1;
	//}
	if (deviceName == L"") {
		_log.Error(L"잘못된 장치 이름 입력입니다");
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
		_log.Normal(L"여러 개의 장치가 검색되었습니다 - 첫 번째 장치로 연결 요청 : %s", deviceName.c_str());
		_log.Developer(L"[$s, $d] PortScanner::CheckDeviceName - [targetDeviceCount] %d", deviceName.c_str(), _deviceHandle, targetDeviceCount);
		return targetDeviceIndex;
	}
	else if (targetDeviceCount == 1) {
		_log.Normal(L"장치 검색 성공 : %s", deviceName.c_str());
		return targetDeviceIndex;
	}
	else if (targetDeviceCount == 0) {
		if (!isBTDevice) {
			_log.Error(L"장치 검색에 실패했습니다 : %s", deviceName.c_str());
			return -1;
		}
		else {
			_log.Normal(L"페어링된 장치가 없습니다 - 페어링 요청 시작 : %s", deviceName.c_str());

			size_t pairingTryCount = 0;
			int    isPaired = 0;

			do {
				if (pairingTryCount == requestCountLimit) {
					_log.Error(L"장치 페어링에 실패했습니다 : %s", deviceName.c_str());
					return -1;
				}

				_log.Normal(L"%s 페어링 요청 (%d 회)", deviceName.c_str(), pairingTryCount + 1);

				isPaired = RequestPairing(deviceName);
				if (isPaired == -2) return -1;
				pairingTryCount++;

				if (_forceStop) return -1;

			} while (isPaired != 1);

			Sleep(5000);

			// 페어링된 장치 다시 검색
			return CheckDeviceName(deviceName, isBTDevice);
		}
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
	std::vector<std::wstring> scannedIDList;
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
			scannedIDList.push_back(currDeviceIDStr);
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
		_log.Developer(L"[%s, %d] PortScanner::GetBTHandle - ID 매칭 실패", _deviceName.c_str(), _deviceHandle);
		_log.Developer(L"[%s, %d] PortScanner::GetBTHandle - 스캔된 장치 개수 : %d", _deviceName.c_str(), _deviceHandle, scanCount);
		_log.Developer(L"[%s, %d] PortScanner::GetBTHandle - 예상된 ID : %s", _deviceName.c_str(), _deviceHandle, _deviceIDList[targetDeviceIndex].c_str());
		_log.Developer(L"[%s, %d] PortScanner::GetBTHandle - 검색된 ID :", _deviceName.c_str(), _deviceHandle);
		for (const std::wstring& wstr : scannedIDList) {
			_log.Developer(wstr);
		}
		_log.Error(L"Failed to connect device");
	}

	return handle;
}

HANDLE PortScanner::GetCOMHandle(std::wstring portName, int baudRate, char dataBits, char parity, char stopBits, eFlowControl flowControl)
{
	HANDLE handle = INVALID_HANDLE_VALUE;

	if (CheckDeviceName(portName, false) == -1) {
		_log.Error(L"GetCOMHandle - CheckDeviceName return -1");
		return handle;
	}

	handle = CreateFile((L"\\\\.\\" + portName).c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	if (handle == INVALID_HANDLE_VALUE) {
		OnFileAccessError(L"TcpServer::GetCOMHandle - CreateFile 매크로 에러 :");
		_log.Error(L"USB 디바이스 연결에 실패했습니다.");
		return handle;
	}

	SetupComm(handle, 8192, 8192);
	PurgeComm(handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	DCB  dcb;
	//dcb의 기본값을 받는다.
	if (!GetCommState(handle, &dcb)) {
		OnFileAccessError(L"TcpServer::GetCOMHandle - GetCommState 메서드 에러 :");
		_log.Error(L"USB 디바이스 연결에 실패했습니다.");
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
		OnFileAccessError(L"TcpServer::GetCOMHandle - SetCommState 메서드 에러 :");
		_log.Error(L"USB 디바이스 연결에 실패했습니다.");
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
		OnFileAccessError(L"TcpServer::GetCOMHandle - SetCommTimeouts 메서드 에러 :");
		_log.Error(L"USB 디바이스 연결에 실패했습니다.");
		return INVALID_HANDLE_VALUE;
	}
}