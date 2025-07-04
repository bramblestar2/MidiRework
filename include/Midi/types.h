#pragma once
#include <libremidi/libremidi.hpp>

using MidiMessage = libremidi::message;
using DeviceMidiMessageCallback = std::function<void(class MidiDevice*, MidiMessage&)>;
using DeviceRefreshCallback = std::function<void(std::vector<class MidiDevice*>)>;
using DeviceAddedCallback = std::function<void(class MidiDevice*)>;
using DeviceRemovedCallback = std::function<void(class MidiDevice*)>;