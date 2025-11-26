
#include "types/types.h"
#include "stl/stl.h"
#include "math/Math.h"
#include "application/Application.h"
#include "ExampleSelect.h"
#include "entry.h"

namespace night
{
	//struct B // texture uniform buffer
	//{
	//	B() = default;
	//	B(s32 x) : i(x) { x = 0; };
	//	s32 i; // texture handle
	//	b8 x; // is depth buffer
	//
	//	//operator s32() const { return i; }
	//};

	//struct A // depth buffer
	//{
	//	A(s32 x) : i(x) {}
	//	s32 i;
	//
	//	operator B() const
	//	{
	//		B b;
	//		b.i = i;
	//		b.x = 1;
	//		return b;
	//	}
	//};
	
	//void fn(initializer_list<B> list)
	//{
	//	//for (s32 i = 0; i < list.size(); i++)
	//	for(const auto& i : list)
	//	{
	//		TRACE("{0}, {1}", i.i, i.x);
	//	}
	//}

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