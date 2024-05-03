#pragma once

#include <SchemaClientLib/IDevTools.h>

namespace SchemaClientLib
{
	class DevToolsAppSettings : public QWidget
	{
		Q_OBJECT

	public:
		DevToolsAppSettings(IDevToolsAppSettings& settings, QWidget* parent);

	public:
		void updateSettings();

	private:
		IDevToolsAppSettings& m_settings;

		QTextEdit* m_textWidget = nullptr;
		QPushButton* m_refreshButton = nullptr;
	};

} // namespace SchemaClientLib
