#include "panic_report.hpp"

#include <hardware/watchdog.h>
#include <pico/platform.h>

#include <cstdarg>
#include <cstdio>

// a panic halts the cpu with usb dead, so its message never reaches the console.
// panic() is wrapped at link time (-Wl,--wrap=panic, which also covers tinyusb and the sdk):
// the message is kept in ram that survives a reset, the chip reboots, and the next boot prints it.

namespace{

    constexpr uint32_t RecordMagic{0x50414E43}; // "PANC"

    struct PanicRecord{
        uint32_t    magic;
        char        message[160];
    };

    PanicRecord __uninitialized_ram(panicRecord);

    char previousMessage[sizeof(PanicRecord::message)]{};
    bool hasPreviousMessage{false};

} // namespace

void PanicReport::initialize(){
    if(panicRecord.magic == RecordMagic){
        panicRecord.message[sizeof(panicRecord.message) - 1] = '\0';
        snprintf(previousMessage, sizeof(previousMessage), "%s", panicRecord.message);
        hasPreviousMessage = true;
    }
    panicRecord.magic = 0;
}

const char *PanicReport::previousPanic(){
    return hasPreviousMessage ? previousMessage : nullptr;
}

extern "C" [[noreturn]] void __wrap_panic(const char *format, ...){
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(panicRecord.message, sizeof(panicRecord.message), format != nullptr ? format : "(no message)", arguments);
    va_end(arguments);
    panicRecord.magic = RecordMagic;

    watchdog_reboot(0, 0, 0);
    while(true) tight_loop_contents();
}
