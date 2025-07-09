#pragma once

//#include "handle/handle.h"
//#include "log/log.h"
#include "core.h"
#include "file/File.h"

namespace night
{

	struct NIGHT_API IResourceManager
	{
		//File settings;

	protected:

		friend struct Application;

		virtual void load_resources() = 0;
		virtual void on_load_resources() = 0;

		virtual void close() = 0;
		virtual void on_close() = 0;
	};

	struct NIGHT_API ResourceManager : public IResourceManager
	{

	protected:

		virtual void load_resources() override;
		virtual void on_load_resources() override;

		virtual void close() override;
		virtual void on_close() override;
	};

}