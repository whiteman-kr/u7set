#include "Settings.h"
#include "IdePropertyEditor.h"
#include "SpecificPropertiesEditor.h"
#include "SvgEditor.h"

//
// IdePropertyEditorHelper
//

ExtWidgets::PropertyTextEditor* IdePropertyEditorHelper::createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent, DbController* dbController)
{
	if (propertyPtr == nullptr || parent == nullptr)
	{
		Q_ASSERT(propertyPtr);
		Q_ASSERT(parent);
		return new ExtWidgets::PropertyPlainTextEditor(parent);
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::TuningFilter)
	{
		// This is Filters Editor for TuningClient
		//
		if (dbController == nullptr)
		{
			Q_ASSERT(dbController);
			return new ExtWidgets::PropertyPlainTextEditor(parent);
		}

		IdeTuningFiltersEditor* editor = new IdeTuningFiltersEditor(dbController, parent);
		return editor;
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::SpecificPropertyStruct)
	{
		// This is Specific Properties
		//

		SpecificPropertiesEditor* editor = new SpecificPropertiesEditor(parent);
		return editor;
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::Svg)
	{
		// This is Specific Properties
		//

		SvgEditor* editor = new SvgEditor(parent);
		return editor;
	}

	if (propertyPtr->isScript() == false)
	{
		return new ExtWidgets::PropertyPlainTextEditor(parent);
	}
	else
	{
		return new IdeCodeEditor(CodeType::JavaScript, parent);
	}
}


//
// IdePropertyEditor
//

IdePropertyEditor::IdePropertyEditor(QWidget* parent, DbController* dbController /*= nullptr*/) :
	PropertyEditor(parent),
	m_dbController(dbController)
{
	QString docPath = QApplication::applicationDirPath()+"/scripthelp/index.html";
	setScriptHelpFile(docPath);
}

IdePropertyEditor::~IdePropertyEditor()
{
}

ExtWidgets::PropertyEditor* IdePropertyEditor::createChildPropertyEditor(QWidget* parent)
{
	return new IdePropertyEditor(parent, m_dbController);
}

ExtWidgets::PropertyTextEditor* IdePropertyEditor::createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent)
{
	return IdePropertyEditorHelper::createPropertyTextEditor(propertyPtr, parent, m_dbController);
}

//
// IdePropertyTable
//

IdePropertyTable::IdePropertyTable(QWidget* parent, DbController* dbController):
	PropertyTable(parent),
	m_dbController(dbController)
{
	QString docPath = QApplication::applicationDirPath()+"/scripthelp/index.html";
	setScriptHelpFile(docPath);
}

IdePropertyTable::~IdePropertyTable()
{

}

ExtWidgets::PropertyEditor* IdePropertyTable::createChildPropertyEditor(QWidget* parent)
{
	return new IdePropertyEditor(parent, m_dbController);
}

ExtWidgets::PropertyTextEditor* IdePropertyTable::createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent)
{
	return IdePropertyEditorHelper::createPropertyTextEditor(propertyPtr, parent, m_dbController);
}


//
// DialogFindReplace
//

bool DialogFindReplace::m_caseSensitive = false;

DialogFindReplace::DialogFindReplace(QWidget* parent)
	:QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	// Edit

	QLabel* labelFind = new QLabel("Find What:", this);
	QLabel* labelReplace = new QLabel("Replace Width:", this);

	m_findEdit = new QLineEdit(this);
	m_replaceEdit = new QLineEdit(this);

	// Completers

	m_findCompleter = new QCompleter(theSettings.m_findCompleter, this);
	m_findCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_findEdit->setCompleter(m_findCompleter);
	connect(m_findEdit, &QLineEdit::textEdited, [this](){m_findCompleter->complete();});
	connect(m_findCompleter, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_findEdit, &QLineEdit::setText);

	m_replaceCompleter = new QCompleter(theSettings.m_replaceCompleter, this);
	m_replaceCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_replaceEdit->setCompleter(m_replaceCompleter);
	connect(m_replaceEdit, &QLineEdit::textEdited, [this](){m_replaceCompleter->complete();});
	connect(m_replaceCompleter, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), m_replaceEdit, &QLineEdit::setText);

	// Buttons

	m_findButton = new QPushButton(tr("Find"), this);
	m_replaceButton = new QPushButton(tr("Replace"), this);
	m_replaceAllButton = new QPushButton(tr("Replace All"), this);

    connect(m_findButton, &QPushButton::clicked, this, &DialogFindReplace::onFind);
    connect(m_replaceButton, &QPushButton::clicked, this, &DialogFindReplace::onReplace);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &DialogFindReplace::onReplaceAll);

	m_caseSensitiveCheck = new QCheckBox(tr("Case Sensitive"));
	m_caseSensitiveCheck->setChecked(m_caseSensitive);

	// Layout

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget(labelFind, 0, 0);
    gridLayout->addWidget(m_findEdit, 0, 1);

    gridLayout->addWidget(labelReplace, 1, 0);
    gridLayout->addWidget(m_replaceEdit, 1, 1);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(m_caseSensitiveCheck);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(m_findButton);
    buttonsLayout->addWidget(m_replaceButton);
    buttonsLayout->addWidget(m_replaceAllButton);

    QVBoxLayout* vl = new QVBoxLayout();
    vl->addLayout(gridLayout);
    vl->addLayout(buttonsLayout);

    setLayout(vl);

    setMinimumWidth(400);
    setMinimumHeight(100);
}

