#pragma once
#include <libremidi/libremidi.hpp>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>
#include <optional>

#include "MidiDevice.h"
#include "types.h"

class MidiPortManager {
public:
    MidiPortManager();

    void scan();

    std::vector<libremidi::input_port> inputs() const noexcept;
    std::vector<libremidi::output_port> outputs() const noexcept;

    void onPortsChanged(std::function<void()> cb);
    void onInputAdded(std::function<void(const libremidi::input_port &)> cb);
    void onInputRemoved(std::function<void(const libremidi::input_port &)> cb);
    void onOutputAdded(std::function<void(const libremidi::output_port &)> cb);
    void onOutputRemoved(std::function<void(const libremidi::output_port &)> cb);
private:
    void InputAdded(const libremidi::input_port &val);
    void InputRemoved(const libremidi::input_port &val);
    void OutputAdded(const libremidi::output_port &val);
    void OutputRemoved(const libremidi::output_port &val);

    std::mutex m_mutex;
    std::vector<libremidi::input_port> m_inPorts;
    std::vector<libremidi::output_port> m_outPorts;

    std::function<void()> m_portsChanged;
    std::function<void(const libremidi::input_port &)> m_inputAdded;
    std::function<void(const libremidi::input_port &)> m_inputRemoved;
    std::function<void(const libremidi::output_port &)> m_outputAdded;
    std::function<void(const libremidi::output_port &)> m_outputRemoved;
    
    libremidi::observer m_observer;
};

class MidiDeviceManager {
public:
    MidiDeviceManager();

    void onMidiMessage(DeviceMidiMessageCallback cb);
    void onDevicesRefresh(DeviceRefreshCallback cb);
    void onDeviceAdded(DeviceAddedCallback cb);
    void onDeviceRemoved(DeviceRemovedCallback cb);

    std::vector<MidiDevice*> getDevices();
    std::vector<MidiDevice*> getAvailableDevices();

    void refresh();

private:
    bool portsMatch(const libremidi::input_port &in, const libremidi::output_port &out);
    void scanPorts();

    void handlePortRefresh();

    std::mutex m_mutex;
    std::vector<std::shared_ptr<MidiDevice>> m_devices;

    DeviceMidiMessageCallback m_midiMessageCallback;
    DeviceRefreshCallback m_devicesRefreshCallback;
    DeviceAddedCallback m_deviceAddedCallback;
    DeviceRemovedCallback m_deviceRemovedCallback;

    MidiPortManager m_portManager;
};

class MidiManager {
public:
    MidiManager();

    void startRecording();
    void stopRecording();
    std::vector<std::pair<std::string, std::vector<MidiMessage>>> recorded();

    void refresh();

    std::vector<MidiDevice*> getDevices();
    std::vector<MidiDevice*> getAvailableDevices();

    void onMidiMessage(DeviceMidiMessageCallback cb);
    void onDevicesRefresh(DeviceRefreshCallback cb);
    void onDevicesAdded(DeviceAddedCallback cb);
    void onDevicesRemoved(DeviceRemovedCallback cb);
private:
    DeviceMidiMessageCallback m_midiMessageCallback;
    DeviceRefreshCallback m_devicesRefreshCallback;
    DeviceAddedCallback m_deviceAddedCallback;
    DeviceRemovedCallback m_deviceRemovedCallback;

    MidiDeviceManager m_deviceManager;
};