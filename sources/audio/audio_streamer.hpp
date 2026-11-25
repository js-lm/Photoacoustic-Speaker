#pragma once

#include "constants.hpp"
#include "aliases.hpp"

#include "usb/usb_audio_device.hpp"
#include "carrier/carrier_manager.hpp"
#include "audio/compressor.hpp"

#include <bit>
#include <cstdint>

class AudioStreamer{
private:
    static constexpr uint32_t RingMask          {constants::audio::RingLength - 1};
    static constexpr uint32_t RingBytes         {constants::audio::RingLength * sizeof(uint32_t)};
    static constexpr uint32_t RingAddressBits   {static_cast<uint32_t>(std::countr_zero(RingBytes))};
    static constexpr int32_t  EnvelopeUnity     {1 << 24};

private:
    alignas(RingBytes) uint32_t ring_[constants::audio::RingLength]{};
    int16_t     readBuffer_[constants::audio::ChannelCount * constants::audio::ReadChunkFrames]{};

    int         dmaChannel_         {-1};
    uint32_t    writtenCount_       {0}; // words written since the dma started
    bool        isRunning_          {false};

private:
    uint32_t    fullScaleLevel_     {0};
    uint32_t    capLevel_           {0};
    uint32_t    periodTicks_        {1};
    int32_t     envelopeStep_       {0};

private:
    bool                isStreamOpen_       {false};
    bool                isPrimed_           {false};
    bool                isMuted_            {false};
    bool                isSilent_           {false};
    bool                isOutputEnabled_    {false};
    int32_t             leftGain_           {1 << 15}; // Q15 host volume, cached every pump
    int32_t             rightGain_          {1 << 15};
    Compressor          compressor_         {};
    int32_t             envelope_           {0}; // Q24, 0 = dark, EnvelopeUnity = full range
    units::TimestampUs  streamOpenedTime_   {0};
    units::TimestampUs  primingStartTime_   {0};
    units::TimestampUs  lastSignalTime_     {0};

private:
    uint32_t    playedCount_        {0};
    int32_t     playedLevelAverage_ {0}; // Q16 levels

private:
    struct Statistics{
        uint32_t    underrunCount;
        uint32_t    ringUnderrunCount;
        uint32_t    overflowCount;
        uint32_t    inputSampleCount;
        int32_t     peakMagnitude;
        uint64_t    sumOfSquares;
        uint32_t    levelCount;
        uint32_t    minimumLevel;
        uint32_t    maximumLevel;
        uint64_t    levelSum;
    };

    Statistics          window_                 {};
    Statistics          stream_                 {};
    uint32_t            reportReceivedFrames_   {0};
    units::TimestampUs  lastReportTime_         {0};
    bool                hasReportedGlitch_      {false};

public:
    AudioStreamer() = default;
    ~AudioStreamer() = default;

    void initialize();

public:
    void start(const CarrierManager &carrierManager);
    void stop();

    void pump(UsbAudioDevice &usbAudioDevice);
    void discard(UsbAudioDevice &usbAudioDevice);

    void setOutputEnabled(bool shouldEnable){ isOutputEnabled_ = shouldEnable;}

public:
    bool isRunning() const{ return isRunning_;}
    bool hasSignal() const{ return isStreamOpen_ && isPrimed_ && !isMuted_ && !isSilent_;}
    bool isFadedIn() const{ return envelope_ >= EnvelopeUnity;}
    bool isFadedOut() const{ return envelope_ <= 0;}

    units::DutyCycle playedDuty() const;

private:
    void startDma();
    uint32_t consumedCount() const;
    void foldPlayedSamples(uint32_t consumed);

private:
    void trackStream(const UsbAudioDevice &usbAudioDevice, units::TimestampUs currentTime);
    void updatePriming(const UsbAudioDevice &usbAudioDevice, units::TimestampUs currentTime);
    void updateSilenceGate(int32_t chunkPeak, units::TimestampUs currentTime);

private:
    void writeFrames(const int16_t *samples, uint16_t frameCount, units::TimestampUs currentTime);
    void writeSilence(uint32_t sampleCount);
    void writeSample(int32_t sample);

private:
    static void resetStatistics(Statistics &statistics);
    static void accumulateStatistics(Statistics &total, const Statistics &window);
    static float toDecibelsFullScale(float magnitude);

private:
    void DEBUG_reportGlitch(const char *format, unsigned long value);
    void DEBUG_reportWindow(const UsbAudioDevice &usbAudioDevice, units::TimestampUs currentTime);
    void DEBUG_reportStreamClosed(units::TimestampUs currentTime);
};
