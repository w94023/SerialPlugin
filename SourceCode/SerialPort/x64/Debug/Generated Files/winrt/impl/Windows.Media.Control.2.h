// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

#pragma once
#ifndef WINRT_Windows_Media_Control_2_H
#define WINRT_Windows_Media_Control_2_H
#include "winrt/impl/Windows.Media.Control.1.h"
WINRT_EXPORT namespace winrt::Windows::Media::Control
{
    struct WINRT_IMPL_EMPTY_BASES CurrentSessionChangedEventArgs : winrt::Windows::Media::Control::ICurrentSessionChangedEventArgs
    {
        CurrentSessionChangedEventArgs(std::nullptr_t) noexcept {}
        CurrentSessionChangedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::ICurrentSessionChangedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES GlobalSystemMediaTransportControlsSession : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSession
    {
        GlobalSystemMediaTransportControlsSession(std::nullptr_t) noexcept {}
        GlobalSystemMediaTransportControlsSession(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSession(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES GlobalSystemMediaTransportControlsSessionManager : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionManager
    {
        GlobalSystemMediaTransportControlsSessionManager(std::nullptr_t) noexcept {}
        GlobalSystemMediaTransportControlsSessionManager(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionManager(ptr, take_ownership_from_abi) {}
        static auto RequestAsync();
    };
    struct WINRT_IMPL_EMPTY_BASES GlobalSystemMediaTransportControlsSessionMediaProperties : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionMediaProperties
    {
        GlobalSystemMediaTransportControlsSessionMediaProperties(std::nullptr_t) noexcept {}
        GlobalSystemMediaTransportControlsSessionMediaProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionMediaProperties(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES GlobalSystemMediaTransportControlsSessionPlaybackControls : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionPlaybackControls
    {
        GlobalSystemMediaTransportControlsSessionPlaybackControls(std::nullptr_t) noexcept {}
        GlobalSystemMediaTransportControlsSessionPlaybackControls(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionPlaybackControls(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES GlobalSystemMediaTransportControlsSessionPlaybackInfo : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionPlaybackInfo
    {
        GlobalSystemMediaTransportControlsSessionPlaybackInfo(std::nullptr_t) noexcept {}
        GlobalSystemMediaTransportControlsSessionPlaybackInfo(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionPlaybackInfo(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES GlobalSystemMediaTransportControlsSessionTimelineProperties : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionTimelineProperties
    {
        GlobalSystemMediaTransportControlsSessionTimelineProperties(std::nullptr_t) noexcept {}
        GlobalSystemMediaTransportControlsSessionTimelineProperties(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionTimelineProperties(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES MediaPropertiesChangedEventArgs : winrt::Windows::Media::Control::IMediaPropertiesChangedEventArgs
    {
        MediaPropertiesChangedEventArgs(std::nullptr_t) noexcept {}
        MediaPropertiesChangedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IMediaPropertiesChangedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES PlaybackInfoChangedEventArgs : winrt::Windows::Media::Control::IPlaybackInfoChangedEventArgs
    {
        PlaybackInfoChangedEventArgs(std::nullptr_t) noexcept {}
        PlaybackInfoChangedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::IPlaybackInfoChangedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SessionsChangedEventArgs : winrt::Windows::Media::Control::ISessionsChangedEventArgs
    {
        SessionsChangedEventArgs(std::nullptr_t) noexcept {}
        SessionsChangedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::ISessionsChangedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES TimelinePropertiesChangedEventArgs : winrt::Windows::Media::Control::ITimelinePropertiesChangedEventArgs
    {
        TimelinePropertiesChangedEventArgs(std::nullptr_t) noexcept {}
        TimelinePropertiesChangedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Media::Control::ITimelinePropertiesChangedEventArgs(ptr, take_ownership_from_abi) {}
    };
}
#endif
