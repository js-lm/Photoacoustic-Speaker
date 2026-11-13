#pragma once

#include "aliases.hpp"
#include "constants.hpp"

#include <tusb.h>

#include <cstdint>

// usb side of the speaker: enumeration, stream state, host controls and the received sample fifo
// the tinyusb callbacks (usb_callbacks.cpp) run inside tud_task(), so they only record state.
// all logging happens in update(), outside tud_task(), because printf over cdc can itself call tud_task()
class UsbAudioDevice{
private:
    // index 0 is the master channel then left and right
    static constexpr uint8_t ControlChannelCount{constants::audio::ChannelCount + 1};

private:
    bool        isMounted_              {false};
    bool        isStreamOpen_           {false};

private:
    int16_t     volume_[ControlChannelCount]{}; // 1/256 dB
    bool        isMuted_[ControlChannelCount]{};
    int32_t     leftGain_               {1 << 15}; // Q15, 1 << 15 is unity
    int32_t     rightGain_              {1 << 15};
    uint32_t    controlChangeCount_     {0};
    uint32_t    rejectedSampleRate_     {0};

private:
    bool                wasMounted_             {false};
    uint32_t            reportedControlChanges_ {0};
    uint32_t            lastControlChangeCount_ {0};
    units::TimestampUs  lastControlChangeTime_  {0};

public:
    static UsbAudioDevice &getInstance();

    void initialize();
    void update();

public:
    bool isMounted() const{ return isMounted_;}
    bool isStreamOpen() const{ return isStreamOpen_;}
    bool isMuted() const;

    int32_t leftGain() const{ return leftGain_;}
    int32_t rightGain() const{ return rightGain_;}

    uint16_t availableFrames() const;
    uint16_t fifoDepthFrames() const;

    uint16_t read(int16_t *samples, uint16_t maximumFrames);
    void discard(uint16_t frameCount);
    bool checkOverflow();

public:
    void handleMount();
    void handleUnmount();
    void handleStreamInterface(uint8_t alternate);
    void handleStreamEndpointsClosing();
    bool handleGetRequest(uint8_t rootHubPort, const tusb_control_request_t *request);
    bool handleSetRequest(uint8_t rootHubPort, const tusb_control_request_t *request, const uint8_t *buffer);
    void handleFeedbackParameters(audio_feedback_params_t *parameters);

private:
    UsbAudioDevice() = default;
    ~UsbAudioDevice() = default;

private:
    bool handleClockGetRequest(uint8_t rootHubPort, const audio_control_request_t *request);
    bool handleClockSetRequest(const audio_control_request_t *request, const uint8_t *buffer);
    bool handleFeatureUnitGetRequest(uint8_t rootHubPort, const audio_control_request_t *request);
    bool handleFeatureUnitSetRequest(const audio_control_request_t *request, const uint8_t *buffer);
    void updateGains();

private:
    void DEBUG_reportEvents();
};
