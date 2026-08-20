#pragma once
#include <utility>
#include <functional>
#include <print>
#include <Error.h>

namespace fire
{
	template <typename T, T invalid = T{}>
	class EnsureClose
	{
	public:
		using Deleter = std::function<void(T&)>;

		EnsureClose(T resource, Deleter func)
			:m_resource(std::move(resource)), m_deleter(std::move(func)), m_owns(true)
		{
			fire::verify(m_resource != invalid, "failed to acquire resource");
			std::println("resource obtained");
		}

		EnsureClose(EnsureClose&& other) noexcept
			:m_resource(std::move(other.m_resource)),
			m_deleter(std::move(other.m_deleter)),
			m_owns(other.m_owns)
		{
			other.m_owns = false;
		}

		EnsureClose& operator=(EnsureClose&& other) noexcept
		{
			if (this != &other)
			{
				cleanup();
				m_resource = std::move(other.m_resource);
				m_deleter = std::move(other.m_deleter);
				m_owns = other.m_owns;
				other.m_owns = false;
			}
			return *this;
		}

		EnsureClose(const EnsureClose&) = delete;
		EnsureClose& operator=(const EnsureClose&) = delete;

		~EnsureClose()
		{
			cleanup();
		}

		T& get() { return m_resource; }
		const T& get()const { return m_resource; }
		operator const T() const { return m_resource; }

	private:

		void cleanup()
		{
			if (m_owns && m_deleter)
			{
				std::println("resource deleted");
				m_deleter(m_resource);
			}
			m_owns = false;
		}

		T m_resource;
		Deleter m_deleter;
		bool m_owns;
	};

}