#ifndef EPIDEMIC_HPP_DEFINED
#define EPIDEMIC_HPP_DEFINED

#include "warped.hpp"
#include "Person.hpp"

WARPED_DEFINE_OBJECT_STATE_STRUCT(LocationState) {

    /* Person map */
    std::map <unsigned int, std::shared_ptr<Person>> person_map_;
};

class EpidemicEvent : public warped::Event {

    std::string receiver_name_;

    WARPED_REGISTER_SERIALIZABLE_MEMBERS(cereal::base_class<warped::Event>(this), receiver_name_, type_, ts_)
};

class LocationObject : public warped::SimulationObject {

    /* Create and send the refresh location state event */
    void refreshLocStateEvent( IntVTime currentTime );

    /* Create and send the data capture event */
    void sendCapturedData( IntVTime     currentTime,
                           unsigned int uninfectedNum,
                           unsigned int latentNum,
                           unsigned int incubatingNum,
                           unsigned int infectiousNum,
                           unsigned int asymptNum,
                           unsigned int recoveredNum );

    /* Update the intra-location disease spread and send related events */
    void diseaseEventAndDataCapture( map <unsigned int, Person *> *personMap,
                                     IntVTime currentTime );

    /* Create and send the diffusion trigger event */
    void triggerDiffusionEvent( IntVTime currentTime );

    /* Migrate person(s) to different location event */
    void migrateLocationEvent(  IntVTime currentTime, 
                                LocationState *locationState  );

    /* Location name */
    string locationName;

    /* Disease Random Number Generator */
    RandomNumGen *diseaseRandGen;

    /* Diffusion Random Number generator */
    RandomNumGen *diffusionRandGen;

    /* Disease model */
    DiseaseModel *diseaseModel;

    /* Diffusion Network */
    DiffusionNetwork *diffusionNetwork;

    /* Initial population */
    vector <Person *> *personVec;

    /* Data capture status */
    bool dataCaptureStatus;

    /* Intra-location disease updation count */
    unsigned int countIntraLocDiseaseUpdate;

    /* Location state refresh interval */
    unsigned int locStateRefreshInterval;

};

#endif
