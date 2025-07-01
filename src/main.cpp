#include <iostream>
#include <libremidi/libremidi.hpp>
#include <spdlog/spdlog.h>

int main() {
    libremidi::observer observer;
    for (const auto& port : observer.get_input_ports()) {
        spdlog::info("{} | {} | {}", port.port_name, port.display_name, port.manufacturer);
    }

    for (const auto& port : observer.get_output_ports()) {
        spdlog::info("{} | {} | {}", port.port_name, port.display_name, port.manufacturer);
    }
    return 0;
}