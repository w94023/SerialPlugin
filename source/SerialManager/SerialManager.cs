using System;
using System.IO;
using System.Text;
using System.Linq;
using System.Runtime.InteropServices;
using System.Diagnostics;
using static SerialManager.CPPImportLayer;
using System.ComponentModel;

namespace SerialManager
{
	public class SerialHandle : EventManager
	{
		// Log 관련
		// 0 : Error, 1 : Error+Normal, 2 : Error+Normal+Developer
		private int _logLevel = 1; public int logLevel { get { return _logLevel; } set { _logLevel = value; } }
		private event EventCallbackWithChar _onLogReceived;
		public  event Action<SerialLog>      onLogReceived;

		// Scan device 관련
		public  event Action<SerialLog>      onScanEnded;
		private event EventCallbackWithChar _onScanEnded;

		// 연결 설정
		private IntPtr _deviceNamePtr = IntPtr.Zero;
		private IntPtr _AIPortsPtr    = IntPtr.Zero;
		private IntPtr _AOPortsPtr    = IntPtr.Zero;
		private IntPtr _DPortsPtr     = IntPtr.Zero;
		private IntPtr _LinesPtr      = IntPtr.Zero;
		private IntPtr _stopBytePtr = IntPtr.Zero;
		private ConnectionConfig _connectionConfig;
		private PacketConfig _packetConfig;
		private int _CPPHandle = 0;

		// Serial 이벤트
		private EventCallback         _onConnected;        // C++ 플러그인에 등록하는 private 이벤트
		private EventCallback         _onConnectionFailed; // C++ 플러그인에 등록하는 private 이벤트
		private EventCallback         _onDisconnected;     // C++ 플러그인에 등록하는 private 이벤트
		private EventCallbackWithChar _onDataReceived;     // C++ 플러그인에 등록하는 private 이벤트
		public event Action                     onConnected;        // C++ 플러그인 __onConnected 호출 시, C# 플러그인 외부로 호출하는 이벤트
		public event Action                     onConnectionFailed; // C++ 플러그인 __onConnectionFailed 호출 시, C# 플러그인 외부로 호출하는 이벤트
		public event Action                     onDisconnected;     // C++ 플러그인 __onDisconnected 호출 시, C# 플러그인 외부로 호출하는 이벤트
		public event Action<double, SerialData> onDataReceived;     // C++ 플러그인 _onDataReceived 호출 시, C# 플러그인 외부로 호출하는 이벤트

		// 연결 상태 flag
		public bool   isConnected { get { return _isConnected; } } // C# 플러그인 외부에서 참조 가능한 flag
		private bool _isConnected = false;                         // C# 플러그인 내부에서 사용하는 flag

		public  int     receiveByteSize   { get { return _packetConfig.receiveByteSize;   } set { _packetConfig.receiveByteSize   = value; SetPacketConfig(_CPPHandle, _packetConfig); } }
		public  int     receiveBufferSize { get { return _packetConfig.receiveBufferSize; } set { _packetConfig.receiveBufferSize = value; SetPacketConfig(_CPPHandle, _packetConfig); } }
		public  int     packetLength	  { get { return _packetConfig.packetLength;      } set { _packetConfig.packetLength      = value; SetPacketConfig(_CPPHandle, _packetConfig); } }
		public  byte[]  stopByte          { get { return _stopByte;                       } set { _stopByte                       = value; SetStopByte(_stopByte);                     } }
		private byte[] _stopByte          = null;
		public  int	    PPSLimit          { get { return _packetConfig.PPSLimit;          } set { _packetConfig.PPSLimit          = value; SetPacketConfig(_CPPHandle, _packetConfig); } }
		public  EncodingType  encodingType { get { return _encodingType; } set { _encodingType = value; } }
		private EncodingType _encodingType = EncodingType.ASCII;

