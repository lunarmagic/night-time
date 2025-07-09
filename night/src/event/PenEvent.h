#pragma once

#include "Event.h"
#include "math/math.h"

namespace night
{

	struct PenEvent : public Event
	{
		PenEvent(u32 id) : _id(id) {};
		EVENT_CLASS_CATEGORY(EventCategoryPen)

		u32 const& id() const { return _id; }

	private:

		u32 _id{ 0 };
	};

	struct PenPressureEvent : public PenEvent
	{
		PenPressureEvent(real pressure, u32 id = 0) : PenEvent(id), _pressure(pressure) {}

		real const& pressure() const { return _pressure; }

		string to_string() const override
		{
			sstream ss;
			ss << "PenPressureEvent: " << pressure();
			return ss.str();
		}

		EVENT_CLASS_TYPE(PenPressure)

	private:

		real _pressure{ 0.0f };
	};

	struct PenInputEvent : public PenEvent
	{
		PenInputEvent(vec2 const& position, u8 is_eraser, u32 id = 0) : PenEvent(id), _position(position), _isEraser(is_eraser) {}

		u8 const& is_eraser() const { return _isEraser; }

		vec2 const& position() const { return _position; }

	private:

		vec2 _position{ vec2(0) };
		u8 _isEraser{ false };
	};

	struct PenDownEvent : public PenInputEvent
	{
		PenDownEvent(vec2 const& window_position, u8 is_eraser, u32 id = 0) : PenInputEvent(window_position, is_eraser, id) {}

		string to_string() const override
		{
			sstream ss;
			ss << "Pen Down, id: " << id() << ", position: " << position().x << ", " << position().y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(PenDown)
	};

	struct PenUpEvent : public PenInputEvent
	{
		PenUpEvent(vec2 const& window_position, u8 is_eraser, u32 id = 0) : PenInputEvent(window_position, is_eraser, id) {}

		string to_string() const override
		{
			sstream ss;
			ss << "Pen Up, id: " << id() << ", position: " << position().x << ", " << position().y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(PenUp)
	};

	struct PenMotionEvent : public PenEvent
	{
		PenMotionEvent(vec2 position, u32 id = 0) : PenEvent(id), _position(position) {}

		string to_string() const override
		{
			sstream ss;
			ss << "Pen Position: " << _position.x << ", " << _position.y;
			return ss.str();
		}

		EVENT_CLASS_TYPE(PenMotion)

		vec2 const& position() const { return _position; }

	private:

		vec2 _position{ vec2(0) };
	};

}