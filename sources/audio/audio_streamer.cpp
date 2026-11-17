#include "audio_streamer.hpp"

#include "constants.hpp"

#include "debug_utilities.hpp"

// TODO
void AudioStreamer::initialize(){}

// TODO: compute the level mapping from the carrier and start the dma
void AudioStreamer::start(const CarrierManager &carrierManager){
    isRunning_ = true;
}

// TODO: stop the dma
void AudioStreamer::stop(){
    isRunning_ = false;
}

// TODO: keep the ring topped up from the usb fifo
void AudioStreamer::pump(UsbAudioDevice &usbAudioDevice){}

// TODO: drain the usb fifo while the output is off
void AudioStreamer::discard(UsbAudioDevice &usbAudioDevice){}

// TODO
units::DutyCycle AudioStreamer::playedDuty() const{ return .0f;}
