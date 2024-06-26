#include "COMManager.h"
#include <vector>
#include <thread>
#include <assert.h>

COMManager::COMManager(LogManager& log, EventManager eventManager, std::wstring portName, int baudRate, char dataBits, char parity, char stopBits, eFlowControl flowControl) :
	PortManager(log, eventManager), _scanner(log)
{
	_threadManager.RegisterCallback(std::bind(&COMManager::RecvThreadFunc, this));

	// 비동기 입출력 이벤트 생성
	_wOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_rOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// COM 포트 개방용 데이터 저장
	_portName     = portName;
	_baudRate     = baudRate;
	_dataBits     = dataBits;
	_parity       = parity;
	_stopBits     = stopBits;
	_flowControl  = flowControl;
	
	// 수신 버퍼 할당
	_buffSize = DEFAULT_BUFF_SIZE;
	_recvBuff = new char[_buffSize] { 0 };
}

COMManager::~COMManager()
{
	Close();
}

bool COMManager::Open()
{
	if (_portName.substr(0, 3) != L"COM") {
		_log.Error(L"Invalid port name : " + _portName);
        return false;
    }

	for (size_t i = 3; i < _portName.size(); ++i) {
        if (!std::isdigit(_portName[i])) {
			_log.Error(L"Invalid port name : " + _portName);
            return false;
        }
    }

	_isConnecting = true;
	_handle = _scanner.GetCOMHandle(_portName, _baudRate, _dataBits, _parity, _stopBits, _flowControl);

	if (!_isAlive) return false;

	if (_handle == INVALID_HANDLE_VALUE) {
		_log.Developer(L"COMManager::Open() - Invalid handle value error");
		_log.Error(L"Failed to connect port : " + _portName);
		_eventManager.Invoke("onConnectionFailed");
		return false;
	}

	_log.Developer(L"COMManager::Open() - success");
	return true;
}

void COMManager::Connect()
{
	_log.Developer(L"COMManager:Connect() - start thread");
	_threadManager.StartThread();	

	_eventManager.Invoke("onConnected");
	_isConnecting = false;
	_log.Normal(L"Port connected : " + _portName);
}

void COMManager::Close()
{
	_isAlive = false;

	if (_isConnecting) {
		_log.Normal(L"Cancel connection process");
	}
	_threadManager.StopThread();
	if (_handle != INVALID_HANDLE_VALUE) {
		CloseHandle(_handle);
		_handle = INVALID_HANDLE_VALUE;

		_eventManager.Invoke("onDisconnected");
		_log.Normal(L"Port disconnected : " + _portName);
	}

	delete[] _recvBuff;
}

void COMManager::GetRecvData(char* data)
{
	if (!_isAlive) return;
	memcpy(data, _recvBuff, _recvLen);
}

void COMManager::GetRecvDone()
{
	if (!_isAlive) return;
	std::thread _thread([this]() {
		std::lock_guard<std::mutex> lock(_mtx);
		_recvReady = true;
		_cv.notify_one();
	});
	_thread.detach();
}

void COMManager::SetBufferSize(int size)
{
	_buffSize = size;
}

int COMManager::GetBufferSize()
{
	return _recvLen;
}

void COMManager::Send(const char* buff, int length)
{
	if (_handle == INVALID_HANDLE_VALUE) return;

	DWORD bytesWritten = 0;
	while (true) {
		BOOL writeResult = WriteFile(_handle, buff, length, &bytesWritten, &_wOverlapped);
		if (!writeResult) {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Developer(L"BTClassicManager::Send - write file error");
				_log.Error(L"Failed to send data");
				return;
			}
			// 비동기 작업이 시작되었으며, 완료를 기다려야 함
			else {
				if (GetOverlappedResult(_handle, &_wOverlapped, &bytesWritten, TRUE)) {
					// bytesWritten에는 이제 실제로 쓴 바이트 수가 저장됩니다.
					break;
				} else {
					// GetOverlappedResult 에러 처리
					break;
				}
			}
		}
	}
}

int COMManager::Recv(char* buff, int length)
{
	if (!_isAlive) return -1;
	if (_handle == INVALID_HANDLE_VALUE) return -1;

	DWORD bytesRead = 0;

	if (_dataReady) {
		if(!ReadFile(_handle, _recvBuff, _buffSize, NULL, &_rOverlapped)) {
			DWORD error = GetLastError();
			if (error != ERROR_IO_PENDING) {
				// 오류 처리
				_log.Error(L"Failed to read data");
				return -1;
			}
		}
		_dataReady = false;
	}
	else {
		if (GetOverlappedResult(_handle, &_rOverlapped, &bytesRead, FALSE)) {
			_dataReady = true;
		}
	}

	return bytesRead;
}

//int COMManager::CountRx()
//{
//	if (_comHandle == INVALID_HANDLE_VALUE) return -1;
//
//	DWORD  comError = 0;
//	COMSTAT comStat;
//	if (ClearCommError(_comHandle, &comError, &comStat)) {
//		if (comError) {
//#ifdef _DEBUG
//			printf("ERROR: COMManager::CountRx(): %s\n", CommErrorString(comError));
//#endif
//		}
//		return comStat.cbInQue;
//	}
//	OnFileAccessError("COMManager::ClearCommError (): ");
//	return -1;
//}
//
//const char* COMManager::CommErrorString(DWORD comError)
//{
//	switch (comError) {
//	case 0:           return "";
//	case CE_BREAK:    return "The hardware detected a break condition.";
//	case CE_FRAME:    return "The hardware detected a framing error.";
//	case CE_OVERRUN:  return "A character-buffer overrun has occurred. The next character is lost.";
//	case CE_RXOVER:   return "An input buffer overflow has occurred. There is either no room in the input buffer, or a character was received after the end-of-file (EOF) character.";
//	case CE_RXPARITY: return "The hardware detected a parity error.";
//	default:          return "Unknown error code.";
//	}
//}

void COMManager::RecvThreadFunc()
{
	std::unique_lock<std::mutex> lock(_mtx);
	_cv.wait(lock, [this]{ return _recvReady; });

	if (!_isAlive) return;
	if (_handle == INVALID_HANDLE_VALUE) return;

	int nRecv = Recv(_recvBuff, _buffSize);

	if (nRecv > 0) {
		_recvLen = nRecv;
		_recvReady = false;

		_eventManager.Invoke("onDataReceived");
	}
}