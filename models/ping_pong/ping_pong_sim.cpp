#include <memory>
#include <string>
#include <vector>

#include "warped.hpp"
#include "tclap/ValueArg.h"

WARPED_DEFINE_OBJECT_STATE_STRUCT(PingPongState) {
    unsigned int balls_created_;
    unsigned int balls_received_;
    unsigned int balls_sent_;
};

class PingPongEvent : public warped::Event {
public:
    PingPongEvent() = default;
    PingPongEvent(const std::string& receiver_name, unsigned int timestamp,
                  const std::string& creator_name)
        : creator_name_(creator_name), receiver_name_(receiver_name), timestamp_(timestamp)
    {}

    const std::string& receiverName() const { return receiver_name_; }
    unsigned int timestamp() const { return timestamp_; }

    std::string creator_name_;
    std::string receiver_name_;
    unsigned int timestamp_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(creator_name_, receiver_name_, timestamp_)
};
WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PingPongEvent)


class PingPongObject : public warped::SimulationObject {
public:
    PingPongObject(const std::string& name, const std::string& target_name,
                   unsigned int balls_to_create)
        : SimulationObject(name), state_(), target_name_(target_name),
          balls_to_create_(balls_to_create) {}

    warped::ObjectState& getState() { return state_; }

    std::vector<std::unique_ptr<warped::Event>> createInitialEvents()  {
        std::vector<std::unique_ptr<warped::Event>> v;
        if (balls_to_create_ > 0) {
            state_.balls_created_++;
            state_.balls_sent_++;
            v.emplace_back(new PingPongEvent {target_name_, 1, name_});
        }
        return v;
    }


    std::vector<std::unique_ptr<warped::Event>> receiveEvent(const warped::Event& event) {
        std::vector<std::unique_ptr<warped::Event>> v;
        state_.balls_received_++;
        auto ping_event = static_cast<const PingPongEvent&>(event);
        std::string creator_name;

        if (ping_event.creator_name_ == name_) {
            if (state_.balls_created_ < balls_to_create_) {
                state_.balls_created_++;
                creator_name = name_;
            } else {
                return v;
            }
        } else {
            creator_name = ping_event.creator_name_;
        }
        auto timestamp = ping_event.timestamp_ + 1;
        v.emplace_back(new PingPongEvent {target_name_, timestamp, creator_name});
        state_.balls_sent_++;
        return v;
    }

    PingPongState state_;
    const std::string target_name_;
    const unsigned int balls_to_create_;
};


int main(int argc, char const* argv[]) {
    unsigned int num_balls = 10;
    unsigned int num_objects = 3;
    unsigned int num_balls_at_once = 1;
    TCLAP::ValueArg<unsigned int> num_balls_arg("b", "balls", "number of balls an object will create",
                                                false, num_balls, "int");
    TCLAP::ValueArg<unsigned int> num_objects_arg("o", "objects", "total number of objects",
                                                  false, num_objects, "int");
    TCLAP::ValueArg<unsigned int> num_at_once_arg("a", "num-at-once",
                                                  "number of objects that will create balls",
                                                  false, num_balls_at_once, "int");

    std::vector<TCLAP::Arg*> cmd_line_args = {&num_balls_arg, &num_objects_arg, &num_at_once_arg};

    warped::Simulation simulation {"Ping Pong Simulation", argc, argv, cmd_line_args};

    num_balls = num_balls_arg.getValue();
    num_objects = num_objects_arg.getValue();
    num_balls_at_once = num_at_once_arg.getValue();

    std::vector<PingPongObject> objects;

    for (unsigned int i = 0; i < num_objects; ++i) {
        std::string name = std::string("Object ") + std::to_string(i + 1);
        std::string target = std::string("Object ") + std::to_string(((i + 1) % num_objects) + 1);
        unsigned int balls = i < num_balls_at_once ? num_balls : 0;

        objects.emplace_back(name, target, balls);
    }

    std::vector<warped::SimulationObject*> object_pointers;
    for (auto& o : objects) {
        object_pointers.push_back(&o);
    }

    simulation.simulate(object_pointers);

    for (auto& ob : objects) {
        std::cout << ob.name_ << " received " << ob.state_.balls_received_ << ", sent " <<
                  ob.state_.balls_sent_ << ", created " << ob.state_.balls_created_ << "\n";
    }

    return 0;
}