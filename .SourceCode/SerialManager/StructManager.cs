using System;
using System.Runtime.InteropServices;

namespace SerialManager
{
    internal static class PtrManager
    {
        internal static void StringToIntPtr(ref IntPtr ptr, string str)
        {
            Marshal.FreeHGlobal(ptr);
			ptr = Marshal.StringToHGlobalUni(str);
        }

        internal static void IntArrToIntPtr(ref IntPtr ptr, int[] arr)
        {
            Marshal.FreeHGlobal(ptr);
            int size = arr.Length * Marshal.SizeOf(typeof(int));
            ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(arr, 0, ptr, arr.Length);
        }

        internal static void ByteArrToIntPtr(ref IntPtr ptr, byte[] arr)
        {
            Marshal.FreeHGlobal(ptr);
            int size = arr.Length * Marshal.SizeOf(typeof(byte));
            ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(arr, 0, ptr, arr.Length);
        }
    }

	public class SerialData
    {
        public string   packet = "";
        public double[] data   = new double[0];

        public SerialData(string packet) { this.packet = packet; }
        public SerialData(double[] data) { this.data = data; }
    }

    public class SerialLog
    {
        public string   log;
        public string[] devices;

        public SerialLog(string log)       { this.log = log; }
        public SerialLog(string[] devices) { this.devices = devices; }
    }

    public struct UUID
    {
        public string service;
		public string tx;
		public string rx;

		public UUID(int a = 0)
		{
			service = "{0000FFE0-0000-1000-8000-00805F9B34FB}";
			tx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
			rx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
		}
    }

    public enum DeviceType
	{
        None,
		USB,
		BTClassic,
		BLE,
		DAQ,
        TCP
	}

    public enum BaudRate
    {
        bps1200     = 1200,
        bps2400     = 2400,
        bps4800     = 4800,
        bps9600     = 9600,
        bps19200    = 19200,
        bps38400    = 38400,
        bps57600    = 57600,
        bps115200   = 115200
    }

    public enum DataBit
    {
        bit5 = 5,
        bit6 = 6,
        bit7 = 7,
        bit8 = 8
    }

    public enum Parity
    {
        None    = 0,
        Odd     = 1,
        Even    = 2
    }

    public enum StopBit
    {
        bit1    = 0,
        bit1p5  = 1,
        bit2    = 2
    }

    public enum FlowControl
    {
        None    = 0,
        XonXoff = 1,
        RTSCTS  = 2
    }

    public enum EncodingType
    {
        DEC,
        HEX,
        ASCII,
        UTF8,
        UTF16,
    }

    public enum SocketType
    {
        Server,
        Clinet
    }
}
