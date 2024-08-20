#include "Settings.h"
#include "IdePropertyEditor.h"
#include "SpecificPropertiesEditor.h"
#include "SvgEditor.h"
#include "DbChooseItemsDialog.h"
#include "Reports/ReportPropertyEditor.h"
#include "Legacy/TuningFilterEditor.h"


//
// IdePropertyEditorHelper
//
ExtWidgets::PropertyTextEditor* IdePropertyEditorHelper::createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QString defaultSpecPropCategory, DbController* dbController, QWidget* parent)
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

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::TuningUi)
	{
		// This is Filters Editor for TuningClient
		//
		if (dbController == nullptr)
		{
			Q_ASSERT(dbController);
			return new ExtWidgets::PropertyPlainTextEditor(parent);
		}

		IdeTuningUiEditor* editor = new IdeTuningUiEditor(parent);
		return editor;
	}

    if (propertyPtr->specificEditor() == E::PropertySpecificEditor::SpecificPropertyStruct)
	{
		// This is Specific Properties
		//
		SpecificPropertiesEditor* editor = new SpecificPropertiesEditor(parent);
		editor->setDefaultCategory(defaultSpecPropCategory);
		return editor;
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::Svg)
	{
		// This is SVG
		//
		SvgEditor* editor = new SvgEditor(parent);
		return editor;
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::Tags)
	{
		// This is Tags
		//
		if (dbController == nullptr)
		{
			Q_ASSERT(dbController);
			return new ExtWidgets::PropertyPlainTextEditor(parent);
		}

		DbChooseItemsDialog* editor = DbChooseItemsDialog::tagsEditor(dbController, parent);
		return editor;
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::MatsUsers)
	{
		// This is MatsUsers
		//
		if (dbController == nullptr)
		{
			Q_ASSERT(dbController);
			return new ExtWidgets::PropertyPlainTextEditor(parent);
		}

		DbChooseItemsDialog* editor = DbChooseItemsDialog::matsUsersEditor(dbController, parent);
		return editor;
	}

    if (propertyPtr->isScript() == true)
	{
		// This is Script
		//
        return new IdeCodePropertyEditor(CodeType::JavaScript, parent);
	}

    if (propertyPtr->specificEditor() == E::PropertySpecificEditor::Report)
    {
        // This is Report
        //
        return new ReportPropertyEditor(parent);
    }

    return new ExtWidgets::PropertyPlainTextEditor(parent);
}

bool IdePropertyEditorHelper::restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog)
{
	if (propertyPtr == nullptr || dialog == nullptr)
	{
		Q_ASSERT(propertyPtr);
		Q_ASSERT(dialog);
		return false;
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::Tags || 
        propertyPtr->specificEditor() == E::PropertySpecificEditor::MatsUsers)
	{
		// Resize depends on monitor size, DPI, resolution
		//
		QRect screen = dialog->parentWidget()->screen()->availableGeometry();

		dialog->resize(static_cast<int>(screen.width() * 0.20),
			   static_cast<int>(screen.height() * 0.30));
		dialog->move(screen.center() - dialog->rect().center());

		return true;
	}

	return false;
}

bool IdePropertyEditorHelper::storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog)
{
	if (propertyPtr == nullptr || dialog == nullptr)
	{
		Q_ASSERT(propertyPtr);
		Q_ASSERT(dialog);
		return false;
	}

	if (propertyPtr->specificEditor() == E::PropertySpecificEditor::Tags || 
        propertyPtr->specificEditor() == E::PropertySpecificEditor::MatsUsers)
	{
		return true;	// Do not save Tags editor size
	}

	return false;
}


//
// IdePropertyEditor
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
	auto editor = IdePropertyEditorHelper::createPropertyTextEditor(propertyPtr, m_defaultSpecificPropertyCategory, m_dbController, parent);
	return editor;
}

bool IdePropertyEditor::restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog)
{
	return IdePropertyEditorHelper::restorePropertyTextEditorSize(propertyPtr, dialog);
}

bool IdePropertyEditor::storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog)
{
	return IdePropertyEditorHelper::storePropertyTextEditorSize(propertyPtr, dialog);
}


//
// IdePropertyTable
//
IdePropertyTable::IdePropertyTable(QWidget* parent, DbController* dbController):
	PropertyTable(parent),
	m_dbController(dbController)
{
	QString docPath = QApplication::applicationDirPath() + "/scripthelp/index.html";
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
	auto editor = IdePropertyEditorHelper::createPropertyTextEditor(propertyPtr, m_defaultSpecificPropertyCategory, m_dbController, parent);
	return editor;
}

bool IdePropertyTable::restorePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog)
{
	return IdePropertyEditorHelper::restorePropertyTextEditorSize(propertyPtr, dialog);
}

