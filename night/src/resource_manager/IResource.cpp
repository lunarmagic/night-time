
#include "nightpch.h"
#include "IResource.h"

namespace night
{
	IResource::IResource(string id, string path)
	{
		_id = id;
		_path = path;
	}

	handle<const IResource> IResource::handle_from_this_const() const
	{
		return handle<const IResource>(shared_from_this());
	}
	handle<IResource> IResource::handle_from_this()
	{
		return handle<IResource>(shared_from_this());
	}
}