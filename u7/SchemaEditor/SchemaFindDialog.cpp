#include "SchemaFindDialog.h"

#include "Settings.h"

#include <VFrame30/SchemaItem.h>

SchemaFindDialog::SchemaFindDialog(bool replaceEnabled, QWidget* parent) :
	QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint),
	m_replaceEnabled(replaceEnabled)
{
	if (replaceEnabled == true)
	{
		setWindowTitle(tr("Find and Replace"));
	}
	else
	{
		setWindowTitle(tr("Find"));
	}

	// FindText/Replace - text for search
	//
	m_findTextEdit = new QLineEdit();

	QStringList completerStringList = QSettings{}.value("SchemaFindDialog/SearchCompleter").toStringList();
	m_findCompleter = new QCompleter(completerStringList, this);
	m_findCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findTextEdit->setCompleter(m_findCompleter);
	connect(m_findTextEdit,
			&QLineEdit::textEdited,
			this,
			[this]()
			{
				m_findCompleter->complete();
			});
	connect(m_findCompleter,
			static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted),
			m_findTextEdit,
			&QLineEdit::setText);

	if (replaceEnabled == true)
	{
		m_replaceTextEdit = new QLineEdit();

		completerStringList = QSettings{}.value("SchemaFindDialog/ReplaceCompleter").toStringList();
		m_replaceCompleter = new QCompleter(completerStringList, this);
		m_replaceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
		m_replaceTextEdit->setCompleter(m_replaceCompleter);
		connect(m_replaceTextEdit,
				&QLineEdit::textEdited,
				this,
				[this]()
				{
					m_replaceCompleter->complete();
				});
		connect(m_replaceCompleter,
				static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted),
				m_replaceTextEdit,
				&QLineEdit::setText);
	}

	// CaseSensivity check box
	//
	m_caseSensitiveCheckBox = new QCheckBox(tr("Case Sensitive"));
	m_caseSensitiveCheckBox->setChecked(theSettings.m_findSchemaItemCaseSensitive);

	// Find Result
	//
	m_findResult = new QTextEdit;
	m_findResult->setReadOnly(true);

	auto p = qApp->palette("QListView");

	QColor highlight = p.highlight().color();
	QColor highlightText = p.highlightedText().color();

	QString selectionColor =
		QString("QTextEdit { selection-background-color: %1; selection-color: %2; }").arg(highlight.name()).arg(highlightText.name());
	m_findResult->setStyleSheet(selectionColor);

	// Find buttons
	//
	m_prevButton = new QPushButton(tr("Find Previous"));
	m_nextButton = new QPushButton(tr("Find Next"));

	// m_nextButton->setShortcut(QKeySequence::FindNext);		// Done via Actions, works much faster
	// m_nextButton->setShortcutVisibleInContextMenu(true);
	// m_prevButton->setShortcut(QKeySequence::FindPrevious);	// Done via Actions, works much faster
	// m_prevButton->setShortcutVisibleInContextMenu(true);

	// Replace buttons
	//
	if (replaceEnabled == true)
	{
		m_replaceAllButton = new QPushButton(tr("Replace All"));
		m_replaceButton = new QPushButton(tr("Replace && Find"));

		connect(m_replaceButton, &QPushButton::clicked, this, &SchemaFindDialog::replaceAndFindPressed);
		connect(m_replaceAllButton, &QPushButton::clicked, this, &SchemaFindDialog::replaceAllPressed);
	}

	// --
	//
	QGridLayout* layout = new QGridLayout();

	layout->addWidget(new QLabel("Find:"), 0, 0, 1, 1);
	layout->addWidget(m_findTextEdit, 0, 1, 1, 3);

	if (replaceEnabled == true)
	{
		layout->addWidget(new QLabel("Replace with:"), 1, 0, 1, 1);
		layout->addWidget(m_replaceTextEdit, 1, 1, 1, 3);
	}

	layout->addWidget(m_caseSensitiveCheckBox, 2, 0, 1, 4);

	layout->addWidget(m_findResult, 3, 0, 1, 4);

	if (replaceEnabled == true)
	{
		layout->addWidget(m_replaceAllButton, 4, 0);
		layout->addWidget(m_replaceButton, 4, 1);
	}

	layout->addWidget(m_prevButton, 4, 2);
	layout->addWidget(m_nextButton, 4, 3);

	setLayout(layout);

	setMinimumWidth(300);

	QAction* nextAction = new QAction(tr("Find Next"), this);
	nextAction->setShortcut(QKeySequence::FindNext);
	nextAction->setShortcutVisibleInContextMenu(true);
	addAction(nextAction);

	QAction* prevAction = new QAction(tr("Find Prev"), this);
	prevAction->setShortcut(QKeySequence::FindPrevious);
	prevAction->setShortcutVisibleInContextMenu(true);
	addAction(prevAction);

	// Find buttons
	//
	connect(m_caseSensitiveCheckBox,
			&QCheckBox::toggled,
			this,
			[](bool checked)
			{
				theSettings.m_findSchemaItemCaseSensitive = checked;
			});

	auto findNextFunc = [this]()
	{
		saveFindCompleter();
		emit findNext(m_caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
	};

	auto findPrevFunc = [this]()
	{
		saveFindCompleter();
		emit findPrev(m_caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
	};

	connect(nextAction, &QAction::triggered, this, findNextFunc);
	connect(prevAction, &QAction::triggered, this, findPrevFunc);

	// --
	//
	connect(m_nextButton, &QPushButton::clicked, this, findNextFunc);
	connect(m_prevButton, &QPushButton::clicked, this, findPrevFunc);

	m_nextButton->setDefault(true);

	QSettings settings;
	restoreGeometry(settings.value("SchemaFindDialog/Geometry").toByteArray());

	ensureVisible();

	return;
}

SchemaFindDialog::~SchemaFindDialog()
{
	qDebug() << Q_FUNC_INFO;
}

QString SchemaFindDialog::findText() const
{
	assert(m_findTextEdit);

	QString text = m_findTextEdit->text().trimmed();

	return text;
}

void SchemaFindDialog::setFocusToEditLine()
{
	assert(m_findTextEdit);

	m_findTextEdit->setFocus();
	m_findTextEdit->selectAll();

	return;
}

void SchemaFindDialog::ensureVisible()
{
	if (QScreen* screen = QGuiApplication::screenAt(geometry().center()); screen == nullptr)
	{
		QScreen* newScreen = QGuiApplication::screens().at(0);

		if (QScreen* parentScreen = parentWidget()->window()->windowHandle()->screen(); parentScreen != nullptr)
		{
			newScreen = parentScreen;
		}

		QRect screenGeometry = newScreen->geometry();

		move(screenGeometry.left() + screenGeometry.width() / 2 - width() / 2,
			 screenGeometry.top() + screenGeometry.height() / 2 - height() / 2);
	}

	return;
}

bool SchemaFindDialog::replaceEnabled() const
{
	return m_replaceEnabled;
}

void SchemaFindDialog::updateCompleter()
{
	// Update completer
	//
	QString searchText = findText();

	if (theSettings.buildSearchCompleter().contains(searchText, Qt::CaseInsensitive) == false)
	{
		theSettings.buildSearchCompleter() << searchText;

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_findTextEdit->completer()->model());
		assert(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(theSettings.buildSearchCompleter());
		}
	}
}

