// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

#pragma once
#ifndef WINRT_Windows_Devices_Custom_1_H
#define WINRT_Windows_Devices_Custom_1_H
#include "winrt/impl/Windows.Devices.Custom.0.h"
WINRT_EXPORT namespace winrt::Windows::Devices::Custom
{
    struct WINRT_IMPL_EMPTY_BASES ICustomDevice :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<ICustomDevice>
    {
        ICustomDevice(std::nullptr_t = nullptr) noexcept {}
        ICustomDevice(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES ICustomDeviceStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<ICustomDeviceStatics>
    {
        ICustomDeviceStatics(std::nullptr_t = nullptr) noexcept {}
        ICustomDeviceStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES IIOControlCode :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IIOControlCode>
    {
        IIOControlCode(std::nullptr_t = nullptr) noexcept {}
        IIOControlCode(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES IIOControlCodeFactory :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IIOControlCodeFactory>
    {
        IIOControlCodeFactory(std::nullptr_t = nullptr) noexcept {}
        IIOControlCodeFactory(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES IKnownDeviceTypesStatics :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IKnownDeviceTypesStatics>
    {
        IKnownDeviceTypesStatics(std::nullptr_t = nullptr) noexcept {}
        IKnownDeviceTypesStatics(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
}
#endif
