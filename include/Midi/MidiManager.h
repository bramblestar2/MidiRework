#pragma once
#include <libremidi/libremidi.hpp>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>
#include <optional>

#include "MidiDevice.h"
#include "types.h"
#include "Utility/Debouncer.h"

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

    void onError(ErrorCallback cb);
    void onWarning(WarningCallback cb);
private:
    void InputAdded(const libremidi::input_port &val);
    void InputRemoved(const libremidi::input_port &val);
    void OutputAdded(const libremidi::output_port &val);
    void OutputRemoved(const libremidi::output_port &val);

    void ErrorMessage(std::string_view info, const libremidi::source_location&);
    void WarningMessage(std::string_view info, const libremidi::source_location&);

    std::mutex m_mutex;
    std::vector<libremidi::input_port> m_inPorts;
    std::vector<libremidi::output_port> m_outPorts;

    std::function<void()> m_portsChanged;
    std::function<void(const libremidi::input_port &)> m_inputAdded;
    std::function<void(const libremidi::input_port &)> m_inputRemoved;
    std::function<void(const libremidi::output_port &)> m_outputAdded;
    std::function<void(const libremidi::output_port &)> m_outputRemoved;

    ErrorCallback m_errorCallback;
    WarningCallback m_warningCallback;
    
    libremidi::observer m_observer;
};

class MidiDeviceManager {
public:
    MidiDeviceManager();

    void startRecording();
    void stopRecording();
    std::vector<std::pair<std::string, std::vector<MidiMessageRecord>>> recorded();

    void onError(ErrorCallback cb);
    void onWarning(WarningCallback cb);

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

    ErrorCallback m_errorCallback;
    WarningCallback m_warningCallback;

    DeviceMidiMessageCallback m_midiMessageCallback;
    DeviceRefreshCallback m_devicesRefreshCallback;
    DeviceAddedCallback m_deviceAddedCallback;
    DeviceRemovedCallback m_deviceRemovedCallback;

    Debouncer<std::vector<MidiDevice*>> m_deviceRefreshDebouncer;
    Debouncer<MidiDevice*> m_deviceAddedDebouncer;
    Debouncer<MidiDevice*> m_deviceRemovedDebouncer;

    MidiPortManager m_portManager;

    bool m_recording;
};

class MidiManager : public MidiDeviceManager {
public:
    MidiManager();

private:
    MidiDeviceManager m_deviceManager;
};