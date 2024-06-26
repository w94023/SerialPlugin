// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

#pragma once
#ifndef WINRT_Windows_Devices_SerialCommunication_2_H
#define WINRT_Windows_Devices_SerialCommunication_2_H
#include "winrt/impl/Windows.Foundation.1.h"
#include "winrt/impl/Windows.Devices.SerialCommunication.1.h"
WINRT_EXPORT namespace winrt::Windows::Devices::SerialCommunication
{
    struct WINRT_IMPL_EMPTY_BASES ErrorReceivedEventArgs : winrt::Windows::Devices::SerialCommunication::IErrorReceivedEventArgs
    {
        ErrorReceivedEventArgs(std::nullptr_t) noexcept {}
        ErrorReceivedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Devices::SerialCommunication::IErrorReceivedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES PinChangedEventArgs : winrt::Windows::Devices::SerialCommunication::IPinChangedEventArgs
    {
        PinChangedEventArgs(std::nullptr_t) noexcept {}
        PinChangedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Devices::SerialCommunication::IPinChangedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SerialDevice : winrt::Windows::Devices::SerialCommunication::ISerialDevice
    {
        SerialDevice(std::nullptr_t) noexcept {}
        SerialDevice(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Devices::SerialCommunication::ISerialDevice(ptr, take_ownership_from_abi) {}
        static auto GetDeviceSelector();
        static auto GetDeviceSelector(param::hstring const& portName);
        static auto GetDeviceSelectorFromUsbVidPid(uint16_t vendorId, uint16_t productId);
        static auto FromIdAsync(param::hstring const& deviceId);
    };
}
#endif
