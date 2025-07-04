#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include <chrono>
#include <libremidi/libremidi.hpp>

#include "types.h"

class MidiTransport {
public:
    MidiTransport(libremidi::input_port inPort, libremidi::output_port outPort, std::function<void(MidiMessage&)> cb);
    MidiTransport(libremidi::input_port inPort, libremidi::output_port outPort);
    ~MidiTransport();

    const libremidi::input_port& inPort() const noexcept;
    const libremidi::output_port& outPort() const noexcept;

    void open(libremidi::input_port inPort, libremidi::output_port outPort);
    void close();

    void send(const std::vector<unsigned char>& msg);
    void onMidiMessage(std::function<void(MidiMessage&)> cb);

    void operator()(MidiMessage& msg);
private:
    void handleMidiMessage(MidiMessage& msg);

    std::mutex m_mutex;
    libremidi::midi_in m_midiIn;
    libremidi::midi_out m_midiOut;

    libremidi::input_port m_inPort;
    libremidi::output_port m_outPort;

    std::function<void(MidiMessage&)> m_userCb;
};


class MidiIdentityVerifier {
public:
    enum Availability {
        NotChecked,
        InProgress,
        Available,
        Unavailable
    };

    MidiIdentityVerifier(MidiTransport& transport, double timeout = 2.0);
    ~MidiIdentityVerifier();

    void verify();
    void onVerified(std::function<void(MidiMessage&, Availability)> cb);

    Availability status() const noexcept;
    std::vector<unsigned char> identity() const noexcept;
    std::string name() const noexcept;
    std::string displayName() const noexcept;

    void operator()(MidiMessage& msg);

private:
    MidiTransport& m_transport;
    std::function<void(MidiMessage& msg, Availability)> m_verifyCallback;
    
    std::mutex m_mutex;
    std::vector<unsigned char> m_identity;
    Availability m_status{Availability::NotChecked};

    std::string m_deviceName;
    std::string m_displayName;
    
    double m_timeout;
    std::chrono::steady_clock::time_point m_verifyStart;
};


class MidiRecorder {
public:
    MidiRecorder(MidiTransport& transport);
    ~MidiRecorder() = default;

    void start();
    void stop();
    void add(const libremidi::message& msg);
    void clear();
    const std::vector<MidiMessage>& recorded() const noexcept;

    bool isRecording() const noexcept { return m_recording; }

private:
    MidiTransport& m_transport;
    std::atomic<bool> m_recording{false};
    std::mutex m_mutex;
    std::vector<MidiMessage> m_recorded;
};


class MidiDispatcher {
public:
    MidiDispatcher(MidiTransport& transport);
    ~MidiDispatcher() = default;

    void onMessage(std::function<void(MidiMessage&)> cb);

    void operator()(MidiMessage& msg);
private:
    MidiTransport& m_transport;
    std::function<void(MidiMessage&)> m_userCb;
};


class MidiDevice {
public:
    MidiDevice(libremidi::input_port inPort, libremidi::output_port outPort);
    ~MidiDevice();

    MidiDevice(const MidiDevice&) = delete;
    MidiDevice& operator=(const MidiDevice&) = delete;
    MidiDevice(MidiDevice&&) = delete;
    MidiDevice& operator=(MidiDevice&&) = delete;
    
    void startRecording();
    void stopRecording();
    const std::vector<libremidi::message>& recorded() const noexcept;

    void onMessage(std::function<void(MidiMessage&)> cb);
    void onVerified(std::function<void(MidiMessage&, MidiIdentityVerifier::Availability)> cb);
    
    MidiIdentityVerifier::Availability status() const noexcept;
    std::vector<unsigned char> identity() const noexcept;
    std::string name() const noexcept;
    std::string displayName() const noexcept;

    void open(libremidi::input_port inPort, libremidi::output_port outPort);
    void close();

    const libremidi::input_port& inPort() const noexcept;
    const libremidi::output_port& outPort() const noexcept;

private:
    void onMidiMessage(MidiMessage& msg);

    MidiTransport m_transport;
    MidiIdentityVerifier m_verifier;
    MidiRecorder m_recorder;
    MidiDispatcher m_dispatcher;
};



struct DeviceIdentifier {
    std::string manufacturerId;
    std::vector<unsigned char> deviceCode;
};


namespace MidiDeviceDB {
    const std::unordered_map<std::string, DeviceIdentifier> KNOWN_DEVICES = {
        { "Novation Launchpad Pro", {{0x00, 0x20, 0x29}, { 0x51 }}}
    };
}