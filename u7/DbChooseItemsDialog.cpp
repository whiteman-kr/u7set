#include "DbChooseItemsDialog.h"
#include "../Builder/DbMatsUsers.h"

//
// DbChooseItemsDialog
//
DbChooseItemsDialog* DbChooseItemsDialog::tagsEditor(DbController* dbController, QWidget* parent)
{
    std::vector<std::pair<QString, QString>> tagsWithDescriptions;

	std::vector<DbTag> dbTags;
	bool ok = dbController->getTags(&dbTags);
	if (ok == true)
	{
		tagsWithDescriptions.reserve(dbTags.size());

		for (const DbTag& dbt : dbTags)
		{
			tagsWithDescriptions.push_back({dbt.tag, dbt.description});
		}
	}

	return new DbChooseItemsDialog(tagsWithDescriptions, parent);
}

DbChooseItemsDialog* DbChooseItemsDialog::matsUsersEditor(DbController* dbController, QWidget* parent)
{
    std::vector<std::pair<QString, QString>> usersWithDescriptions;
	
	Builder::DbMatsUserStorage storage;

	QString errorCode;
	if (storage.load(dbController, errorCode) == true)
	{
		std::vector<OnlineLib::MatsUser> users = storage.users();
		for (const OnlineLib::MatsUser& user : users)
		{
			usersWithDescriptions.push_back({user.login(), user.description()});
		}
	}
	else
	{
		QMessageBox::critical(parent, qAppName(), tr("MATS users loading error!"));
	}

	return new DbChooseItemsDialog(usersWithDescriptions, parent);
}

DbChooseItemsDialog::DbChooseItemsDialog(const std::vector<std::pair<QString, QString>>& tagsWithDescriptions, QWidget* parent) :
	PropertyTextEditor(parent)
{
    m_tagsWidget = new UiLib::ChooseItemsWidget(tagsWithDescriptions, this);

    connect(m_tagsWidget, &UiLib::ChooseItemsWidget::okPressed, this, &DbChooseItemsDialog::okButtonPressed);
    connect(m_tagsWidget, &UiLib::ChooseItemsWidget::cancelPressed, this, &DbChooseItemsDialog::cancelButtonPressed);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tagsWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

	return;
}

DbChooseItemsDialog::~DbChooseItemsDialog()
{
	return;
}

QString DbChooseItemsDialog::text() const
{
    return m_tagsWidget->text();
}

void DbChooseItemsDialog::setText(const QString& text)
{
    m_tagsWidget->setText(text);
	return;
}

bool DbChooseItemsDialog::readOnly() const
{
    return m_tagsWidget->readOnly();
}

void DbChooseItemsDialog::setReadOnly(bool value)
{
    m_tagsWidget->setReadOnly(value);
	return;
}

bool DbChooseItemsDialog::externalOkCancelButtons() const
{
	return false;
}

bool DbChooseItemsDialog::isModified() const
{
	return false;
}
