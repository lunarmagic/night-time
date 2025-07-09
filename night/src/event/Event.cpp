
#include "nightpch.h"
#include "Event.h"

#include "utility.h"

namespace night
{
	Event::Event()
	{
		_timestamp = utility::window().precise_time_elapsed(); // TODO: may want imprecise time ellapsed.
	}
}