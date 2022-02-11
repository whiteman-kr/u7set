#include "DbTagsEditor.h"

DbTagsEditor::DbTagsEditor(DbController* dbController, QWidget* parent):
    PropertyTextEditor(parent)
{
    if (dbController == nullptr)
	{
		Q_ASSERT(dbController);
		return;
	}

    std::vector<std::pair<QString, QString>> tags;

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

    m_cw = new ChooseTagsWidget(tags, this);

    connect(m_cw, &ChooseTagsWidget::okPressed, this, &DbTagsEditor::okButtonPressed);
    connect(m_cw, &ChooseTagsWidget::cancelPressed, this, &DbTagsEditor::cancelButtonPressed);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_cw);
    mainLayout->setContentsMargins(0, 0, 0, 0);

	return;
}

DbTagsEditor::~DbTagsEditor()
{
	return;
}

QString DbTagsEditor::text() const
{
    return m_cw->text();
}

void DbTagsEditor::setText(const QString& text)
{
    m_cw->setText(text);

	return;
}

bool DbTagsEditor::readOnly() const
{
    return m_cw->readOnly();
}

void DbTagsEditor::setReadOnly(bool value)
{
    m_cw->setReadOnly(value);

    return;
}

bool DbTagsEditor::externalOkCancelButtons() const
{
	return false;
}
