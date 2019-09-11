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
	QFile file(":/ScriptHelp/scripthelp.html");
	setScriptHelp(file);
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
	QFile file(":/ScriptHelp/scripthelp.html");
	setScriptHelp(file);
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

DialogFindReplace::DialogFindReplace(QWidget* parent)
    :QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    QLabel* labelFind = new QLabel("Find What:");
    QLabel* labelReplace = new QLabel("Replace Width:");

    m_findEdit = new QLineEdit();
    m_replaceEdit = new QLineEdit();

    m_findButton = new QPushButton(tr("Find"));
    m_replaceButton = new QPushButton(tr("Replace"));
    m_replaceAllButton = new QPushButton(tr("Replace All"));

    connect(m_findButton, &QPushButton::clicked, this, &DialogFindReplace::onFind);
    connect(m_replaceButton, &QPushButton::clicked, this, &DialogFindReplace::onReplace);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &DialogFindReplace::onReplaceAll);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->addWidget(labelFind, 0, 0);
    gridLayout->addWidget(m_findEdit, 0, 1);

    gridLayout->addWidget(labelReplace, 1, 0);
    gridLayout->addWidget(m_replaceEdit, 1, 1);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
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

void DialogFindReplace::onFind()
{
    QString text = m_findEdit->text();
    if (text.isEmpty() == true)
    {
        return;
    }

    emit findFirst(text);
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

    emit replace(textFind, textReplace);
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

    emit replaceAll(textFind, textReplace);
}

//
// IdeCodeEditor
//

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
        m_textEdit->setMarginWidth(0, 40);
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
}

IdeCodeEditor::~IdeCodeEditor()
{
}

bool IdeCodeEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_textEdit)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            if (keyEvent->key() == Qt::Key_Escape)
            {
                emit escapePressed();
                return true;
            }


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
        }
    }

    // pass the event on to the parent class
    return PropertyTextEditor::eventFilter(obj, event);
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

void IdeCodeEditor::setReadOnly(bool value)
{
    m_textEdit->setReadOnly(value);
}


void IdeCodeEditor::findFirst(QString findText)
{
    if (findText.isEmpty())
    {
        return;
    }

    bool result = false;

    if (m_findText == findText)
    {
        result = m_textEdit->findNext();
    }
    else
    {
        result = m_textEdit->findFirst(findText, false/*regular*/, false/*caseSens*/, false/*whole*/, true/*wrap*/);

        m_findText = findText;
    }

    if (result == false)
    {
        QMessageBox::information(this, qAppName(), tr("Text was not found."));
    }
}

void IdeCodeEditor::findNext()
{
    m_textEdit->findNext();
}

void IdeCodeEditor::replace(QString findText, QString replaceText)
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

    if (m_textEdit->findFirst(findText, false/*regular*/, false/*caseSens*/, false/*whole*/, true/*wrap*/) == false)
    {
        return;
    }

    m_textEdit->replace(replaceText);
}

void IdeCodeEditor::replaceAll(QString findText, QString replaceText)
{
    if (findText.isEmpty() || replaceText.isEmpty())
    {
        return;
    }

    int counter = 0;

    m_findText = findText;

    if (m_textEdit->findFirst(findText, false/*regular*/, false/*caseSens*/, false/*whole*/, true/*wrap*/) == false)
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

