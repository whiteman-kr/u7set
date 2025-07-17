#pragma once

#include <TestSuiteLib/TestScriptsStorage.h>

namespace UiLib
{
	class CodeEditorWidget;
}

class TestViewTabPage : public QWidget
{
public:
	TestViewTabPage(const TestSuite::TestScript& script, QWidget* parent);
	~TestViewTabPage();

	void setScript(const TestSuite::TestScript& script);
	const TestSuite::TestScript& script() const;

	void scrollToFunction(const QString& functionName);


private:
	UiLib::CodeEditorWidget* m_codeEditor = nullptr;

	TestSuite::TestScript m_script;
};