#include "BLEManager.h"
#include <thread>
#include <chrono>
#include <iomanip> // for std::setw and std::setfill
#include <future>

static void OnNotified(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
	auto handleData = std::async(std::launch::async, [EventOutParameter, Context]() {
		BLEManager* instance = static_cast<BLEManager*>(Context);

		PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;
		instance->RecvData(ValueChangedEventParameters);
    });
}

BLEManager::BLEManager(LogManager& log, EventManager eventManager, std::function<void()> closeFunc, std::wstring deviceName, std::wstring uuidService, std::wstring uuidTx, std::wstring uuidRx) :
	PortManager(log, eventManager, closeFunc), _scanner(log), _threadManager(log)
{
	_threadManager.RegisterCallback(std::bind(&BLEManager::RecvThreadFunc, this, std::placeholders::_1));

	// BT 연결을 위한 데이터 저장
	_deviceName  = deviceName;
	_uuidService = uuidService;
	_uuidTx      = uuidTx;
	_uuidRx      = uuidRx;

	// 송신 버퍼 할당
	_writeBuff   = new char[_buffSize] { 0 };
}

BLEManager::~BLEManager()
{
	delete[] _writeBuff;
	Close();
}

bool BLEManager::Open()
{
	if (_deviceName == L"") {
		_log.Error(L"Invalid device name : " + _deviceName);
		_eventManager.Invoke("onConnectionFailed");
		return false;
	}

	if (!CheckUUID(_uuidService)) {
		_log.Error(L"Invalid service uuid : " + _uuidService);
		_eventManager.Invoke("onConnectionFailed");
		return false;
	}

	if (!CheckUUID(_uuidTx)) {
		_log.Error(L"Invalid Tx uuid : " + _uuidTx);
		_eventManager.Invoke("onConnectionFailed");
		return false;
	}

	if (!CheckUUID(_uuidRx)) {
		_log.Error(L"Invalid Rx uuid : " + _uuidRx);
		_eventManager.Invoke("onConnectionFailed");
		return false;
	}

	_handle = _scanner.GetBTHandle(_deviceName, _uuidService, true);

	HRESULT hr = S_OK;
	BTH_LE_GATT_EVENT_TYPE EventType;

	if (_handle == INVALID_HANDLE_VALUE) {
		goto CLOSE_RET;
	}

	if (!SetBLEServiceBuffer()) {
		goto CLOSE_RET;
	}

	if (!SetBLECharacteristicBuffer()) {
		goto CLOSE_RET;
	}

	if (!SetBLEDescriptorBuffer()) {
		goto CLOSE_RET;
	}

	if (!_pCharBuffer->IsNotifiable) {
		_log.Developer(L"BLEManager::Open() - failed to enable notification");
		goto CLOSE_RET;
	}

	// 알림 방식으로 데이터를 읽어옴
	EventType = CharacteristicValueChangedEvent;

	BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
	EventParameterIn.Characteristics[0] = *_pCharBuffer;
	EventParameterIn.NumCharacteristics = 1;
	hr = BluetoothGATTRegisterEvent(
		_handle,
		EventType,
		&EventParameterIn,
		OnNotified,
		this,
		&_EventHandle,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr != S_OK) {
		_log.Developer(L"BLEManager::Open() - failed to register reading event");
		goto CLOSE_RET;
	}

	return true;

CLOSE_RET:
	_log.Error(L"Failed to connect BLE device : " + _deviceName);
	_closeFunc();
	_eventManager.Invoke("onConnectionFailed");
	return false;
}

void BLEManager::Connect()
{
	if (_handle == INVALID_HANDLE_VALUE) {
		Open();
	}
	else {
		_isAlive = true;
		_dataReceivedTimePoint = std::chrono::steady_clock::now();
		_threadManager.StartThread();	
		_log.Normal(L"Device connected : " + _deviceName);
		_eventManager.Invoke("onConnected");
	}	
}

void BLEManager::Close()
{
	_isAlive = false;
	_threadManager.StopThread();
	if (_EventHandle != NULL) {
		BluetoothGATTUnregisterEvent(_EventHandle, BLUETOOTH_GATT_FLAG_NONE);
		_EventHandle = NULL;
	}
	if (_handle != INVALID_HANDLE_VALUE) {
		CloseHandle(_handle);
		_handle = INVALID_HANDLE_VALUE;

		_eventManager.Invoke("onDisconnected");
	}
}