		public bool usePacketLength { get { return _packetConfig.GetPacketLengthUsage(); } set { _packetConfig.SetPacketLengthUsage(value); SetPacketConfig(_CPPHandle, _packetConfig); } }
		public bool useStopByte     { get { return _packetConfig.GetStopByteUsage();     } set { _packetConfig.SetStopByteUsage(value);     SetPacketConfig(_CPPHandle, _packetConfig); } }
		public bool usePPSLimit     { get { return _packetConfig.GetPPSLimitUsage();     } set { _packetConfig.SetPPSLimitUsage(value);     SetPacketConfig(_CPPHandle, _packetConfig); } }
		
		public  bool  getPPSOnDataReceived = false;
		public  int   PPS { get { return _PPS; } }
		private int  _PPS = 0;

		public  bool  getReceivedBufferLength = false;
		public  int   receivedBufferLength { get { return _receivedBufferLength; } }
		private int  _receivedBufferLength = 0;

		public float flushRatio { get { return _packetConfig.flushRatio; } set { _packetConfig.flushRatio = value; SetPacketConfig(_CPPHandle, _packetConfig); } }

		// 데이터 수신 시간 및 PPS 측정 측정
		private int       _packetCount = 0;  // 일정 시간 동안 수집된 packet 수
		private Stopwatch _stopwatch;        // 경과 시간 확인
        private double    _checkPoint;       // PPS 계산을 위한 데이터 수집 시작 지점
		//private double    _connectedTime;    // 디바이스 연결이 완료된 시점
		private TimeTracker _timeTracker = new TimeTracker();

		// Resource config 설정
		private ResourceConfig _resourceConfig;

		// CPU limit, memory limit 설정
		public  bool    useCPULimit    { get { return _resourceConfig.GetCPULimitUsage();    } set { _resourceConfig.SetCPULimitUsage(value);    SetResourceConfig(_CPPHandle, _resourceConfig); } }
		public  bool    useMemoryLimit { get { return _resourceConfig.GetMemoryLimitUsage(); } set { _resourceConfig.SetMemoryLimitUsage(value); SetResourceConfig(_CPPHandle, _resourceConfig); } }
		public  double  CPULimit    { get { return _resourceConfig.CPULimit;    } set { _resourceConfig.CPULimit    = value; SetResourceConfig(_CPPHandle, _resourceConfig); } }
		public  int     memoryLimit { get { return _resourceConfig.memoryLimit; } set { _resourceConfig.memoryLimit = value; SetResourceConfig(_CPPHandle, _resourceConfig); } }

		// 로그 파일
		private string _filePath = "log.txt";
		private long _maxFileSizeInBytes = 1024 * 1024; // 최대 파일 크기: 1MB

		public SerialHandle()
		{
			//OpenConsoleAndPrint();

			_packetConfig = new PacketConfig();
			_packetConfig.receiveByteSize = 1;
			_packetConfig.receiveBufferSize = 10000;
			_packetConfig.usePacketLength = 0;
			_packetConfig.packetLength = 1;
			_packetConfig.useStopByte = 0;
			_packetConfig.stopByte = IntPtr.Zero;
			_packetConfig.stopByteLength = 0;
			_packetConfig.usePPSLimit = 0;
			_packetConfig.PPSLimit = 300;
			_packetConfig.flushRatio = 0.9f;
			_stopByte = new byte[0];

			_resourceConfig.useCPULimit = 0;
			_resourceConfig.CPULimit = 95;
			_resourceConfig.useMemoryLimit = 0;
			_resourceConfig.memoryLimit = 100;

			PtrManager.StringToIntPtr(ref _deviceNamePtr, "");
			_connectionConfig.deviceName = _deviceNamePtr;
			_connectionConfig.useTimeout = 1;
			_connectionConfig.connectionTimeout = 30000;

			_onScanEnded    = new EventCallbackWithChar(OnScanEnded);
			_onLogReceived  = new EventCallbackWithChar(OnLogReceived);
			_onDataReceived = new EventCallbackWithChar(OnDataReceived);

			_onConnected        = new EventCallback(OnConnected);
			_onConnectionFailed = new EventCallback(OnConnectionFailed);
			_onDisconnected     = new EventCallback(OnDisconnected);

			SetScanCallback(_onScanEnded);
			SetLogCallback(_onLogReceived);
			SetLogLevel(2);

			SetLogFile(" ");
			SetLogFile(" ");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(GetCurrentTime());
			SetLogFile("Instance created");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(" ");
			SetLogFile(" ");
		}

