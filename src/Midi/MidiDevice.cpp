#include "Midi/MidiDevice.h"

MidiDevice::MidiDevice(libremidi::input_port inPort, libremidi::output_port outPort)
    : m_transport()
    , m_verifier(m_transport)
    , m_recorder(m_transport)
    , m_dispatcher(m_transport)
{
    open(inPort, outPort);
}

void MidiDevice::open(libremidi::input_port inPort, libremidi::output_port outPort) {
    m_transport.open(inPort, outPort);
}

void MidiDevice::onMidiMessage(libremidi::message& msg) {
    // If not verified, verify first
    // If available
    //  - if recorder is recording, add an entry
    //  - if user has a callback, call it

    if (m_verifier.status() == MidiIdentityVerifier::Availability::NotChecked) {
        m_verifier.verify();
    }
    else if (m_verifier.status() == MidiIdentityVerifier::Availability::InProgress) {
        m_verifier(msg);
    } 
    else if (m_verifier.status() == MidiIdentityVerifier::Availability::Available) {
        if (msg.size() == 3) {
            if (m_recorder.isRecording()) {
                m_recorder.add(msg);
            }

            m_dispatcher(msg);
        }
    }
}


void MidiTransport::open(libremidi::input_port inPort, libremidi::output_port outPort) {
    m_midiIn.open_port(inPort);
    m_midiOut.open_port(outPort);
}

void MidiTransport::close() {
    m_midiIn.close_port();
    m_midiOut.close_port();
}