void BLEManager::RecvData(PBLUETOOTH_GATT_VALUE_CHANGED_EVENT valueChangedEventParameters)
{
	std::unique_lock<std::mutex> lock(_mtx);
	_cv.wait(lock, [this]{ return _recvReady; });

	for (ULONG i = 0; i < valueChangedEventParameters->CharacteristicValue->DataSize; i++) {
		if (!_isAlive) return;
		_recvBuffTemp[i] = static_cast<char>(valueChangedEventParameters->CharacteristicValue->Data[i]);
	}

	if (!_isAlive) return;

	// Flush
	// 지정된 recvBufferSize를 넘을 경우 flush 실행
	// recvBufferSize의 1/10 데이터만 남기고 나머지는 삭제
	if (_recvLen >= _recvBufferSize) {
		for (int i = 0; i < _recvLen; i++) {
			if (i < _recvBufferSize / 10) {
				_recvBuff[i] = _recvBuff[i + _recvLen - _recvBufferSize / 10];
			}
			else {
				_recvBuff[i] = '\0';
			}
		}
		_log.Developer(L"Flush data [deviceName] : " + _deviceName + L" [current data byte] : " + std::to_wstring(_recvLen) + L" [flushed data byte] : " + std::to_wstring(_recvBufferSize / 10));
		_recvLen = _recvBufferSize / 10;
	}

	int buffStartPoint = _recvLen;
	int bytesRead = valueChangedEventParameters->CharacteristicValue->DataSize;
	//if (_recvLen + bytesRead > DEFAULT_BUFF_SIZE) {
	//	buffStartPoint = _recvLen - bytesRead;
	//}
	//memcpy(_recvBuff + buffStartPoint, _recvBuffTemp, bytesRead);
	//_recvLen = buffStartPoint + bytesRead;
	if (_recvLen + bytesRead < _recvBufferSize) {
		memcpy(_recvBuff + _recvLen, _recvBuffTemp, bytesRead);
		_recvLen += bytesRead;
	}
}

void BLEManager::RecvThreadFunc(bool doWork)
{
	std::unique_lock<std::mutex> lock(_mtx);
	_cv.wait(lock, [this]{ return _recvReady; });

	if (doWork) {
		if (_recvLen > 0) {
			_recvReady = false;
			_eventManager.Invoke("onDataReceived");
		}
	}
}

int BLEManager::GetRecvData(char* data, int recvByteSize)
{
	if (!_isAlive) return 0;

	int bytesForRead = (recvByteSize > _recvLen) ? _recvLen : recvByteSize;
	if (bytesForRead < recvByteSize) {
		_recvBufferSize += recvByteSize * 10;
		if (_recvBufferSize > DEFAULT_BUFF_SIZE) {
			_recvBufferSize = DEFAULT_BUFF_SIZE;
		}
		_log.Developer(L"Buffer size adjusted : " + std::to_wstring(_recvBufferSize));
	}

	memcpy(data, _recvBuff, bytesForRead);
	for (int i = 0; i < _recvLen; i++) {
		if (i < _recvLen - bytesForRead) {
			_recvBuff[i] = _recvBuff[i + bytesForRead];
		}
		else {
			_recvBuff[i] = '\0';
		}
	}
	_recvLen -= bytesForRead;

	// Data read request 및 Flush를 위한 reading byte 정보 저장
	_recvByteForRead = recvByteSize;

	return bytesForRead;
}

void BLEManager::GetRecvDone()
{
	if (!_isAlive) return;
	std::thread _thread([this]() {
		std::lock_guard<std::mutex> lock(_mtx);
		_recvReady = true;
		_cv.notify_all();
	});
	_thread.detach();
}

void BLEManager::SetRecvBufferSize(int size)
{
	_recvBufferSize = size;
}

int BLEManager::GetRecvBufferSize()
{
	return _recvBufferSize;
}

void BLEManager::SetResourceLimit(ResourceLimitType resourceLimitType, double limit)
{
	_threadManager.SetResourceLimit(resourceLimitType, limit);
}

void BLEManager::CheckResources(double* cpuUsage, int* memoryRemained)
{
	_threadManager.CheckResource(cpuUsage, memoryRemained);
}

