#pragma once

#include "Event.h"
#include "math/math.h"
#include "input/EInput.h"

namespace night
{

	struct MouseButtonEvent : public Event
	{
		EMouse button() const { return _button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

		vec2 const& position() const { return _position; }

	protected:

		MouseButtonEvent(EMouse button, vec2 const& position) : _button(button), _position(position) {}

	private:

		EMouse _button;
		vec2 _position;
	};

	struct MouseButtonPressedEvent : public MouseButtonEvent
	{
		MouseButtonPressedEvent(EMouse button, vec2 const& position) : MouseButtonEvent(button, position) {}

		string to_string() const override
		{
			sstream ss;
			ss << "MouseButtonPressedEvent: button: " << (u32)button() << ", position: " << position().x << ", " << position().y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	struct MouseButtonReleasedEvent : public MouseButtonEvent
	{
		MouseButtonReleasedEvent(EMouse button, vec2 const& position) : MouseButtonEvent(button, position) {}

		string to_string() const override
		{
			sstream ss;
			ss << "MouseButtonReleasedEvent: button: " << (u32)button() << ", position: " << position().x << ", " << position().y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

	struct MouseMotionEvent : public Event
	{
		MouseMotionEvent(vec2 const& motion) : _motion(motion) {}

		//real x() const { return _x; }
		//real y() const { return _y; }

		vec2 const& motion() const { return _motion; }

		string to_string() const override
		{
			sstream ss;
			ss << "MouseMotionEvent: " << _motion.x << ", " << _motion.y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMotion)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:

		vec2 _motion{ vec2(0) };
	};

	struct MouseWheelEvent : public Event
	{
		MouseWheelEvent(real dx, real dy) : _dx(dx), _dy(dy) {}

		real dx() const { return _dx; }
		real dy() const { return _dy; }

		string to_string() const override
		{
			sstream ss;
			ss << "MouseWheelEvent: " << dx() << ", " << dy();
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseWheel)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:

		real _dx, _dy;
	};

}

