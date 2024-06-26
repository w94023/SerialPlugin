using System;
using System.Runtime.InteropServices;

namespace SerialManager
{
	internal class DAQManager : ComHandle
	{
		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void CreateConnectionManager(string label, byte[] result);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void SetDAQConfig(string label, string deviceName, byte[] ports, int portsNum);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void RegisterEvents(string label, EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected, EventCallback onDataReceived);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "Connect", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPConnect(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, EntryPoint = "Disconnect", CallingConvention = CallingConvention.Cdecl)]
		private static extern void CPPDisconnect(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void DeleteConnectionManager(string label);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetRecvData(string label, byte[] data);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		private static extern void GetRecvDone(string label);

		private byte[]   _ports;
		private int	     _portsNum = 0;
		private byte[]   _recvByteData;
		private double[] _recvData;

		internal DAQManager(string deviceName, int[] ports)
		{
			CreateConnection();

			_portsNum = ports.Length;
			_ports = new byte[_portsNum];
			for (int i = 0; i < _portsNum; i++) {
				if (ports[i] < 0 || ports[i] > 255) continue;
				_ports[i] = (byte)ports[i];
			}

			SetDAQConfig(_label, deviceName, _ports, _portsNum);

			_recvByteData = new byte[8 * _portsNum];
			_recvData = new double[_portsNum];
		}

		~DAQManager()
		{
			if (_label == "") return;
			DeleteConnectionManager(_label);
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
            GetRecvData(_label, _recvByteData);

			for (int i = 0; i < _portsNum; i++) {
				byte[] byteBuff = new byte[8];
				for (int j = 0; j < 8; j++) {
					byteBuff[j] = _recvByteData[j + 8 * i];
				}
				_recvData[i] = BitConverter.ToDouble(byteBuff, 0);
			}

			if (getPPSOnDataReceived) _PPS = _ppsChecker.GetPPSOnDataReceived();
			Invoke(EventType.OnDataReceived, new SerialData(_recvData), null);

			GetRecvDone(_label);
		}
	}
}
