#pragma once

#include "Event.h"
//#include "node/INode.h"


namespace night
{
	struct INode;

	struct NodeMovedEvent : public Event
	{
		NodeMovedEvent(handle<INode> old_parent, handle<INode> new_parent) : _oldParent(old_parent), _newParent(new_parent) {}

		string to_string() const override
		{
			sstream ss;
			ss << "NodeMovedEvent";
			return ss.str();
		}

		EVENT_CLASS_TYPE(NodeMoved)
		EVENT_CLASS_CATEGORY(EventCategoryNode)

		handle<INode> const& old_parent() const { return _oldParent; }
		handle<INode> const& new_parent() const { return _newParent; }

	private:

		handle<INode> _oldParent;
		handle<INode> _newParent;
	};
}