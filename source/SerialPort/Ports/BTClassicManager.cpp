#include "BTClassicManager.h"

BTClassicManager::BTClassicManager(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc) : 
	PortManager(log, config, dataManager, threadManager, closeFunc), _scanner(log)
{
	// 비동기 입출력 이벤트 생성
	_wOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_rOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

BTClassicManager::~BTClassicManager()
{
	Close();
}

bool BTClassicManager::Open()
{
	_scanner.SetDeviceInfo(_config.deviceName, handle);

	std::wstring uuidService = L"{00001101-0000-1000-8000-00805f9b34fb}";

	if (_config.deviceName == L"") {
		_log.Error(L"Invalid device name : " + _config.GetDeviceName());
		return false;
	}

	_isConnecting = true;
	_handle = _scanner.GetBTHandle(_config.GetDeviceName(), uuidService, false);

	if (_handle == INVALID_HANDLE_VALUE) {
		_log.Developer(L"BTClassicManager::Open() - Invalid handle value error");
		return false;
	}

	return true;
}

bool BTClassicManager::Connect()
{
	return true;
}

void BTClassicManager::Close()
{
	_isAlive = false;

	_threadManager.StopThread();
	if (_handle != INVALID_HANDLE_VALUE) {
		CloseHandle(_handle);
		_handle = INVALID_HANDLE_VALUE;
	}
}

void BTClassicManager::ForceClose()
{
	_scanner.ForceStop();
}

int BTClassicManager::SendData(const char* buff, int length)
{
	if (_handle == INVALID_HANDLE_VALUE) return -1;

	DWORD bytesWritten = 0;
	while (true) {
		BOOL writeResult = WriteFile(_handle, buff, length, &bytesWritten, &_wOverlapped);
		if (!writeResult) {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Developer(L"BTClassicManager::Send - write file error");
				_log.Error(L"Failed to send data");
				return -1;
			}
			// 비동기 작업이 시작되었으며, 완료를 기다려야 함
			else {
				//printf("ERROR_IO_PENDING\n");
				if (GetOverlappedResult(_handle, &_wOverlapped, &bytesWritten, FALSE)) {
					// bytesWritten에는 이제 실제로 쓴 바이트 수가 저장됩니다.
					//printf("True : %d\n", bytesWritten);
					break;
				} 
				else {
					// GetOverlappedResult 에러 처리
					//printf("False : %d\n", bytesWritten);
					return -1;
				}
			}
		}
	}

	return bytesWritten;
}

int BTClassicManager::RecvData(char* data, int len)
{
	if (!_isAlive) return -1;
	if (_handle == INVALID_HANDLE_VALUE) return -1;

	DWORD bytesRead = 0;

	if (_dataReady) {
		if (ReadFile(_handle, data, len, &bytesRead, &_rOverlapped)) {
			// 데이터 수신 성공
			goto READ_DATA;
		}
		else {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Error(L"Failed to read data");
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
		if (GetOverlappedResult(_handle, &_rOverlapped, &bytesRead, FALSE)) {
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
