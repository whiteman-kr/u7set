#ifndef TESTVIEWTABPAGE_H
#define TESTVIEWTABPAGE_H

#include "../lib/CodeEditor.h"
#include "../TestSuiteLib/TestScriptsStorage.h"

class TestViewTabPage : public QWidget
{
public:
	TestViewTabPage(const TestSuite::TestScript& script, QWidget* parent);
	~TestViewTabPage();

	void setScript(const TestSuite::TestScript& script);
	const TestSuite::TestScript& script() const;

	void scrollToFunction(const QString& functionName);


private:
	CodeEditor* m_codeEditor = nullptr;

	TestSuite::TestScript m_script;
};

#endif // TESTVIEWTABPAGE_H
