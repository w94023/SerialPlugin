// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

#pragma once
#ifndef WINRT_Windows_Perception_Spatial_2_H
#define WINRT_Windows_Perception_Spatial_2_H
#include "winrt/impl/Windows.Foundation.2.h"
#include "winrt/impl/Windows.Foundation.Collections.2.h"
#include "winrt/impl/Windows.Foundation.Numerics.2.h"
#include "winrt/impl/Windows.Storage.Streams.2.h"
#include "winrt/impl/Windows.System.RemoteSystems.2.h"
#include "winrt/impl/Windows.Perception.Spatial.1.h"
WINRT_EXPORT namespace winrt::Windows::Perception::Spatial
{
    struct SpatialBoundingBox
    {
        winrt::Windows::Foundation::Numerics::float3 Center;
        winrt::Windows::Foundation::Numerics::float3 Extents;
    };
    inline bool operator==(SpatialBoundingBox const& left, SpatialBoundingBox const& right) noexcept
    {
        return left.Center == right.Center && left.Extents == right.Extents;
    }
    inline bool operator!=(SpatialBoundingBox const& left, SpatialBoundingBox const& right) noexcept
    {
        return !(left == right);
    }
    struct SpatialBoundingFrustum
    {
        winrt::Windows::Foundation::Numerics::plane Near;
        winrt::Windows::Foundation::Numerics::plane Far;
        winrt::Windows::Foundation::Numerics::plane Right;
        winrt::Windows::Foundation::Numerics::plane Left;
        winrt::Windows::Foundation::Numerics::plane Top;
        winrt::Windows::Foundation::Numerics::plane Bottom;
    };
    inline bool operator==(SpatialBoundingFrustum const& left, SpatialBoundingFrustum const& right) noexcept
    {
        return left.Near == right.Near && left.Far == right.Far && left.Right == right.Right && left.Left == right.Left && left.Top == right.Top && left.Bottom == right.Bottom;
    }
    inline bool operator!=(SpatialBoundingFrustum const& left, SpatialBoundingFrustum const& right) noexcept
    {
        return !(left == right);
    }
    struct SpatialBoundingOrientedBox
    {
        winrt::Windows::Foundation::Numerics::float3 Center;
        winrt::Windows::Foundation::Numerics::float3 Extents;
        winrt::Windows::Foundation::Numerics::quaternion Orientation;
    };
    inline bool operator==(SpatialBoundingOrientedBox const& left, SpatialBoundingOrientedBox const& right) noexcept
    {
        return left.Center == right.Center && left.Extents == right.Extents && left.Orientation == right.Orientation;
    }
    inline bool operator!=(SpatialBoundingOrientedBox const& left, SpatialBoundingOrientedBox const& right) noexcept
    {
        return !(left == right);
    }
    struct SpatialBoundingSphere
    {
        winrt::Windows::Foundation::Numerics::float3 Center;
        float Radius;
    };
    inline bool operator==(SpatialBoundingSphere const& left, SpatialBoundingSphere const& right) noexcept
    {
        return left.Center == right.Center && left.Radius == right.Radius;
    }
    inline bool operator!=(SpatialBoundingSphere const& left, SpatialBoundingSphere const& right) noexcept
    {
        return !(left == right);
    }
    struct SpatialRay
    {
        winrt::Windows::Foundation::Numerics::float3 Origin;
        winrt::Windows::Foundation::Numerics::float3 Direction;
    };
    inline bool operator==(SpatialRay const& left, SpatialRay const& right) noexcept
    {
        return left.Origin == right.Origin && left.Direction == right.Direction;
    }
    inline bool operator!=(SpatialRay const& left, SpatialRay const& right) noexcept
    {
        return !(left == right);
    }
    struct WINRT_IMPL_EMPTY_BASES SpatialAnchor : winrt::Windows::Perception::Spatial::ISpatialAnchor,
        impl::require<SpatialAnchor, winrt::Windows::Perception::Spatial::ISpatialAnchor2>
    {
        SpatialAnchor(std::nullptr_t) noexcept {}
        SpatialAnchor(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialAnchor(ptr, take_ownership_from_abi) {}
        static auto TryCreateRelativeTo(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem);
        static auto TryCreateRelativeTo(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem, winrt::Windows::Foundation::Numerics::float3 const& position);
        static auto TryCreateRelativeTo(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem, winrt::Windows::Foundation::Numerics::float3 const& position, winrt::Windows::Foundation::Numerics::quaternion const& orientation);
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialAnchorExportSufficiency : winrt::Windows::Perception::Spatial::ISpatialAnchorExportSufficiency
    {
        SpatialAnchorExportSufficiency(std::nullptr_t) noexcept {}
        SpatialAnchorExportSufficiency(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialAnchorExportSufficiency(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialAnchorExporter : winrt::Windows::Perception::Spatial::ISpatialAnchorExporter
    {
        SpatialAnchorExporter(std::nullptr_t) noexcept {}
        SpatialAnchorExporter(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialAnchorExporter(ptr, take_ownership_from_abi) {}
        static auto GetDefault();
        static auto RequestAccessAsync();
    };
    struct SpatialAnchorManager
    {
        SpatialAnchorManager() = delete;
        static auto RequestStoreAsync();
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialAnchorRawCoordinateSystemAdjustedEventArgs : winrt::Windows::Perception::Spatial::ISpatialAnchorRawCoordinateSystemAdjustedEventArgs
    {
        SpatialAnchorRawCoordinateSystemAdjustedEventArgs(std::nullptr_t) noexcept {}
        SpatialAnchorRawCoordinateSystemAdjustedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialAnchorRawCoordinateSystemAdjustedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialAnchorStore : winrt::Windows::Perception::Spatial::ISpatialAnchorStore
    {
        SpatialAnchorStore(std::nullptr_t) noexcept {}
        SpatialAnchorStore(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialAnchorStore(ptr, take_ownership_from_abi) {}
    };
    struct SpatialAnchorTransferManager
    {
        SpatialAnchorTransferManager() = delete;
        static auto TryImportAnchorsAsync(winrt::Windows::Storage::Streams::IInputStream const& stream);
        static auto TryExportAnchorsAsync(param::async_iterable<winrt::Windows::Foundation::Collections::IKeyValuePair<hstring, winrt::Windows::Perception::Spatial::SpatialAnchor>> const& anchors, winrt::Windows::Storage::Streams::IOutputStream const& stream);
        static auto RequestAccessAsync();
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialBoundingVolume : winrt::Windows::Perception::Spatial::ISpatialBoundingVolume
    {
        SpatialBoundingVolume(std::nullptr_t) noexcept {}
        SpatialBoundingVolume(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialBoundingVolume(ptr, take_ownership_from_abi) {}
        static auto FromBox(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem, winrt::Windows::Perception::Spatial::SpatialBoundingBox const& box);
        static auto FromOrientedBox(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem, winrt::Windows::Perception::Spatial::SpatialBoundingOrientedBox const& box);
        static auto FromSphere(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem, winrt::Windows::Perception::Spatial::SpatialBoundingSphere const& sphere);
        static auto FromFrustum(winrt::Windows::Perception::Spatial::SpatialCoordinateSystem const& coordinateSystem, winrt::Windows::Perception::Spatial::SpatialBoundingFrustum const& frustum);
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialCoordinateSystem : winrt::Windows::Perception::Spatial::ISpatialCoordinateSystem
    {
        SpatialCoordinateSystem(std::nullptr_t) noexcept {}
        SpatialCoordinateSystem(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialCoordinateSystem(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialEntity : winrt::Windows::Perception::Spatial::ISpatialEntity
    {
        SpatialEntity(std::nullptr_t) noexcept {}
        SpatialEntity(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialEntity(ptr, take_ownership_from_abi) {}
        explicit SpatialEntity(winrt::Windows::Perception::Spatial::SpatialAnchor const& spatialAnchor);
        SpatialEntity(winrt::Windows::Perception::Spatial::SpatialAnchor const& spatialAnchor, winrt::Windows::Foundation::Collections::ValueSet const& propertySet);
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialEntityAddedEventArgs : winrt::Windows::Perception::Spatial::ISpatialEntityAddedEventArgs
    {
        SpatialEntityAddedEventArgs(std::nullptr_t) noexcept {}
        SpatialEntityAddedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialEntityAddedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialEntityRemovedEventArgs : winrt::Windows::Perception::Spatial::ISpatialEntityRemovedEventArgs
    {
        SpatialEntityRemovedEventArgs(std::nullptr_t) noexcept {}
        SpatialEntityRemovedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialEntityRemovedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialEntityStore : winrt::Windows::Perception::Spatial::ISpatialEntityStore
    {
        SpatialEntityStore(std::nullptr_t) noexcept {}
        SpatialEntityStore(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialEntityStore(ptr, take_ownership_from_abi) {}
        [[nodiscard]] static auto IsSupported();
        static auto TryGet(winrt::Windows::System::RemoteSystems::RemoteSystemSession const& session);
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialEntityUpdatedEventArgs : winrt::Windows::Perception::Spatial::ISpatialEntityUpdatedEventArgs
    {
        SpatialEntityUpdatedEventArgs(std::nullptr_t) noexcept {}
        SpatialEntityUpdatedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialEntityUpdatedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialEntityWatcher : winrt::Windows::Perception::Spatial::ISpatialEntityWatcher
    {
        SpatialEntityWatcher(std::nullptr_t) noexcept {}
        SpatialEntityWatcher(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialEntityWatcher(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialLocation : winrt::Windows::Perception::Spatial::ISpatialLocation,
        impl::require<SpatialLocation, winrt::Windows::Perception::Spatial::ISpatialLocation2>
    {
        SpatialLocation(std::nullptr_t) noexcept {}
        SpatialLocation(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialLocation(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialLocator : winrt::Windows::Perception::Spatial::ISpatialLocator
    {
        SpatialLocator(std::nullptr_t) noexcept {}
        SpatialLocator(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialLocator(ptr, take_ownership_from_abi) {}
        static auto GetDefault();
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialLocatorAttachedFrameOfReference : winrt::Windows::Perception::Spatial::ISpatialLocatorAttachedFrameOfReference
    {
        SpatialLocatorAttachedFrameOfReference(std::nullptr_t) noexcept {}
        SpatialLocatorAttachedFrameOfReference(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialLocatorAttachedFrameOfReference(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialLocatorPositionalTrackingDeactivatingEventArgs : winrt::Windows::Perception::Spatial::ISpatialLocatorPositionalTrackingDeactivatingEventArgs
    {
        SpatialLocatorPositionalTrackingDeactivatingEventArgs(std::nullptr_t) noexcept {}
        SpatialLocatorPositionalTrackingDeactivatingEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialLocatorPositionalTrackingDeactivatingEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialStageFrameOfReference : winrt::Windows::Perception::Spatial::ISpatialStageFrameOfReference
    {
        SpatialStageFrameOfReference(std::nullptr_t) noexcept {}
        SpatialStageFrameOfReference(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialStageFrameOfReference(ptr, take_ownership_from_abi) {}
        [[nodiscard]] static auto Current();
        static auto CurrentChanged(winrt::Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable> const& handler);
        using CurrentChanged_revoker = impl::factory_event_revoker<winrt::Windows::Perception::Spatial::ISpatialStageFrameOfReferenceStatics, &impl::abi_t<winrt::Windows::Perception::Spatial::ISpatialStageFrameOfReferenceStatics>::remove_CurrentChanged>;
        [[nodiscard]] static auto CurrentChanged(auto_revoke_t, winrt::Windows::Foundation::EventHandler<winrt::Windows::Foundation::IInspectable> const& handler);
        static auto CurrentChanged(winrt::event_token const& cookie);
        static auto RequestNewStageAsync();
    };
    struct WINRT_IMPL_EMPTY_BASES SpatialStationaryFrameOfReference : winrt::Windows::Perception::Spatial::ISpatialStationaryFrameOfReference
    {
        SpatialStationaryFrameOfReference(std::nullptr_t) noexcept {}
        SpatialStationaryFrameOfReference(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Perception::Spatial::ISpatialStationaryFrameOfReference(ptr, take_ownership_from_abi) {}
    };
}
#endif
