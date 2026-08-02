#pragma once

#include <juce_core/juce_core.h>

/** Local Universal Device Inquiry constants for this tool.
    Update manually if Matrix Device Inquiry protocol bytes change. */
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
    /** Valid Oberheim/family reply that is neither Matrix-1000 nor Matrix-6/6R. */
    constexpr juce::uint8 kUnknownMemberLow = 0x00;
    constexpr juce::uint8 kUnknownMemberHigh = 0x00;

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

    /** Digits only, max 4. May be empty (no default). */
    inline juce::String extractFirmwareDigits(juce::StringRef raw)
    {
        juce::String digits;
        const auto text = juce::String(raw);
        for (int i = 0; i < text.length() && digits.length() < 4; ++i)
        {
            const auto c = text[i];
            if (c >= '0' && c <= '9')
                digits << c;
        }
        return digits;
    }

    /** Digits only, max 4. Empty / invalid → "111" (Oberheim 1.11 without decimal). */
    inline juce::String normalizeFirmwareDigits(juce::StringRef raw)
    {
        const auto digits = extractFirmwareDigits(raw);
        return digits.isEmpty() ? juce::String("111") : digits;
    }

    /** Pack up to 4 digits into right-justified ASCII (Oberheim Device Inquiry).
        Example: "111" → ' ','1','1','1' for human version 1.11. */
    inline void packFirmwareVersionBytes(juce::StringRef firmwareVersion, juce::uint8 out[4])
    {
        out[0] = out[1] = out[2] = out[3] = ' ';
        const auto digits = normalizeFirmwareDigits(firmwareVersion);
        const int start = 4 - digits.length();
        for (int i = 0; i < digits.length(); ++i)
            out[static_cast<size_t>(start + i)] = static_cast<juce::uint8>(digits[i] & 0x7f);
    }

    /** F0 7E <chan> 06 02 10 06 00 <memb-lo> <memb-hi> <rev0..3> F7 */
    inline juce::MemoryBlock encodeReply(juce::uint8 memberLow,
                                         juce::uint8 memberHigh,
                                         juce::StringRef firmwareVersion = "111",
                                         juce::uint8 channel = 0x00)
    {
        juce::uint8 versionBytes[4];
        packFirmwareVersionBytes(firmwareVersion, versionBytes);

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
