
#include "types/types.h"
#include "stl/stl.h"
#include "math/math.h"
#include "application/Application.h"
#include "ExampleSelect.h"
#include "entry.h"


namespace night
{
	struct Sandbox : public Application
	{
		Sandbox() = default;

	protected:

		virtual void on_initialized() override;

	private:

		handle<ExampleSelect> _exampleSelect;
	};

	void Sandbox::on_initialized()
	{
		ASSERT(root() != nullptr);
		_exampleSelect = root()->create<ExampleSelect>("Example Select");
	}
}


night::Application* night::create_application()
{
	return new night::Sandbox();
}