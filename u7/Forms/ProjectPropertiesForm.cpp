#include "../Forms/ProjectPropertiesForm.h"
#include "../../lib/PropertyEditorDialog.h"
#include <memory>

//
// ProjectProperties
//
ProjectProperties::ProjectProperties()
{
	auto p = ADD_PROPERTY_GETTER_SETTER(QString, "Description", true, ProjectProperties::description, ProjectProperties::setDescription);
	p->setDescription("Project description");

	p = ADD_PROPERTY_GETTER_SETTER(QString, "SuppressWarnings", true, ProjectProperties::suppressWarningsAsString, ProjectProperties::setSuppressWarnings);
	p->setDescription("Comma separated suppress warning list. Example: 4004, 4005, 2000");

	return;
}

bool ProjectProperties::load(QWidget* parent, DbController* db)
{
	if (db == nullptr)
	{
		assert(db);
		return false;
	}

	if (bool ok = db->getProjectProperty("Description", &m_description, parent);
		ok == false)
	{
		return false;
	}

	QString suppressWarningsStr;
	if (bool ok = db->getProjectProperty("SuppressWarnings", &suppressWarningsStr, parent);
		ok == false)
	{
		return false;
	}
	else
	{
		setSuppressWarnings(suppressWarningsStr);
	}

	return true;
}

bool ProjectProperties::save(QWidget* parent, DbController* db)
{
	if (db == nullptr)
	{
		assert(db);
		return false;
	}

	if (bool ok = db->setProjectProperty("Description", m_description, parent);
		ok == false)
	{
		return false;
	}

	if (bool ok = db->setProjectProperty("SuppressWarnings", suppressWarningsAsString(), parent);
		ok == false)
	{
		return false;
	}

	return true;
}

QString ProjectProperties::description() const
{
	return m_description;
}

void ProjectProperties::setDescription(const QString& value)
{
	m_description = value;
}

std::vector<int> ProjectProperties::suppressWarnings() const
{
	return m_suppressWarnings;
}

QString ProjectProperties::suppressWarningsAsString() const
{
	QString result;
	for (int w : m_suppressWarnings)
	{
		if (result.isEmpty() == false)
		{
			result += QStringLiteral(", ");
		}

		result += QString::number(w);
	}

	return result;
}

void ProjectProperties::setSuppressWarnings(const QString& value)
{
	QStringList sl = value.split(QRegExp("\\W+"), QString::SkipEmptyParts);

	m_suppressWarnings.clear();
	m_suppressWarnings.reserve(sl.size());

	for (QString& sw : sl)
	{
		bool ok = false;
		int warning = sw.toInt(&ok);

		if (ok == true)
		{
			m_suppressWarnings.push_back(warning);
		}
	}

	return;
}

//
// ProjectPropertiesForm
//
bool ProjectPropertiesForm::show(QWidget* parent, DbController* db)
{
	if (db == nullptr)
	{
		assert(db);
		return false;
	}

	if (db->isProjectOpened() == false)
	{
		return false;
	}

	// Load from project db
	//
	std::shared_ptr<ProjectProperties> propertyObject = std::make_shared<ProjectProperties>();

	if (bool loadResult = propertyObject->load(parent, db);
		loadResult == false)
	{
		return false;
	}

	// Only Administrator can edit @Description@
	//
	if (auto prop = propertyObject->propertyByCaption("Description");
		prop != nullptr)
	{
		prop->setReadOnly(!db->currentUser().isAdminstrator());
	}
	else
	{
		assert(prop);
	}

	// Only Administrator can edit @SuppressWarnings@
	//
	if (auto prop = propertyObject->propertyByCaption("SuppressWarnings");
		prop != nullptr)
	{
		prop->setReadOnly(!db->currentUser().isAdminstrator());
	}
	else
	{
		assert(prop);
	}


	//--
	//
	PropertyEditorDialog dialog(parent);

	dialog.setObject(propertyObject);

	int result = dialog.exec();

	if (result == QDialog::Accepted)
	{
		// Save to project db
		//
		if (bool saveResult = propertyObject->save(parent, db);
			saveResult == false)
		{
			return false;
		}
	}

	return true;
}
