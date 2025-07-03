#include "Midi/MidiManager.h"
#include <spdlog/spdlog.h>

MidiManager::MidiManager()
    : m_observer(libremidi::observer_configuration{
        .input_added = std::bind(&MidiManager::onInputAdded, this, std::placeholders::_1),
        .input_removed = std::bind(&MidiManager::onInputRemoved, this, std::placeholders::_1),
        .output_added = std::bind(&MidiManager::onOutputAdded, this, std::placeholders::_1),
        .output_removed = std::bind(&MidiManager::onOutputRemoved, this, std::placeholders::_1)
    })
{
}


void MidiManager::onInputAdded(const libremidi::input_port &val) {
    spdlog::info("Input added: {}", val.port_name);
}

void MidiManager::onInputRemoved(const libremidi::input_port &val) {
    spdlog::info("Input removed: {}", val.port_name);
}

void MidiManager::onOutputAdded(const libremidi::output_port &val) {
    spdlog::info("Output added: {}", val.port_name);
}

void MidiManager::onOutputRemoved(const libremidi::output_port &val) {
    spdlog::info("Output removed: {}", val.port_name);
}