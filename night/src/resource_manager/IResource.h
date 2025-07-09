#pragma once

#include "core.h"
#include "handle/handle.h"

namespace night
{

	struct NIGHT_API IResource : public std::enable_shared_from_this<IResource>
	{
		IResource(string id, string path);
		virtual void __tempInitHandle() = 0;

		// TODO: use proper handle from this
		handle<const IResource> handle_from_this_const() const;
		handle<IResource> handle_from_this();

		string const& id() const { return _id; }
		string const& path() const { return _path; }

	private:

		string _id;
		string _path;
	};

}