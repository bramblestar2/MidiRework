#include "Midi/MidiManager.h"
#include <spdlog/spdlog.h>

MidiManager::MidiManager()
{
    libremidi::observer_configuration observer_configuration = {
        .input_added = [](const libremidi::input_port &val) {
            spdlog::info("Input Added: {} | {} | {} | {}", val.port_name, val.display_name, val.device_name, val.port);
        },
        .input_removed = [](const libremidi::input_port &val) {
            spdlog::info("Input Removed: {} | {} | {} | {}", val.port_name, val.display_name, val.device_name, val.port);
        },
        .output_added = [](const libremidi::output_port &val) {
            spdlog::info("Output Added: {} | {} | {} | {}", val.port_name, val.display_name, val.device_name, val.port);
        },
        .output_removed = [](const libremidi::output_port &val) {
            spdlog::info("Output Removed: {} | {} | {} | {}", val.port_name, val.display_name, val.device_name, val.port);
        }
    };

    observer = libremidi::observer{std::move(observer_configuration)};
}