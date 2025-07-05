#include <iostream>
#include <libremidi/libremidi.hpp>
#include <spdlog/spdlog.h>

#include "Midi/MidiManager.h"
#include "Midi/MidiDevice.h"

int main() {
    libremidi::observer observer;
    for (const auto& port : observer.get_input_ports()) {
        spdlog::info("{} | {} | {} | {}", port.port_name, port.display_name, port.device_name, port.port);
    }

    for (const auto& port : observer.get_output_ports()) {
        spdlog::info("{} | {} | {} | {}", port.port_name, port.display_name, port.device_name, port.port);
    }

    auto my_callback = [](const libremidi::message& msg) {
        for (int i = 0; i < msg.size(); i++) {
            std::cout << (int)msg[i] << " ";
        }
        std::cout << "\n";
        std::cout.flush();
    };

    // libremidi::midi_in input{
    //     libremidi::input_configuration{ .on_message = my_callback }
    // };

    // input.open_port(libremidi::midi1::in_default_port().value());

    // MidiDevice device(libremidi::midi1::in_default_port().value(), libremidi::midi1::out_default_port().value());
    // device.onMessage(my_callback);
    // device.onVerified([](libremidi::message& msg, MidiIdentityVerifier::Availability) {
    //     std::ostringstream ss;
    //     for (int i = 0; i < msg.size(); i++) {
    //         ss << (int)msg[i] << " ";
    //     }
    //     spdlog::info("Message: {}", ss.str());
    // });

    MidiManager manager;
    manager.startRecording();


    manager.onMidiMessage([](MidiDevice* device, MidiMessage& msg) {
        std::ostringstream ss;
        for (int i = 0; i < msg.size(); i++) {
            ss << (int)msg[i] << " ";
        }
        spdlog::info("From {} | Channel: {} | Type: {} | Message: {}", device->name(), msg.get_channel(), (int)msg.get_message_type(), ss.str());
    });
    manager.onDevicesAdded([](MidiDevice* device) {
        spdlog::info("Device added: {}", device->name());
    });
    manager.onDevicesRemoved([](MidiDevice* device) {
        spdlog::info("Device removed: {}", device->inPort().port_name);
    });


    std::cin.get();

    manager.stopRecording();

    for (auto recording : manager.recorded()) {
        spdlog::info("Device: {}", recording.first);
        for (auto msg : recording.second) {
            std::ostringstream ss;
            for (int i = 0; i < msg.size(); i++) {
                ss << (int)msg[i] << " ";
            }
            spdlog::info("Message: {}", ss.str());
        }
    }

    return 0;
}