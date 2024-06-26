using System;
using System.Collections;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace SerialAPI
{
    internal class SerialHandle : EventManager
	{
        internal delegate void EventCallback();

        // Packet per seconds config
        internal PPSChecker _ppsChecker          = new PPSChecker();
        internal bool       getPPSOnDataReceived = false;
        internal int        pps                  { get { return _pps; } }
        internal int        _pps                 = 0;
        // Connection check config
        internal bool       isConnected          { get { return _isConnected; } }
        internal bool       _isConnected         = false;
		// Packet check, encoding config
        internal PacketManager _packetManager = new PacketManager();
        // Event log
        internal string _eventLog = "";

        internal string _label = "";

		// Callbacks
        internal EventCallback _onScanEnded;
        internal EventCallback _onConnected;
		internal EventCallback _onConnectionFailed;
		internal EventCallback _onDisconnected;
		internal EventCallback _onDataReceived;

        internal virtual void OnScanEnded()        { }
        internal virtual void OnConnected()        { _isConnected = true; }
        internal virtual void OnConnectionFailed() { _isConnected = false; }
        internal virtual void OnDisconnected()     { _isConnected = false; }
        internal virtual void OnDataReceived()     { }

        // Events
        internal enum EventType
        {
            OnScanEnded,
            OnConnected,
            OnConnectionFailed,
            OnDisconnected,
            OnDataReceived,
        }
        internal virtual event Action<SerialLog>  onScanEnded;
        internal virtual event Action             onConnected;
        internal virtual event Action             onConnectionFailed;
        internal virtual event Action             onDisconnected;
        internal virtual event Action<SerialData> onDataReceived;

        internal SerialHandle()
        {
            _onScanEnded        = new EventCallback(OnScanEnded);
            _onConnected        = new EventCallback(OnConnected);
			_onConnectionFailed = new EventCallback(OnConnectionFailed);
			_onDisconnected     = new EventCallback(OnDisconnected);
			_onDataReceived     = new EventCallback(OnDataReceived);
        }

        ~SerialHandle()
        {
            _onScanEnded        = null;
            _onConnected        = null;
			_onConnectionFailed = null;
			_onDisconnected     = null;
			_onDataReceived     = null;
        }
        
        internal virtual void ScanDevices() { }
        internal virtual void Connect() { }
        internal virtual void Disconnect() { }
        internal virtual void SendData(string data) { }
        internal virtual void SendData(byte[] data) { }

        internal virtual void SetBufferSize(int size)            { } 
        internal virtual void SetStopByte(byte[] stopByte)       { _packetManager.stopByte = stopByte; }
		internal virtual void SetPacketLength(int packetLength)  { _packetManager.packetLength = packetLength; }
		internal virtual void SetEncodingType(EncodingType type) { _packetManager.encodingType = type; }

        internal void Invoke(EventType type, SerialData data, SerialLog log)
        {
            switch (type) {
                case EventType.OnScanEnded:        Invoke(onScanEnded, log); break;
                case EventType.OnConnected:        Invoke(onConnected); break;
                case EventType.OnConnectionFailed: Invoke(onConnectionFailed); break;
                case EventType.OnDisconnected:     Invoke(onDisconnected); break;
                case EventType.OnDataReceived:     Invoke(onDataReceived, data); break;
            }
        }
    }

    internal class EventManager
    {
        internal static SynchronizationContext mainThreadContext;

        internal EventManager()
        {
            mainThreadContext = SynchronizationContext.Current;
        }

        internal static void Invoke(Action action)
        {
            if (mainThreadContext == null) action?.Invoke();
            else mainThreadContext.Post(state => { action?.Invoke(); }, null);
        }

        internal static void Invoke(Action<SerialData> action, SerialData input)
        {
            if (mainThreadContext == null) action?.Invoke(input);
            else mainThreadContext.Post(state => { action?.Invoke(input); }, null);
        }

		internal static void Invoke(Action<SerialLog> action, SerialLog input)
        {
            if (mainThreadContext == null) action?.Invoke(input);
            else mainThreadContext.Post(state => { action?.Invoke(input); }, null);
        }
    }

    internal class PPSChecker
    {
        private int      _pps         = 0;
        private DateTime _startTime   = DateTime.Now;
        private int      _packetCount = 0;

        internal int GetPPSOnDataReceived()
        {
            TimeSpan ts = DateTime.Now - _startTime;
            _packetCount++;

            double elapsedMilliseconds = ts.TotalMilliseconds;
            if (elapsedMilliseconds > 1000) {
                _startTime = DateTime.Now;
                _pps = _packetCount;
                _packetCount = 0;
            }

            return _pps;
        }
    }

    internal class PacketManager
    {
        public  byte[]       stopByte      = new byte[0];
        public  int          packetLength  = 0;
        public  EncodingType encodingType  = EncodingType.ASCII;
        private List<byte>   _buffer       = new List<byte>();
        public  string       recvData  { get { return _recvData; } }
        private string       _recvData = "";

        public List<byte> buffer {get {return _buffer;}}

        public bool EncodePacket(byte[] data)
		{
            if (stopByte.Length == 0) {
                _recvData = EncodeData(data);
                return true;
            }

            _buffer.AddRange(data);
            if(!ContainsArray(_buffer, stopByte)) {
                return false;
            }

            int index = _buffer.IndexOf(stopByte[stopByte.Length - 1]);
            byte[] _packet = _buffer.GetRange(0, index + 1).ToArray();
            _packet = _packet.Where(b => b != 0).ToArray(); // 배열에서 0 바이트 데이터 제거
            _recvData = EncodeData(_packet);
            _buffer.RemoveRange(0, index + 1);

            if (packetLength <= 0) return true;
            
            if (_packet.Length == packetLength) return true;
            else                                return false;
        }

        private bool ContainsArray(List<byte> mainArray, byte[] subArray)
        {
            for (int i = 0; i < mainArray.Count - subArray.Length + 1; i++)
            {
                bool isMatch = true;
                for (int j = 0; j < subArray.Length; j++)
                {
                    if (mainArray[i + j] != subArray[j])
                    {
                        isMatch = false;
                        break;
                    }
                }

                if (isMatch)
                    return true;
            }
            return false;
        }

        private string EncodeData(byte[] data)
        {
            string str = "";
            if (encodingType == EncodingType.DEC) {
                for (int i = 0; i < data.Length; i++) {
                    str += data[i].ToString();
                    if (i != data.Length-1) {
                        str += "-";
                    }
                }
            }
            else if (encodingType == EncodingType.HEX) {
                str = BitConverter.ToString(data).Replace("-", "");
            }
            else if (encodingType == EncodingType.ASCII) {
                str = Encoding.ASCII.GetString(data, 0, data.Length);
            }
            else if (encodingType == EncodingType.UTF8) {
                str = Encoding.UTF8.GetString(data);
            }
            else if (encodingType == EncodingType.UTF16) {
                str = Encoding.Unicode.GetString(data);
            }
            return str;
        }
    }

    //public class SerialData
    //{
    //    public string[] scannedDevice = new string[0];

    //    public string       packet = "";
    //    public double[]     data = new double[0];
    //    public string       port = "";
    //    public string       log  = "";

    //    public SerialData()                        { }
    //    public SerialData(double[] data)           { this.data =   data; }
    //    public SerialData(string packet)           { this.packet = packet; }
    //    public SerialData(string port, string log) { this.port = port; this.log  = log; }
    //    public SerialData(string[] scannedDevice)  { this.scannedDevice = scannedDevice; }
    //}

    public class SerialData
    {
        public string   packet = "";
        public double[] data   = new double[0];

        public SerialData(string packet) { this.packet = packet; }
        public SerialData(double[] data) { this.data = data; }
    }

    public class SerialLog
    {
        public string log;
        public string[] devices;

        public SerialLog(string log) => this.log = log;
        public SerialLog(string[] devices) => this.devices = devices;
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
}
