#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include <libremidi/libremidi.hpp>

class MidiTransport;
class MidiIdentityVerifier;
class MidiRecorder;
class MidiDispatcher;

class MidiDevice {
public:
    MidiDevice(libremidi::input_port inPort, libremidi::output_port outPort);
    MidiDevice();
    ~MidiDevice();
    
    void startRecording();
    void stopRecording();
    const std::vector<libremidi::message>& recorded() const noexcept;

    void onMessage(std::function<void(libremidi::message&)> cb);

    void open(libremidi::input_port inPort, libremidi::output_port outPort);
    void close();

private:
    void onMidiMessage(libremidi::message& msg);

    MidiTransport m_transport;
    MidiIdentityVerifier m_verifier;
    MidiRecorder m_recorder;
    MidiDispatcher m_dispatcher;
};


class MidiTransport {
public:
    MidiTransport();
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
    void onVerified(std::function<void(Availability)> cb);

    Availability status() const noexcept;
    std::vector<unsigned char> identity() const noexcept;

    void operator()(libremidi::message& msg);

private:
    MidiTransport& m_transport;
    Availability m_status{Availability::NotChecked};
    std::vector<unsigned char> m_identity;
    std::function<void(libremidi::message& msg)> m_verifyCallback;
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
    std::mutex m_mutex;
    bool m_recording{false};
    std::vector<libremidi::message> m_recorded;
};


class MidiDispatcher {
public:
    MidiDispatcher(MidiTransport& transport);
    ~MidiDispatcher() = default;

    void onMessage(std::function<void(libremidi::message&)> cb);

    void operator()(const libremidi::message& msg);
private:
    MidiTransport& m_transport;
    std::function<void(libremidi::message&)> m_userCb;
};