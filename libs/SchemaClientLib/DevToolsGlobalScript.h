#pragma once

#include <SchemaClientLib/IDevTools.h>

namespace SchemaClientLib
{
	class DevToolsGlobalScript : public QWidget
	{
		Q_OBJECT

	public:
		DevToolsGlobalScript(IDevToolsGlobalScript& provider, QWidget* parent);

	protected slots:
		void updateGlobalScript();
		void applyGlobalScript();

	private:
		IDevToolsGlobalScript& m_provider;

		QTextEdit* m_textWidget = nullptr;

		QPushButton* m_refreshButton = nullptr;
		QPushButton* m_applyButton = nullptr;
	};
} // namespace SchemaClientLib
