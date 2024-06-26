// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

#pragma once
#ifndef WINRT_Windows_Networking_PushNotifications_2_H
#define WINRT_Windows_Networking_PushNotifications_2_H
#include "winrt/impl/Windows.Foundation.1.h"
#include "winrt/impl/Windows.System.1.h"
#include "winrt/impl/Windows.Networking.PushNotifications.1.h"
WINRT_EXPORT namespace winrt::Windows::Networking::PushNotifications
{
    struct WINRT_IMPL_EMPTY_BASES PushNotificationChannel : winrt::Windows::Networking::PushNotifications::IPushNotificationChannel
    {
        PushNotificationChannel(std::nullptr_t) noexcept {}
        PushNotificationChannel(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Networking::PushNotifications::IPushNotificationChannel(ptr, take_ownership_from_abi) {}
    };
    struct PushNotificationChannelManager
    {
        PushNotificationChannelManager() = delete;
        static auto CreatePushNotificationChannelForApplicationAsync();
        static auto CreatePushNotificationChannelForApplicationAsync(param::hstring const& applicationId);
        static auto CreatePushNotificationChannelForSecondaryTileAsync(param::hstring const& tileId);
        static auto GetForUser(winrt::Windows::System::User const& user);
        static auto GetDefault();
        static auto ChannelsRevoked(winrt::Windows::Foundation::EventHandler<winrt::Windows::Networking::PushNotifications::PushNotificationChannelsRevokedEventArgs> const& handler);
        using ChannelsRevoked_revoker = impl::factory_event_revoker<winrt::Windows::Networking::PushNotifications::IPushNotificationChannelManagerStatics4, &impl::abi_t<winrt::Windows::Networking::PushNotifications::IPushNotificationChannelManagerStatics4>::remove_ChannelsRevoked>;
        [[nodiscard]] static auto ChannelsRevoked(auto_revoke_t, winrt::Windows::Foundation::EventHandler<winrt::Windows::Networking::PushNotifications::PushNotificationChannelsRevokedEventArgs> const& handler);
        static auto ChannelsRevoked(winrt::event_token const& token);
    };
    struct WINRT_IMPL_EMPTY_BASES PushNotificationChannelManagerForUser : winrt::Windows::Networking::PushNotifications::IPushNotificationChannelManagerForUser,
        impl::require<PushNotificationChannelManagerForUser, winrt::Windows::Networking::PushNotifications::IPushNotificationChannelManagerForUser2>
    {
        PushNotificationChannelManagerForUser(std::nullptr_t) noexcept {}
        PushNotificationChannelManagerForUser(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Networking::PushNotifications::IPushNotificationChannelManagerForUser(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES PushNotificationChannelsRevokedEventArgs : winrt::Windows::Networking::PushNotifications::IPushNotificationChannelsRevokedEventArgs
    {
        PushNotificationChannelsRevokedEventArgs(std::nullptr_t) noexcept {}
        PushNotificationChannelsRevokedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Networking::PushNotifications::IPushNotificationChannelsRevokedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES PushNotificationReceivedEventArgs : winrt::Windows::Networking::PushNotifications::IPushNotificationReceivedEventArgs
    {
        PushNotificationReceivedEventArgs(std::nullptr_t) noexcept {}
        PushNotificationReceivedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Networking::PushNotifications::IPushNotificationReceivedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES RawNotification : winrt::Windows::Networking::PushNotifications::IRawNotification,
        impl::require<RawNotification, winrt::Windows::Networking::PushNotifications::IRawNotification2, winrt::Windows::Networking::PushNotifications::IRawNotification3>
    {
        RawNotification(std::nullptr_t) noexcept {}
        RawNotification(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Networking::PushNotifications::IRawNotification(ptr, take_ownership_from_abi) {}
    };
}
#endif