int BLEManager::SendData(const char *msg, int len)
{
	char _msg[20] { 0 };
	int  msgSize = len;

	char writeBuff[DEFAULT_BUFF_SIZE] { 0 };
	int  writeCount = 0;
	// 송신 문자열이 20보다 길 경우, 송신이 되지 않음
	// BLE의 MTU 사이즈 문제로 추정됨
	if (msgSize > 20) {
		msgSize = 20;
		for (int i = 0; i < len-msgSize; i++) {
			if (writeCount == DEFAULT_BUFF_SIZE) writeCount = DEFAULT_BUFF_SIZE-1;

			writeBuff[writeCount] = msg[i+msgSize];
			writeCount++;
		}
	}
	for (int i = 0; i < msgSize; i++) {
		_msg[i] = msg[i];
	}

	BTH_LE_GATT_RELIABLE_WRITE_CONTEXT relialeWriteContext = NULL;
	if (_pCharWriteBuffer == NULL) {
		_log.Error(L"Failed to send data : " + _deviceName);
		_log.Developer(L"BLEManager::Send - pTXCharacteristic is null");
		return 0;
	}
	if (_pCharWriteBuffer->IsSignedWritable || _pCharWriteBuffer->IsWritable || _pCharWriteBuffer->IsWritableWithoutResponse) {
		HRESULT hr = BluetoothGATTBeginReliableWrite(
			_handle,
			&relialeWriteContext,
			BLUETOOTH_GATT_FLAG_NONE);

		if (SUCCEEDED(hr)) {
			PBTH_LE_GATT_CHARACTERISTIC_VALUE newValue = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)new UCHAR[sizeof(ULONG)+msgSize];
			newValue->DataSize = (ULONG)msgSize;
			memcpy(newValue->Data, _msg, msgSize);
			HRESULT hr = BluetoothGATTSetCharacteristicValue(
				_handle,
				_pCharWriteBuffer,
				newValue,
				NULL,
				BLUETOOTH_GATT_FLAG_WRITE_WITHOUT_RESPONSE);
			
			delete newValue;
		}

		if (NULL != relialeWriteContext)
		{
			BluetoothGATTEndReliableWrite(
				_handle,
				relialeWriteContext,
				BLUETOOTH_GATT_FLAG_NONE);
		}	}
	else
	{
		_log.Error(L"Failed to send data : " + _deviceName);
		_log.Developer(L"BLEManager::Send - characteristic is not writable");
	}


	if (writeCount > 0) {
		SendData(writeBuff, writeCount);
	}

	return 0;
} 

bool BLEManager::CheckUUID(std::wstring uuid)
{
	if (uuid.front() != L'{') return false;
	if (uuid.back()  != L'}') return false;

	std::wstring trimmedUUID = uuid.substr(1, uuid.length() - 2);

	std::vector<std::wstring> splitUUID;
    std::wstring token;

    for (wchar_t character : trimmedUUID) {
        if (character == L'-') {

            if (!token.empty()) {
                splitUUID.push_back(token);
                token.clear();
            }
        } else {
            token += character;
        }
    }
    if (!token.empty()) {
        splitUUID.push_back(token);
    }

	if (splitUUID.size() != 5)     return false;
	if (splitUUID[0].size() != 8)  return false;
    if (splitUUID[1].size() != 4)  return false;
    if (splitUUID[2].size() != 4)  return false;
    if (splitUUID[3].size() != 4)  return false;
    if (splitUUID[4].size() != 12) return false;

	return true;
}

