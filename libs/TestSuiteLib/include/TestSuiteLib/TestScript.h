#pragma once

#include <QString>

#include <type_traits>

namespace TestSuite
{
	class TestScript
	{
	public:
		TestScript() = default;
		explicit TestScript(const QString& name, const QString& contents);

	public:
		[[nodiscard]] Hash fileNameHash() const;

		[[nodiscard]] const QString fileName() const;
		[[nodiscard]] const QString shortFileName() const;
		void setFileName(const QString& name);

		[[nodiscard]] const QString& script() const;
		void setScript(const QString& contents);

		[[nodiscard]] bool isGlobalScript() const;

		static inline const QString GlobalScriptID = "GlobalScript";

	private:
		Hash m_fileNameHash{UNDEFINED_HASH};
		QString m_fileName;
		QString m_script;
	};

	static_assert(std::is_copy_constructible_v<TestScript>);
	static_assert(std::is_move_constructible_v<TestScript>);
	static_assert(std::is_copy_assignable_v<TestScript>);
	static_assert(std::is_move_assignable_v<TestScript>);
	static_assert(std::is_nothrow_move_constructible_v<TestScript>);
	static_assert(std::is_nothrow_move_assignable_v<TestScript>);
} // namespace TestSuite