		~SerialHandle()
		{
			Marshal.FreeHGlobal(_deviceNamePtr);
			Marshal.FreeHGlobal(_AIPortsPtr);
			Marshal.FreeHGlobal(_AOPortsPtr);
			Marshal.FreeHGlobal(_DPortsPtr);
			Marshal.FreeHGlobal(_LinesPtr);
			Marshal.FreeHGlobal(_stopBytePtr);
		}

		private string GetCurrentTime()
		{
			// 현재 시간 가져오기
			DateTime now = DateTime.Now;

			// 연월일시분초 형태로 문자열 변환
			return now.ToString("yyyy-MM-dd-HH-mm-ss");
		}

		private void SetLogFile(string lineToAdd)
		{
			// 파일의 크기 확인
            if (File.Exists(_filePath) && new FileInfo(_filePath).Length > _maxFileSizeInBytes) {
                // 파일 크기가 초과되면 파일 내용 삭제
                File.WriteAllText(_filePath, string.Empty);
            }

			// 내용 추가
			using (StreamWriter writer = File.AppendText(_filePath)) {
				writer.WriteLine(lineToAdd);
			}
		}

		private void OnLogReceived(IntPtr ptr, ref int length)
		{
			byte header = Marshal.ReadByte(ptr);
			string log = Marshal.PtrToStringUni(ptr + 1, (length - 1) / 2);
			// ASCII '0' : 48, '1' : 49, '2' : 50
			// log level을 ASCII 문자로 반환
			if (header - 48 <= _logLevel) {
				Invoke(onLogReceived, new SerialLog(log));
			}
			SetLogFile(log);
			DeleteLogMemory(ptr);
		}

		public void ScanDevices()
		{
			SetLogFile(" ");
			SetLogFile(" ");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(GetCurrentTime());
			SetLogFile("Device scan requested");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(" ");
			SetLogFile(" ");

			CPPImportLayer.ScanDevices();
		}

		private void OnScanEnded(IntPtr ptr, ref int length)
		{
			string devicesStr = Marshal.PtrToStringUni(ptr, length / 2);
			string[] devicesTokens = devicesStr.Split('\\');
			if (devicesTokens.Length > 1) {
				string[] devices = new string[devicesTokens.Length - 1];
				Array.Copy(devicesTokens, devices, devices.Length);
				Invoke(onScanEnded, new SerialLog(devicesStr.Split('\\')));
			}
			else {
				Invoke(onScanEnded, new SerialLog(new string[] { }));
			}
			DeleteLogMemory(ptr);
		}

		private void CreateConnection()
		{
			Random rand = new Random();
			int randNum = (rand.Next(1000, 10000));
			if (CPPImportLayer.CreateConnection(randNum) == 0) {
				CreateConnection();
			}
			else {
				_CPPHandle = randNum;
			}
		}

		public void Connect(string comPort, int baudRate,
			DataBit dataBit = DataBit.bit8, Parity parity = Parity.None, StopBit stopBit = StopBit.bit1, FlowControl flowControl = FlowControl.None)
		{
			if (_isConnected) Disconnect();

			_connectionConfig.deviceType = DeviceType.USB;

			PtrManager.StringToIntPtr(ref _deviceNamePtr, comPort);
			_connectionConfig.deviceName = _deviceNamePtr;

			_connectionConfig.baudRate = baudRate;
			_connectionConfig.dataBit = dataBit;
			_connectionConfig.parity = parity;
			_connectionConfig.stopBit = stopBit;
			_connectionConfig.flowControl = flowControl;
			
			Connect();
		}

		public void Connect(string deviceName)
		{
			if (_isConnected) Disconnect();

			_connectionConfig.deviceType = DeviceType.BTClassic;

			PtrManager.StringToIntPtr(ref _deviceNamePtr, deviceName);
			_connectionConfig.deviceName = _deviceNamePtr;

			Connect();
		}

