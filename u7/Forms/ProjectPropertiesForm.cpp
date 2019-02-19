#include "../Forms/ProjectPropertiesForm.h"
#include "../../lib/PropertyEditorDialog.h"
#include <memory>

//
// ProjectProperties
//
ProjectProperties::ProjectProperties()
{
	auto p = ADD_PROPERTY_GETTER_SETTER(QString, Db::ProjectProperty::Description, true, ProjectProperties::description, ProjectProperties::setDescription);
	p->setDescription("Project description");

	p = ADD_PROPERTY_GETTER_SETTER(QString, Db::ProjectProperty::SuppressWarnings, true, ProjectProperties::suppressWarningsAsString, ProjectProperties::setSuppressWarnings);
	p->setDescription("Comma separated suppress warning list. Example: 4004, 4005, 2000");

	ADD_PROPERTY_GETTER_SETTER(bool, Db::ProjectProperty::UppercaseAppSignalId, true, ProjectProperties::uppercaseAppSignalId, ProjectProperties::setUppercaseAppSignalId);
	p->setDescription("Uppercase AppSignalIDs, to apply option reopen project is required");

	return;
}

bool ProjectProperties::load(QWidget* parent, DbController* db)
{
	if (db == nullptr)
	{
		assert(db);
		return false;
	}

	if (bool ok = db->getProjectProperty(Db::ProjectProperty::Description, &m_description, parent);
		ok == false)
	{
		return false;
	}

	QString suppressWarningsStr;
	if (bool ok = db->getProjectProperty(Db::ProjectProperty::SuppressWarnings, &suppressWarningsStr, parent);
		ok == false)
	{
		return false;
	}
	else
	{
		setSuppressWarnings(suppressWarningsStr);
	}

	if (bool ok = db->getProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, &m_uppercaseAppSignalId, parent);
		ok == false)
	{
		return false;
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

	if (bool ok = db->setProjectProperty(Db::ProjectProperty::Description, m_description, parent);
		ok == false)
	{
		return false;
	}

	if (bool ok = db->setProjectProperty(Db::ProjectProperty::SuppressWarnings, suppressWarningsAsString(), parent);
		ok == false)
	{
		return false;
	}

	if (bool ok = db->setProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, uppercaseAppSignalId(), parent);
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

bool ProjectProperties::uppercaseAppSignalId() const
{
	return m_uppercaseAppSignalId;
}

void ProjectProperties::setUppercaseAppSignalId(bool value)
{
	m_uppercaseAppSignalId = value;
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
	if (auto prop = propertyObject->propertyByCaption(Db::ProjectProperty::Description);
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
	if (auto prop = propertyObject->propertyByCaption(Db::ProjectProperty::SuppressWarnings);
		prop != nullptr)
	{
		prop->setReadOnly(!db->currentUser().isAdminstrator());
	}
	else
	{
		assert(prop);
	}

	// Only Administrator can edit @UppercaseAppSignalID@
	//
	bool uppercaseAppSignalIdOldValue = true;

	if (auto prop = propertyObject->propertyByCaption(Db::ProjectProperty::UppercaseAppSignalId);
		prop != nullptr)
	{
		uppercaseAppSignalIdOldValue = prop->value().toBool();
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

		bool showInformationBox = false;

		if (auto prop = propertyObject->propertyByCaption(Db::ProjectProperty::UppercaseAppSignalId);
			prop != nullptr &&
			prop->value().toBool() != uppercaseAppSignalIdOldValue)
		{
			showInformationBox = true;
		}

		if (showInformationBox == true)
		{
			QMessageBox::information(parent, qAppName(), "To apply option some properties project reopen can be required.");
		}
	}

	return true;
}
