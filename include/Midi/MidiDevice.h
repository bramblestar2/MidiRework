#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include <chrono>
#include <libremidi/libremidi.hpp>
#include <source_location>

#include "types.h"

class MidiTransport {
public:
    MidiTransport(libremidi::input_port inPort, libremidi::output_port outPort, MidiMessageCallback cb);
    MidiTransport(libremidi::input_port inPort, libremidi::output_port outPort);
    ~MidiTransport();

    const libremidi::input_port& inPort() const noexcept;
    const libremidi::output_port& outPort() const noexcept;

    void open(libremidi::input_port inPort, libremidi::output_port outPort);
    void close();

    void send(const std::vector<unsigned char>& msg);
    void onMidiMessage(MidiMessageCallback cb);
    void onErrorMessage(ErrorCallback cb);
    void onWarningMessage(WarningCallback cb);

    void operator()(MidiMessage& msg);
private:
    void handleMidiMessage(MidiMessage& msg);
    void handleErrorMessage(std::string_view info, const std::source_location&);
    void handleWarningMessage(std::string_view info, const std::source_location&);

    std::mutex m_mutex;
    libremidi::midi_in m_midiIn;
    libremidi::midi_out m_midiOut;

    libremidi::input_port m_inPort;
    libremidi::output_port m_outPort;

    MidiMessageCallback m_userCb;

    ErrorCallback m_errorCb;
    WarningCallback m_warningCb;
};


class MidiIdentityVerifier {
public:
    MidiIdentityVerifier(MidiTransport& transport, double timeout = 2.0);
    ~MidiIdentityVerifier();

    void verify();
    void onVerified(VerificationCallback cb);

    Availability status() const noexcept;
    std::vector<unsigned char> identity() const noexcept;
    std::string name() const noexcept;
    std::string displayName() const noexcept;

    void operator()(MidiMessage& msg);

private:
    MidiTransport& m_transport;
    VerificationCallback m_verifyCallback;
    
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
    const std::vector<MidiMessageRecord>& recorded() const noexcept;

    bool isRecording() const noexcept { return m_recording; }

private:
    MidiTransport& m_transport;
    std::atomic<bool> m_recording{false};
    std::mutex m_mutex;
    std::vector<MidiMessageRecord> m_recorded;

    std::chrono::steady_clock::time_point m_start;
};


class MidiDispatcher {
public:
    MidiDispatcher(MidiTransport& transport);
    ~MidiDispatcher() = default;

    void onMessage(MidiMessageCallback cb);

    void operator()(MidiMessage& msg);
private:
    MidiTransport& m_transport;
    MidiMessageCallback m_userCb;
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
    const std::vector<MidiMessageRecord>& recorded() const noexcept;

    void onMessage(MidiMessageCallback cb);
    void onVerified(VerificationCallback cb);
    
    Availability status() const noexcept;
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




// class TestDevice {
// public:
//     TestDevice();

//     virtual void startRecording() = 0;
//     virtual void stopRecording() = 0;
//     virtual const std::vector<libremidi::message>& recorded() const noexcept = 0;

//     virtual void onMessage(MidiMessageCallback cb) = 0;
//     virtual void onVerified(VerificationCallback cb) = 0;

//     Availability status() const noexcept;
//     std::vector<unsigned char> identity() const noexcept;
//     std::string name() const noexcept;
//     std::string displayName() const noexcept;

// protected:
//     void setName(std::string_view name) { m_name = name; }
//     void setDisplayName(std::string_view name) { m_displayName = name; }
//     void setStatus(Availability status) { m_status = status; }
//     void setIdentity(std::vector<unsigned char> identity) { m_identity = identity; }
    
// private:
//     std::string m_name;
//     std::string m_displayName;
//     Availability m_status{Availability::NotChecked};
//     std::vector<unsigned char> m_identity;

//     std::vector<MidiMessage> m_recorded;

//     MidiMessageCallback m_userCb;

// };



// class VirtualMidiDevice : public TestDevice {
// public:
//     VirtualMidiDevice();
//     ~VirtualMidiDevice();

//     void startRecording() override;
//     void stopRecording() override;
//     const std::vector<libremidi::message>& recorded() const noexcept override;

//     void onMessage(MidiMessageCallback cb) override;
//     void onVerified(VerificationCallback cb) override;

    
// private:
    
// };


// class TestMidiDevice : public TestDevice {
// public:
//     TestMidiDevice(libremidi::input_port inPort, libremidi::output_port outPort);
//     ~TestMidiDevice();

//     void startRecording() override;
//     void stopRecording() override;
//     const std::vector<libremidi::message>& recorded() const noexcept override;

//     void onMessage(MidiMessageCallback cb) override;
//     void onVerified(VerificationCallback cb) override;

//     void open(libremidi::input_port inPort, libremidi::output_port outPort);
//     void close();

//     const libremidi::input_port& inPort() const noexcept;
//     const libremidi::output_port& outPort() const noexcept;
// private:
//     MidiTransport m_transport;
//     MidiIdentityVerifier m_verifier;
//     MidiDispatcher m_dispatcher;
//     MidiRecorder m_recorder;
// };




struct DeviceIdentifier {
    std::string manufacturerId;
    std::vector<unsigned char> deviceCode;
};


namespace MidiDeviceDB {
    const std::unordered_map<std::string, DeviceIdentifier> KNOWN_DEVICES = {
        { "Novation Launchpad Pro", {{0x00, 0x20, 0x29}, { 0x51 }}}
    };
}