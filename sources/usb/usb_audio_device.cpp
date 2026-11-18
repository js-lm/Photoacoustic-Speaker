#include "usb_audio_device.hpp"

#include "constants.hpp"
#include "usb/usb_descriptors.hpp"

#include <pico/time.h>
#include <hardware/address_mapped.h>
#include <hardware/structs/usb.h>

#include <algorithm>
#include <cmath>

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

bool UsbAudioDevice::isMuted() const{
    return isMuted_[0] || (isMuted_[1] && isMuted_[2]);
}

uint16_t UsbAudioDevice::availableFrames() const{
    return tud_audio_available() / constants::audio::BytesPerFrame;
}

uint16_t UsbAudioDevice::fifoDepthFrames() const{
    return tu_fifo_depth(tud_audio_get_ep_out_ff()) / constants::audio::BytesPerFrame;
}

uint16_t UsbAudioDevice::read(int16_t *samples, uint16_t maximumFrames){
    const uint16_t frameCount{std::min(availableFrames(), maximumFrames)};
    if(frameCount == 0) return 0;

    return tud_audio_read(samples, frameCount * constants::audio::BytesPerFrame) / constants::audio::BytesPerFrame;
}

void UsbAudioDevice::discard(uint16_t frameCount){
    int16_t scratch[2 * constants::audio::ReadChunkFrames];

    while(frameCount > 0){
        const uint16_t frameRead{read(scratch, std::min(frameCount, constants::audio::ReadChunkFrames))};
        if(frameRead == 0) return;
        frameCount -= frameRead;
    }
}

bool UsbAudioDevice::checkOverflow(){
    tu_fifo_t *fifo{tud_audio_get_ep_out_ff()};
    if(!tu_fifo_overflowed(fifo)) return false;

    tu_fifo_correct_read_pointer(fifo);
    return true;
}

void UsbAudioDevice::handleMount(){ isMounted_ = true;}

void UsbAudioDevice::handleUnmount(){
    isMounted_    = false;
    isStreamOpen_ = false;
}

void UsbAudioDevice::handleStreamInterface(uint8_t alternate){ isStreamOpen_ = alternate != 0;}

void UsbAudioDevice::handleStreamEndpointsClosing(){
    // workaround for tinyusb 0.18 on the rp2040: iso endpoints are never closed on an alternate setting change,
    // so the receive (and feedback) buffer armed for the previous stream stays armed in hardware. arming it again
    // when the host reopens the stream panics with "ep 01 was already available", which linux does while probing.
    // abort both endpoints and clear their buffer control so the next activation starts from a clean slate
    constexpr uint8_t EndpointNumber{usb_descriptors::endpoint::AudioOut & 0x0F};
    static_assert(EndpointNumber == (usb_descriptors::endpoint::AudioFeedback & 0x0F), "data and feedback must share a number");

    constexpr uint32_t AbortMask{(USB_EP_ABORT_EP0_IN_BITS | USB_EP_ABORT_EP0_OUT_BITS) << (2 * EndpointNumber)};
    constexpr uint32_t BufferStatusMask{(USB_BUFF_STATUS_EP0_IN_BITS | USB_BUFF_STATUS_EP0_OUT_BITS) << (2 * EndpointNumber)};

    // only works from chip revision b2 on (rp2040-e2)
    const bool canAbort{rp2040_chip_version() >= 2};
    if(canAbort){
        hw_set_bits(&usb_hw->abort, AbortMask);
        while((usb_hw->abort_done & AbortMask) != AbortMask) tight_loop_contents();
    }

    usb_dpram->ep_buf_ctrl[EndpointNumber].in  = 0;
    usb_dpram->ep_buf_ctrl[EndpointNumber].out = 0;
    hw_clear_bits(&usb_hw->buf_status, BufferStatusMask);

    if(canAbort){
        hw_clear_bits(&usb_hw->abort_done, AbortMask);
        hw_clear_bits(&usb_hw->abort, AbortMask);
    }
}

void UsbAudioDevice::handleReceived(uint16_t byteCount){
    receivedFrames_ += byteCount / constants::audio::BytesPerFrame;
}

