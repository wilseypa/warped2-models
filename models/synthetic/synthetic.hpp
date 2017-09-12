#ifndef Synthetic_HPP_DEFINED
#define Synthetic_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(SyntheticState) {
    std::string stream_;
};

class InternalEvent : public warped::Event {
public:
    InternalEvent() = default;
    InternalEvent(const unsigned int timestamp) : ts_(timestamp) {}

    const std::string& receiverName() const { return sender_name_; }
    unsigned int timestamp() const { return ts_; }

    unsigned int size() const { return sizeof(ts_); }

    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), ts_)
};

class ExternalEvent : public warped::Event {
public:
    ExternalEvent() = default;
    ExternalEvent(  const std::string receiver_name,
                    const unsigned int processing_delay,
                    const double percent_state_size_change,
                    const double percent_state_change,
                    const unsigned int timestamp  )

        :   receiver_name_(receiver_name),
            processing_delay_(processing_delay),
            percent_state_size_change_(percent_state_size_change),
            percent_state_change_(percent_state_change),
            ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    unsigned int size() const {
        unsigned int size = sizeof(receiver_name_) +
                            sizeof(processing_delay_) +
                            sizeof(percent_state_size_change_) +
                            sizeof(percent_state_change_) +
                            sizeof(ts_);
        return size;
    }

    std::string receiver_name_;
    unsigned int processing_delay_;
    double percent_state_size_change_;
    double percent_state_change_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this),
            receiver_name_, processing_delay_, percent_state_size_change_,
            percent_state_change_, ts_)
};

class Synthetic : public warped::LogicalProcess {
public:
    Synthetic(  const std::string name,
                const unsigned int num_nodes,
                const unsigned int mean_time,
                const unsigned int state_size,
                const double percent_state_size_change,
                const double percent_state_change,
                const unsigned int index)
        :   LogicalProcess(name),
            state_(),
            rng_(new std::default_random_engine(index)),
            num_nodes_(num_nodes),
            mean_time_(mean_time),
            state_size_(state_size),
            percent_state_size_change_(percent_state_size_change),
            percent_state_change_(percent_state_change),
            index_(index) {

        state_.stream_.resize(state_size_, '0');
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    SyntheticState state_;

    static inline std::string lpName(const unsigned int);

    std::vector<std::string> adjacency_list_;

protected:
    std::shared_ptr<std::default_random_engine> rng_;
    const unsigned int num_nodes_;
    const unsigned int mean_time_;
    const unsigned int state_size_;
    const double percent_state_size_change_;
    const double percent_state_change_;
    const unsigned int index_;
};

#endif
