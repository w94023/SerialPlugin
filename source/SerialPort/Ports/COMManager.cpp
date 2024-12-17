#include "COMManager.h"
#include <vector>
#include <thread>
#include <assert.h>

COMManager::COMManager(LogManager& log, ConnectionConfig& config, DataManager& dataManager, CThreadManager& threadManager, std::function<void()> closeFunc) :
	PortManager(log, config, dataManager, threadManager, closeFunc), _scanner(log)
{
	// �񵿱� ����� �̺�Ʈ ����
	_wOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_rOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// COM ��Ʈ ����� ������ ����
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
		_log.Error(L"�߸��� ��Ʈ �̸��Դϴ� : " + _portName);
        return false;
    }

	for (size_t i = 3; i < _portName.size(); ++i) {
        if (!std::isdigit(_portName[i])) {
			_log.Error(L"�߸��� ��Ʈ �̸��Դϴ� : " + _portName);
            return false;
        }
    }

	_isConnecting = true;
	_comHandle = _scanner.GetCOMHandle(_portName, _baudRate, _dataBits, _parity, _stopBits, _flowControl);

	if (!_isAlive) return false;

	if (_comHandle == INVALID_HANDLE_VALUE) {
		_log.Developer(L"[%s, %d] COMManager::Open() - Invalid handle value ����", _config.deviceName, handle);
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
				// ���� ó��
				_log.Developer(L"[%d, %s] COMManager::Send - write file error", _config.deviceName, handle);
				_log.Error(L"������ �۽ſ� �����߽��ϴ�.");
				return bytesWritten;
			}
			// �񵿱� �۾��� ���۵Ǿ�����, �ϷḦ ��ٷ��� ��
			else {
				if (GetOverlappedResult(_comHandle, &_wOverlapped, &bytesWritten, TRUE)) {
					// bytesWritten���� ���� ������ �� ����Ʈ ���� ����˴ϴ�.
					break;
				} else {
					// GetOverlappedResult ���� ó��
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
			// ������ ���� ����
			goto READ_DATA;
		}
		else {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// ���� ó��
				_log.Error(L"������ ���ſ� �����߽��ϴ�.");
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
		if (GetOverlappedResult(_comHandle, &_rOverlapped, &bytesRead, FALSE)) {
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