void UsbAudioDevice::handleFeedbackParameters(audio_feedback_params_t *parameters){
    parameters->method      = AUDIO_FEEDBACK_METHOD_FIFO_COUNT;
    parameters->sample_freq = constants::audio::SampleRate;
}

bool UsbAudioDevice::handleGetRequest(uint8_t rootHubPort, const tusb_control_request_t *request){
    const auto *audioRequest{reinterpret_cast<const audio_control_request_t *>(request)};

    switch(audioRequest->bEntityID){
    case usb_descriptors::entity::Clock:        return handleClockGetRequest(rootHubPort, audioRequest);
    case usb_descriptors::entity::FeatureUnit:  return handleFeatureUnitGetRequest(rootHubPort, audioRequest);
    default:                                    return false;
    }
}

bool UsbAudioDevice::handleSetRequest(uint8_t rootHubPort, const tusb_control_request_t *request, const uint8_t *buffer){
    (void)rootHubPort;
    const auto *audioRequest{reinterpret_cast<const audio_control_request_t *>(request)};

    if(audioRequest->bRequest != AUDIO_CS_REQ_CUR) return false;

    switch(audioRequest->bEntityID){
    case usb_descriptors::entity::Clock:        return handleClockSetRequest(audioRequest, buffer);
    case usb_descriptors::entity::FeatureUnit:  return handleFeatureUnitSetRequest(audioRequest, buffer);
    default:                                    return false;
    }
}

bool UsbAudioDevice::handleClockGetRequest(uint8_t rootHubPort, const audio_control_request_t *request){
    auto *controlRequest{reinterpret_cast<const tusb_control_request_t *>(request)};

    if(request->bControlSelector == AUDIO_CS_CTRL_SAM_FREQ){
        if(request->bRequest == AUDIO_CS_REQ_CUR){
            audio_control_cur_4_t current{static_cast<int32_t>(tu_htole32(constants::audio::SampleRate))};
            return tud_audio_buffer_and_schedule_control_xfer(rootHubPort, controlRequest, &current, sizeof(current));
        }
        if(request->bRequest == AUDIO_CS_REQ_RANGE){
            audio_control_range_4_n_t(1) range{};
            range.wNumSubRanges         = tu_htole16(1);
            range.subrange[0].bMin      = static_cast<int32_t>(constants::audio::SampleRate);
            range.subrange[0].bMax      = static_cast<int32_t>(constants::audio::SampleRate);
            range.subrange[0].bRes      = 0;
            return tud_audio_buffer_and_schedule_control_xfer(rootHubPort, controlRequest, &range, sizeof(range));
        }
    }else if(request->bControlSelector == AUDIO_CS_CTRL_CLK_VALID && request->bRequest == AUDIO_CS_REQ_CUR){
        audio_control_cur_1_t isValid{.bCur = 1};
        return tud_audio_buffer_and_schedule_control_xfer(rootHubPort, controlRequest, &isValid, sizeof(isValid));
    }

    return false;
}

bool UsbAudioDevice::handleClockSetRequest(const audio_control_request_t *request, const uint8_t *buffer){
    if(request->bControlSelector != AUDIO_CS_CTRL_SAM_FREQ) return false;
    if(request->wLength != sizeof(audio_control_cur_4_t)) return false;

    const uint32_t requestedRate{static_cast<uint32_t>(reinterpret_cast<const audio_control_cur_4_t *>(buffer)->bCur)};
    if(requestedRate == constants::audio::SampleRate) return true;

    rejectedSampleRate_ = requestedRate;
    return false;
}