		public void Connect(string deviceName, int[] AIPorts, int[] AOPorts, int[] DPorts, int[] lines)
		{
			if (_isConnected) Disconnect();
			
			_connectionConfig.deviceType = DeviceType.DAQ;

			PtrManager.StringToIntPtr(ref _deviceNamePtr, deviceName);
			_connectionConfig.deviceName = _deviceNamePtr;

			PtrManager.IntArrToIntPtr(ref _AIPortsPtr, AIPorts);
			_connectionConfig.AIPorts      = _AIPortsPtr;
			_connectionConfig.AIPortsCount = AIPorts.Length;

			PtrManager.IntArrToIntPtr(ref _AOPortsPtr, AOPorts);
			_connectionConfig.AOPorts      = _AOPortsPtr;
			_connectionConfig.AOPortsCount = AOPorts.Length;

			PtrManager.IntArrToIntPtr(ref _DPortsPtr, DPorts);
			_connectionConfig.DPorts      = _DPortsPtr;
			_connectionConfig.DPortsCount = DPorts.Length;

			PtrManager.IntArrToIntPtr(ref _LinesPtr, lines);
			_connectionConfig.lines      = _LinesPtr;
			_connectionConfig.linesCount = lines.Length;

			Connect();
		}

		public void Connect(int localIP, int port)
		{
			// TCP server
			if (_isConnected) Disconnect();
			
			PtrManager.StringToIntPtr(ref _deviceNamePtr, "TCP server");
			_connectionConfig.deviceName = _deviceNamePtr;
			_connectionConfig.deviceType = DeviceType.TCP;
			_connectionConfig.socketType = SocketType.Server;
			_connectionConfig.localIP = localIP;
			_connectionConfig.port = port;
			_connectionConfig.useTimeout = 0;

			Connect();
		}

		private void SetStopByte(byte[] stopByte)
		{
			PtrManager.ByteArrToIntPtr(ref _stopBytePtr, stopByte);
			_packetConfig.stopByte = _stopBytePtr;
			_packetConfig.stopByteLength = stopByte.Length;

			SetPacketConfig(_CPPHandle, _packetConfig);
		}

		private void Connect()
		{
			SetLogFile(" ");
			SetLogFile(" ");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(GetCurrentTime());
			SetLogFile("Connect requested");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(" ");
			SetLogFile(" ");

			CreateConnection();

			SetPacketConfig(_CPPHandle, _packetConfig);
			SetStopByte(_stopByte);
			SetResourceConfig(_CPPHandle, _resourceConfig);
			RegisterEvents(_CPPHandle, _onConnected, _onConnectionFailed, _onDisconnected);
			RegisterDataEvent(_CPPHandle, _onDataReceived);

			CPPImportLayer.Connect(_CPPHandle, _connectionConfig);

			_stopwatch = Stopwatch.StartNew();
            _checkPoint = _stopwatch.Elapsed.TotalSeconds * 1000;
			//_connectedTime = _stopwatch.Elapsed.TotalSeconds;
		}

		private void OnConnected()
		{
			_isConnected = true;
			_timeTracker.Reset(); // 연결 완료 시점으로 부터 데이터 수집 시간을 측정하기 위해 time tracker reset
			Invoke(onConnected);
		}

		private void OnConnectionFailed()
		{
			_isConnected = false;
			Invoke(onConnectionFailed);
		}

		public void Disconnect()
		{
			SetLogFile(" ");
			SetLogFile(" ");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(GetCurrentTime());
			SetLogFile("Disconnect requested");
			SetLogFile("////////////////////////////////////////////////////////////");
			SetLogFile(" ");
			SetLogFile(" ");

			CPPImportLayer.Disconnect(_CPPHandle);
		}

		private void OnDisconnected()
		{
			_isConnected = false;
			Invoke(onDisconnected);
			DeleteConnection(_CPPHandle);
		}

