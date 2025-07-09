#pragma once

//#include "ref/ref.h"

//namespace night
//{
//
//	//template<typename T>
//	//using shandle = sref<T>;
//
//	//template<typename T>
//	//using handle = ref<T>;
//
//	template<typename T>
//	using shandle = T*;
//
//	template<typename T>
//	using handle = T*;
//
//}

namespace night
{
	//template<typename T>
	//using wref = std::weak_ptr<T>;

	template<typename T>
	using shandle = std::shared_ptr<T>;

	template<typename T>
	struct handle
	{
		handle() = default;

		handle(const handle<T>& other) = default;

		template<typename O>
		handle(const handle<O>& other)
		{
			_ptr = std::static_pointer_cast<T>(other.ptr().lock());
		}

		handle(shandle<T> t)
		{
			_ptr = t;
		}

		template<typename O>
		handle(shandle<O> other)
		{
			_ptr = std::static_pointer_cast<T>(other);
		}

		template<typename O>
		handle(const std::weak_ptr<O>& other)
		{
			_ptr = other;
		}

		handle(const nullptr_t& null)
		{
			_ptr = {};
		}

		template<typename O>
		void operator=(handle<O> other)
		{
			_ptr = std::static_pointer_cast<T>(other.ptr().lock());
		}

		void operator=(handle<T> other)
		{
			_ptr = other.ptr().lock();
		}

		template<typename O>
		void operator=(shandle<O> other)
		{
			_ptr = std::static_pointer_cast<T>(other);
		}

		void operator=(shandle<T> other)
		{
			_ptr = other;
		}

		void operator=(const std::nullptr_t& null)
		{
			_ptr = std::weak_ptr<T>();
		}

		T* operator->() const
		{
			return _ptr.lock().get();
		}

		T* operator->()
		{
			return _ptr.lock().get();
		}

		u8 operator==(const std::nullptr_t& null) const
		{
			return _ptr.expired();
		}

		u8 operator==(const handle<T>& other) const
		{
			if (_ptr.lock() == other._ptr.lock())
			{
				return true;
			}

			return false;
		}

		template<typename O>
		u8 operator==(const shandle<O>& other) const
		{
			return _ptr.lock() == other;
		}

		u8 operator!=(const std::nullptr_t& null) const
		{
			return !_ptr.expired();
		}

		u8 operator!=(const handle<T>& other) const
		{
			return _ptr.lock() != other._ptr.lock();
		}

		template<typename O>
		u8 operator!=(const handle<O>& other) const
		{
			return _ptr.lock() != other.ptr().lock();
		}

		T& operator*()
		{
			return *_ptr.lock();
		}

		u8 operator<(handle<T> other) const
		{
			return _ptr.lock().get() < other._ptr.lock().get();
		}

		inline operator u8 () { return !_ptr.expired(); }

		const std::weak_ptr<T>& ptr() const { return _ptr; }

		inline operator handle<T>() { return std::static_pointer_cast<T>(_ptr.lock()); }

		template<typename O>
		inline operator handle<O>() { return std::static_pointer_cast<O>(_ptr.lock()); }

		inline operator shandle<T>() { return std::static_pointer_cast<T>(_ptr.lock()); }

		template<typename O>
		inline operator shandle<O>() { return std::static_pointer_cast<O>(_ptr.lock()); }

	private:

		std::weak_ptr<T> _ptr;
	};
}

namespace std
{
	template<typename T>
	struct hash<night::handle<T>>
	{
		std::size_t operator()(const night::handle<T>& p) const noexcept
		{
			return (std::size_t)p.ptr().lock().get();
		}
	};
}