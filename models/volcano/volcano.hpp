// A customized implementation of volcano model

#ifndef VOLCANO_HPP_DEFINED
#define VOLCANO_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"

WARPED_DEFINE_LP_STATE_STRUCT(VolcanoState) {
};

enum volcano_event_t {
};

class VolcanoEvent : public warped::Event {
public:
    VolcanoEvent() = default;
    VolcanoEvent(   const std::string& receiver_name, 
                    const volcano_event_t type, 
                    const unsigned int timestamp    )
        : receiver_name_(receiver_name), type_(type), ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    std::string receiver_name_;
    volcano_event_t type_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), receiver_name_, type_, ts_)
};

class Volcano : public warped::LogicalProcess {
public:
    Volcano(const std::string& name, const unsigned int index)
        :   LogicalProcess(name), 
            state_(), 
            rng_(new std::default_random_engine(index)),
            index_(index) {}

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    VolcanoState state_;

    static inline std::string lp_name(const unsigned int);

protected:
    std::shared_ptr<std::default_random_engine> rng_;
    const unsigned int index_;
};

#endif
