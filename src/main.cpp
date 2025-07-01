#include <iostream>
#include <libremidi/libremidi.hpp>

int main() {
    libremidi::observer observer;
    for (const auto& port : observer.get_input_ports()) {
        std::cout << port.port_name << std::endl;
    }
    return 0;
}