#include "COMManager.h"
#include <vector>
#include <thread>
#include <assert.h>

COMManager::COMManager(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc) :
	PortManager(log, config, dataManager, threadManager, closeFunc), _scanner(log)
{
	// 비동기 입출력 이벤트 생성
	_wOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_rOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// COM 포트 개방용 데이터 저장
	_portName     = _config.deviceName;
	_baudRate     = _config.baudRate;
	_dataBits     = (char)_config.dataBit;
	_parity       = (char)_config.parity;
	_stopBits     = (char)_config.stopBit;
	_flowControl  = (eFlowControl)_config.flowControl;
}

COMManager::~COMManager()
{
	Close();
}

bool COMManager::Open()
{
	_scanner.SetDeviceInfo(_portName, handle);

	if (_portName.substr(0, 3) != L"COM") {
		_log.Error(L"잘못된 포트 이름입니다 : " + _portName);
        return false;
    }

	for (size_t i = 3; i < _portName.size(); ++i) {
        if (!std::isdigit(_portName[i])) {
			_log.Error(L"잘못된 포트 이름입니다 : " + _portName);
            return false;
        }
    }

	_isConnecting = true;
	_comHandle = _scanner.GetCOMHandle(_portName, _baudRate, _dataBits, _parity, _stopBits, _flowControl);

	if (!_isAlive) return false;

	if (_comHandle == INVALID_HANDLE_VALUE) {
		_log.Developer(L"[%s, %d] COMManager::Open() - Invalid handle value 에러", _config.deviceName, handle);
		return false;
	}

	return true;
}

bool COMManager::Connect()
{
	return true;
}

void COMManager::Close()
{
	_isAlive = false;
	if (_comHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(_comHandle);
		_comHandle = INVALID_HANDLE_VALUE;
	}
}

int COMManager::SendData(const char* buff, int length)
{
	if (_comHandle == INVALID_HANDLE_VALUE) return 0;

	DWORD bytesWritten = 0;
	while (true) {
		BOOL writeResult = WriteFile(_comHandle, buff, length, &bytesWritten, &_wOverlapped);
		if (!writeResult) {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Developer(L"[%d, %s] COMManager::Send - write file error", _config.deviceName, handle);
				_log.Error(L"데이터 송신에 실패했습니다.");
				return bytesWritten;
			}
			// 비동기 작업이 시작되었으며, 완료를 기다려야 함
			else {
				if (GetOverlappedResult(_comHandle, &_wOverlapped, &bytesWritten, TRUE)) {
					// bytesWritten에는 이제 실제로 쓴 바이트 수가 저장됩니다.
					break;
				} else {
					// GetOverlappedResult 에러 처리
					break;
				}
			}
		}
	}

	return bytesWritten;
}

int COMManager::RecvData(char* data, int len)
{
	if (!_isAlive) return -1;
	if (_comHandle == INVALID_HANDLE_VALUE) return -1;

	DWORD bytesRead = 0;

	if (_dataReady) {
		if (ReadFile(_comHandle, data, len, &bytesRead, &_rOverlapped)) {
			// 데이터 수신 성공
			goto READ_DATA;
		}
		else {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Error(L"데이터 수신에 실패했습니다.");
				return 0;
			}
			else {
				// 데이터 수신 대기
				goto WAIT_FOR_DATA;
			}
		}
	}
	else {
		// 데이터 수신 확인
		if (GetOverlappedResult(_comHandle, &_rOverlapped, &bytesRead, FALSE)) {
			// 데이터 수신 성공
			goto READ_DATA;
		}
		else {
			// 데이터 수신 대기
			goto WAIT_FOR_DATA;
		}
	}

WAIT_FOR_DATA:
	_dataReady = false;
	return 0;

READ_DATA:
	_dataReady = true;
	return bytesRead;
}