bool BLEManager::SetBLEServiceBuffer()
{
	HRESULT hr;
	USHORT serviceBufferCount;

	// Service 버퍼 사이즈 결정
	hr = BluetoothGATTGetServices(
		_handle,
		0,
		NULL,
		&serviceBufferCount,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
		_log.Developer(L"BLEManager::SetBLEServiceBuffer() - failed to determine service buffer size");
        return false;
    }
	if (serviceBufferCount == 0) {
		_log.Developer(L"BLEManager::SetBLEServiceBuffer() - failed to find service matched with given UUID");
		return false;
	}

	// Service 버퍼 생성
	PBTH_LE_GATT_SERVICE pServiceBuffer = (PBTH_LE_GATT_SERVICE)malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);
	if (NULL == pServiceBuffer) {
		_log.Developer(L"BLEManager::SetBLEServiceBuffer() - service buffer out of memory");
		return false;
	}
	RtlZeroMemory(pServiceBuffer, sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

	// Service 버퍼 받아오기
	USHORT numServices;
	hr = BluetoothGATTGetServices(
		_handle,
		serviceBufferCount,
		pServiceBuffer,
		&numServices,
		BLUETOOTH_GATT_FLAG_NONE);

	if (!SUCCEEDED(hr)) {
		_log.Developer(L"BLEManager::SetBLEServiceBuffer() - failed to get service buffer");
		return false;
	}
	if (numServices != serviceBufferCount) {
		_log.Developer(L"BLEManager::SetBLEServiceBuffer() - the buffer size and the actual received size of the service were not matched");
		return false;
	}

	//주어진 UUID와 비교
	for (size_t i = 0; i < serviceBufferCount; i++) {
		if (_uuidService == ConvertUUIDToWString(pServiceBuffer[0].ServiceUuid)) {
			_pServiceBuffer = &pServiceBuffer[i];
			_log.Developer(L"BLEManager::SetBLEServiceBuffer() - service found [_uuidServce]" + _uuidService);
			return true;
		}
	}

	_log.Developer(L"BLEManager::SetBLEServiceBuffer() - failed to find service matched with given UUID");
	return false;
}