bool UsbAudioDevice::handleFeatureUnitGetRequest(uint8_t rootHubPort, const audio_control_request_t *request){
    auto *controlRequest{reinterpret_cast<const tusb_control_request_t *>(request)};
    const uint8_t channel{request->bChannelNumber};
    if(channel >= ControlChannelCount) return false;

    if(request->bControlSelector == AUDIO_FU_CTRL_MUTE && request->bRequest == AUDIO_CS_REQ_CUR){
        audio_control_cur_1_t mute{.bCur = isMuted_[channel]};
        return tud_audio_buffer_and_schedule_control_xfer(rootHubPort, controlRequest, &mute, sizeof(mute));
    }

    if(request->bControlSelector == AUDIO_FU_CTRL_VOLUME){
        if(request->bRequest == AUDIO_CS_REQ_RANGE){
            audio_control_range_2_n_t(1) range{};
            range.wNumSubRanges     = tu_htole16(1);
            range.subrange[0].bMin  = tu_htole16(constants::audio::MinimumVolume);
            range.subrange[0].bMax  = tu_htole16(constants::audio::MaximumVolume);
            range.subrange[0].bRes  = tu_htole16(constants::audio::VolumeResolution);
            return tud_audio_buffer_and_schedule_control_xfer(rootHubPort, controlRequest, &range, sizeof(range));
        }
        if(request->bRequest == AUDIO_CS_REQ_CUR){
            audio_control_cur_2_t volume{.bCur = static_cast<int16_t>(tu_htole16(volume_[channel]))};
            return tud_audio_buffer_and_schedule_control_xfer(rootHubPort, controlRequest, &volume, sizeof(volume));
        }
    }

    return false;
}

bool UsbAudioDevice::handleFeatureUnitSetRequest(const audio_control_request_t *request, const uint8_t *buffer){
    const uint8_t channel{request->bChannelNumber};
    if(channel >= ControlChannelCount) return false;

    if(request->bControlSelector == AUDIO_FU_CTRL_MUTE){
        if(request->wLength != sizeof(audio_control_cur_1_t)) return false;
        isMuted_[channel] = reinterpret_cast<const audio_control_cur_1_t *>(buffer)->bCur != 0;
    }else if(request->bControlSelector == AUDIO_FU_CTRL_VOLUME){
        if(request->wLength != sizeof(audio_control_cur_2_t)) return false;
        volume_[channel] = std::clamp<int16_t>(
            reinterpret_cast<const audio_control_cur_2_t *>(buffer)->bCur,
            constants::audio::MinimumVolume,
            constants::audio::MaximumVolume
        );
    }else{
        return false;
    }

    updateGains();
    controlChangeCount_++;
    return true;
}

void UsbAudioDevice::updateGains(){
    auto channelGain{[this](uint8_t channel)->int32_t{
        if(isMuted_[0] || isMuted_[channel]) return 0;

        const float decibels{std::max(
            (volume_[0] + volume_[channel]) / 256.0f,
            constants::audio::MinimumVolume / 256.0f
        )};
        return static_cast<int32_t>(std::pow(10.0f, decibels / 20.0f) * (1 << 15) + .5f);
    }};

    leftGain_  = channelGain(1);
    rightGain_ = channelGain(2);
}

void UsbAudioDevice::DEBUG_reportEvents(){
    if(isMounted_ != wasMounted_){
        wasMounted_ = isMounted_;
        DEBUG_PRINT("USB: %s", isMounted_ ? "mounted, speaker available to the host" : "unmounted");
    }

    if(rejectedSampleRate_ != 0){
        DEBUG_PRINT(
            "USB: host asked for %lu Hz, only %lu Hz is supported",
            static_cast<unsigned long>(rejectedSampleRate_),
            static_cast<unsigned long>(constants::audio::SampleRate)
        );
        rejectedSampleRate_ = 0;
    }

    const units::TimestampUs currentTime{time_us_64()};
    if(controlChangeCount_ != lastControlChangeCount_){

        lastControlChangeCount_ = controlChangeCount_;
        lastControlChangeTime_  = currentTime;

    }else if(reportedControlChanges_ != controlChangeCount_
    && currentTime - lastControlChangeTime_ >= units::Ms2Us(constants::audio::ControlReportDelay)
    ){
        reportedControlChanges_ = controlChangeCount_;

        DEBUG_PRINT(
            "Host volume: left %.1f dB, right %.1f dB%s -> gain %.3f / %.3f",
            volume_[1] / 256.0f,
            volume_[2] / 256.0f,
            isMuted() ? " (MUTED)" : "",
            leftGain_ / 32768.0f,
            rightGain_ / 32768.0f
        );
    }
}
