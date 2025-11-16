
#include "types/types.h"
#include "stl/stl.h"
#include "math/Math.h"
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
		//{
		//	WARNING("TEST WARNING: {0}, {1}, {2}", "HI", 5, 2.0f);
		//	ERROR("TEST WARNING: {0}, {1}, {2}", "HI", 5, 2.0f);
		//
		//	vec4 v = vec4{ 35.0f, 1.5f, 67.344f, 12.3f };
		//	PRINT(v);
		//
		//	mat3x4 m = mat3x4(1);
		//	m[1][2] = 35.0f;
		//	PRINT(m);
		//
		//	quat q = quat(vec3(1, 4, 5));
		//	PRINT(q);
		//}

		ASSERT(root() != nullptr);
		_exampleSelect = root()->create<ExampleSelect>("Example Select");
	}
}


night::Application* night::create_application()
{
	return new night::Sandbox();
}