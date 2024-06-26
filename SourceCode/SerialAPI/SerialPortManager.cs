using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Text;
using System.Threading;
using System.Reflection.Emit;
using System.Reflection;

namespace SerialAPI
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

    internal class SerialPortManager : SerialHandle
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
        private static extern void SetRecvBufferSize(string label, int size);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern int  GetRecvBufferSize(string label);
		
		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void GetRecvData(string label, byte[] data);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void GetRecvDone(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "ScanDevices", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPScanDevices(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void GetScannedDevice(string label, byte[] byteData);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void RegisterOnScanEnded(string label, EventCallback onScanEnded);

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
			CPPSendData(_label, data, data.Length);
		}

		internal override void SetBufferSize(int size)
		{
			base.SetBufferSize(size);
			if (_label == "") return;
			SetRecvBufferSize(_label, size);
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
			int nRecvBuff = GetRecvBufferSize(_label);
			if (nRecvBuff < 1) return;

			byte[] data = new byte[nRecvBuff];
			GetRecvData(_label, data);

			if (_packetManager.EncodePacket(data)) {
				if (getPPSOnDataReceived) _pps = _ppsChecker.GetPPSOnDataReceived();
				Invoke(EventType.OnDataReceived, new SerialData(_packetManager.recvData), null);
			}

			GetRecvDone(_label);
		}
	}
}