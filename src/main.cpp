#include <iostream>
#include <libremidi/libremidi.hpp>
#include <spdlog/spdlog.h>

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

    libremidi::midi_in input{
        libremidi::input_configuration{ .on_message = my_callback }
    };

    input.open_port(libremidi::midi1::in_default_port().value());

    std::cin.get();

    return 0;
}