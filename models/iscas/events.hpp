#ifndef ISCAS_EVENTS_HPP
#define ISCAS_EVENTS_HPP

#include "warped.hpp"

#include <string>

class SignalEvent : public warped::Event {
public:
    SignalEvent() = default;
    SignalEvent(const std::string& receiver_name, unsigned int timestamp,
                unsigned int input_index, bool input_value)
        : receiver_name_(receiver_name), timestamp_(timestamp), input_index_(input_index),
          input_value_(input_value)
    {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return timestamp_; }

    std::string receiver_name_;
    unsigned int timestamp_;
    unsigned int input_index_;
    bool input_value_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(receiver_name_, timestamp_, input_index_, input_value_)
};

class ClockEvent : public warped::Event {
public:
    ClockEvent() = default;
    ClockEvent(const std::string& receiver_name, unsigned int timestamp)
        : receiver_name_(receiver_name), timestamp_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return timestamp_; }

    std::string receiver_name_;
    unsigned int timestamp_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(receiver_name_, timestamp_)
};

#endif
