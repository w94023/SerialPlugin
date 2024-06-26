#include "BTClassicManager.h"

BTClassicManager::BTClassicManager(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc) : 
	PortManager(log, config, dataManager, threadManager, closeFunc), _scanner(log)
{
	// �񵿱� ����� �̺�Ʈ ����
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
				// ���� ó��
				_log.Developer(L"BTClassicManager::Send - write file error");
				_log.Error(L"Failed to send data");
				return -1;
			}
			// �񵿱� �۾��� ���۵Ǿ�����, �ϷḦ ��ٷ��� ��
			else {
				//printf("ERROR_IO_PENDING\n");
				if (GetOverlappedResult(_handle, &_wOverlapped, &bytesWritten, FALSE)) {
					// bytesWritten���� ���� ������ �� ����Ʈ ���� ����˴ϴ�.
					//printf("True : %d\n", bytesWritten);
					break;
				} 
				else {
					// GetOverlappedResult ���� ó��
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
			// ������ ���� ����
			goto READ_DATA;
		}
		else {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// ���� ó��
				_log.Error(L"Failed to read data");
				return 0;
			}
			else {
				// ������ ���� ���
				goto WAIT_FOR_DATA;
			}
		}
	}
	else {
		// ������ ���� Ȯ��
		if (GetOverlappedResult(_handle, &_rOverlapped, &bytesRead, FALSE)) {
			// ������ ���� ����
			goto READ_DATA;
		}
		else {
			// ������ ���� ���
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
