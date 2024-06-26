#pragma once
#include <windows.h>
#include "PortManager.h"
#include "PortScanner.h"
#include "ThreadManager.h"

class BLEManager : public PortManager
{
public:
	BLEManager(LogManager& log, EventManager eventManager, std::function<void()> closeFunc, std::wstring deviceName, std::wstring uuidService, std::wstring uuidTx, std::wstring uuidRx);
	virtual ~BLEManager();

	bool         Open()							                                     override;
	void         Connect()						                                     override;
	void         Close()						                                     override;
											                                                           
	int          GetRecvData(char* data, int recvByteSize)                           override;
	void         GetRecvDone()			                                             override;
	void         SetRecvBufferSize(int size)                                         override;
	int          GetRecvBufferSize()		                                         override;

	void         SetResourceLimit(ResourceLimitType resourceLimitType, double limit) override;
	void         CheckResources(double* cpuUsage, int* memoryRemained)               override;

	int          SendData(const char* msg, int len)                                  override;
	virtual void RecvData(PBLUETOOTH_GATT_VALUE_CHANGED_EVENT valueChangedEventParameters);

private:
	bool		 CheckUUID(std::wstring uuid);
	bool         SetBLEServiceBuffer();
	bool         SetBLECharacteristicBuffer();
	bool         SetBLEDescriptorBuffer();
	std::wstring ConvertUUIDToWString(BTH_LE_UUID uuid);

	void		 RecvThreadFunc(bool doWork);

	std::mutex			    _mtx;
	std::condition_variable _cv;
	PortScanner             _scanner;
	CThreadManager          _threadManager;

	HANDLE                  _handle = INVALID_HANDLE_VALUE;

	std::wstring            _deviceName;
	std::wstring            _uuidService;
	std::wstring            _uuidTx;
	std::wstring            _uuidRx;

	PBTH_LE_GATT_SERVICE              _pServiceBuffer;
	PBTH_LE_GATT_CHARACTERISTIC       _pCharBuffer;
	PBTH_LE_GATT_CHARACTERISTIC_VALUE _pCharValueBuffer;
	PBTH_LE_GATT_CHARACTERISTIC       _pCharWriteBuffer;
	PBTH_LE_GATT_DESCRIPTOR			  _pDescriptorBuffer;
	BLUETOOTH_GATT_EVENT_HANDLE		  _EventHandle = NULL;

	std::chrono::time_point<std::chrono::steady_clock> _dataReceivedTimePoint;

	bool  _isAlive = true;
	bool   _recvReady     = true;
	bool _writeReady = true;

	int	  _buffSize;
	int                     _recvBufferSize = DEFAULT_BUFF_SIZE;
	char					_recvBuffTemp[DEFAULT_BUFF_SIZE] { 0 };
	char                    _recvBuff	 [DEFAULT_BUFF_SIZE] { 0 };
	int					    _recvByteForRead = 1;
	int	                    _recvLen = 0;
	int   _writeBuffCount = 0;
	char* _writeBuff;
};