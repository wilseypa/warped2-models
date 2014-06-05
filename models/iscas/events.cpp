#include "events.hpp"
#include "warped.hpp"


//unfortunately, the cereal serialization library doesn't support registering classes in a header file
WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(SignalEvent);
WARPED_REGISTER_POLYMORPHIC_SERIALIZABLE_CLASS(ClockEvent);
