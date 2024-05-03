#include "DevToolsAppSettings.h"

namespace SchemaClientLib
{
	DevToolsAppSettings::DevToolsAppSettings(IDevToolsAppSettings& settings, QWidget* parent) :
		QWidget{parent},
		m_settings{settings}
	{
		m_textWidget = new QTextEdit{this};
		m_textWidget->setReadOnly(true);

		m_refreshButton = new QPushButton{tr("Refresh"), this};
		connect(m_refreshButton, &QPushButton::clicked, this, &DevToolsAppSettings::updateSettings);

		auto layout = new QVBoxLayout{this};

		layout->addWidget(m_textWidget);
		layout->addWidget(m_refreshButton);

		setLayout(layout);

		updateSettings();

		return;
	}

	void DevToolsAppSettings::updateSettings()
	{
		Q_ASSERT(m_textWidget);

		m_textWidget->clear();

		QString text;
		text.reserve(1024);

		text = "EquipmentID: " + m_settings.appEquipmentId() + "\n";

		auto settings = m_settings.settings();
		std::sort(begin(settings),
				  end(settings),
				  [](const auto& l, const auto& r)
				  {
					  return std::make_pair(l.section, l.key) < std::make_pair(r.section, r.key);
				  });

		QString currentSection;
		for (const auto& s : settings)
		{
			if (currentSection != s.section)
			{
				text += s.section + "\n";
				currentSection = s.section;
			}

			text += QString{"    %1: %2\n"}.arg(s.key, s.value);
		}

		m_textWidget->setText(text);

		return;
	}
}