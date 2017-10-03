#ifndef SYNTHETIC_HPP_DEFINED
#define SYNTHETIC_HPP_DEFINED

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "warped.hpp"
#include "distribution.hpp"


WARPED_DEFINE_LP_STATE_STRUCT(NodeState) {
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
                    const unsigned int floating_point_ops_cnt,
                    const unsigned int timestamp  )

        :   receiver_name_(receiver_name),
            floating_point_ops_cnt_(floating_point_ops_cnt),
            ts_(timestamp) {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return ts_; }

    unsigned int size() const {
        unsigned int size = sizeof(receiver_name_) +
                            sizeof(floating_point_ops_cnt_) +
                            sizeof(ts_);
        return size;
    }

    std::string receiver_name_;
    unsigned int floating_point_ops_cnt_;
    unsigned int ts_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this),
                                    receiver_name_, floating_point_ops_cnt_, ts_)
};

class Node : public warped::LogicalProcess {
public:
    Node (      const std::string name,
                const unsigned int num_nodes,
                const unsigned int floating_point_ops_cnt,
                const unsigned int state_size,
                const unsigned int index)
        :   LogicalProcess(name),
            state_(),
            rng_(new std::default_random_engine(index)),
            num_nodes_(num_nodes),
            floating_point_ops_cnt_(floating_point_ops_cnt),
            state_size_(state_size),
            index_(index) {

        state_.stream_.resize(state_size_, '0');
    }

    virtual std::vector<std::shared_ptr<warped::Event> > initializeLP() override;
    virtual std::vector<std::shared_ptr<warped::Event> > receiveEvent(const warped::Event&);
    virtual warped::LPState& getState() { return this->state_; }

    NodeState state_;

    static inline std::string lpName(const unsigned int);

    std::vector<std::string> adjacency_list_;

    Distribution *send_distribution_ = nullptr;

    Distribution *node_sel_distribution_ = nullptr; // Node select distribution

    std::shared_ptr<std::default_random_engine> rng_;

protected:
    const unsigned int num_nodes_;
    const unsigned int floating_point_ops_cnt_;
    const unsigned int state_size_;
    const unsigned int index_;
};

#endif
