#pragma once

class SafetyMonitor{
public:
    SafetyMonitor() = default;
    ~SafetyMonitor() = default;

    void initialize();

public:
    bool isLatchArmed() const;
    bool isInterlockClosed() const;

private:
    void trip();
};