void SchemaFindDialog::updateFoundInformation(SchemaItemPtr item,
											  const std::list<std::pair<QString, QString>>& foundProps,
											  QString /*searchText*/,
											  Qt::CaseSensitivity /*cs*/)
{
	QString itemCaption = QString(item->metaObject()->className());

	if (itemCaption.startsWith("VFrame30::SchemaItem") == true)
	{
		itemCaption.remove(0, 20); // 20 is length of VFrame30::SchemaItem
	}

	QString infoText = itemCaption + "\n";

	for (auto p : foundProps)
	{
		infoText.append(QString("%1 : %2\n").arg(p.first).arg(p.second));
	}

	m_findResult->setText(infoText);

	// To do: highlight all searchText with CaseSensitivity
	//

	return;
}

void SchemaFindDialog::replaceAndFindPressed()
{
	if (m_replaceTextEdit == nullptr)
	{
		Q_ASSERT(m_replaceTextEdit);
		return;
	}

	QString findText = m_findTextEdit->text();
	QString replaceWith = m_replaceTextEdit->text();
	Qt::CaseSensitivity cs = m_caseSensitiveCheckBox->isChecked() == true ? Qt::CaseSensitive : Qt::CaseInsensitive;

	if (findText.trimmed().isEmpty() == true)
	{
		return;
	}

	saveFindCompleter();
	saveReplaceCompleter();

	emit replaceAndFind(findText, replaceWith, cs);

	return;
}

void SchemaFindDialog::replaceAllPressed()
{
	if (m_replaceTextEdit == nullptr)
	{
		Q_ASSERT(m_replaceTextEdit);
		return;
	}

	QString findText = m_findTextEdit->text();
	QString replaceWith = m_replaceTextEdit->text();
	Qt::CaseSensitivity cs = m_caseSensitiveCheckBox->isChecked() == true ? Qt::CaseSensitive : Qt::CaseInsensitive;

	if (findText.trimmed().isEmpty() == true)
	{
		return;
	}

	saveFindCompleter();
	saveReplaceCompleter();

	emit replaceAll(findText, replaceWith, cs);

	return;
}

void SchemaFindDialog::closeEvent(QCloseEvent*)
{
	saveSettings();
}

void SchemaFindDialog::done(int r)
{
	saveSettings();
	QDialog::done(r);
}

void SchemaFindDialog::saveSettings()
{
	QSettings settings;
	settings.setValue("SchemaFindDialog/Geometry", saveGeometry());

	return;
}

void SchemaFindDialog::saveFindCompleter()
{
	QString findText = m_findTextEdit->text();
	if (findText.isEmpty() == true)
	{
		return;
	}

	QStringListModel* model = dynamic_cast<QStringListModel*>(m_findCompleter->model());
	if (model == nullptr)
	{
		assert(model != nullptr);
		return;
	}

	QStringList completerStringList = model->stringList();
	if (completerStringList.contains(findText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(findText);
		while (completerStringList.size() > 50)
		{
			completerStringList.pop_front();
		}
		QSettings{}.setValue("SchemaFindDialog/SearchCompleter", completerStringList);
		model->setStringList(completerStringList);
	}
}

void SchemaFindDialog::saveReplaceCompleter()
{
	if (m_replaceTextEdit == nullptr)
	{
		Q_ASSERT(m_replaceTextEdit);
		return;
	}


	QString replaceText = m_replaceTextEdit->text();
	if (replaceText.isEmpty() == true)
	{
		return;
	}

	QStringListModel* model = dynamic_cast<QStringListModel*>(m_replaceCompleter->model());
	if (model == nullptr)
	{
		assert(model != nullptr);
		return;
	}

	QStringList completerStringList = model->stringList();

	if (completerStringList.contains(replaceText, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(replaceText);
		while (completerStringList.size() > 50)
		{
			completerStringList.pop_front();
		}
		model->setStringList(completerStringList);
		QSettings{}.setValue("SchemaFindDialog/ReplaceCompleter", completerStringList);
	}
}
