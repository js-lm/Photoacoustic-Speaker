#include "music_box.hpp"

#include "constants.hpp"

#include <pico/stdlib.h>

#include "debug_utilities.hpp"

MusicBox &MusicBox::getInstance(){
    static MusicBox instance{};
    return instance;
}

int MusicBox::run(){
    initialize();

    while(true){
        update();
    }

    return 0;
}

void MusicBox::initialize(){
    carrierManager_.initialize();

    stdio_init_all();
    DEBUG_PRINT("Initializing...");
    DEBUG_SLEEP_MS(2000);

    lastUpdateTime_ = time_us_64();
}

void MusicBox::update(){

    
}

