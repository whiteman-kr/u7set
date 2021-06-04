#include "TagsEditor.h"

QString TagsEditor::m_filterText = QString();

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
	m_tagsList = new QTreeWidget();
	m_tagsList->setRootIsDecorated(false);

	QStringList l;
	l << tr("Tag");
	l << tr("Description");
	m_tagsList->setHeaderLabels(l);
	m_tagsList->setColumnCount(l.size());

	connect(m_tagsList, &QTreeWidget::itemChanged, this, &TagsEditor::tagsListItemChanged);
	connect(m_tagsList, &QTreeWidget::itemPressed, this, &TagsEditor::tagsListItemPressed);

	// Buttons and Filter Layout
	//
	QHBoxLayout* buttonsLayout = new QHBoxLayout();

	m_filterEdit = new QLineEdit();
	m_filterEdit->setClearButtonEnabled(true);
	m_filterEdit->setPlaceholderText(tr("Filter"));
	m_filterEdit->setToolTip(tr("Start typing to filter tags"));
	m_filterEdit->setText(m_filterText);
	connect(m_filterEdit, &QLineEdit::textEdited, this, &TagsEditor::filterTextChanged);
	buttonsLayout->addWidget(m_filterEdit);

	buttonsLayout->addStretch();

	m_okButton = new QPushButton(tr("OK"));
	connect(m_okButton, &QPushButton::clicked, this, &TagsEditor::okButtonPressed);
	buttonsLayout->addWidget(m_okButton);

	m_cancelButton = new QPushButton(tr("Cancel"));
	connect(m_cancelButton, &QPushButton::clicked, this, &TagsEditor::cancelButtonPressed);
	buttonsLayout->addWidget(m_cancelButton);

	// Main Layout
	//
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(m_textEdit);
	mainLayout->addWidget(new QLabel("Predefined tags:"));
	mainLayout->addWidget(m_tagsList);
	mainLayout->addLayout(buttonsLayout);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Fill tags
	//
	bool ok = dbController->getTags(&m_dbTags);
	if (ok == true)
	{
		fillDbTags();
	}

	return;
}

TagsEditor::~TagsEditor()
{
	m_filterText = m_filterEdit->text();
	return;
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
	m_okButton->setEnabled(value == false);
}

bool TagsEditor::externalOkCancelButtons() const
{
	return false;
}

void TagsEditor::tagsTextChanged(const QString& text)
{
	updateChecks(text);

	return;
}

void TagsEditor::tagsListItemChanged(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(item);
	Q_UNUSED(column);

	updateTags();

	return;
}

void TagsEditor::tagsListItemPressed(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);

	if (item == nullptr)
	{
		Q_ASSERT(item);
		return;
	}

	if (item->checkState(0) == Qt::Checked)
	{
		item->setCheckState(0, Qt::Unchecked);
	}
	else
	{
		item->setCheckState(0, Qt::Checked);
	}

	return;
}

void TagsEditor::filterTextChanged(const QString& text)
{
	Q_UNUSED(text);

	fillDbTags();

	return;
}

void TagsEditor::fillDbTags()
{
	QString filterText = m_filterEdit->text();

	m_tagsList->clear();

	for (const DbTag& dbTag :  m_dbTags)
	{
		if (filterText.isEmpty() == false)
		{
			if (dbTag.tag.contains(filterText, Qt::CaseInsensitive) == false)
			{
				continue;
			}
		}

		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, dbTag.tag);
		item->setText(1, dbTag.description);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(0, Qt::Unchecked);

		m_tagsList->addTopLevelItem(item);
	}

	m_tagsList->resizeColumnToContents(0);

	updateChecks(m_textEdit->text());

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

	int count = m_tagsList->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_tagsList->topLevelItem(i);

		if (textTags.contains(item->text(0)) == true)
		{
			item->setCheckState(0, Qt::Checked);
		}
		else
		{
			item->setCheckState(0, Qt::Unchecked);
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

	int count = m_tagsList->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_tagsList->topLevelItem(i);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			continue;
		}

		tagsListState[item->text(0)] = item->checkState(0) == Qt::Checked;
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
