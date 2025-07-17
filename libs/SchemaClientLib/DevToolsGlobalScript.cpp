#include "DevToolsGlobalScript.h"
#include <UiLib/CodeEditor.h>

namespace SchemaClientLib
{
	DevToolsGlobalScript::DevToolsGlobalScript(IDevToolsGlobalScript& provider, QWidget* parent) :
		QWidget{parent},
		m_provider{provider}
	{
		m_codeEditor = new UiLib::CodeEditor(UiLib::CodeEditor::CodeType::JavaScript, this);

		m_refreshButton = new QPushButton{tr("Refresh"), this};
		connect(m_refreshButton, &QPushButton::clicked, this, &DevToolsGlobalScript::updateGlobalScript);

		m_applyButton = new QPushButton{tr("Apply"), this};
		connect(m_applyButton, &QPushButton::clicked, this, &DevToolsGlobalScript::applyGlobalScript);

		auto layout = new QGridLayout{this};

		layout->addWidget(m_codeEditor, 0, 0, 1, 2);
		layout->addWidget(m_refreshButton, 1, 0);
		layout->addWidget(m_applyButton, 1, 1);

		setLayout(layout);

		updateGlobalScript();

		return;
	}

	void DevToolsGlobalScript::updateGlobalScript()
	{
		Q_ASSERT(m_codeEditor);

		auto globalScript = m_provider.globalScript();
		m_codeEditor->setText(globalScript);

		return;
	}

	void DevToolsGlobalScript::applyGlobalScript()
	{
		Q_ASSERT(m_codeEditor);

		auto globalScript = m_codeEditor->toPlainText();
		m_provider.setGlobalScript(globalScript);

		return;
	}
}