DialogFindReplace::~DialogFindReplace()
{
	m_caseSensitive = m_caseSensitiveCheck->isChecked();
}

void DialogFindReplace::onFind()
{
    QString text = m_findEdit->text();
    if (text.isEmpty() == true)
    {
        return;
    }

	saveCompleters();

	emit findFirst(text, m_caseSensitiveCheck->isChecked());
}

void DialogFindReplace::onReplace()
{
    QString textFind = m_findEdit->text();
    if (textFind.isEmpty() == true)
    {
        return;
    }

    QString textReplace = m_replaceEdit->text();
    if (textReplace.isEmpty() == true)
    {
        return;
    }

	saveCompleters();

	emit replace(textFind, textReplace, m_caseSensitiveCheck->isChecked());
}

void DialogFindReplace::onReplaceAll()
{
    QString textFind = m_findEdit->text();
    if (textFind.isEmpty() == true)
    {
        return;
    }

    QString textReplace = m_replaceEdit->text();
    if (textReplace.isEmpty() == true)
    {
        return;
    }

	saveCompleters();

	emit replaceAll(textFind, textReplace, m_caseSensitiveCheck->isChecked());
}

void DialogFindReplace::saveCompleters()
{
	QString findText = m_findEdit->text();

	if (findText.isEmpty() == false && theSettings.m_findCompleter.contains(findText) == false)
	{
		theSettings.m_findCompleter.append(findText);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_findCompleter->model());
		if (completerModel != nullptr)
		{
			completerModel->setStringList(theSettings.m_findCompleter);
		}
	}

	while (theSettings.m_findCompleter.size() > 100)
	{
		theSettings.m_findCompleter.pop_front();
	}

	//

	QString replaceText = m_replaceEdit->text();

	if (replaceText.isEmpty() == false && theSettings.m_replaceCompleter.contains(replaceText) == false)
	{
		theSettings.m_replaceCompleter.append(replaceText);

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_replaceCompleter->model());
		if (completerModel != nullptr)
		{
			completerModel->setStringList(theSettings.m_replaceCompleter);
		}
	}

	while (theSettings.m_replaceCompleter.size() > 100)
	{
		theSettings.m_replaceCompleter.pop_front();
	}
}

//
// IdeCodeEditor
//

QString IdeCodeEditor::m_findText;

bool IdeCodeEditor::m_findCaseSensitive = false;

IdeCodeEditor::IdeCodeEditor(CodeType codeType, QWidget* parent) :
    PropertyTextEditor(parent),
    m_parent(parent)
{
    m_textEdit = new QsciScintilla();

    m_textEdit->setUtf8(true);

    m_textEdit->installEventFilter(this);

    QHBoxLayout* l = new QHBoxLayout(this);
	l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_textEdit);

    // Set up default font
    //
#if defined(Q_OS_WIN)
        QFont f = QFont("Consolas");
        //f.setPixelSize(font().pixelSize());
#elif defined(Q_OS_MAC)
        QFont f = QFont("Courier");
        //f.setPixelSize(font().pixelSize());
#else
        QFont f = QFont("Courier");
        //f.setPixelSize(font().pixelSize());
