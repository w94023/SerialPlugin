using System;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;
using static SerialManager.ComHandle;

namespace SerialManager
{
	internal static class LogManager
    {
        private static string _currentDirectory = Directory.GetCurrentDirectory();
		private static string _logFileName      = "\\Log.txt";
		private static string _filePath         = _currentDirectory + _logFileName;

        internal static void SaveLog(string log)
        {
            using (StreamWriter writer = new StreamWriter(_filePath, true)) {
				writer.WriteLine(log);
			}
        }
    }

	public class SerialHandle
	{
		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern int CheckMemory();

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern int GetLog(byte[] log);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern void RegisterLogging(EventCallback callback);

		private ComHandle _handle;

		// Log level 설정
		// 0 : Error, 1 : Error+Normal, 2 : Error+Normal+Developer
		public int logLevel = 1;
		// PPS 확인 설정
		public bool getPPSOnDataReceived = false;
		public int  PPS					 { get { if (_handle == null) { return -1; } else { return _handle.PPS; } } }
		// 연결 상태 확인
		public bool isConnected { get { if (_handle == null) { return false; } else { return _handle.isConnected; } } }
		// 데이터 수신 관련 설정
		public  int           receiveByteSize   { get { return _receiveByteSize; } set { _receiveByteSize = value; if (_handle != null) _handle.receiveByteSize = _receiveByteSize; } }
		private int          _receiveByteSize   = 1;
		public  int           receiveBufferSize { get { return _receiveBufferSize; } set { _receiveBufferSize = value; if (_handle != null) _handle.SetBufferSize(_receiveBufferSize); } }
		private int          _receiveBufferSize = 10000;
		public  byte[]        stopByte          { get { return _stopByte; } set { _stopByte = value; if (_handle != null) _handle.SetStopByte(_stopByte); } }
		private byte[]       _stopByte          = new byte[0];
		public  int           packetLength	    { get { return _packetLength; } set { _packetLength = value; if (_handle != null) _handle.SetPacketLength(_packetLength); } }
		private int          _packetLength	    = 0;
		public  EncodingType  encodingType      { get { return _encodingType; } set { _encodingType = value; if (_handle != null) _handle.SetEncodingType(_encodingType); } }
		private EncodingType _encodingType      = EncodingType.ASCII;
		// 리소스 관리 설정
		private bool         _usePPSLimit = false;
		public  bool          usePPSLimit { 
			get { return _usePPSLimit; } 
			set { 
				_usePPSLimit = value; 
				if (_handle != null) {
					if (_usePPSLimit) {
						_handle.SetPPSLimit(_PPSLimit);
					}
					else {
						_handle.SetPPSLimit(-1);
					}
				}
			} 
		}
		private bool         _useCPULimit = false;
		public  bool	      useCPULimit {
			get { return _useCPULimit; } 
			set { 
				_useCPULimit = value; 
				if (_handle != null) {
					if (_useCPULimit) {
						_handle.SetCPULimit(_CPULimit);
					}
					else {
						_handle.SetCPULimit(-1);
					}
				}
			} 
		}
		private bool         _useMemoryLimit = false;
		public  bool          useMemoryLimit {
			get { return _useMemoryLimit; } 
			set { 
				_useMemoryLimit = value; 
				if (_handle != null) {
					if (_useMemoryLimit) {
						_handle.SetMemoryLimit(_memoryLimit);
					}
					else {
						_handle.SetMemoryLimit(-1);
					}
				}
			} 
		}
		public  int			  PPSLimit      { get { return _PPSLimit; } set { _PPSLimit = value; if (_handle != null && _usePPSLimit) _handle.SetPPSLimit(_PPSLimit); } }
		private int          _PPSLimit	    = 300; // Hz
		public  double        CPULimit      { get { return _CPULimit; } set { _CPULimit = value; if (_handle != null && _useCPULimit) _handle.SetCPULimit(_CPULimit); } }
		private double       _CPULimit      = 95; // 사용량의 % 수치
		public  int           memoryLimit   { get { return _memoryLimit; } set { _memoryLimit = value; if (_handle != null && _useCPULimit) _handle.SetMemoryLimit(_memoryLimit); } }
		private int          _memoryLimit   = 100; // 남은 RAM의 MB 수치

		public  int connectionTimeout { get { return _connectionTimeout; } set { _connectionTimeout = value; if (_handle != null) _handle.SetConnectionTimeout(_connectionTimeout); } }
		private int _connectionTimeout = 5000;

		// Events
		private event EventCallback _onLogReceived;
		private int registeredOnScanEndedCallback = 0;

