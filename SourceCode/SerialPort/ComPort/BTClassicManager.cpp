#include "BTClassicManager.h"

BTClassicManager::BTClassicManager(LogManager& log, EventManager eventManager, std::wstring deviceName) : 
	PortManager(log, eventManager), _scanner(log)
{
	_threadManager.RegisterCallback(std::bind(&BTClassicManager::RecvThreadFunc, this));

	// 비동기 입출력 이벤트 생성
	_wOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	_rOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// BT 연결을 위한 데이터 저장
	_deviceName = deviceName;
	
	// 수신 버퍼 할당
	_buffSize = DEFAULT_BUFF_SIZE;
	_recvBuff = new char[_buffSize] { 0 };
}

BTClassicManager::~BTClassicManager()
{
	Close();
}

bool BTClassicManager::Open()
{
	if (_deviceName == L"") {
		_log.Error(L"Invalid device name : " + _deviceName);
		_eventManager.Invoke("onConnectionFailed");
		return false;
	}

	_isConnecting = true;
	std::wstring uuidService = L"{00001101-0000-1000-8000-00805f9b34fb}";
	_handle = _scanner.GetBTHandle(_deviceName, uuidService, false);

	if (!_isAlive) return false;

	if (_handle == INVALID_HANDLE_VALUE) {
		_log.Developer(L"BTClassicManager::Open() - Invalid handle value error");
		_log.Error(L"Failed to connect device : " + _deviceName);
		_eventManager.Invoke("onConnectionFailed");
		return false;
	}

	_log.Developer(L"BTClassicManager::Open() - success");
	return true;
}

void BTClassicManager::Connect()
{
	_log.Developer(L"BTClassicManager::Connect() - start thread");
	_threadManager.StartThread();	

	_eventManager.Invoke("onConnected");
	_isConnecting = false;
	_log.Normal(L"Device connected : " + _deviceName);
}

void BTClassicManager::Close()
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
		_log.Normal(L"Device disconnected : " + _deviceName);
	}

	delete[] _recvBuff;
}

void BTClassicManager::ForceClose()
{
	_scanner.ForceStop();
}

void BTClassicManager::GetRecvData(char* data)
{
	memcpy(data, _recvBuff, _recvLen);
}

void BTClassicManager::GetRecvDone()
{
	std::thread _thread([this]() {
		std::lock_guard<std::mutex> lock(_mtx);
		_recvReady = true;
		_cv.notify_one();
	});
	_thread.detach();
}

void BTClassicManager::SetBufferSize(int size)
{
	_buffSize = size;
}

int BTClassicManager::GetBufferSize()
{
	return _recvLen;
}

void BTClassicManager::Send(const char* buff, int length)
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

int BTClassicManager::Recv(char* buff, int length)
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

void BTClassicManager::RecvThreadFunc()
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