		private string EncodeData(IntPtr ptr, int length)
        {
			byte[] buffer = new byte[length];
			Marshal.Copy(ptr, buffer, 0, length);

            string str = "";
            if (_encodingType == EncodingType.DEC) {
                for (int i = 0; i < buffer.Length; i++) {
                    str += buffer[i].ToString();
                    if (i != buffer.Length-1) {
                        str += "-";
                    }
                }
            }
            else if (_encodingType == EncodingType.HEX) {
                str = BitConverter.ToString(buffer);
            }
            else if (_encodingType == EncodingType.ASCII) {
                str = Encoding.ASCII.GetString(buffer, 0, buffer.Length);
            }
            else if (_encodingType == EncodingType.UTF8) {
                str = Encoding.UTF8.GetString(buffer);
            }
            else if (_encodingType == EncodingType.UTF16) {
				str = Encoding.Unicode.GetString(buffer);
            }
            return str;
        }

		private double[] EncodeData(IntPtr ptr, int length, int portsCount)
		{
			double[] recvData = new double[portsCount];

			byte[] buffer = new byte[length];
			Marshal.Copy(ptr, buffer, 0, length);

			for (int i = 0; i < portsCount; i++) {
				byte[] byteBuff = new byte[8];
				for (int j = 0; j < 8; j++) {
					byteBuff[j] = buffer[j + 8 * i];
				}
				recvData[i] = BitConverter.ToDouble(byteBuff, 0);
			}

			return recvData;
		}

		private void OnDataReceived(IntPtr ptr, ref int length)
		{
			double elapsedTime = _stopwatch.Elapsed.TotalSeconds * 1000 - _checkPoint;
			//double timeAfterConnected = _stopwatch.Elapsed.TotalSeconds - _connectedTime;
			// 연결 완료 후 경과 시간 수집
			double timeAfterConnected = (double)_timeTracker.GetElapsedTime() / 1000 / 1000; // 마이크로 초 단위를 초 단위로 변환
			_packetCount++;

			if (elapsedTime >= 1000) {
				_PPS = _packetCount;
				_packetCount = 0;
				_checkPoint = _stopwatch.Elapsed.TotalSeconds * 1000;
			}

			if (getReceivedBufferLength && _isConnected) {
				GetReceivedBufferLength(_CPPHandle, ref _receivedBufferLength);
			}
			else {
				_receivedBufferLength = 0;
			}
			
			if (_connectionConfig.deviceType == DeviceType.DAQ) {
				double[] data = EncodeData(ptr, length, _connectionConfig.AIPortsCount);
				Invoke(onDataReceived, timeAfterConnected, new SerialData(data));
			}
			else {
				string packet = EncodeData(ptr, length);
				Invoke(onDataReceived, timeAfterConnected, new SerialData(packet));
			}
		}

		public void SendData(string data)
		{
			if (!_isConnected) return;
			byte[] byteToSend;
            if (_encodingType == EncodingType.UTF8) {
                byteToSend = Encoding.UTF8.GetBytes(data);
            }
            else if (_encodingType == EncodingType.UTF16) {
				byteToSend = Encoding.Unicode.GetBytes(data);
            }
			else {
				// Encoding type = ASCII, DEC, HEX의 경우
                byteToSend = Encoding.ASCII.GetBytes(data);
            }
			CPPImportLayer.SendPacketData(_CPPHandle, byteToSend, byteToSend.Length);
		}

		public void SendData(byte[] data)
		{
			if (!_isConnected) return;
			CPPImportLayer.SendPacketData(_CPPHandle, data, data.Length);
		}

		public void SendData(double[] data)
		{
			if (!_isConnected) return;
			CPPImportLayer.SendFloatData(_CPPHandle, data, data.Length);
		}

		public void SendData(bool[] data)
		{
			if (!_isConnected) return;
			int[] intData = new int[data.Length];
			for (int i = 0; i < data.Length; i++) {
				intData[i] = data[i] ? 1 : 0;
			}
			CPPImportLayer.SendIntData(_CPPHandle, intData, intData.Length);
		}

		public float[] CheckResources()
		{
			if (_CPPHandle == 0) return new float[] { -1, -1 };
			else {
				double CPUUsage = -1;
				int memoryRemained = -1;
				CheckResource(_CPPHandle, ref CPUUsage, ref memoryRemained);

				return new float[] { (float)CPUUsage, memoryRemained };
			}
		}
	}
}