		public event Action<SerialLog> onLogReceived;
		public event Action<SerialLog> onScanEnded;
		public event Action onConnected;
		public event Action onConnectionFailed;
		public event Action onDisconnected;
		public event Action<SerialData> onDataReceived;

		public SerialHandle()
		{
			_onLogReceived = new EventCallback(OnLogReceived);
			RegisterLogging(_onLogReceived);
		}

		~SerialHandle()
		{
			_onLogReceived = null;
		}

		public void ScanDevices()
		{
			if (_handle == null) {
				_handle = new SerialPortManager();
				if (registeredOnScanEndedCallback == 0) {
					_handle.onScanEnded += OnScanEnded;
					registeredOnScanEndedCallback++;
				}
				_handle.ScanDevices();
			}
			else {
				if (registeredOnScanEndedCallback == 0) {
					_handle.onScanEnded += OnScanEnded;
					registeredOnScanEndedCallback++;
				}
				_handle.ScanDevices();
			}
		}

		public void Connect(string deviceName)
		{
			Disconnect();
			_handle = new SerialPortManager(deviceName);
			ConnectHandle();
		}

		public void Connect(string deviceName, UUID uuid)
		{
			Disconnect();
			_handle = new SerialPortManager(deviceName, uuid);
			ConnectHandle();
		}

		public void Connect(string comPort, BaudRate baudRate,
			DataBit dataBit = DataBit.bit8, Parity parity = Parity.None, StopBit stopBit = StopBit.bit1, FlowControl flowControl = FlowControl.None)
		{
			Disconnect();
			_handle = new SerialPortManager(comPort, baudRate, dataBit, parity, stopBit, flowControl);
			ConnectHandle();
		}

		public void Connect(string deviceName, int[] ports)
		{
			Disconnect();
			_handle = new DAQManager(deviceName, ports);
			ConnectHandle();
		}

		private void ConnectHandle()
		{
			if (_handle == null) return;
			_handle.getPPSOnDataReceived = getPPSOnDataReceived;
			_handle.receiveByteSize = _receiveByteSize;
			_handle.SetStopByte(_stopByte);
			_handle.SetPacketLength(_packetLength);
			_handle.SetEncodingType(_encodingType);
			_handle.SetBufferSize(_receiveBufferSize);
			_handle.SetPPSLimit(_usePPSLimit ? _PPSLimit : -1);
			_handle.SetCPULimit(_useCPULimit ? _CPULimit : -1);
			_handle.SetMemoryLimit(_useMemoryLimit ? _memoryLimit : -1);
			_handle.SetConnectionTimeout(_connectionTimeout);
			RegisterEventsToHandle();
			_handle.Connect();
		}

		private void RegisterEventsToHandle()
		{
			_handle.onConnected += OnConnected;
			_handle.onConnectionFailed += OnConnectionFailed;
			_handle.onDisconnected += OnDisconnected;
			_handle.onDataReceived += OnDataReceived;
		}

		public void Disconnect()
		{
			if (_handle == null) return;
			_handle.Disconnect(); 
			registeredOnScanEndedCallback = 0;
		}

		public void SendData(string msg)
		{
			if (_handle == null) return;
			_handle.SendData(msg);
		}

		public void SendData(byte[] data)
		{
			if (_handle == null) return;
			_handle.SendData(data);
		}

		public float[] CheckResources()
		{
			if (_handle == null) return new float[] { -1, -1 };
			else				 return _handle.CheckResources();
		}

		public void Test(double cpuUsage, int memoryRemained)
		{
			if (_handle == null) return;
			_handle.Test(cpuUsage, memoryRemained);
		}

		private void OnLogReceived()
		{
			byte[] log = new byte[260];
			int logLevel = GetLog(log);

			string header = "";
			switch (logLevel) {
				case 0: header = "[Error log]"; break;
				case 1: header = "[Normal log]"; break;
				case 2: header = "[Developer log]"; break;
				default: header = ""; break;
			}
			string logStr = header + Encoding.Unicode.GetString(log, 0, log.Length);

			//LogManager.SaveLog(logStr);

			if (logLevel > this.logLevel) return;
			onLogReceived?.Invoke(new SerialLog(logStr));
		}

		private void OnScanEnded(SerialLog e) => onScanEnded?.Invoke(e);

		private void OnConnected() => onConnected?.Invoke();

		private void OnConnectionFailed() => onConnectionFailed?.Invoke();

		private void OnDisconnected() => onDisconnected?.Invoke();

		private void OnDataReceived(SerialData e) => onDataReceived?.Invoke(e);
	}
}