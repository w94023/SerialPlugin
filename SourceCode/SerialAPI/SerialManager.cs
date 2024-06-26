using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using static SerialAPI.SerialHandle;
using static SerialAPI.SerialPortManager;
using static System.Collections.Specialized.BitVector32;

namespace SerialAPI
{
	public class SerialManager
	{
		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int CheckMemory();

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetLog(byte[] log);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void RegisterLogging(EventCallback callback);

		private SerialHandle _handle;

		// Log level 설정
		// 0 : Error, 1 : Error+Normal, 2 : Error+Normal+Developer
		public int logLevel = 1;
		// Packet per seconds config
        public  bool getPPSOnDataReceived = false;
        public  int  pps                  { get { if (_handle == null) { return -1; } else { return _handle.pps; } } }
		// Connection check config
		public  bool isConnected { get { if (_handle == null) { return false; } else { return _handle.isConnected; } } }
		// Packet check, encoding config
		public  int			 bufferSize     { get { return _bufferSize; } set { _bufferSize = value; if (_handle != null) _handle.SetBufferSize(_bufferSize); } }
		//public  int			 bufferSize     { get { return _bufferSize; } set { _bufferSize = value; } }
		private int			 _bufferSize    = 10000;
		public  byte[]       stopByte	    { get { return _stopByte; } set { _stopByte = value; if (_handle != null) _handle.SetStopByte(_stopByte); } }
		private byte[]       _stopByte      = new byte[0];
		public  int          packetLength   { get { return _packetLength; } set { _packetLength = value; if (_handle != null) _handle.SetPacketLength(_packetLength); } }
		private int          _packetLength  = 0;
		public  EncodingType encodingType   { get { return _encodingType; } set { _encodingType = value; if (_handle != null) _handle.SetEncodingType(_encodingType); } }
		private EncodingType _encodingType  = EncodingType.ASCII;

		// Events
		private event EventCallback _onLogReceived;
		private int registeredOnScanEndedCallback = 0;

		public event Action<SerialLog>  onLogReceived;
		public event Action<SerialLog>  onScanEnded;
		public event Action             onConnected;
		public event Action             onConnectionFailed;
		public event Action             onDisconnected;
		public event Action<SerialData> onDataReceived;

		public SerialManager()
		{
			Thread processThread = new Thread(ProcessCleaning);
			processThread.Start();

			_onLogReceived = new EventCallback(OnLogReceived);
			RegisterLogging(_onLogReceived);
		}

		~SerialManager()
		{
			_onLogReceived = null;
		}

		private void ProcessCleaning()
		{
			Thread.Sleep(10);
			GC.Collect();
			CheckMemory();
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
			_handle.SetStopByte(_stopByte);
			_handle.SetPacketLength(_packetLength);
			_handle.SetEncodingType(_encodingType);
			_handle.SetBufferSize(_bufferSize);
			RegisterEventsToHandle();
			_handle.Connect();
		}

		private void RegisterEventsToHandle()
		{
			_handle.onConnected		   += OnConnected;
			_handle.onConnectionFailed += OnConnectionFailed;
			_handle.onDisconnected     += OnDisconnected;
			_handle.onDataReceived     += OnDataReceived;
		}

		public void Disconnect()
		{
			if (_handle == null) return;
			_handle.Disconnect();
			_handle = null;
			registeredOnScanEndedCallback = 0;
			GC.Collect();
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

		private void OnLogReceived()
		{
			byte[] log = new byte[260];
			int logLevel = GetLog(log);
			if (logLevel > this.logLevel) return; 

			string header = "";
			switch (logLevel) {
				case 0: header = "[Error log]";     break;
				case 1: header = "[Normal log]";    break;
				case 2: header = "[Developer log]"; break;
				default: header = ""; break;
			}
			string logStr = header + Encoding.Unicode.GetString(log, 0, log.Length);
			onLogReceived?.Invoke(new SerialLog(logStr));
		}

		private void OnScanEnded(SerialLog e) => onScanEnded?.Invoke(e);

		private void OnConnected() => onConnected?.Invoke();

		private void OnConnectionFailed() => onConnectionFailed?.Invoke();

		private void OnDisconnected() => onDisconnected?.Invoke();

		private void OnDataReceived(SerialData e) => onDataReceived?.Invoke(e);
	}
}