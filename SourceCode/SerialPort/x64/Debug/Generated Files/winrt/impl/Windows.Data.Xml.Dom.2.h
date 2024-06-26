// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.230706.1

#pragma once
#ifndef WINRT_Windows_Data_Xml_Dom_2_H
#define WINRT_Windows_Data_Xml_Dom_2_H
#include "winrt/impl/Windows.Foundation.1.h"
#include "winrt/impl/Windows.Foundation.Collections.1.h"
#include "winrt/impl/Windows.Storage.1.h"
#include "winrt/impl/Windows.Data.Xml.Dom.1.h"
WINRT_EXPORT namespace winrt::Windows::Data::Xml::Dom
{
    struct WINRT_IMPL_EMPTY_BASES DtdEntity : winrt::Windows::Data::Xml::Dom::IDtdEntity
    {
        DtdEntity(std::nullptr_t) noexcept {}
        DtdEntity(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IDtdEntity(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES DtdNotation : winrt::Windows::Data::Xml::Dom::IDtdNotation
    {
        DtdNotation(std::nullptr_t) noexcept {}
        DtdNotation(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IDtdNotation(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlAttribute : winrt::Windows::Data::Xml::Dom::IXmlAttribute
    {
        XmlAttribute(std::nullptr_t) noexcept {}
        XmlAttribute(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlAttribute(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlCDataSection : winrt::Windows::Data::Xml::Dom::IXmlCDataSection
    {
        XmlCDataSection(std::nullptr_t) noexcept {}
        XmlCDataSection(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlCDataSection(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlComment : winrt::Windows::Data::Xml::Dom::IXmlComment
    {
        XmlComment(std::nullptr_t) noexcept {}
        XmlComment(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlComment(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlDocument : winrt::Windows::Data::Xml::Dom::IXmlDocument,
        impl::require<XmlDocument, winrt::Windows::Data::Xml::Dom::IXmlDocumentIO, winrt::Windows::Data::Xml::Dom::IXmlDocumentIO2>
    {
        XmlDocument(std::nullptr_t) noexcept {}
        XmlDocument(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlDocument(ptr, take_ownership_from_abi) {}
        XmlDocument();
        static auto LoadFromUriAsync(winrt::Windows::Foundation::Uri const& uri);
        static auto LoadFromUriAsync(winrt::Windows::Foundation::Uri const& uri, winrt::Windows::Data::Xml::Dom::XmlLoadSettings const& loadSettings);
        static auto LoadFromFileAsync(winrt::Windows::Storage::IStorageFile const& file);
        static auto LoadFromFileAsync(winrt::Windows::Storage::IStorageFile const& file, winrt::Windows::Data::Xml::Dom::XmlLoadSettings const& loadSettings);
    };
    struct WINRT_IMPL_EMPTY_BASES XmlDocumentFragment : winrt::Windows::Data::Xml::Dom::IXmlDocumentFragment
    {
        XmlDocumentFragment(std::nullptr_t) noexcept {}
        XmlDocumentFragment(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlDocumentFragment(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlDocumentType : winrt::Windows::Data::Xml::Dom::IXmlDocumentType
    {
        XmlDocumentType(std::nullptr_t) noexcept {}
        XmlDocumentType(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlDocumentType(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlDomImplementation : winrt::Windows::Data::Xml::Dom::IXmlDomImplementation
    {
        XmlDomImplementation(std::nullptr_t) noexcept {}
        XmlDomImplementation(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlDomImplementation(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlElement : winrt::Windows::Data::Xml::Dom::IXmlElement
    {
        XmlElement(std::nullptr_t) noexcept {}
        XmlElement(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlElement(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlEntityReference : winrt::Windows::Data::Xml::Dom::IXmlEntityReference
    {
        XmlEntityReference(std::nullptr_t) noexcept {}
        XmlEntityReference(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlEntityReference(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlLoadSettings : winrt::Windows::Data::Xml::Dom::IXmlLoadSettings
    {
        XmlLoadSettings(std::nullptr_t) noexcept {}
        XmlLoadSettings(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlLoadSettings(ptr, take_ownership_from_abi) {}
        XmlLoadSettings();
    };
    struct WINRT_IMPL_EMPTY_BASES XmlNamedNodeMap : winrt::Windows::Data::Xml::Dom::IXmlNamedNodeMap
    {
        XmlNamedNodeMap(std::nullptr_t) noexcept {}
        XmlNamedNodeMap(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlNamedNodeMap(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlNodeList : winrt::Windows::Data::Xml::Dom::IXmlNodeList
    {
        XmlNodeList(std::nullptr_t) noexcept {}
        XmlNodeList(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlNodeList(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlProcessingInstruction : winrt::Windows::Data::Xml::Dom::IXmlProcessingInstruction
    {
        XmlProcessingInstruction(std::nullptr_t) noexcept {}
        XmlProcessingInstruction(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlProcessingInstruction(ptr, take_ownership_from_abi) {}
    };
    struct WINRT_IMPL_EMPTY_BASES XmlText : winrt::Windows::Data::Xml::Dom::IXmlText
    {
        XmlText(std::nullptr_t) noexcept {}
        XmlText(void* ptr, take_ownership_from_abi_t) noexcept : winrt::Windows::Data::Xml::Dom::IXmlText(ptr, take_ownership_from_abi) {}
    };
}
#endif