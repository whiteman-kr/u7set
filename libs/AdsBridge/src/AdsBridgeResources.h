#pragma once
#include <set>
#include <shared_mutex>
#include <string>

namespace AdsBridge
{
	class Resources
	{
	public:
		Resources() = default;
		~Resources() = default;

		Resources(const Resources&) = delete;
		Resources(Resources&&) = delete;
		Resources& operator=(const Resources&) = delete;
		Resources& operator=(Resources&&) = delete;

	public:
		const char* getString(const std::string& string) const;
		const char* getString(std::string&& string) const;

	private:
		// This is a cache for const char* pointers.
		// This is required to return const char* pointers to the caller.
		// The caller must not delete the pointers.
		//
		mutable std::shared_mutex m_stringTableMutex;
		mutable std::set<std::string> m_stringTable;
	};
} // namespace AdsBridge