#include "usb_descriptors.hpp"

#include "constants.hpp"

#include <tusb.h>
#include <pico/unique_id.h>

#include <algorithm>
#include <cstring>

namespace usb_descriptors{

    constexpr uint16_t VendorId {0xCafe}; // tinyusb test vendor id (probably fine?)
    constexpr uint16_t ProductId{0x4D42};

    enum String : uint8_t{
        Language,
        Manufacturer,
        Product,
        Serial,
        Speaker,
        Console,
        STRING_COUNT
    };

    constexpr uint16_t ConfigurationLength{TUD_CONFIG_DESC_LEN + USB_AUDIO_SPEAKER_DESCRIPTOR_LENGTH + TUD_CDC_DESC_LEN};

    constexpr tusb_desc_device_t Device{
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = 0x0200,
        // interface association descriptors are used by both the audio and the cdc function
        .bDeviceClass       = TUSB_CLASS_MISC,
        .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol    = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor           = VendorId,
        .idProduct          = ProductId,
        .bcdDevice          = 0x0100,
        .iManufacturer      = String::Manufacturer,
        .iProduct           = String::Product,
        .iSerialNumber      = String::Serial,
        .bNumConfigurations = 1
    };

    constexpr uint16_t FeatureUnitControls{
        AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS
    };

    const uint8_t Configuration[]{
        TUD_CONFIG_DESCRIPTOR(1, Interface::COUNT, 0, ConfigurationLength, 0x00, 100),

        /* UAC2 speaker */
        TUD_AUDIO_DESC_IAD(Interface::Audio_Control, 2, 0),
        TUD_AUDIO_DESC_STD_AC(Interface::Audio_Control, 0, String::Speaker),
        TUD_AUDIO_DESC_CS_AC(
            0x0200,
            AUDIO_FUNC_DESKTOP_SPEAKER,
            TUD_AUDIO_DESC_CLK_SRC_LEN + TUD_AUDIO_DESC_INPUT_TERM_LEN + TUD_AUDIO_DESC_OUTPUT_TERM_LEN + TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN,
            AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS
        ),
        TUD_AUDIO_DESC_CLK_SRC(
            entity::Clock,
            AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK,
            AUDIO_CTRL_RW << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS,
            entity::InputTerminal,
            0
        ),
        TUD_AUDIO_DESC_INPUT_TERM(
            entity::InputTerminal,
            AUDIO_TERM_TYPE_USB_STREAMING,
            0,
            entity::Clock,
            USB_AUDIO_CHANNEL_COUNT,
            AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,
            0,
            0,
            0
        ),
        TUD_AUDIO_DESC_OUTPUT_TERM(
            entity::OutputTerminal,
            AUDIO_TERM_TYPE_OUT_DESKTOP_SPEAKER,
            entity::InputTerminal,
            entity::FeatureUnit,
            entity::Clock,
            0x0000,
            0
        ),
        TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(
            entity::FeatureUnit,
            entity::InputTerminal,
            0,
            FeatureUnitControls,
            FeatureUnitControls,
            0
        ),
        // alternate 0 - zero bandwidth, the host selects it when the stream is closed
        TUD_AUDIO_DESC_STD_AS_INT(Interface::Audio_Streaming, 0, 0, 0),
        // alternate 1 - streaming
        TUD_AUDIO_DESC_STD_AS_INT(Interface::Audio_Streaming, 1, 2, 0),
        TUD_AUDIO_DESC_CS_AS_INT(
            entity::InputTerminal,
            AUDIO_CTRL_NONE,
            AUDIO_FORMAT_TYPE_I,
            AUDIO_DATA_FORMAT_TYPE_I_PCM,
            USB_AUDIO_CHANNEL_COUNT,
            AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,
            0
        ),
        TUD_AUDIO_DESC_TYPE_I_FORMAT(USB_AUDIO_BYTES_PER_SAMPLE, USB_AUDIO_BITS_PER_SAMPLE),
        TUD_AUDIO_DESC_STD_AS_ISO_EP(
            endpoint::AudioOut,
            static_cast<uint8_t>(
                static_cast<uint8_t>(TUSB_XFER_ISOCHRONOUS)
                | static_cast<uint8_t>(TUSB_ISO_EP_ATT_ASYNCHRONOUS)
                | static_cast<uint8_t>(TUSB_ISO_EP_ATT_DATA)
            ),
            CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX,
            1
        ),
        TUD_AUDIO_DESC_CS_AS_ISO_EP(
            AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK,
            AUDIO_CTRL_NONE,
            AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC,
            0x0001
        ),
        TUD_AUDIO_DESC_STD_AS_ISO_FB_EP(endpoint::AudioFeedback, 4, 1),

        /* CDC serial (stdio) */
        TUD_CDC_DESCRIPTOR(
            Interface::Cdc_Control,
            String::Console,
            endpoint::CdcNotification,
            8,
            endpoint::CdcOut,
            endpoint::CdcIn,
            CFG_TUD_CDC_EP_BUFSIZE
        )
    };

    static_assert(sizeof(Configuration) == ConfigurationLength);

    const char *const Strings[]{
        nullptr,
        "Joshua Lam",
        "Fast-Switching LED Driver",
        nullptr, // replaced with the pico id
        "Photoacoustic Speaker",
        "Debug Console"
    };

    static_assert(sizeof(Strings) / sizeof(Strings[0]) == String::STRING_COUNT);

    inline uint16_t stringBuffer[33]{};

} // namespace usb_descriptors

extern "C"{

uint8_t const *tud_descriptor_device_cb(void){
    return reinterpret_cast<uint8_t const *>(&usb_descriptors::Device);
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index){
    (void)index;
    return usb_descriptors::Configuration;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t languageId){
    (void)languageId;

    using namespace usb_descriptors;
    constexpr size_t MaximumCharacters{sizeof(stringBuffer) / sizeof(stringBuffer[0]) - 1};

    size_t characterCount{0};

    if(index == String::Language){
        stringBuffer[1] = 0x0409; // english
        characterCount  = 1;
    }else{
        if(index >= String::STRING_COUNT) return nullptr;

        char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1]{};
        const char *text{Strings[index]};
        if(index == String::Serial){
            pico_get_unique_board_id_string(serial, sizeof(serial));
            text = serial;
        }

        characterCount = std::min(std::strlen(text), MaximumCharacters);
        for(size_t characterIndex{0}; characterIndex < characterCount; characterIndex++){
            stringBuffer[1 + characterIndex] = text[characterIndex];
        }
    }

    stringBuffer[0] = static_cast<uint16_t>((TUSB_DESC_STRING << 8) | (2 * characterCount + 2));
    return stringBuffer;
}

} // extern "C"