bool IdePropertyTable::storePropertyTextEditorSize(std::shared_ptr<Property> propertyPtr, QDialog* dialog)
{
	return IdePropertyEditorHelper::storePropertyTextEditorSize(propertyPtr, dialog);
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
    connect(m_replaceAllButton, &QPushButton::clicked, this, &DialogFindReplace::onReplaceAllButton);

    // Replace menu

    m_replaceSelectedAction = new QAction("Process Selected Text", this);
    connect(m_replaceSelectedAction, &QAction::triggered, [this](){onReplaceAll(true/*selectedOnly*/);});
    m_replaceMenu.addAction(m_replaceSelectedAction);

    m_replaceAllAction = new QAction("Process All Text", this);
    connect(m_replaceAllAction, &QAction::triggered, [this](){onReplaceAll(false/*selectedOnly*/);});
    m_replaceMenu.addAction(m_replaceAllAction);

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

void DialogFindReplace::onReplaceAllButton()
{
    bool hasSelection = false;

    emit hasSelectedText(&hasSelection);

    if (hasSelection == true)
    {
        m_replaceMenu.popup(QCursor::pos());
    }
    else
    {
        onReplaceAll(false/*selectedOnly*/);
    }
}

void DialogFindReplace::onReplaceAll(bool selectedOnly)
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

    emit replaceAll(textFind, textReplace, selectedOnly, m_caseSensitiveCheck->isChecked());
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

IdeCodeEditor::IdeCodeEditor(CodeType codeType, QWidget* parent) :
    UiLib::CodeEditor(parent),
    m_parent(parent),
    m_codeType(codeType)
{

    setCaretLineVisible(true);
    setCaretLineBackgroundColor(0xf0f0f0);

    installEventFilter(this);

    // Set up default font
    //
#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas", 11);
#else
		QFont f = QFont("Courier");
#endif
    setFont(f);

    // Set up lexer
    //

    if (m_codeType == CodeType::JavaScript)
    {
        UiLib::JsHighlighter::createJsHighlighter(this);
    }

    if (m_codeType == CodeType::Xml)
    {
        UiLib::XmlHighlighter::createXmlHighlighter(this);
    }

    // Set up margins

    if (codeType == CodeType::JavaScript || codeType == CodeType::Xml)
    {
        setLineNumberAreaForegroundColor(QColor(0xc0c0c0));
        setLineNumberAreaBackgroundColor(QColor(0xf0f0f0));
    }
    else
    {
        setLineNumberAreaVisible(false);
    }

    //

    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &IdeCodeEditor::onCursorPositionChanged);
}

IdeCodeEditor::~IdeCodeEditor()
{
}

void IdeCodeEditor::onFind(QString findText, bool caseSensitive)
{
    bool result = false;

    if (m_findText.isEmpty() == false && m_findText == findText && caseSensitive == m_findCaseSensitive)
    {
        result = findNext();
    }
    else
    {
        if (findText.isEmpty() == true)
        {
            return;
        }

        m_findText = findText;

        m_findCaseSensitive = caseSensitive;

        result = findFirst(findText, caseSensitive, false/*whole*/);
    }

    if (result == false)
    {
        if (QMessageBox::question(this, qAppName(), tr("Search has reached the end of the document. Do you want to start searching from the beginning?")) == QMessageBox::Yes)
        {
            findFirst(findText, caseSensitive, false/*whole*/);
        }
    }
}

void IdeCodeEditor::onReplace(QString findText, QString replaceText, bool caseSensitive)
{
    if (findText.isEmpty() || replaceText.isEmpty())
    {
        return;
    }

    if (hasSelectedText() && selectedText() == findText)
    {
        replace(replaceText);
        return;
    }

    if (findText == m_findText)
    {
        if (findNext() == false)
        {
            QMessageBox::information(this, qAppName(), tr("Text was not found."));
            return;
        }
    }
    else
    {
        m_findText = findText;

        if (findFirst(findText, caseSensitive, false/*whole*/) == false)
        {
            QMessageBox::information(this, qAppName(), tr("Text was not found."));
            return;
        }
    }

    replace(replaceText);
}


void IdeCodeEditor::onReplaceAll(QString findText, QString replaceText, bool selectedOnly, bool caseSensitive)
{
    if (findText.isEmpty() || replaceText.isEmpty())
    {
        return;
    }

    QString st;

    if (selectedOnly == true)
    {
        st = selectedText();
    }
    else
    {
        st = text();
    }

    qsizetype counter = st.count(findText, caseSensitive == true ? Qt::CaseSensitive : Qt::CaseInsensitive);

    if (counter == 0)
    {
        QMessageBox::information(this, qAppName(), tr("Text was not found."));
        return;
    }

    st.replace(findText, replaceText, caseSensitive == true ? Qt::CaseSensitive : Qt::CaseInsensitive);

    if (selectedOnly == true)
    {
        replace(st);
    }
    else
    {
        setText(st);

		setModified(true);
    }

    QMessageBox::information(this, qAppName(), tr("%1 replacements occured.").arg(counter));
}

  void IdeCodeEditor::onHasSelectedText(bool* result)
{
    if (result == nullptr)
    {
        Q_ASSERT(result);
        return;
    }

    *result = hasSelectedText();
    return;
}

