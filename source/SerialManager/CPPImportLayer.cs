using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static SerialManager.EventManager;

namespace SerialManager
{
	internal static class CPPImportLayer
	{
		[StructLayout(LayoutKind.Sequential)]
		public struct CPP_UUID
		{
			public string service;
			public string tx;
			public string rx;

			public CPP_UUID(UUID uuid)
			{
				service = uuid.service;
				tx      = uuid.tx;
				rx      = uuid.rx;
			}
		}

		[StructLayout(LayoutKind.Sequential)]
		internal struct ConnectionConfig
		{
			public DeviceType deviceType;
			public IntPtr deviceName;
			// For USB
			public int    baudRate;
			public DataBit dataBit;
			public Parity parity;
			public StopBit stopBit;
			public FlowControl flowControl;
			// For DAQ
			public IntPtr AIPorts;
			public int    AIPortsCount;
			public IntPtr AOPorts;
			public int    AOPortsCount;
			public IntPtr DPorts;
			public int    DPortsCount;
			public IntPtr lines;
			public int    linesCount;
			//// For BLE
			//public UUID uuid;
			// For TCP
			public SocketType socketType;
			public int localIP;
			public int port;

			public int useTimeout;
			public int connectionTimeout;
		}

		[StructLayout(LayoutKind.Sequential)]
		internal struct ResourceConfig
		{
			public int    useCPULimit;
			public double CPULimit;
			public int    useMemoryLimit;
			public int    memoryLimit;

			public bool GetCPULimitUsage()    { return (useCPULimit    == 0) ? false : true; }
			public bool GetMemoryLimitUsage() { return (useMemoryLimit == 0) ? false : true; }

			public void SetCPULimitUsage(bool value)    { useCPULimit    = (value) ? 1 : 0; }
			public void SetMemoryLimitUsage(bool value) { useMemoryLimit = (value) ? 1 : 0; }
		}

		[StructLayout(LayoutKind.Sequential)]
		internal struct PacketConfig
		{
			public int    receiveByteSize;
			public int    receiveBufferSize;
			public int    usePacketLength;
			public int    packetLength;
			public int    useStopByte;
			public IntPtr stopByte;
			public int    stopByteLength;
			public int    usePPSLimit;
			public int    PPSLimit;
			public float  flushRatio;

			public bool GetPacketLengthUsage() { return (usePacketLength == 0) ? false : true; }
			public bool GetStopByteUsage()     { return (useStopByte     == 0) ? false : true; }
			public bool GetPPSLimitUsage()     { return (usePPSLimit    == 0)  ? false : true; }

			public void SetPacketLengthUsage(bool value) { usePacketLength = (value) ? 1 : 0; }
			public void SetStopByteUsage(bool value)     { useStopByte     = (value) ? 1 : 0; }
			public void SetPPSLimitUsage(bool value)     { usePPSLimit     = (value) ? 1 : 0; }
		}

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void OpenConsoleAndPrint();

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SetScanCallback(EventCallbackWithChar callback);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SetLogCallback(EventCallbackWithChar callback);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SetLogLevel(int logLevel);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void DeleteLogMemory(IntPtr logPtr);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern int ScanDevices();

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern int CreateConnection(int CPPHandle);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void DeleteConnection(int CPPHandle);

		[DllImport("SerialPort.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		internal static extern void Connect(int CPPHandle, ConnectionConfig connectionConfig);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void Disconnect(int CPPHandle);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SetPacketConfig(int CPPHandle, PacketConfig packetConfig);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SetResourceConfig(int CPPHandle, ResourceConfig resourceConfig);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void CheckResource(int CPPHandle, ref double CPUUsage, ref int memoryRemained);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void RegisterDataEvent(int CPPHandle, EventCallbackWithChar callback);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void RegisterEvents(int CPPHandle, EventCallback onConnected, EventCallback onConnectionFailed, EventCallback onDisconnected);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void GetReceivedBufferLength(int CPPHandle, ref int length);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SendPacketData(int CPPHandle, byte[] data, int dataLength);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SendFloatData(int CPPHandle, double[] data, int dataLength);

		[DllImport("SerialPort.dll", CallingConvention = CallingConvention.Cdecl)]
		internal static extern void SendIntData(int CPPHandle, int[] data, int dataLength);
	}
}
