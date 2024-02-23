#include "ProjectDefaults.h"
#include <QTemporaryFile>


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
		QMessageBox::critical(parentWidget, qAppName(), QObject::tr("Error parsing project property \"Project defaults\"."));
	}

	return pdParseOk;
}

bool ProjectDefaults::parse(const QString& value)
{
	m_defaults.clear();

	// Save value to a temporary file
	//
	QTemporaryFile iniFile;
	iniFile.setFileTemplate("u7_project_defaults_XXXXXX.ini");

	bool ok = iniFile.open();
	if (ok == false)
	{
		return false;
	}

	qDebug() << iniFile.fileName();
	iniFile.write(value.toUtf8());
	iniFile.close();

	// Read the setting from the temporary file
	//
	QSettings settings(iniFile.fileName(), QSettings::IniFormat);
	if (settings.status() != QSettings::NoError)
	{
		return false;
	}

	// Get the default settings and write them into the map m_defaults
	//
	QStringList sections = settings.childGroups();
	for (const QString& section : sections)
	{
		settings.beginGroup(section);

		QStringList keys = settings.childKeys();
		for (const QString& key : keys)
		{
			Key settingKey = std::make_pair(section, key);
			m_defaults[settingKey] = settings.value(key);
		}

		settings.endGroup();
	}

	return true;
}

QVariant ProjectDefaults::value(const QString& section, const QString& key) const
{
	QVariant result;

	auto it = m_defaults.find(std::make_pair(section, key));
	if (it != m_defaults.end())
	{
		result = it->second;
	}

	return result;
}
