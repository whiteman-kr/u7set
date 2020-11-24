#include "TagsEditor.h"

TagsEditor::TagsEditor(DbController* dbController, QWidget* parent):
	PropertyTextEditor(parent),
	m_parent(parent)
{
	if (m_parent == nullptr || dbController == nullptr)
	{
		Q_ASSERT(m_parent);
		Q_ASSERT(dbController);
		return;
	}

	// TextEditor
	//
	m_textEdit = new QLineEdit();
	connect(m_textEdit, &QLineEdit::textChanged, this, &TagsEditor::tagsTextChanged);

	// Tags list
	//
	m_tagsList = new QListWidget();

	QStringList tags;
	bool ok = dbController->getTags(&tags);


	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Could not read tags from the database!"));
	}
	else
	{
		tags.sort();

		for (const QString& tag :  tags)
		{
			QListWidgetItem* item = new QListWidgetItem(tag);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Unchecked);
			m_tagsList->addItem(item);
		}
	}

	connect(m_tagsList, &QListWidget::itemChanged, this, &TagsEditor::tagsListItemChanged);

	// Main Layout
	//
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(m_textEdit);
	mainLayout->addWidget(new QLabel("Predefined tags:"));
	mainLayout->addWidget(m_tagsList);
	mainLayout->setContentsMargins(0, 0, 0, 0);
}

TagsEditor::~TagsEditor()
{

}

QString TagsEditor::text() const
{
	return m_textEdit->text();
}

void TagsEditor::setText(const QString& text)
{
	m_textEdit->blockSignals(true);
	m_textEdit->setText(text);
	m_textEdit->blockSignals(false);

	updateChecks(text);

	return;
}

bool TagsEditor::readOnly() const
{
	return m_textEdit->isReadOnly();
}

void TagsEditor::setReadOnly(bool value)
{
	m_textEdit->setReadOnly(value);
}

void TagsEditor::tagsTextChanged(const QString& text)
{
	updateChecks(text);

	return;
}

void TagsEditor::tagsListItemChanged(QListWidgetItem*)
{
	updateTags();

	return;
}

void TagsEditor::updateChecks(const QString& text)
{
	// Get exitsing tags

	QStringList textTags = text.split(QRegExp("\\W+"), Qt::SkipEmptyParts);
	for (QString& t : textTags)
	{
		t = t.trimmed();
	}

	// Check existing tags in list

	m_tagsList->blockSignals(true);

	int count = m_tagsList->count();
	for (int i = 0; i < count; i++)
	{
		QListWidgetItem* item = m_tagsList->item(i);

		if (textTags.contains(item->text()) == true)
		{
			item->setCheckState(Qt::Checked);
		}
		else
		{
			item->setCheckState(Qt::Unchecked);
		}
	}

	m_tagsList->blockSignals(false);


	return;
}

void TagsEditor::updateTags()
{
	const QString& text = m_textEdit->text();

	// Get exitsing tags

	QStringList tags = text.split(QRegExp("\\W+"), Qt::SkipEmptyParts);
	for (QString& t : tags)
	{
		t = t.trimmed();
	}

	// Build tag checks map

	std::map<QString, bool> tagsListState;

	int count = m_tagsList->count();
	for (int i = 0; i < count; i++)
	{
		QListWidgetItem* item = m_tagsList->item(i);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			continue;
		}

		tagsListState[item->text()] = item->checkState() == Qt::Checked;
	}

	// Add manually added and previously checked tags to the result

	QStringList resultTags;

	for (const QString& tag : tags)
	{
		auto it = tagsListState.find(tag);
		if (it == tagsListState.end())
		{
			resultTags.push_back(tag);
		}
		else
		{
			bool checked = it->second;
			if (checked == true)
			{
				resultTags.push_back(tag);
			}
		}
	}

	// Add newly checked tags to the result

	for (auto it : tagsListState)
	{
		bool checked = it.second;
		if (checked == true)
		{
			const QString& tag = it.first;
			if (resultTags.contains(tag) == false)
			{
				resultTags.push_back(tag);
			}
		}
	}

	m_textEdit->blockSignals(true);
	m_textEdit->setText(resultTags.join(' '));
	m_textEdit->blockSignals(false);

	return;
}
