#pragma once

#include <juce_core/juce_core.h>

/** Local Universal Device Inquiry constants for this tool.
    Independent of Matrix-Control — update manually if protocol bytes change. */
namespace DeviceInquiry
{
    constexpr juce::uint8 kSysExStart = 0xF0;
    constexpr juce::uint8 kSysExEnd = 0xF7;

    constexpr juce::uint8 kUniversalNonRealtimeId = 0x7E;
    constexpr juce::uint8 kDeviceIdAll = 0x7F;
    constexpr juce::uint8 kSubIdGeneralInfo = 0x06;
    constexpr juce::uint8 kSubIdDeviceIdRequest = 0x01;
    constexpr juce::uint8 kSubIdDeviceIdReply = 0x02;

    constexpr juce::uint8 kRequestMessage[] = { 0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7 };
    constexpr size_t kRequestMessageLength = 6;

    constexpr juce::uint8 kExpectedManufacturer = 0x10;
    constexpr juce::uint8 kExpectedFamily = 0x06;
    constexpr juce::uint8 kExpectedFamilyHigh = 0x00;
    constexpr juce::uint8 kExpectedMemberLow = 0x02;
    constexpr juce::uint8 kExpectedMemberHigh = 0x00;
    constexpr juce::uint8 kMatrix6MemberLow = 0x01;
    constexpr juce::uint8 kMatrix6MemberHigh = 0x00;

    inline bool isDeviceInquiryRequest(const juce::MemoryBlock& sysEx) noexcept
    {
        if (sysEx.getSize() != kRequestMessageLength)
            return false;

        const auto* data = static_cast<const juce::uint8*>(sysEx.getData());
        for (size_t i = 0; i < kRequestMessageLength; ++i)
        {
            if (data[i] != kRequestMessage[i])
                return false;
        }
        return true;
    }

    /** F0 7E <chan> 06 02 10 06 00 <memb-lo> <memb-hi> <rev0..3> F7 */
    inline juce::MemoryBlock encodeReply(juce::uint8 memberLow,
                                         juce::uint8 memberHigh,
                                         juce::StringRef firmwareVersion = "1.11",
                                         juce::uint8 channel = 0x00)
    {
        juce::uint8 versionBytes[4] = { ' ', ' ', ' ', ' ' };
        auto version = juce::String(firmwareVersion);
        if (version.isEmpty())
            version = "1.11";

        const int copyLen = juce::jmin(4, version.length());
        for (int i = 0; i < copyLen; ++i)
            versionBytes[static_cast<size_t>(i)] = static_cast<juce::uint8>(version[i] & 0x7f);

        const juce::uint8 reply[] = {
            kSysExStart,
            kUniversalNonRealtimeId,
            channel,
            kSubIdGeneralInfo,
            kSubIdDeviceIdReply,
            kExpectedManufacturer,
            kExpectedFamily,
            kExpectedFamilyHigh,
            memberLow,
            memberHigh,
            versionBytes[0],
            versionBytes[1],
            versionBytes[2],
            versionBytes[3],
            kSysExEnd
        };

        return juce::MemoryBlock(reply, sizeof(reply));
    }
}
