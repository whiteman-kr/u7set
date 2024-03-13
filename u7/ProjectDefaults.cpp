#include "ProjectDefaults.h"

const std::vector<ProjectDefaults::Property> ProjectDefaults::s_empty;


ProjectDefaults& ProjectDefaults::instance()
{
	static ProjectDefaults instance;
	return instance;
}

bool ProjectDefaults::update(DbController& db, QWidget* parentWidget)
{
	DbProjectProperties projectProperties;
	db.getProjectProperties(&projectProperties, parentWidget);

	ProjectDefaults& pd = ProjectDefaults::instance();
	bool pdParseOk = pd.parse(projectProperties.projectDefaults());

	if (pdParseOk == false && parentWidget != nullptr)
	{
		QMessageBox::critical(parentWidget,
							  qAppName(),
							  QObject::tr("Error parsing project property \"Project defaults\"."));
	}

	return pdParseOk;
}

bool ProjectDefaults::parse(const QString& value)
{
	m_defaults.clear();

	// Value is a simple INI file, we cannot use QSettings as it sorts the keys
	// and we need to keep the order to apply property values in the order,
	// it can be useful for setting ColumnCount for SchemaItemSignals and other schema items.
	//
	QStringList lines = value.split('\n', Qt::SkipEmptyParts);

	QString currentSection;

	for (QString line : lines)
	{
		// Find comments starting with // and remove them
		//
		line.replace(QRegularExpression(QStringLiteral("//.*")), QStringLiteral(""));

		line = line.trimmed();
		if (line.isEmpty() == true)
		{
			continue;
		}
		// If it is a section [], then create a new section
		//
		if (line.startsWith('[') == true && line.endsWith(']') == true)
		{
			currentSection = line.mid(1, line.size() - 2);
			m_defaults[currentSection].reserve(8);
		}
		else
		{
			// If it is a property, then add it to the current section
			//
			QStringList parts = line.split('=');
			if (parts.size() == 2)
			{
				QString propertyName = parts[0].trimmed();
				QString propertyValue = parts[1].trimmed();

				m_defaults[currentSection].emplace_back(propertyName, propertyValue);
			}
		}
	}

	return true;
}

const std::vector<ProjectDefaults::Property>& ProjectDefaults::values(const QString& section) const
{
	auto it = m_defaults.find(section);
	if (it != m_defaults.end())
	{
		return it->second;
	}

	return s_empty;
}

bool ProjectDefaults::hasSection(const QString& section) const
{
	return m_defaults.contains(section);
}