bool BLEManager::SetBLECharacteristicBuffer()
{
	HRESULT hr;
	USHORT charBufferSize;

	// Characteristic 버퍼 사이즈 결정
	hr = BluetoothGATTGetCharacteristics(
		_handle,
		_pServiceBuffer,
		0,
		NULL,
		&charBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
		_log.Developer(L"BLEManager::SetBLECharacteristicBuffer() - failed to determine characteristic buffer size");
        return false;
    }
	if (charBufferSize == 0) {
		_log.Developer(L"BLEManager::SetBLECharacteristicBuffer() - failed to find characteristic matched with given UUID");
		return false;
	}

	// Characteristic 버퍼 생성
	PBTH_LE_GATT_CHARACTERISTIC pCharBuffer = NULL;
	pCharBuffer = (PBTH_LE_GATT_CHARACTERISTIC) malloc(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
	if (NULL == _pServiceBuffer) {
		_log.Developer(L"BLEManager::SetBLECharacteristicBuffer() - characteristic buffer out of memory");
		return false;
	}
	RtlZeroMemory(pCharBuffer, charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

	// Characteristic 버퍼 받아오기
	USHORT numChars;
	hr = BluetoothGATTGetCharacteristics(
		_handle,
		_pServiceBuffer,
		charBufferSize,
		pCharBuffer,
		&numChars,
		BLUETOOTH_GATT_FLAG_NONE);

	if (!SUCCEEDED(hr)) {
		_log.Developer(L"BLEManager::SetBLECharacteristicBuffer() - failed to get characteristic buffer");
		return false;
	}
	if (numChars != charBufferSize) {
		_log.Developer(L"BLEManager::SetBLECharacteristicBuffer() - the buffer size and the actual received size of the characteristic were not matched");
		return false;
	}

	// 주어진 uuid와 비교
	bool _foundReadBuffer = false;
	bool _foundWriteBuffer = false;
	for (size_t i = 0; i < charBufferSize; i++) {
		if (_uuidTx == ConvertUUIDToWString(pCharBuffer[i].CharacteristicUuid)) {
			_pCharBuffer = &pCharBuffer[i];
			_log.Developer(L"BLEManager::SetBLEServiceBuffer() - characteristic(TX) found [_uuidTx]" + _uuidTx);
			_foundReadBuffer = true;
		}
		if (_uuidRx == ConvertUUIDToWString(pCharBuffer[i].CharacteristicUuid)) {
			_pCharWriteBuffer = &pCharBuffer[i];
			_log.Developer(L"BLEManager::SetBLEServiceBuffer() - characteristic(RX) found [_uuidRx]" + _uuidRx);
			_foundWriteBuffer = true;
		}
	}

	if (_foundReadBuffer && _foundWriteBuffer) {
		return true;
	}
	else {
		_log.Developer(L"BLEManager::SetBLECharacteristicBuffer() - failed to find characteristic matched with given UUID");
		return false;
	}
}

bool BLEManager::SetBLEDescriptorBuffer()
{
	HRESULT hr = S_OK;

	// Descriptor 버퍼 사이즈 결정
	USHORT descriptorBufferSize;
	hr = BluetoothGATTGetDescriptors(
		_handle,
		_pCharBuffer,
		0,
		NULL,
		&descriptorBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);

	if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
		_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - failed to determine descriptor buffer size");
        return false;
    }
	if (descriptorBufferSize == 0) {
		_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - The buffer size of descriptor was 0");
		return false;
	}

	// Descriptor 버퍼 생성
	_pDescriptorBuffer = (PBTH_LE_GATT_DESCRIPTOR)malloc(descriptorBufferSize * sizeof(BTH_LE_GATT_DESCRIPTOR));
	if (NULL == _pDescriptorBuffer) {
		_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - descriptor buffer out of memory");
		return false;
	}
	RtlZeroMemory(_pDescriptorBuffer, descriptorBufferSize);

	// Descriptor 버퍼 받아오기
	USHORT numDescriptors;
	hr = BluetoothGATTGetDescriptors(
		_handle,
		_pCharBuffer,
		descriptorBufferSize,
		_pDescriptorBuffer,
		&numDescriptors,
		BLUETOOTH_GATT_FLAG_NONE);

	if (!SUCCEEDED(hr)) {
		_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - failed to get descriptor buffer");
		return false;
	}
	if (numDescriptors != descriptorBufferSize) {
		_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - the buffer size and the actual received size of the descriptor were not matched");
		return false;
	}
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
	for (size_t i = 0; i < numDescriptors; i++) {
		PBTH_LE_GATT_DESCRIPTOR currGattDescriptor = &_pDescriptorBuffer[i];

		// Descriptor Value 버퍼 사이즈 결정
		USHORT descValueDataSize;
		hr = BluetoothGATTGetDescriptorValue(
			_handle,
			currGattDescriptor,
			0,
			NULL,
			&descValueDataSize,
			BLUETOOTH_GATT_FLAG_NONE);

		if (hr != HRESULT_FROM_WIN32(ERROR_MORE_DATA)) {
			_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - failed to determine descriptor value buffer size");
			continue;
		}

		// Descriptor value 버퍼 생성
		PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);
		if (NULL == pDescValueBuffer) {
			_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - failed to create descriptor buffer");
			continue;
		}
		RtlZeroMemory(pDescValueBuffer, descValueDataSize);

		// Descriptor value 버퍼 받아오기
		hr = BluetoothGATTGetDescriptorValue(
			_handle,
			currGattDescriptor,
			(ULONG)descValueDataSize,
			pDescValueBuffer,
			NULL,
			BLUETOOTH_GATT_FLAG_NONE);
		if (!SUCCEEDED(hr)) {
			_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - failed to get descriptor value buffer");
			continue;
		}

		// Descriptor notification enabled로 설정
		BTH_LE_GATT_DESCRIPTOR_VALUE newValue;
		RtlZeroMemory(&newValue, sizeof(newValue));

		newValue.DescriptorType = ClientCharacteristicConfiguration;
		newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

		HRESULT hr = BluetoothGATTSetDescriptorValue(
			_handle,
			currGattDescriptor,
			&newValue,
			BLUETOOTH_GATT_FLAG_NONE);

		if (!SUCCEEDED(hr)) {
			_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - failed to enable notification");
			continue;
		}

		_log.Developer(L"BLEManager::SetBLEDescriptorBuffer() - set descriptor notifiaction enabled [uuidDescriptor]" + ConvertUUIDToWString(currGattDescriptor->DescriptorUuid));
		return true;
	}

	return false;
}

std::wstring BLEManager::ConvertUUIDToWString(BTH_LE_UUID uuid)
{
	// Bluetooth Base UUID: 00000000-0000-1000-8000-00805F9B34FB
	GUID baseUUID = {0x00000000, 0x0000, 0x1000, {0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB}};
	if (uuid.IsShortUuid) {
		baseUUID.Data1 = uuid.Value.ShortUuid;
	} 
	else {
		baseUUID = uuid.Value.LongUuid;
	}

	WCHAR uuidStr[40]; // UUID를 저장할 버퍼
	::StringFromGUID2(baseUUID, uuidStr, sizeof(uuidStr) / sizeof(uuidStr[0]));

	std::wstring uuidWStr(uuidStr);

    return uuidWStr;
}