bool IdeCodeEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            emit escapePressed();
            return true;
        }

        if (obj == this)
        {
            if ((keyEvent->key() == Qt::Key_F && (keyEvent->modifiers() & Qt::ControlModifier)) ||
                    (keyEvent->key() == Qt::Key_F3 && m_findText.isEmpty() == true))
            {
                if (m_findReplace == nullptr)
                {
                    m_findReplace = new DialogFindReplace(this);

                    connect(m_findReplace, &DialogFindReplace::findFirst, this, &IdeCodeEditor::onFind);
                    connect(m_findReplace, &DialogFindReplace::replace, this, &IdeCodeEditor::onReplace);
                    connect(m_findReplace, &DialogFindReplace::replaceAll, this, &IdeCodeEditor::onReplaceAll);
                    connect(m_findReplace, &DialogFindReplace::hasSelectedText, this, &IdeCodeEditor::onHasSelectedText, Qt::DirectConnection);
                }

                m_findReplace->show();

                return true;
            }

            if (keyEvent->key() == Qt::Key_F3)
            {
                onFind(m_findText, m_findCaseSensitive);
                return true;
            }

            if (keyEvent->key() == Qt::Key_Tab && (keyEvent->modifiers() & Qt::ControlModifier) && 
                hasSelectedText() == false) // When text is selected, Ctrl+Tab removes tab ident level
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
    return CodeEditor::eventFilter(obj, event);
}

void IdeCodeEditor::onCursorPositionChanged()
{
    int line = 0;
    int index = 0;

    getCursorPosition(&line, &index);

    emit cursorPositionChangedTo(line, index);

}

//
// IdeCodePropertyEditor
//
IdeCodePropertyEditor::IdeCodePropertyEditor(CodeType codeType, QWidget* parent)
    :ExtWidgets::PropertyTextEditor(parent)
{
    m_textEdit = new IdeCodeEditor(codeType, this);

	connect(m_textEdit, &IdeCodeEditor::escapePressed, this, [this](){
		emit escapePressed();
	});

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_textEdit);
}

IdeCodePropertyEditor::~IdeCodePropertyEditor()
{

}

QString IdeCodePropertyEditor::text() const
{
    return m_textEdit->text();

}

void IdeCodePropertyEditor::setText(const QString& text)
{
	m_textEdit->blockSignals(true);
    m_textEdit->setText(text);
	m_textEdit->blockSignals(false);
}

bool IdeCodePropertyEditor::readOnly() const
{
    return m_textEdit->isReadOnly();
}

void IdeCodePropertyEditor::setReadOnly(bool value)
{
    m_textEdit->setReadOnly(value);
}

bool IdeCodePropertyEditor::externalOkCancelButtons() const
{
    return true;
}

bool IdeCodePropertyEditor::isModified() const
{
	return m_textEdit->isModified();
}

//
// IdeTuningUiEditor
//

IdeTuningUiEditor::IdeTuningUiEditor(QWidget* parent):
  PropertyTextEditor(parent)
{
}

IdeTuningUiEditor::~IdeTuningUiEditor()
{
}

void IdeTuningUiEditor::setText(const QString& text)
{
    if (m_tuningUiEditor != nullptr)
    {
        assert(false);
        return;
    }

	// Load presets

	QString errorCode;

	QByteArray rawData = text.toUtf8();

	bool ok = m_storage.load(rawData, &errorCode);

    if (ok == false)
    {
		QMessageBox::critical(this, qAppName(), errorCode);
    }



	m_tuningUiEditor = new TuningUiEditor(m_storage,
										  false, /*readOnly*/
										  true,  /*typeTreeEnabled*/
										  true,  /*typeButtonEnabled*/
										  true,  /*typeTabEnabled*/
										  true,  /*typeCounterEnabled*/
										  true   /*typeSchemasTabsEnabled*/
	);

    QHBoxLayout* l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_tuningUiEditor);
}

QString IdeTuningUiEditor::text() const
{
    QByteArray data;

    bool ok = m_storage.save(data);

    if (ok == true)
    {
		QString s = QString::fromUtf8(data);

		return s;

    }

    return QString();
}

bool IdeTuningUiEditor::readOnly() const
{
	return false;
}

void IdeTuningUiEditor::setReadOnly(bool value)
{
	m_tuningUiEditor->setReadOnly(value);

}

bool IdeTuningUiEditor::externalOkCancelButtons() const
{
	return true;
}

bool IdeTuningUiEditor::isModified() const
{
	return false;
}
