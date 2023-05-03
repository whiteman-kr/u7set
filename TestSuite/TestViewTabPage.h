#ifndef TESTVIEWTABPAGE_H
#define TESTVIEWTABPAGE_H

#include "../lib/CodeEditor.h"
#include "../TestSuiteLib/TestScriptsStorage.h"

class TestViewTabPage : public QWidget
{
public:
	TestViewTabPage(const TestSuite::TestScript& script, QWidget* parent);
	~TestViewTabPage();

	const TestSuite::TestScript& script() const;


private:
	CodeEditor* m_codeEditor = nullptr;

	TestSuite::TestScript m_script;
};

#endif // TESTVIEWTABPAGE_H
