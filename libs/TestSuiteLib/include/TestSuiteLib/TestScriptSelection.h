#pragma once

#include <QStringList>

#include <map>


namespace TestSuite
{
	class TestScriptSelection
	{
	public:
		TestScriptSelection() = default;
		explicit TestScriptSelection(const QString& testMasks);

	public:
		// Tests masks operations
		//
		[[nodiscard]] const QStringList& testMasks() const;

		// Selected files and tests operations
		//
		[[nodiscard]] bool isEmpty() const; // returns true if list of selected functions is empty
		[[nodiscard]] QStringList selectedFiles() const;

		const QStringList& selectedFunctions(const QString& scriptName) const;
		void setSelectedFunctions(const QString& scriptName, const QStringList& functons);

	private:
		QStringList m_testMasks;
		std::map<QString, QStringList> m_testFunctions; // Key is script filename, value is list of functions
	};
} // namespace TestSuite