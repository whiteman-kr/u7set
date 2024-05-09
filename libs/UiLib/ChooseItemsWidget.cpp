#include <UiLib/ChooseItemsWidget.h>

//
// ChooseItemsWidget
//
namespace UiLib
{
	QString ChooseItemsWidget::m_filterText = QString();


	ChooseItemsWidget::ChooseItemsWidget(const QStringList& tags, QWidget* parent) :
		QWidget(parent)
	{
		for (const QString& s : tags)
		{
			m_tagsWithDescriptions.push_back({s, QString()});
		}

		setupUi();

		fillTags();

		return;
	}

	ChooseItemsWidget::ChooseItemsWidget(const std::vector<std::pair<QString, QString>>& tagsWithDescriptions, QWidget* parent) :
		QWidget(parent),
		m_tagsWithDescriptions(tagsWithDescriptions)
	{
		setupUi();

		fillTags();

		return;
	}

	ChooseItemsWidget::~ChooseItemsWidget()
	{
		m_filterText = m_filterEdit->text();
	}

	QString ChooseItemsWidget::text() const
	{
		return m_textEdit->text();
	}

	void ChooseItemsWidget::setText(const QString& text)
	{
		static const auto re = QRegularExpression("\\W+");
		QStringList separators = re.match(text).capturedTexts();
		if (separators.contains(';') == true) 
		{
			m_separator = ';';
		}
		else 
		{
			m_separator = QChar::Space;
		}

		m_textEdit->blockSignals(true);
		m_textEdit->setText(text);
		m_textEdit->blockSignals(false);

		updateChecks(text);

		return;
	}

	bool ChooseItemsWidget::readOnly() const
	{
		return m_textEdit->isReadOnly();
	}

	void ChooseItemsWidget::setReadOnly(bool value)
	{
		m_textEdit->setReadOnly(value);
		m_okButton->setEnabled(value == false);
	}

	void ChooseItemsWidget::tagsTextChanged(const QString& text)
	{
		updateChecks(text);

		return;
	}

	void ChooseItemsWidget::tagsListItemChanged(QTreeWidgetItem* item, int column)
	{
		Q_UNUSED(item);
		Q_UNUSED(column);

		updateTags();

		return;
	}

	void ChooseItemsWidget::tagsListItemPressed(QTreeWidgetItem* item, int column)
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

	void ChooseItemsWidget::filterTextChanged(const QString& text)
	{
		Q_UNUSED(text);

		fillTags();

		return;
	}

	void ChooseItemsWidget::setupUi()
	{
		// TextEditor
		//
		m_textEdit = new QLineEdit();
		connect(m_textEdit, &QLineEdit::textChanged, this, &ChooseItemsWidget::tagsTextChanged);

		// Tags list
		//
		m_list = new QTreeWidget();
		m_list->setRootIsDecorated(false);

		QStringList l;
		l << tr("Item");
		l << tr("Description");
		m_list->setHeaderLabels(l);
		m_list->setColumnCount(static_cast<int>(l.size()));

		bool hasDescriptions = false;
		for (const auto& t : m_tagsWithDescriptions)
		{
			if (t.second.isEmpty() == false)
			{
				hasDescriptions = true;
				break;
			}
		}

		m_list->setHeaderHidden(hasDescriptions == false);
		m_list->setColumnHidden(1, hasDescriptions == false);

		connect(m_list, &QTreeWidget::itemChanged, this, &ChooseItemsWidget::tagsListItemChanged);
		connect(m_list, &QTreeWidget::itemPressed, this, &ChooseItemsWidget::tagsListItemPressed);

		// Buttons and Filter Layout
		//
		QHBoxLayout* buttonsLayout = new QHBoxLayout();

		m_filterEdit = new QLineEdit();
		m_filterEdit->setClearButtonEnabled(true);
		m_filterEdit->setPlaceholderText(tr("Filter"));
		m_filterEdit->setToolTip(tr("Start typing to filter items"));
		m_filterEdit->setText(m_filterText);
		connect(m_filterEdit, &QLineEdit::textEdited, this, &ChooseItemsWidget::filterTextChanged);
		buttonsLayout->addWidget(m_filterEdit);

		buttonsLayout->addStretch();

		m_okButton = new QPushButton(tr("OK"));
		connect(m_okButton,
				&QPushButton::clicked,
				[this]()
				{
					emit okPressed();
				});
		buttonsLayout->addWidget(m_okButton);

		m_cancelButton = new QPushButton(tr("Cancel"));
		connect(m_cancelButton,
				&QPushButton::clicked,
				[this]()
				{
					emit cancelPressed();
				});
		buttonsLayout->addWidget(m_cancelButton);

		// Main Layout
		//
		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->addWidget(m_textEdit);
		mainLayout->addWidget(new QLabel(tr("Predefined items:")));
		mainLayout->addWidget(m_list);
		mainLayout->addLayout(buttonsLayout);
		mainLayout->setContentsMargins(0, 0, 0, 0);

		return;
	}

	void ChooseItemsWidget::fillTags()
	{
		QString filterText = m_filterEdit->text();

		m_list->clear();

		for (const std::pair<QString, QString>& tagPair : m_tagsWithDescriptions)
		{
			if (filterText.isEmpty() == false)
			{
				if (tagPair.first.contains(filterText, Qt::CaseInsensitive) == false)
				{
					continue;
				}
			}

			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setText(0, tagPair.first);
			item->setText(1, tagPair.second);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(0, Qt::Unchecked);

			m_list->addTopLevelItem(item);
		}

		m_list->resizeColumnToContents(0);

		updateChecks(m_textEdit->text());

		return;
	}

	void ChooseItemsWidget::updateChecks(const QString& text)
	{
		// Get exitsing tags
		static const auto re = QRegularExpression("\\W+");
		QStringList textTags = text.split(re, Qt::SkipEmptyParts);
		for (QString& t : textTags)
		{
			t = t.trimmed();
		}

		// Check existing tags in list

		m_list->blockSignals(true);

		int count = m_list->topLevelItemCount();
		for (int i = 0; i < count; i++)
		{
			QTreeWidgetItem* item = m_list->topLevelItem(i);

			if (textTags.contains(item->text(0)) == true)
			{
				item->setCheckState(0, Qt::Checked);
			}
			else
			{
				item->setCheckState(0, Qt::Unchecked);
			}
		}

		m_list->blockSignals(false);

		return;
	}

	void ChooseItemsWidget::updateTags()
	{
		const QString& text = m_textEdit->text();

		// Get exitsing tags

		static const auto re = QRegularExpression("\\W+");
		QStringList tags = text.split(re, Qt::SkipEmptyParts);
		for (QString& t : tags)
		{
			t = t.trimmed();
		}

		// Build tag checks map

		std::map<QString, bool> tagsListState;

		int count = m_list->topLevelItemCount();
		for (int i = 0; i < count; i++)
		{
			QTreeWidgetItem* item = m_list->topLevelItem(i);
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

		for (const auto& it : tagsListState)
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
		m_textEdit->setText(resultTags.join(m_separator));
		m_textEdit->blockSignals(false);

		return;
	}
} // namespace SchemaClientLib