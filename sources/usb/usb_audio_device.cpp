#include "usb_audio_device.hpp"

#include "constants.hpp"

#include "debug_utilities.hpp"

UsbAudioDevice &UsbAudioDevice::getInstance(){
    static UsbAudioDevice instance{};
    return instance;
}

void UsbAudioDevice::initialize(){
    tusb_init();
}

void UsbAudioDevice::update(){
    tud_task();

#ifdef DEBUG
    DEBUG_reportEvents();
#endif
}

// TODO
bool UsbAudioDevice::isMuted() const{ return false;}

// TODO
uint16_t UsbAudioDevice::availableFrames() const{ return 0;}

// TODO
uint16_t UsbAudioDevice::fifoDepthFrames() const{ return 0;}

// TODO
uint16_t UsbAudioDevice::read(int16_t *samples, uint16_t maximumFrames){ return 0;}

// TODO
void UsbAudioDevice::discard(uint16_t frameCount){}

// TODO
bool UsbAudioDevice::checkOverflow(){ return false;}

void UsbAudioDevice::handleMount(){ isMounted_ = true;}

void UsbAudioDevice::handleUnmount(){
    isMounted_    = false;
    isStreamOpen_ = false;
}

// TODO
void UsbAudioDevice::handleStreamInterface(uint8_t alternate){}

// TODO
void UsbAudioDevice::handleFeedbackParameters(audio_feedback_params_t *parameters){}

// TODO
bool UsbAudioDevice::handleGetRequest(uint8_t rootHubPort, const tusb_control_request_t *request){ return false;}

// TODO
bool UsbAudioDevice::handleSetRequest(uint8_t rootHubPort, const tusb_control_request_t *request, const uint8_t *buffer){ return false;}

void UsbAudioDevice::DEBUG_reportEvents(){
    if(isMounted_ != wasMounted_){
        wasMounted_ = isMounted_;
        DEBUG_PRINT("USB: %s", isMounted_ ? "mounted, speaker available to the host" : "unmounted");
    }

    // TODO: host volume report
}
