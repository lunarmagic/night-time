#pragma once

//#include "stl/stl.h"

namespace night
{
	//template<typename T>
	//using wref = std::weak_ptr<T>;

	template<typename T>
	using sref = std::shared_ptr<T>;

	template<typename T>
	struct ref
	{
		ref() = default;

		ref(const ref<T>& other) = default;

		template<typename O>
		ref(const ref<O>& other)
		{
			_ptr = std::static_pointer_cast<T>(other.ptr().lock());
		}

		ref(sref<T> t)
		{
			_ptr = t;
		}

		template<typename O>
		ref(sref<O> other)
		{
			_ptr = std::static_pointer_cast<T>(other);
		}

		ref(const nullptr_t& null)
		{
			_ptr = {};
		}

		template<typename O>
		void operator=(ref<O> other)
		{
			_ptr = std::static_pointer_cast<T>(other.ptr().lock());
		}

		void operator=(ref<T> other)
		{
			_ptr = other.ptr().lock();
		}

		template<typename O>
		void operator=(sref<O> other)
		{
			_ptr = std::static_pointer_cast<T>(other);
		}

		void operator=(sref<T> other)
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

		u8 operator==(const ref<T>& other) const
		{
			if (_ptr.lock() == other._ptr.lock())
			{
				return true;
			}

			return false;
		}

		template<typename O>
		u8 operator==(const sref<O>& other) const
		{
			return _ptr.lock() == other;
		}

		u8 operator!=(const std::nullptr_t& null) const
		{
			return !_ptr.expired();
		}

		u8 operator!=(const ref<T>& other) const
		{
			return _ptr.lock() != other._ptr.lock();
		}

		template<typename O>
		u8 operator!=(const ref<O>& other) const
		{
			return _ptr.lock() != other.ptr().lock();
		}

		T& operator*()
		{
			return *_ptr.lock();
		}

		u8 operator<(ref<T> other) const
		{
			return _ptr.lock().get() < other._ptr.lock().get();
		}

		inline operator u8 () { return !_ptr.expired(); }

		const std::weak_ptr<T>& ptr() const { return _ptr; }

		inline operator ref<T>() { return std::static_pointer_cast<T>(_ptr.lock()); }

		template<typename O>
		inline operator ref<O>() { return std::static_pointer_cast<O>(_ptr.lock()); }

		inline operator sref<T>() { return std::static_pointer_cast<T>(_ptr.lock()); }

		template<typename O>
		inline operator sref<O>() { return std::static_pointer_cast<O>(_ptr.lock()); }

	private:

		std::weak_ptr<T> _ptr;
	};
}

namespace std
{
	template<typename T>
	struct hash<night::ref<T>>
	{
		std::size_t operator()(const night::ref<T>& p) const noexcept
		{
			return (std::size_t)p.ptr().lock().get();
		}
	};
}