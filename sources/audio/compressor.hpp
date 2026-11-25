#pragma once

#include <cstdint>

class Compressor{
private:
    static constexpr int32_t    EnvelopeShift   {12}; // envelope is Q12 magnitude
    static constexpr int32_t    GainShift       {12};  // gain is Q12, 4096 = 0 dB

private:
    int32_t     envelope_           {0};
    int32_t     gain_               {1 << GainShift};
    int32_t     gainStep_           {0};
    uint16_t    blockPosition_      {0};

private:
    float       minimumGain_        {.0f}; // dB, over the current report window
    float       maximumGain_        {.0f};
    uint32_t    clippedCount_       {0};
    bool        hasWindowGain_      {false};

public:
    Compressor() = default;
    ~Compressor() = default;

    void reset();

    int32_t process(int32_t sample);

public:
    float       minimumGain() const{ return minimumGain_;}
    float       maximumGain() const{ return maximumGain_;}
    uint32_t    clippedCount() const{ return clippedCount_;}
    void        resetWindow();

private:
    void updateGain();
    static float targetGain(float envelopeDecibels);
};
