#include "../Forms/ProjectPropertiesForm.h"
#include "../../lib/PropertyEditorDialog.h"
#include <memory>

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
	std::shared_ptr<DbProjectProperties> propertyObject = std::make_shared<DbProjectProperties>();

	if (bool loadResult = db->getProjectProperties(propertyObject.get(), parent);
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
		if (bool saveResult = db->setProjectProperties(*propertyObject, parent);
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
