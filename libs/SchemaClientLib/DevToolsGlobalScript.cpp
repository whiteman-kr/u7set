#include "DevToolsGlobalScript.h"

namespace SchemaClientLib
{
	DevToolsGlobalScript::DevToolsGlobalScript(IDevToolsGlobalScript& provider, QWidget* parent) :
		QWidget{parent},
		m_provider{provider}
	{
		m_textWidget = new QTextEdit{this};

		// Set up default font
		//
#ifdef Q_OS_WIN
		QFont font = QFont("Consolas", 11);
#else
		QFont font = QFont("Courier");
#endif
		m_textWidget->setFont(font);
		m_textWidget->setTabStopDistance(QFontMetrics{font}.horizontalAdvance(" ") * 4);

		m_refreshButton = new QPushButton{tr("Refresh"), this};
		connect(m_refreshButton, &QPushButton::clicked, this, &DevToolsGlobalScript::updateGlobalScript);

		m_applyButton = new QPushButton{tr("Apply"), this};
		connect(m_applyButton, &QPushButton::clicked, this, &DevToolsGlobalScript::applyGlobalScript);

		auto layout = new QGridLayout{this};

		layout->addWidget(m_textWidget, 0, 0, 1, 2);
		layout->addWidget(m_refreshButton, 1, 0);
		layout->addWidget(m_applyButton, 1, 1);

		setLayout(layout);

		updateGlobalScript();

		return;
	}

	void DevToolsGlobalScript::updateGlobalScript()
	{
		Q_ASSERT(m_textWidget);

		auto globalScript = m_provider.globalScript();
		m_textWidget->setText(globalScript);

		return;
	}

	void DevToolsGlobalScript::applyGlobalScript()
	{
		Q_ASSERT(m_textWidget);

		auto globalScript = m_textWidget->toPlainText();
		m_provider.setGlobalScript(globalScript);

		return;
	}
}