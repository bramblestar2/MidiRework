#pragma once
#include <libremidi/libremidi.hpp>
#include <chrono>
#include <source_location>

enum class Availability {
    NotChecked,
    InProgress,
    Virtual,
    Available,
    Unavailable,
    TimedOut
};

struct MidiMessageRecord {
    libremidi::message message;
    int64_t timestamp;
};

using MidiMessage = libremidi::message;
using MidiMessageCallback = std::function<void(MidiMessage&)>;
using DeviceMidiMessageCallback = std::function<void(class MidiDevice*, MidiMessage&)>;
using DeviceRefreshCallback = std::function<void(std::vector<class MidiDevice*>)>;
using DeviceAddedCallback = std::function<void(class MidiDevice*)>;
using DeviceRemovedCallback = std::function<void(class MidiDevice*)>;

using ErrorCallback = std::function<void(std::string_view, const std::source_location&)>;
using WarningCallback = std::function<void(std::string_view, const std::source_location&)>;

using VerificationCallback = std::function<void(MidiMessage&, Availability)>;