#endif

    // Set up lexer
    //
	m_lexerJavaScript.setDefaultFont(f);
    m_lexerXml.setDefaultFont(f);

	if (codeType == CodeType::JavaScript)
    {
		m_textEdit->setLexer(&m_lexerJavaScript);
    }

    if (codeType == CodeType::Xml)
    {
        m_textEdit->setLexer(&m_lexerXml);
    }

    // Set up margins

	if (codeType == CodeType::JavaScript || codeType == CodeType::Xml)
    {
        m_textEdit->setMarginType(0, QsciScintilla::NumberMargin);

		QFontMetrics fm(f);
		int width = static_cast<int>(fm.boundingRect("0000").width() * 1.2);

		m_textEdit->setMarginWidth(0, width);
    }
    else
    {
        m_textEdit->setMargins(0);
    }

    //

    m_textEdit->setFont(f);
    m_textEdit->setTabWidth(4);
    m_textEdit->setAutoIndent(true);

    connect(m_textEdit, &QsciScintilla::textChanged, this, &PropertyTextEditor::textChanged);

	connect(m_textEdit, &QsciScintilla::textChanged, this, &IdeCodeEditor::onTextChanged);
	connect(m_textEdit, &QsciScintilla::cursorPositionChanged, this, &IdeCodeEditor::onCursorPositionChanged);

}

IdeCodeEditor::~IdeCodeEditor()
{
}

void IdeCodeEditor::setText(const QString& text)
{
	m_textEdit->blockSignals(true);

	m_textEdit->setText(text);

	m_textEdit->blockSignals(false);
}

QString IdeCodeEditor::text()
{
	return m_textEdit->text();
}

int IdeCodeEditor::lines() const
{
	return m_textEdit->lines();
}

void IdeCodeEditor::getCursorPosition(int* line, int* index) const
{
	m_textEdit->getCursorPosition(line, index);
}

void IdeCodeEditor::setCursorPosition(int line, int index)
{
	m_textEdit->setCursorPosition(line, index);
}

void IdeCodeEditor::setReadOnly(bool value)
{
	m_textEdit->setReadOnly(value);
}

void IdeCodeEditor::activateEditor()
{
	m_textEdit->setFocus();
}


void IdeCodeEditor::findFirst(QString findText, bool caseSensitive)
{
	if (findText.isEmpty() == true)
	{
		return;
	}

	bool result = false;

	if (m_findText == findText && caseSensitive == m_findCaseSensitive)
	{
		result = m_textEdit->findNext();
	}
	else
	{
		result = m_textEdit->findFirst(findText, false/*regular*/, caseSensitive, false/*whole*/, true/*wrap*/);

		m_findCaseSensitive = caseSensitive;

		m_findText = findText;
	}

	if (result == false)
	{
		m_textEdit->selectAll(false);

		QMessageBox::information(this, qAppName(), tr("Text was not found."));
	}
}

void IdeCodeEditor::findNext()
{
	if (m_findText.isEmpty() == true)
	{
		return;
	}

	if (m_findFirst == true)
	{
		m_findFirst = false;

		bool result = m_textEdit->findFirst(m_findText, false/*regular*/, m_findCaseSensitive, false/*whole*/, true/*wrap*/);

		if (result == false)
		{
			QMessageBox::information(this, qAppName(), tr("Text was not found."));
		}
	}
	else
	{
		m_textEdit->findNext();
	}

	return;
}

void IdeCodeEditor::replace(QString findText, QString replaceText, bool caseSensitive)
{
	if (findText.isEmpty() || replaceText.isEmpty())
	{
		return;
	}

	if (findText == m_findText)
	{
		m_textEdit->replace(replaceText);
		return;
	}

	m_findText = findText;

	if (m_textEdit->findFirst(findText, false/*regular*/, caseSensitive, false/*whole*/, true/*wrap*/) == false)
	{
		return;
	}

	m_textEdit->replace(replaceText);
}

void IdeCodeEditor::replaceAll(QString findText, QString replaceText, bool caseSensitive)
{
	if (findText.isEmpty() || replaceText.isEmpty())
	{
		return;
	}

	int counter = 0;

	m_findText = findText;

	if (m_textEdit->findFirst(findText, false/*regular*/, caseSensitive, false/*whole*/, true/*wrap*/) == false)
	{
		return;
	}

	while (counter < 1000)
	{
		m_textEdit->replace(replaceText);

		counter++;

		if (m_textEdit->findNext() == false)
		{
			break;
		}

		if (counter >= 1000)
		{
			// for not to hang
			assert(false);
			break;
		}
	}

	QMessageBox::information(this, qAppName(), tr("%1 replacements occured.").arg(counter));
}


