#include "warped.hpp"
#include "epidemic.hpp"

WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(EpidemicEvent)

std::vector<std::shared_ptr<warped::Event> > Location::createInitialEvents() {

    std::vector<std::shared_ptr<warped::Event> > events;

    events.emplace_back(new EpidemicEvent {this->location_name_, 
                    this->location_state_refresh_interval_, nullptr, DISEASE_UPDATE_TRIGGER});
    events.emplace_back(new EpidemicEvent {this->location_name_, 
                    this->location_diffusion_trigger_interval_, nullptr, DIFFUSION_TRIGGER});

    return events;
}

std::vector<std::shared_ptr<warped::Event> > Location::receiveEvent(const warped::Event& event) {

    std::vector<std::shared_ptr<warped::Event> > events;
    auto epidemic_event = static_cast<const EpidemicEvent&>(event);
    auto timestamp = epidemic_event.loc_arrival_timestamp_;

    switch (epidemic_event.event_type_) {

        case event_type_t::DISEASE_UPDATE_TRIGGER: {
            disease_model_->reaction(state_.current_population_, timestamp);
            events.emplace_back(new EpidemicEvent {location_name_, 
                                timestamp + location_state_refresh_interval_, 
                                nullptr, DISEASE_UPDATE_TRIGGER});
        } break;

        case event_type_t::DIFFUSION_TRIGGER: {
            // TODO: add the diffusion selection
            events.emplace_back(new EpidemicEvent {location_name_, 
                                timestamp + location_diffusion_trigger_interval_, 
                                nullptr, DIFFUSION_TRIGGER});
        } break;

        case event_type_t::DIFFUSION: {
            std::shared_ptr<Person> person = std::make_shared<Person>(
                        epidemic_event->pid_, epidemic_event->susceptibility_, 
                        epidemic_event->vaccination_status_, epidemic_event->infection_state_,
                        timestamp, epidemic_event->prev_state_change_timestamp_);
            state_.current_population_.insert( 
                std::pair <unsigned int, std::shared_ptr<Person>> (epidemic_event->pid_, person));
        } break;

        default: {}
    }
    return response_events;
}

void Location::updateIntraLocDiseaseSpread(
                std::map<unsigned int, std::shared_ptr<Person>> population, 
                unsigned int current_time) {

}

int main(int argc, const char** argv) {

    return 0;
}
