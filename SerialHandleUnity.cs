using System;
using System.Threading;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using SerialManager;

namespace SerialManager
{
    public class SerialHandleUnity : MonoBehaviour
    {
        public enum DeviceType
        {
            USB,
            DAQ,
            BTClassic,
            BLE
        }

        private SerialHandle _manager;

        private bool _isAlive = false;

        [HideInInspector] public  DeviceType deviceType = DeviceType.USB;
        // USB option
        [HideInInspector] public string      portName    = "";
        [HideInInspector] public BaudRate    baudRate    = BaudRate.bps9600;
        [HideInInspector] public DataBit     dataBit     = DataBit.bit8;
        [HideInInspector] public Parity      parity      = Parity.None;
        [HideInInspector] public StopBit     stopBit     = StopBit.bit1;
        [HideInInspector] public FlowControl flowControl = FlowControl.None;
        // BT options
        [HideInInspector] public string deviceName = "";
        [HideInInspector] public string uuidService = "{0000FFE0-0000-1000-8000-00805F9B34FB}";
        [HideInInspector] public string uuidTx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
        [HideInInspector] public string uuidRx      = "{0000FFE1-0000-1000-8000-00805F9B34FB}";
        // DAQ options
        [HideInInspector] public int[] ports;

        [HideInInspector]                 public  bool         getPPSOnDataReceived = false;
                                          public  int          PPS { get { return _PPS; } }
                                          private int         _PPS = 0;
        [SerializeField, HideInInspector] private int        __PPS = 0;

                                          public  bool         isConnected { get { return _isConnected; } }
                                          private bool        _isConnected = false;
        [SerializeField, HideInInspector] private bool       __isConnected = false;

        [HideInInspector] public  int          logLevel             = 1;
        [HideInInspector] public  int          receiveByteSize      = 1;
        [HideInInspector] public  int          receiveBufferSize    = 10000;
        [HideInInspector] public  byte[]       stopByte             = new byte[0];
        [HideInInspector] public  int          packetLength         = 0;
        [HideInInspector] public  EncodingType encodingType         = EncodingType.ASCII;

        public bool   usePPSLimit    { set { _usePPSLimit    = value; if (_isConnected) _manager.usePPSLimit    = _usePPSLimit;    } get { return _usePPSLimit;    } } [SerializeField, HideInInspector] bool   _usePPSLimit    = false;
        public bool   useCPULimit    { set { _useCPULimit    = value; if (_isConnected) _manager.useCPULimit    = _useCPULimit;    } get { return _useCPULimit;    } } [SerializeField, HideInInspector] bool   _useCPULimit    = false;
        public bool   useMemoryLimit { set { _useMemoryLimit = value; if (_isConnected) _manager.useMemoryLimit = _useMemoryLimit; } get { return _useMemoryLimit; } } [SerializeField, HideInInspector] bool   _useMemoryLimit = false;
        public int    PPSLimit       { set { _PPSLimit       = value; if (_isConnected) _manager.PPSLimit       = _PPSLimit;       } get { return _PPSLimit;       } } [SerializeField, HideInInspector] int    _PPSLimit       = 300;
        public double CPULimit       { set { _CPULimit       = value; if (_isConnected) _manager.CPULimit       = _CPULimit;       } get { return _CPULimit;       } } [SerializeField, HideInInspector] double _CPULimit       = 95;
        public int    memoryLimit    { set { _memoryLimit    = value; if (_isConnected) _manager.memoryLimit    = _memoryLimit;    } get { return _memoryLimit;    } } [SerializeField, HideInInspector] int    _memoryLimit    = 200;

        [HideInInspector] public UnityEvent<SerialLog>  onScanEnded;
        [HideInInspector] public UnityEvent             onConnected;
        [HideInInspector] public UnityEvent             onConnectionFailed;
        [HideInInspector] public UnityEvent             onDisconnected;
        [HideInInspector] public UnityEvent<SerialData> onDataReceived;

        private SynchronizationContext _mainThreadContext;

