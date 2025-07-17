#pragma once

#include <SchemaClientLib/IDevTools.h>

namespace UiLib
{
	class CodeEditor;
}

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

		UiLib::CodeEditor* m_codeEditor = nullptr;

		QPushButton* m_refreshButton = nullptr;
		QPushButton* m_applyButton = nullptr;
	};
} // namespace SchemaClientLib
