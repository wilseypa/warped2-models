// An implementation of the synthetic PHOLD simulation model
// See "Performance of Time Warp Under Synthetic Workloads" by R.M. Fujimoto

#include<string>
#include<vector>
#include<memory>

#include "warped.hpp"
#include "tclap/ValueArg.h"

WARPED_DEFINE_OBJECT_STATE_STRUCT(PholdState) { }

class PholdEvent : public warped::Event {
public:
  PholdEvent();
  PholdEvent()(const std::string& receiver_name, const unsigned int timestamp,
               const std::string& creator_name)
    : creator_name(creator_name), receiver_name(receiver_name), timestamp(timestamp) {}

  const std::string& receiverName() const { return this->receiver_name; }
  unsigned int timestamp() const { return this->timestamp; }

protected:
  std::string creator_name;
  std::string receiever_name;
  unsigned int timestamp;

  WARPED_REGISTER_SERIALIZABLE_MEMBERS(creator_name, receiver_name, timestamp)
};

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(PholdEvent)

class PholdObject : public warped::SimulationObject {
public:
  PholdObject(const std::string& name, const unsigned int hold_mean,
              const unsigned int initial_events, const unsigned int num_objects)
    : name(name), state(), hold_mean(hold_mean), initial_events(initial_events),
      num_objects(num_objects) {}

  warped::ObjectState& getState() { return this->state; }

  std::vector<std::unique_ptr<warped::Event> > createInitialEvents() {
    std::vector<std::unique_ptr<warped::Event> > events;
    for (unsigned int i = 0; i < this->initial_events; i++) {
      events.emplace_back(new PholdEvent { this->get_destination(), this->get_timestamp(),
            this->name });
    }
    return events;
  }

  std::vector<std::unique_ptr<warped::Event> > receiveEvent(const warped::Event& event) {
    std::vector<std::unique_ptr<warped::Event> > response_events;
    auto received_event = static_cast<const PholdEvent&>(event);

    // Handle PHOLD events

    return response_events;
  }

protected:
  PholdState state;
  const unsigned int hold_mean;
  const unsigned int initial_events;
  const unsigned int num_objects;

  const std::string& get_destination() const {
    // TODO: Generate random destination LP
  }

  const unsigned int get_timestamp(const unsigned int exponential_mean) const {
    // TODO: Generate exponentially distributed random timestamp
  }
};

int main(int argc, const char** argv) {
  unsigned int hold_mean;
  unsigned int initial_events;
  unsigned int num_objects;

  return 0;
}
