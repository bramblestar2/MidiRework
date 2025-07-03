#pragma once
#include <libremidi/libremidi.hpp>
#include <vector>

#include "MidiDevice.h"

class MidiManager {
public:
    MidiManager();
private:
    void onInputAdded(const libremidi::input_port &val);
    void onInputRemoved(const libremidi::input_port &val);
    void onOutputAdded(const libremidi::output_port &val);
    void onOutputRemoved(const libremidi::output_port &val);

    libremidi::observer m_observer;

    std::vector<MidiDevice> m_devices;
};