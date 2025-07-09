#pragma once

#include "Event.h"
#include "input/EInput.h"

namespace night
{

struct KeyEvent : public Event
{
	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

	EKey keycode() const { return _keycode; }

protected:

	KeyEvent(EKey keycode) : _keycode(keycode) {}

private:

	EKey _keycode; // TODO create Keycode class.
};

struct KeyPressedEvent : public KeyEvent
{
	KeyPressedEvent(EKey keycode, u8 isRepeat) : KeyEvent(keycode), _isRepeat(isRepeat) {}

	string to_string() const override
	{
		sstream ss;
		ss << "KeyPressedEvent: " << (u32)keycode() << " (repeat = " << isRepeat() << ")";
		return ss.str();
	}

	u8 isRepeat() const { return _isRepeat; }
	EVENT_CLASS_TYPE(KeyPressed)

private:

	u8 _isRepeat;
};

struct KeyReleasedEvent : public KeyEvent
{
	KeyReleasedEvent(EKey keycode) : KeyEvent(keycode) {}

	string to_string() const override
	{
		sstream ss;
		ss << "KeyReleasedEvent: " << (u32)keycode();
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased)
};

//class KeyTypedEvent : public KeyEvent
//{
//public:
//
//	KeyTypedEvent(u32 keycode) : KeyEvent(keycode) {}
//
//	string to_string() const override
//	{
//		sstream ss;
//		ss << "KeyTypedEvent: " << keycode();
//		return ss.str();
//	}
//
//	EVENT_CLASS_TYPE(KeyTyped)
//};

}

