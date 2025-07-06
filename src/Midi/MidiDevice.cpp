#include "Midi/MidiDevice.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <regex>



namespace {
    std::unordered_map<std::string, int> device_type_count;

    std::mutex ns_mutex;


    void AddDeviceCount(std::string name) {
        std::lock_guard<std::mutex> lock(ns_mutex);
        
        if (device_type_count.find(name) == device_type_count.end()) device_type_count[name] = 0;

        device_type_count[name] += 1;
    }

    void RemoveDeviceCount(std::string name) {
        std::lock_guard<std::mutex> lock(ns_mutex);

        name = std::regex_replace(name, std::regex(" \\(\\d\\)+$"), "");

        if (device_type_count.find(name) == device_type_count.end()) return;

        if (device_type_count[name] > 0)
            device_type_count[name]--;

        if (device_type_count[name] == 0)
            device_type_count.erase(name);
    }


    const std::string GetNameWithCount(std::string name) {
        if (device_type_count.find(name) == device_type_count.end()) return name;

        return name + " (" + std::to_string(device_type_count.at(name)) + ")";
    }
    
    
    void ResetDeviceCount() noexcept {
        device_type_count.clear();
    }
}


MidiDevice::MidiDevice(libremidi::input_port inPort, libremidi::output_port outPort)
    : m_transport(inPort, outPort, [this](MidiMessage& msg) { 
        onMidiMessage(msg);
    })
    , m_verifier(m_transport)
    , m_recorder(m_transport)
    , m_dispatcher(m_transport)
{
    open(inPort, outPort);
}

MidiDevice::~MidiDevice() {
    close();
}

void MidiDevice::startRecording() {
    m_recorder.start();
}

void MidiDevice::stopRecording() {
    m_recorder.stop();
}

const std::vector<MidiMessageRecord>& MidiDevice::recorded() const noexcept {
    return m_recorder.recorded();
}

void MidiDevice::onMessage(MidiMessageCallback cb) {
    m_dispatcher.onMessage(cb);
}

void MidiDevice::onVerified(VerificationCallback cb) {
    m_verifier.onVerified(cb);
}

void MidiDevice::open(libremidi::input_port inPort, libremidi::output_port outPort) {
    m_transport.open(inPort, outPort);

    if (m_verifier.status() == Availability::NotChecked) {
        m_verifier.verify();
    }
}

void MidiDevice::close() {
    m_transport.close();
}

void MidiDevice::onMidiMessage(MidiMessage& msg) {
    // If not verified, verify first
    // If available
    //  - if recorder is recording, add an entry
    //  - if user has a callback, call it

    // if (msg.size() > 6 &&
    //     msg[0] == 0xF0 &&
    //     msg[1] == 0x7E &&
    //     msg[3] == 0x06 &&
    //     msg[4] == 0x02) {
    //     m_verifier(msg);
    // }

    if (m_verifier.status() == Availability::NotChecked) {
        m_verifier.verify();
    }
    else if (m_verifier.status() == Availability::InProgress) {
        m_verifier(msg);
    } 
    else if (m_verifier.status() == Availability::Available) {
        if (msg.size() == 3) {
            if (m_recorder.isRecording()) {
                m_recorder.add(msg);
            }

            m_dispatcher(msg);
        }
    }
}

Availability MidiDevice::status() const noexcept {
    return m_verifier.status();
}

std::vector<unsigned char> MidiDevice::identity() const noexcept {
    return m_verifier.identity();
}

std::string MidiDevice::name() const noexcept {
    return m_verifier.name();
}

std::string MidiDevice::displayName() const noexcept {
    return m_verifier.displayName();
}

const libremidi::input_port& MidiDevice::inPort() const noexcept {
    return m_transport.inPort();
}

const libremidi::output_port& MidiDevice::outPort() const noexcept {
    return m_transport.outPort();
}


MidiTransport::MidiTransport(libremidi::input_port inPort, 
                             libremidi::output_port outPort, 
                             MidiMessageCallback cb)
    : m_midiIn(libremidi::input_configuration{
        .on_message = [this](MidiMessage msg) { this->handleMidiMessage(msg); },
        .on_error = std::bind(&MidiTransport::handleErrorMessage, this, std::placeholders::_1, std::placeholders::_2),
        .on_warning = std::bind(&MidiTransport::handleWarningMessage, this, std::placeholders::_1, std::placeholders::_2),
        .ignore_sysex = false,
        .ignore_timing = false,
        .ignore_sensing = true,
    })
    , m_midiOut(libremidi::output_configuration{})
    , m_userCb(cb)
    , m_inPort(inPort)
    , m_outPort(outPort)
{

}

MidiTransport::MidiTransport(libremidi::input_port inPort, libremidi::output_port outPort)
    : m_midiIn(libremidi::input_configuration{
        .on_message = [this](MidiMessage msg) { this->handleMidiMessage(msg); },
        .on_error = std::bind(&MidiTransport::handleErrorMessage, this, std::placeholders::_1, std::placeholders::_2),
        .on_warning = std::bind(&MidiTransport::handleWarningMessage, this, std::placeholders::_1, std::placeholders::_2),
        .ignore_sysex = false,
        .ignore_timing = false,
        .ignore_sensing = true,
    })
    , m_midiOut(libremidi::output_configuration{})
    , m_inPort(inPort)
    , m_outPort(outPort)
{
}

MidiTransport::~MidiTransport() {
    close();
}

const libremidi::input_port& MidiTransport::inPort() const noexcept {
    return m_inPort;
}

