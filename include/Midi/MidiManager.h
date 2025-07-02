#pragma once
#include <libremidi/libremidi.hpp>

class MidiManager {
public:
    MidiManager();
private:
    libremidi::observer observer;
};