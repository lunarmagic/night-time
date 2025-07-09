#pragma once

#include "core.h"
#include "EInput.h"
//#include "stl/stl.h"

namespace night
{

struct InputKey
{
	union
	{
		struct
		{
			EKey key;
			EMouse mouse;
			EButton button;
			EInputType type;
		};

		u64 hash;
	};

	InputKey() = default;
	InputKey(EKey const& input, EInputType const& type = EInputType::Pressed) : key(input), mouse(EMouse::Invalid), button(EButton::Invalid), type(type) {};
	InputKey(EMouse const& input, EInputType const& type = EInputType::Pressed) : key(EKey::Invalid), mouse(input), button(EButton::Invalid), type(type) {};
	InputKey(EButton const& input, EInputType const& type = EInputType::Pressed) : key(EKey::Invalid), mouse(EMouse::Invalid), button(input), type(type) {};

	u8 operator==(const InputKey& rhs) const
	{
		return key == rhs.key && mouse == rhs.mouse && button == rhs.button && type == rhs.type;
	}
};

}

namespace std
{
	template<>
	struct hash<night::InputKey>
	{
		uint64_t operator()(const night::InputKey& key) const
		{
			return hash<uint64_t>()((uint64_t)key.hash);
		}
	};
};