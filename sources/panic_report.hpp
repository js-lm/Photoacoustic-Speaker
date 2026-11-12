#pragma once

// can survive a reboot after a panic
class PanicReport{
public:
    static void initialize();

    static const char *previousPanic(); // message of the panic that caused this boot, or nullptr
};
