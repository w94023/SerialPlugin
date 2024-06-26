#pragma once
#include <windows.h>
#include "PortManager.h"
#include "PortScanner.h"
#include "ThreadManager.h"

class BLEManager : public PortManager
{
public:
	BLEManager(LogManager& log, EventManager eventManager, std::wstring deviceName, std::wstring uuidService, std::wstring uuidTx, std::wstring uuidRx);
	virtual ~BLEManager();

	virtual bool Open();
	virtual void Connect();
	virtual void Close();

	virtual void GetRecvData(char* data);
	virtual void GetRecvDone();
	virtual void SetBufferSize(int size);
	virtual int  GetBufferSize();

	virtual void Send(const char* msg, int len);
	virtual int  Recv(char* msg, int len) {return 1;}
	virtual void Purge() { }

	void RecvThreadFunc() {}
	void RecvData(PBLUETOOTH_GATT_VALUE_CHANGED_EVENT valueChangedEventParameters);

private:
	bool		 CheckUUID(std::wstring uuid);
	bool         SetBLEServiceBuffer();
	bool         SetBLECharacteristicBuffer();
	bool         SetBLEDescriptorBuffer();
	std::wstring ConvertUUIDToWString(BTH_LE_UUID uuid);

	std::mutex			    _mtx;
	std::condition_variable _cv;
	PortScanner             _scanner;

	HANDLE                  _handle = INVALID_HANDLE_VALUE;

	std::wstring            _deviceName;
	std::wstring            _uuidService;
	std::wstring            _uuidTx;
	std::wstring            _uuidRx;

	PBTH_LE_GATT_SERVICE              _pServiceBuffer;
	PBTH_LE_GATT_CHARACTERISTIC       _pCharBuffer;
	PBTH_LE_GATT_CHARACTERISTIC_VALUE _pCharValueBuffer;
	PBTH_LE_GATT_DESCRIPTOR			  _pDescriptorBuffer;
	BLUETOOTH_GATT_EVENT_HANDLE		  _EventHandle = NULL;

	bool  _isAlive = true;
	int	  _buffSize;

	bool   _recvReady     = true;
	char* _recvBuff;
	int   _recvCount = 0;
	int	  _recvLen;

	bool _writeReady = true;
	int   _writeBuffCount = 0;
	char* _writeBuff;
};