const libremidi::output_port& MidiTransport::outPort() const noexcept {
    return m_outPort;
}

void MidiTransport::open(libremidi::input_port inPort, libremidi::output_port outPort) {
    close();

    m_midiIn.open_port(inPort);
    m_midiOut.open_port(outPort);
}

void MidiTransport::close() {
    m_midiIn.close_port();
    m_midiOut.close_port();
}

void MidiTransport::send(const std::vector<unsigned char>& msg) {
    m_midiOut.send_message(msg);
}

void MidiTransport::onMidiMessage(MidiMessageCallback cb) {
    m_userCb = cb;
}

void MidiTransport::operator()(MidiMessage& msg) {
    m_userCb(msg);
}

void MidiTransport::handleMidiMessage(MidiMessage& msg) {
    if (m_userCb) {
        m_userCb(msg);
    }
}

void MidiTransport::handleErrorMessage(std::string_view info, const libremidi::source_location& source) {
    spdlog::error("(File({}) | Ln({})) Midi Error: {}", source.file_name(), source.line(), info);
    if (m_errorCb) {
        m_errorCb(info, source);
    }
}

void MidiTransport::handleWarningMessage(std::string_view info, const libremidi::source_location& source) {
    spdlog::warn("(File({}) | Ln({})) Midi Error: {}", source.file_name(), source.line(), info);
    if (m_warningCb) {
        m_warningCb(info, source);
    }
}


MidiIdentityVerifier::MidiIdentityVerifier(MidiTransport& transport, double timeout) 
    : m_transport(transport)
    , m_timeout(timeout)
{
}

MidiIdentityVerifier::~MidiIdentityVerifier() {
    RemoveDeviceCount(m_deviceName);
}

void MidiIdentityVerifier::verify() {
    m_transport.send({0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7});
    
    m_status = Availability::InProgress;
    m_verifyStart = std::chrono::steady_clock::now();
}

void MidiIdentityVerifier::onVerified(std::function<void(MidiMessage& msg, Availability)> cb) {
    m_verifyCallback = cb;
}

Availability MidiIdentityVerifier::status() const noexcept {
    return m_status;
}

std::vector<unsigned char> MidiIdentityVerifier::identity() const noexcept {
    return m_identity;
}

std::string MidiIdentityVerifier::name() const noexcept {
    return m_deviceName;
}

std::string MidiIdentityVerifier::displayName() const noexcept {
    return m_displayName;
}

void MidiIdentityVerifier::operator()(MidiMessage& msg) {
    if (m_status == Availability::InProgress) {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - m_verifyStart).count() > m_timeout) {
            m_status = Availability::TimedOut;
        }
    }

    if (msg.size() < 6 ||
        msg[0] != 0xF0 ||
        msg[1] != 0x7E ||
        msg[3] != 0x06 ||
        msg[4] != 0x02) {
        return;
    }

    constexpr size_t headerSize = 5, footerSize = 1;
    if (msg.size() < headerSize + footerSize) {
        spdlog::error("Message too short.");
        return;
    }

    auto payloadBegin = msg.begin() + headerSize;
    auto payloadEnd = msg.end() - footerSize;

    Availability status = Availability::Unavailable;

    for (auto const& [deviceName, id] : MidiDeviceDB::KNOWN_DEVICES) {
        const auto& man = id.manufacturerId;
        const auto& dev = id.deviceCode;

        if (std::distance(payloadBegin, payloadEnd) <
            static_cast<std::ptrdiff_t>(man.size() + dev.size())) {
            spdlog::debug(deviceName + ": payload too small");
            continue;
        }

        bool manOk = std::equal(
            payloadBegin,
            payloadBegin + man.size(),
            man.begin()
        );

        if (!manOk) {
            spdlog::warn(deviceName + ": manufacturer mismatch\n");
            continue;
        }

        bool devOk = std::equal(
            payloadBegin + man.size(),
            payloadBegin + man.size() + dev.size(),
            dev.begin()
        );

        if (devOk) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_status = Availability::Available;
            m_displayName = deviceName;

            m_deviceName = GetNameWithCount(deviceName);
            AddDeviceCount(deviceName);

            m_identity = std::vector<unsigned char>(payloadBegin, payloadEnd);
            break;
        } else {
            spdlog::warn(deviceName + ": device mismatch\n");
            m_deviceName = "Unknown";
            m_displayName = "Unknown";
            continue;
        }
    }

    if (m_verifyCallback) {
        m_verifyCallback(msg, this->status());
    }
}


MidiRecorder::MidiRecorder(MidiTransport& transport) 
    : m_transport(transport) 
{}

void MidiRecorder::start() {
    m_recording = true;
    m_start = std::chrono::steady_clock::now();
}

void MidiRecorder::stop() {
    m_recording = false;
}

void MidiRecorder::add(const MidiMessage& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    MidiMessageRecord record;
    record.message = msg;
    auto now = std::chrono::steady_clock::now();
    record.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();

    m_recorded.push_back(record);
}

void MidiRecorder::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_recorded.clear();
}

const std::vector<MidiMessageRecord>& MidiRecorder::recorded() const noexcept {
    return m_recorded;
}


MidiDispatcher::MidiDispatcher(MidiTransport& transport) 
    : m_transport(transport) 
{}

void MidiDispatcher::onMessage(MidiMessageCallback cb) {
    m_userCb = cb;
}

void MidiDispatcher::operator()(MidiMessage& msg) {
    m_userCb(msg);
}