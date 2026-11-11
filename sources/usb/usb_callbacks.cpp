#include "usb/usb_audio_device.hpp"
#include "usb/usb_descriptors.hpp"

#include <tusb.h>

// all of them run inside tud_task() (so never print from here)

extern "C"{

void tud_mount_cb(void){ UsbAudioDevice::getInstance().handleMount();}
void tud_umount_cb(void){ UsbAudioDevice::getInstance().handleUnmount();}

bool tud_audio_set_itf_cb(uint8_t rootHubPort, tusb_control_request_t const *request){
    (void)rootHubPort;
    const uint8_t interface{tu_u16_low(tu_le16toh(request->wIndex))};
    const uint8_t alternate{tu_u16_low(tu_le16toh(request->wValue))};

    if(interface == usb_descriptors::Interface::Audio_Streaming){
        UsbAudioDevice::getInstance().handleStreamInterface(alternate);
    }

    return true;
}

bool tud_audio_get_req_entity_cb(uint8_t rootHubPort, tusb_control_request_t const *request){
    return UsbAudioDevice::getInstance().handleGetRequest(rootHubPort, request);
}

bool tud_audio_set_req_entity_cb(uint8_t rootHubPort, tusb_control_request_t const *request, uint8_t *buffer){
    return UsbAudioDevice::getInstance().handleSetRequest(rootHubPort, request, buffer);
}

void tud_audio_feedback_params_cb(uint8_t functionId, uint8_t alternate, audio_feedback_params_t *parameters){
    (void)functionId;
    (void)alternate;

    UsbAudioDevice::getInstance().handleFeedbackParameters(parameters);
}

} // extern "C"
