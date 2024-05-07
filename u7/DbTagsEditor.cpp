#include "DbTagsEditor.h"
#include "../Builder/DbMatsUsers.h"

//
// DbTagsEditor
//
DbTagsEditor* DbTagsEditor::tagsEditor(DbController* dbController, QWidget* parent)
{
	return new DbTagsEditor(dbController, true, false, parent);
}

DbTagsEditor* DbTagsEditor::matsUsersEditor(DbController* dbController, QWidget* parent)
{
	return new DbTagsEditor(dbController, false, true, parent);
}

DbTagsEditor::DbTagsEditor(DbController* dbController, bool showTags, bool showUsers, QWidget* parent) :
	PropertyTextEditor(parent)
{
	if (dbController == nullptr)
	{
		Q_ASSERT(dbController);
		return;
	}

    std::vector<std::pair<QString, QString>> tags;
	std::vector<OnlineLib::MatsUser> users;

    if (showTags == true)
	{
	    std::vector<DbTag> dbTags;
		bool ok = dbController->getTags(&dbTags);
		if (ok == true)
		{
			tags.reserve(dbTags.size());

			for (const DbTag& dbt : dbTags)
			{
				tags.push_back({dbt.tag, dbt.description});
			}
		}
	}

	if (showUsers == true)
	{
		Builder::DbMatsUserStorage storage;

		QString errorCode;
		if (storage.load(dbController, errorCode) == true)
		{
			users = storage.users();
		}
		else
		{
			QMessageBox::critical(this, qAppName(), tr("MATS users loading error!"));
		}
	}

    m_tagsWidget = new SchemaClientLib::ChooseTagsWidget(tags, users, ' ', this);

    connect(m_tagsWidget, &SchemaClientLib::ChooseTagsWidget::okPressed, this, &DbTagsEditor::okButtonPressed);
    connect(m_tagsWidget, &SchemaClientLib::ChooseTagsWidget::cancelPressed, this, &DbTagsEditor::cancelButtonPressed);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_tagsWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

	return;
}

DbTagsEditor::~DbTagsEditor()
{
	return;
}

QString DbTagsEditor::text() const
{
    return m_tagsWidget->text();
}

void DbTagsEditor::setText(const QString& text)
{
    m_tagsWidget->setText(text);
	return;
}

bool DbTagsEditor::readOnly() const
{
    return m_tagsWidget->readOnly();
}

void DbTagsEditor::setReadOnly(bool value)
{
    m_tagsWidget->setReadOnly(value);
	return;
}

bool DbTagsEditor::externalOkCancelButtons() const
{
	return false;
}

bool DbTagsEditor::isModified() const
{
	return false;
}
