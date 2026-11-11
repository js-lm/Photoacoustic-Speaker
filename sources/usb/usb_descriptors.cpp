#include "usb_descriptors.hpp"

#include "constants.hpp"

#include <tusb.h>

// TODO: device, configuration (uac2 speaker + cdc console) and string descriptors

extern "C"{

uint8_t const *tud_descriptor_device_cb(void){
    return nullptr;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index){
    (void)index;
    return nullptr;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t languageId){
    (void)index;
    (void)languageId;
    return nullptr;
}

} // extern "C"
