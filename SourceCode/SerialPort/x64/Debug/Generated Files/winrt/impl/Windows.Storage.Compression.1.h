// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

#pragma once
#ifndef WINRT_Windows_Storage_Compression_1_H
#define WINRT_Windows_Storage_Compression_1_H
#include "winrt/impl/Windows.Foundation.0.h"
#include "winrt/impl/Windows.Storage.Streams.0.h"
#include "winrt/impl/Windows.Storage.Compression.0.h"
WINRT_EXPORT namespace winrt::Windows::Storage::Compression
{
    struct WINRT_IMPL_EMPTY_BASES ICompressor :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<ICompressor>,
        impl::require<winrt::Windows::Storage::Compression::ICompressor, winrt::Windows::Foundation::IClosable, winrt::Windows::Storage::Streams::IOutputStream>
    {
        ICompressor(std::nullptr_t = nullptr) noexcept {}
        ICompressor(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES ICompressorFactory :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<ICompressorFactory>
    {
        ICompressorFactory(std::nullptr_t = nullptr) noexcept {}
        ICompressorFactory(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES IDecompressor :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IDecompressor>,
        impl::require<winrt::Windows::Storage::Compression::IDecompressor, winrt::Windows::Foundation::IClosable, winrt::Windows::Storage::Streams::IInputStream>
    {
        IDecompressor(std::nullptr_t = nullptr) noexcept {}
        IDecompressor(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES IDecompressorFactory :
        winrt::Windows::Foundation::IInspectable,
        impl::consume_t<IDecompressorFactory>
    {
        IDecompressorFactory(std::nullptr_t = nullptr) noexcept {}
        IDecompressorFactory(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
}
#endif
