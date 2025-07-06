#include <iostream>
#include <libremidi/libremidi.hpp>
#include <spdlog/spdlog.h>

#include "Midi/MidiManager.h"
#include "Midi/MidiDevice.h"

int main() {
    spdlog::set_level(spdlog::level::debug);

    MidiManager manager;
    manager.startRecording();


    manager.onMidiMessage([](MidiDevice* device, MidiMessage& msg) {
        std::ostringstream ss;
        for (int i = 0; i < msg.size(); i++) {
            ss << (int)msg[i] << " ";
        }
        spdlog::info("From {} | Channel: {} | Type: {} | Message: {}", device->displayName(), msg.get_channel(), (int)msg.get_message_type(), ss.str());
    });
    manager.onDeviceAdded([](MidiDevice* device) {
        spdlog::info("Device added: {}", device->name());
        std::ostringstream ss;
        for (auto h : device->identity()) {
            ss << std::hex << (int)h << " ";
        }
        spdlog::info("Identity: {}", ss.str());
    });
    manager.onDeviceRemoved([](MidiDevice* device) {
        spdlog::info("Device removed: {}", device->inPort().port_name);
    });

    std::cin.get();

    manager.stopRecording();

    for (auto recording : manager.recorded()) {
        spdlog::info("Device: {}", recording.first);
        for (auto msg : recording.second) {
            std::ostringstream ss;
            for (int i = 0; i < msg.message.size(); i++) {
                ss << (int)msg.message[i] << " ";
            }
            ss << " | " << msg.timestamp;
            spdlog::info("Message: {}", ss.str());
        }
    }

    return 0;
}