        private void Awake()
        {
            _isAlive = true;
            _manager = new SerialHandle();
            _manager.onScanEnded        += OnScanEnded;
            _manager.onLogReceived      += OnLogReceived;
            _manager.onConnected        += OnConnected;
            _manager.onConnectionFailed += OnConnectionFailed;
            _manager.onDisconnected     += OnDisconnected;
            _manager.onDataReceived     += OnDataReceived;

            _mainThreadContext = SynchronizationContext.Current;
        }

        private void OnValidate()
        {
            if (!_isConnected) {
                __PPS         = 0;
                __isConnected = false;
            }
            else {
                _manager.usePPSLimit          = _usePPSLimit;
                _manager.useCPULimit          = _useCPULimit;
                _manager.useMemoryLimit       = _useMemoryLimit;
                _manager.PPSLimit             = _PPSLimit;
                _manager.CPULimit             = _CPULimit;
                _manager.memoryLimit          = _memoryLimit;

                __PPS = _PPS;
                __isConnected = true;
            }
        }

        public void ScanDevices()
        {
            _manager.ScanDevices();
        }

        public void Connect()
        {
            _manager.logLevel             = logLevel;
            _manager.getPPSOnDataReceived = getPPSOnDataReceived;
            _manager.receiveByteSize      = receiveByteSize;
            _manager.receiveBufferSize    = receiveBufferSize;
            _manager.encodingType         = encodingType;
            _manager.stopByte             = stopByte;
            _manager.packetLength         = packetLength;
            _manager.usePPSLimit          = _usePPSLimit;
            _manager.useCPULimit          = _useCPULimit;
            _manager.useMemoryLimit       = _useMemoryLimit;
            _manager.PPSLimit             = _PPSLimit;
            _manager.CPULimit             = _CPULimit;
            _manager.memoryLimit          = _memoryLimit;

            switch (deviceType) {
                case DeviceType.USB:       _manager.Connect(portName, baudRate, dataBit, parity, stopBit, flowControl); break;
                case DeviceType.DAQ:       _manager.Connect(deviceName, ports); break;
                case DeviceType.BTClassic: _manager.Connect(deviceName); break;
                case DeviceType.BLE:       
                    UUID uuid = new UUID();
                    uuid.service = uuidService;
                    uuid.tx      = uuidTx;
                    uuid.rx      = uuidRx;
                    _manager.Connect(deviceName, uuid); break;
            }
        }

        public void Disconnect()
        {
            _manager.Disconnect();
        }

        public void SendData(string data)
        {
            if (!_isAlive) return;
            _manager.SendData(data);
        }

        public void SendData(byte[] data)
        {
            if (!_isAlive) return;
            _manager.SendData(data);
        }

        public float[] CheckResources()
        {
            if(!_isAlive) return new float[] { -1f, -1f };
            else          return _manager.CheckResources();
        }

        private void OnLogReceived(SerialLog e)
        {
            if (!_isAlive) return;
            _mainThreadContext.Post(state => { 
                if (e.log.Contains("Error")) {
                    Debug.LogError(e.log);
                }
                else {
                    Debug.Log(e.log);
                }
            }, null);
        }

        private void OnScanEnded(SerialLog e)
        {
            if (!_isAlive) return;
            onScanEnded?.Invoke(e);
        }

        private void OnConnected()
        {
            if (!_isAlive) return;
            onConnected?.Invoke();
            _isConnected = true;
            __isConnected = _isConnected;
        }

        private void OnConnectionFailed()
        {
            if (!_isAlive) return;
            onConnectionFailed?.Invoke();
            _isConnected = false;
            __isConnected = _isConnected;
        }

        private void OnDisconnected()
        {
            if (!_isAlive) return;
            onDisconnected?.Invoke();
            _isConnected = false;
            __isConnected = _isConnected;
        }

        private void OnDataReceived(SerialData e)
        {
            if (!_isAlive) return;
            _PPS = _manager.PPS;
            onDataReceived?.Invoke(e);
        }

        private void OnApplicationQuit()
        {
            _isAlive = false;
            _manager.Disconnect();
            Destroy(gameObject);
            GC.Collect();
        }
    }
}
