#pragma once

#include <cstdint>

namespace usb_descriptors{

    enum Interface : uint8_t{
        Audio_Control,
        Audio_Streaming,
        Cdc_Control,
        Cdc_Data,
        COUNT
    };

    namespace entity{
        constexpr uint8_t InputTerminal {0x01};
        constexpr uint8_t FeatureUnit   {0x02};
        constexpr uint8_t OutputTerminal{0x03};
        constexpr uint8_t Clock         {0x04};
    } // namespace entity

    namespace endpoint{
        constexpr uint8_t AudioOut          {0x01};
        constexpr uint8_t AudioFeedback     {0x81};
        constexpr uint8_t CdcNotification   {0x82};
        constexpr uint8_t CdcOut            {0x03};
        constexpr uint8_t CdcIn             {0x83};
    } // namespace endpoint

} // namespace usb_descriptors
