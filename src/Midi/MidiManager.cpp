#include "Midi/MidiManager.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <regex>
#include <iostream>

MidiPortManager::MidiPortManager()
    : m_portsChanged(nullptr)
    , m_inputAdded(nullptr)
    , m_inputRemoved(nullptr)
    , m_outputAdded(nullptr)
    , m_outputRemoved(nullptr)
    , m_observer(libremidi::observer_configuration{
        .on_error = std::bind(&MidiPortManager::ErrorMessage, this, std::placeholders::_1, std::placeholders::_2),
        .on_warning = std::bind(&MidiPortManager::WarningMessage, this, std::placeholders::_1, std::placeholders::_2),
        .input_added = std::bind(&MidiPortManager::InputAdded, this, std::placeholders::_1),
        .input_removed = std::bind(&MidiPortManager::InputRemoved, this, std::placeholders::_1),
        .output_added = std::bind(&MidiPortManager::OutputAdded, this, std::placeholders::_1),
        .output_removed = std::bind(&MidiPortManager::OutputRemoved, this, std::placeholders::_1),
    })
{
}

void MidiPortManager::scan() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_inPorts = m_observer.get_input_ports();
    m_outPorts = m_observer.get_output_ports();
}

std::vector<libremidi::input_port> MidiPortManager::inputs() const noexcept {
    return m_inPorts;
}

std::vector<libremidi::output_port> MidiPortManager::outputs() const noexcept {
    return m_outPorts;
}

void MidiPortManager::onPortsChanged(std::function<void()> cb) {
    m_portsChanged = cb;
}

void MidiPortManager::onInputAdded(std::function<void(const libremidi::input_port &)> cb) {
    m_inputAdded = cb;
}

void MidiPortManager::onInputRemoved(std::function<void(const libremidi::input_port &)> cb) {
    m_inputRemoved = cb;
}

void MidiPortManager::onOutputAdded(std::function<void(const libremidi::output_port &)> cb) {
    m_outputAdded = cb;
}

void MidiPortManager::onOutputRemoved(std::function<void(const libremidi::output_port &)> cb) {
    m_outputRemoved = cb;
}

void MidiPortManager::onError(ErrorCallback cb) {
    m_errorCallback = cb;
}

void MidiPortManager::onWarning(WarningCallback cb) {
    m_warningCallback = cb;
}

void MidiPortManager::ErrorMessage(std::string_view info, const libremidi::source_location& source) {
    spdlog::error("(File({}) | Ln({})) Midi Error: {}", source.file_name(), source.line(), info);

    if (m_errorCallback) {
        m_errorCallback(info, source);
    }
}

void MidiPortManager::WarningMessage(std::string_view info, const libremidi::source_location& source) {
    spdlog::warn("(File({}) | Ln({})) Midi Error: {}", source.file_name(), source.line(), info);

    if (m_warningCallback) {
        m_warningCallback(info, source);
    }
}

void MidiPortManager::InputAdded(const libremidi::input_port &val) {
    bool didAdd = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find_if(m_inPorts.begin(), m_inPorts.end(), 
        [&val](const libremidi::input_port &p) {
            return p.port_name == val.port_name;
        });

        if (it == m_inPorts.end()) {
            m_inPorts.push_back(val);
            didAdd = true;
        }
    }

    if (!didAdd) return;

    
    if (m_portsChanged) m_portsChanged();
    if (m_inputAdded) m_inputAdded(val);
}

void MidiPortManager::OutputAdded(const libremidi::output_port &val) {
    bool didAdd = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find_if(m_outPorts.begin(), m_outPorts.end(), 
        [&val](const libremidi::output_port &p) {
            return p.port_name == val.port_name;
        });

        if (it == m_outPorts.end()) {
            m_outPorts.push_back(val);
            didAdd = true;
        }
    }

    if (!didAdd) return;

    if (m_portsChanged) m_portsChanged();
    if (m_outputAdded) m_outputAdded(val);
}

void MidiPortManager::InputRemoved(const libremidi::input_port &val) {
    bool didRemove = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find_if(m_inPorts.begin(), m_inPorts.end(), 
        [&val](const libremidi::input_port &p) {
            return p.port_name == val.port_name;
        });

        if (it != m_inPorts.end()) {
            m_inPorts.erase(it);
            didRemove = true;
        }
    }

    if (!didRemove) return;
    
    if (m_portsChanged) m_portsChanged();
    if (m_inputRemoved) m_inputRemoved(val);
}

void MidiPortManager::OutputRemoved(const libremidi::output_port &val) {
    bool didRemove = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::find_if(m_outPorts.begin(), m_outPorts.end(), 
        [&val](const libremidi::output_port &p) {
            return p.port_name == val.port_name;
        });

        if (it != m_outPorts.end()) {
            m_outPorts.erase(it);
            didRemove = true;
        }
    }

    if (!didRemove) return;

    if (m_portsChanged) m_portsChanged();
    if (m_outputRemoved) m_outputRemoved(val);
}



