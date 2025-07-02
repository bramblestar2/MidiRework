#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include <libremidi/libremidi.hpp>

class MidiTransport {
public:
    MidiTransport(libremidi::input_port inPort, libremidi::output_port outPort);
    ~MidiTransport();

    void open(libremidi::input_port inPort, libremidi::output_port outPort);
    void close();

    void send(const std::vector<unsigned char>& msg);
    void onReceived(std::function<void(libremidi::message&)> cb);

    void operator()(libremidi::message& msg);
private:
    std::mutex m_mutex;
    libremidi::midi_in m_midiIn;
    libremidi::midi_out m_midiOut;

    std::function<void(libremidi::message&)> m_userCb;
};


class MidiIdentityVerifier {
public:
    enum Availability {
        NotChecked,
        InProgress,
        Available,
        Unavailable
    };

    MidiIdentityVerifier(MidiTransport& transport);
    ~MidiIdentityVerifier() = default;

    void verify();
    void onVerified(std::function<void(libremidi::message&, Availability)> cb);

    Availability status() const noexcept;
    std::vector<unsigned char> identity() const noexcept;

    void operator()(libremidi::message& msg);

private:
    MidiTransport& m_transport;
    Availability m_status{Availability::NotChecked};
    std::vector<unsigned char> m_identity;
    std::function<void(libremidi::message& msg, Availability)> m_verifyCallback;
};


class MidiRecorder {
public:
    MidiRecorder(MidiTransport& transport);
    ~MidiRecorder() = default;

    void start();
    void stop();
    void add(const libremidi::message& msg);
    void clear();
    const std::vector<libremidi::message>& recorded() const noexcept;

    bool isRecording() const noexcept { return m_recording; }

private:
    MidiTransport& m_transport;
    std::atomic<bool> m_recording{false};
    std::mutex m_mutex;
    std::vector<libremidi::message> m_recorded;
};


class MidiDispatcher {
public:
    MidiDispatcher(MidiTransport& transport);
    ~MidiDispatcher() = default;

    void onMessage(std::function<void(libremidi::message&)> cb);

    void operator()(libremidi::message& msg);
private:
    MidiTransport& m_transport;
    std::function<void(libremidi::message&)> m_userCb;
};


class MidiDevice {
public:
    MidiDevice(libremidi::input_port inPort, libremidi::output_port outPort);
    ~MidiDevice();
    
    void startRecording();
    void stopRecording();
    const std::vector<libremidi::message>& recorded() const noexcept;

    void onMessage(std::function<void(libremidi::message&)> cb);
    void onVerified(std::function<void(libremidi::message&, MidiIdentityVerifier::Availability)> cb);

    void open(libremidi::input_port inPort, libremidi::output_port outPort);
    void close();

private:
    void onMidiMessage(libremidi::message& msg);

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