bool IdeCodeEditor::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

		if (keyEvent->key() == Qt::Key_Escape)
		{
			return true;
		}

		if (obj == m_textEdit)
		{
			if (keyEvent->key() == Qt::Key_F && (keyEvent->modifiers() & Qt::ControlModifier))
			{
				if (m_findReplace == nullptr)
				{
					m_findReplace = new DialogFindReplace(this);

					connect(m_findReplace, &DialogFindReplace::findFirst, this, &IdeCodeEditor::findFirst);
					connect(m_findReplace, &DialogFindReplace::replace, this, &IdeCodeEditor::replace);
					connect(m_findReplace, &DialogFindReplace::replaceAll, this, &IdeCodeEditor::replaceAll);
				}

				m_findReplace->show();

				return true;
			}

			if (keyEvent->key() == Qt::Key_F3)
			{
				findNext();
				return true;
			}

			if (keyEvent->key() == Qt::Key_Tab && (keyEvent->modifiers() & Qt::ControlModifier))
			{
				emit ctrlTabKeyPressed();
				return true;
			}

			if (keyEvent->key() == Qt::Key_S && (keyEvent->modifiers() & Qt::ControlModifier))
			{
				emit saveKeyPressed();
			}

			if (keyEvent->key() == Qt::Key_W && (keyEvent->modifiers() & Qt::ControlModifier))
			{
				emit closeKeyPressed();
			}
		}
	}

	// pass the event on to the parent class
	return PropertyTextEditor::eventFilter(obj, event);
}

void IdeCodeEditor::onCursorPositionChanged(int line, int index)
{
	emit cursorPositionChanged(line, index);
}

void IdeCodeEditor::onTextChanged()
{
	emit textChanged();
}

//
// IdeTuningFiltersEditor
//

IdeTuningFiltersEditor::IdeTuningFiltersEditor(DbController* dbController, QWidget* parent):
  PropertyTextEditor(parent),
  m_dbController(dbController)
{
	SignalSet tuningSignalSet;
	::Proto::AppSignalSet appSignalSet;

	// Load tuning signals
	//

	bool ok = m_dbController->getTunableSignals(&tuningSignalSet, parent);
	if (ok == true)
	{
		int count = tuningSignalSet.count();

		for (int i = 0; i < count; i++)
		{
			Proto::AppSignal* pas = appSignalSet.add_appsignal();
			tuningSignalSet[i].serializeTo(pas);
		}
	}

	m_signals.load(appSignalSet);
}

IdeTuningFiltersEditor::~IdeTuningFiltersEditor()
{
	if (m_tuningFilterEditor != nullptr)
	{
		m_tuningFilterEditor->saveUserInterfaceSettings(&theSettings.m_tuningFiltersSplitterPosition, &theSettings.m_tuningFiltersPropertyEditorSplitterPos);
	}
}

void IdeTuningFiltersEditor::setText(const QString& text)
{
    if (m_tuningFilterEditor != nullptr)
    {
        assert(false);
        return;
    }

	// Load presets

	QString errorCode;

	QByteArray rawData = text.toUtf8();

	bool ok = m_filters.load(rawData, &errorCode);

    if (ok == false)
    {
		QMessageBox::critical(this, qAppName(), errorCode);
    }



	m_tuningFilterEditor = new TuningFilterEditor(&m_filters,
												  &m_signals,
												  false,	/*readOnly*/
												  false,	/*setCurrentEnabled*/
												  true,		/*typeTreeEnabled*/
												  true,		/*typeButtonEnabled*/
												  true,		/*typeTabEnabled*/
												  true,		/*typeCounterEnabled*/
												  TuningFilter::Source::Project,
												  theSettings.m_tuningFiltersSplitterPosition,
												  theSettings.m_tuningFiltersPropertyEditorSplitterPos
												  );

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_tuningFilterEditor);

}

QString IdeTuningFiltersEditor::text()
{
    QByteArray data;

	bool ok = m_filters.save(data, TuningFilter::Source::Project);

    if (ok == true)
    {
		QString s = QString::fromUtf8(data);

		return s;

    }

    return QString();
}

void IdeTuningFiltersEditor::setReadOnly(bool value)
{
    Q_UNUSED(value);

}