MidiDeviceManager::MidiDeviceManager()
    : m_recording(false)
    , m_deviceRefreshDebouncer(std::chrono::milliseconds(100), 
        [this](std::vector<MidiDevice*> devices) {
            if (m_devicesRefreshCallback) {
                m_devicesRefreshCallback(devices);
            }
        })
    , m_deviceAddedDebouncer(std::chrono::milliseconds(100), 
        [this](MidiDevice* device) {
            if (m_deviceAddedCallback) {
                m_deviceAddedCallback(device);
            }
        })
    , m_deviceRemovedDebouncer(std::chrono::milliseconds(100), 
        [this](MidiDevice* device) {
            if (m_deviceRemovedCallback) {
                m_deviceRemovedCallback(device);
            }
        })
{
    m_portManager.onPortsChanged(std::bind(&MidiDeviceManager::handlePortRefresh, this));
    m_portManager.onInputAdded([this](const libremidi::input_port &val) { });
    m_portManager.onInputRemoved([this](const libremidi::input_port &val) { });
    m_portManager.onOutputAdded([this](const libremidi::output_port &val) { });
    m_portManager.onOutputRemoved([this](const libremidi::output_port &val) { });
    m_portManager.onError([this](std::string_view info, const libremidi::source_location& source) {
        if (m_errorCallback) {
            m_errorCallback(info, source);
        }
    });
    m_portManager.onWarning([this](std::string_view info, const libremidi::source_location& source) {
        if (m_warningCallback) {
            m_warningCallback(info, source);
        }
    });

    handlePortRefresh();
}

void MidiDeviceManager::startRecording() {
    m_recording = true;
    for (auto d : this->getAvailableDevices()) {
        d->startRecording();
    }
}

void MidiDeviceManager::stopRecording() {
    m_recording = false;
    for (auto d : this->getAvailableDevices()) {
        d->stopRecording();
    }
}

std::vector<std::pair<std::string, std::vector<MidiMessageRecord>>> MidiDeviceManager::recorded() {
    std::vector<std::pair<std::string, std::vector<MidiMessageRecord>>> result;

    for (auto d : this->getAvailableDevices()) {
        if (d->recorded().empty()) {
            continue;
        }
        result.push_back(std::make_pair(d->name(), d->recorded()));
    }

    return result;
}

void MidiDeviceManager::onError(ErrorCallback cb) {
    m_errorCallback = cb;
}

void MidiDeviceManager::onWarning(WarningCallback cb) {
    m_warningCallback = cb;
}

void MidiDeviceManager::onMidiMessage(DeviceMidiMessageCallback cb) {
    m_midiMessageCallback = cb;
}

void MidiDeviceManager::onDevicesRefresh(DeviceRefreshCallback cb) {
    m_devicesRefreshCallback = cb;
}

void MidiDeviceManager::onDeviceAdded(DeviceAddedCallback cb) {
    m_deviceAddedCallback = cb;
}

void MidiDeviceManager::onDeviceRemoved(DeviceRemovedCallback cb) {
    m_deviceRemovedCallback = cb;
}

std::vector<MidiDevice*> MidiDeviceManager::getDevices() {
    std::vector<MidiDevice*> result;
    result.reserve(m_devices.size());

    std::transform(m_devices.begin(), m_devices.end(), std::back_inserter(result), [](const std::shared_ptr<MidiDevice> &d) {
        return d.get();
    });

    result.shrink_to_fit();

    return result;
}

std::vector<MidiDevice*> MidiDeviceManager::getAvailableDevices() {
    std::vector<MidiDevice*> result;
    result.reserve(m_devices.size());

    auto devices = this->getDevices();
    std::copy_if(devices.begin(), devices.end(), std::back_inserter(result), [](MidiDevice *d) {
        return d->status() == Availability::Available;
    });

    result.shrink_to_fit();

    return result;
}

void MidiDeviceManager::refresh() {
    scanPorts();
    handlePortRefresh();
}

bool MidiDeviceManager::portsMatch(const libremidi::input_port &inPort, const libremidi::output_port &outPort) {
    std::string in = inPort.port_name;
    std::string out = outPort.port_name;

    // Go to Uppercase
    std::transform(in.begin(), in.end(), in.begin(), ::toupper);
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    
    //Remove index number at the end
    // in = std::regex_replace(in, std::regex("\\d+$"), "");
    // out = std::regex_replace(out, std::regex("\\d+$"), "");

    // Remove IN AND OUT from the names
    in = std::regex_replace(in, std::regex("IN"), "");
    out = std::regex_replace(out, std::regex("OUT"), "");

#ifdef _WIN32
    in = std::regex_replace(in, std::regex("\\d$"), "");
    out = std::regex_replace(out, std::regex("\\d$"), "");
#endif
    // std::cout << "Device Match: " << in << " - " << out << "\n";

    return in == out;
}

void MidiDeviceManager::scanPorts() {
    m_portManager.scan();
}

void MidiDeviceManager::handlePortRefresh() {
    bool devicesChanged = false;

    auto inPorts = m_portManager.inputs();
    auto outPorts = m_portManager.outputs();

    std::lock_guard<std::mutex> lock(m_mutex);

    for (auto &d : m_devices) {
        d->close();
        d->onVerified(nullptr);
        d->onMessage(nullptr);
    }

    m_devices.clear();

    // Find matching ports in the updated lists
    for (auto &in : inPorts) {
        for (auto &out : outPorts) {
            if (portsMatch(in, out)) {
                continue;
            }

            auto device = std::make_shared<MidiDevice>(in, out);
            m_devices.push_back(device);

            device->onMessage([this, device](MidiMessage &m) {
                if (m_midiMessageCallback) {
                    m_midiMessageCallback(device.get(), m);
                }
            });

            device->onVerified([this, device](MidiMessage &m, Availability status) {
                m_deviceAddedDebouncer.trigger(device.get());

                if (m_recording) {
                    device->startRecording();
                }

            });

            devicesChanged = true;
        }
    }

    if (devicesChanged) {
        m_deviceRefreshDebouncer.trigger(this->getDevices());
    }
}


MidiManager::MidiManager()
{
}