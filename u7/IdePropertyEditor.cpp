
#include "IdePropertyEditor.h"
#include "Settings.h"

#include <QMessageBox>
#include <QHBoxLayout>

//
// IdePropertyEditor
//

IdePropertyEditor::IdePropertyEditor(QWidget* parent, DbController* dbController /*= nullptr*/) :
	PropertyEditor(parent),
	m_dbController(dbController)
{
    // Set script help data
    //
    QFile file(":/ScriptHelp/scripthelp.html");

    if (file.open(QIODevice::ReadOnly) == true)
    {
        QByteArray data = file.readAll();
        if (data.size() > 0)
        {
            setScriptHelp(QString::fromUtf8(data));
        }
    }

    setScriptHelpWindowPos(theSettings.m_scriptHelpWindowPos);
    setScriptHelpWindowGeometry(theSettings.m_scriptHelpWindowGeometry);
}

IdePropertyEditor::~IdePropertyEditor()
{
}

void IdePropertyEditor::saveSettings()
{
    theSettings.m_scriptHelpWindowPos = scriptHelpWindowPos();
    theSettings.m_scriptHelpWindowGeometry = scriptHelpWindowGeometry();
}

ExtWidgets::PropertyTextEditor* IdePropertyEditor::createCodeEditor(Property *property, QWidget* parent)
{
    // THIS IS TEMPORARY SOLUTION, NO OTHER WAY TO DECIDE IF THIS IS TUNINGCLIENT FILTERS

    if (property->caption() == "Filters" && property->description() == "Tuning signal filters description in XML format")
    {
        // This is Filters Editor for TuningClient
        //

		IdeTuningFiltersEditor* editor = new IdeTuningFiltersEditor(m_dbController, parent);
        return editor;
    }

    CodeType codeType = property->isScript() ? CodeType::Cpp : CodeType::Unknown;
    return new IdeCodeEditor(codeType, parent);
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
    m_lexerCpp.setDefaultFont(f);
    m_lexerXml.setDefaultFont(f);

    if (codeType == CodeType::Cpp)
    {
        m_textEdit->setLexer(&m_lexerCpp);
    }

    if (codeType == CodeType::Xml)
    {
        m_textEdit->setLexer(&m_lexerXml);
    }

    // Set up margins

    if (codeType == CodeType::Cpp || codeType == CodeType::Xml)
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

	// Load tuning signals
	//

	bool ok = m_dbController->getTuningableSignals(&tuningSignalSet, parent);

	if (ok == true)
	{
		int count = tuningSignalSet.count();

		Proto::AppSignal pas;
		AppSignalParam asp;

		for (int i = 0; i < count; i++)
		{
			tuningSignalSet[i].serializeTo(&pas);

			ok = asp.load(pas);

			if (ok == false)
			{
				assert(false);
				return;
			}

			m_signalStorage.addSignal(asp);
		}
	}
}

IdeTuningFiltersEditor::~IdeTuningFiltersEditor()
{
	m_tuningFilterEditor->saveUserInterfaceSettings(&theSettings.m_tuningFiltersPropertyEditorSplitterPos, &theSettings.m_tuningFiltersDialogChooseSignalGeometry);
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

	bool ok = m_filterStorage.load(text.toUtf8(), &errorCode);
    if (ok == false)
    {
        QLabel* errorLabel = new QLabel(errorCode);

        QHBoxLayout* l = new QHBoxLayout(this);
        l->addWidget(errorLabel);
        return;
    }



	m_tuningFilterEditor = new TuningFilterEditor(&m_filterStorage, &m_signalStorage, false, false, TuningFilter::Source::Project,
												  theSettings.m_tuningFiltersPropertyEditorSplitterPos,
												  theSettings.m_tuningFiltersDialogChooseSignalGeometry);

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_tuningFilterEditor);

}

QString IdeTuningFiltersEditor::text()
{
    QByteArray data;

    bool ok = m_filterStorage.save(data);
    if (ok == true)
    {
        return data.toStdString().c_str();
    }

    return QString();
}

void IdeTuningFiltersEditor::setReadOnly(bool value)
{
    Q_UNUSED(value);

}

