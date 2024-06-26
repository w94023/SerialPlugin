using System;
using System.Text;
using System.Runtime.InteropServices;

namespace SerialManager
{
    public class UUID
    {
        public string service;
	    public string tx;
	    public string rx;

		public UUID()
		{
			service = "{0000FFE0-0000-1000-8000-00805F9B34FB}";
			tx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
			rx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
		}
    }

    internal class SerialPortManager : ComHandle
    {
		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void CreateConnectionManager(string label, byte[] result);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void SetBTClassicConfig(string label, string deviceName);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void SetBLEConfig(string label, string deviceName, string uuidService, string uuidTx, string uuidRx);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void SetCOMConfig(string label, string comPort, int comBps, int comDataBits, int comParity, int comStopBits, int comFlowControl);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void RegisterEvents(string label, EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected, EventCallback onDataReceived);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "Connect", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPConnect(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "Disconnect", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPDisconnect(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void DeleteConnectionManager(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "SendData", CallingConvention = CallingConvention.Cdecl)]
        private static extern void CPPSendData(string label, byte[] data, int length);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetConnectionTimeout(string label, int timeout);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetRecvBufferSize(string label, int size);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern int  GetRecvBufferSize(string label);
		
		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern int  GetRecvData(string label, byte[] data, int byteSize);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void GetRecvDone(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "ScanDevices", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPScanDevices(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void GetScannedDevice(string label, byte[] byteData);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void RegisterOnScanEnded(string label, EventCallback onScanEnded);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "SetPPSLimit", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPSetPPSLimit(string label, int PPSLimit);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "SetCPULimit", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPSetCPULimit(string label, double CPULimit);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "SetMemoryLimit", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPSetMemoryLimit(string label, int memoryLimit);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "CheckResources", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPCheckResources(string label, out double cpuUsage, out int memoryRemained);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "Test", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPTest(string label, double cpuUsage, int memoryRemained);

		internal SerialPortManager()
        {
			CreateConnection();
        }

		internal SerialPortManager(string deviceName)
        {
			CreateConnection();
			SetBTClassicConfig(_label, deviceName);
        }

		internal SerialPortManager(string deviceName, UUID uuid)
        {
			CreateConnection();
			SetBLEConfig(_label, deviceName, uuid.service, uuid.tx, uuid.rx);
        }

		internal SerialPortManager(string comPort, BaudRate baudRate, DataBit dataBit, Parity parity, StopBit stopBit, FlowControl flowControl)
        {
			CreateConnection();
			SetCOMConfig(_label, comPort, (int)baudRate, (int)dataBit, (int)parity, (int)stopBit, (int)flowControl);
        }

		~SerialPortManager()
		{
			if (_label == "") return;
			DeleteConnectionManager(_label);
		}

		internal override void ScanDevices()
		{ 
			RegisterOnScanEnded(_label, _onScanEnded);
			CPPScanDevices(_label);
		}

		private void CreateConnection()
		{
			Random rand = new Random();
			int randNum = (rand.Next(1, 10000));

			byte[] result = new byte[1];
			CreateConnectionManager(randNum.ToString(), result);

			if (result[0] != 2) {
				CreateConnection();
			}
			else {
				_label = randNum.ToString();
			}
		}

		internal override void Connect()
		{
			if (_label == "") return;
			RegisterEvents(_label, _onConnected, _onConnectionFailed, _onDisconnected, _onDataReceived);
			CPPConnect(_label);
		}

		internal override void Disconnect()
		{
			if (_label == "") return;
			CPPDisconnect(_label);
		}

		internal override void SendData(string message)
		{
			if (_label == "") return;
			byte[] asciiBytes = Encoding.ASCII.GetBytes(message);
			CPPSendData(_label, asciiBytes, asciiBytes.Length);
		}

		internal override void SendData(byte[] data)
		{
			if (_label == "") return;
			if (!_isConnected) return;
			CPPSendData(_label, data, data.Length);
		}

		internal override void SetConnectionTimeout(int timeout)
		{
			base.SetConnectionTimeout(timeout);
			if (_label == "") return;
			SetConnectionTimeout(_label, timeout);
		}

		internal override void SetBufferSize(int size)
		{
			base.SetBufferSize(size);
			if (_label == "") return;
			SetRecvBufferSize(_label, size);
		}

		internal override void SetPPSLimit(int PPSLimit)
		{
			if (_label == "") return;
			CPPSetPPSLimit(_label, PPSLimit);
		}

		internal override void SetCPULimit(double CPULimit)
		{
			if (_label == "") return;
			CPPSetCPULimit(_label, CPULimit);
		}

		internal override void SetMemoryLimit(int memoryLimit)
		{
			if (_label == "") return;
			CPPSetMemoryLimit(_label, memoryLimit);
		}

		internal override float[] CheckResources()
		{
			if (_label == "") return new float[]{ -1, -1 };
			double cpuUsage;
			int    memoryRemained;
			CPPCheckResources(_label, out cpuUsage, out memoryRemained);
			return new float[] { (float)cpuUsage, memoryRemained };
		}

		internal override void Test(double cpuUsage, int memoryRemained)
		{
			if (_label == "") return;
			CPPTest(_label, cpuUsage, memoryRemained);
		}

		internal override void OnScanEnded()
		{
			base.OnScanEnded();
			byte[] scannedDeviceInByte = new byte[10000];
			GetScannedDevice(_label, scannedDeviceInByte);
			string scannedDeviceInStr = Encoding.Unicode.GetString(scannedDeviceInByte, 0, scannedDeviceInByte.Length);
			string[] splitStr = scannedDeviceInStr.Split('\\');
			string[] scannedDevice = new string[splitStr.Length - 1];
			for (int i = 0; i < splitStr.Length - 1; i++) {
				scannedDevice[i] = splitStr[i];
			}
			Invoke(EventType.OnScanEnded, null, new SerialLog(scannedDevice));
		}

		internal override void OnConnected()
		{
			base.OnConnected();
			Invoke(EventType.OnConnected, null, null);
		}

		internal override void OnConnectionFailed()
		{
			base.OnConnectionFailed();
			Invoke(EventType.OnConnectionFailed, null, null);
		}

		internal override void OnDisconnected()
		{
			base.OnDisconnected();
			Invoke(EventType.OnDisconnected, null, null);
		}

		internal override void OnDataReceived()
		{
			byte[] data = new byte[receiveByteSize];
			int bytesRead = GetRecvData(_label, data, data.Length);
			if (bytesRead != data.Length) {
				byte[] newData = new byte[bytesRead];
				for (int i = 0; i < newData.Length; i++) {
					newData[i] = data[i];
				}
				data = newData;
			}

			bool encodingResult = _packetManager.EncodePacket(data);

			if (encodingResult) {
				if (getPPSOnDataReceived) _PPS = _ppsChecker.GetPPSOnDataReceived();
				Invoke(EventType.OnDataReceived, new SerialData(_packetManager.recvData), null);
				GetRecvDone(_label);
			}
			else {
				GetRecvDone(_label);
				if (bytesRead > 0) {
					OnDataReceived();
				}
			}
		}
	}
}