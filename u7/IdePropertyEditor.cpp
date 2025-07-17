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
        return new IdeCodePropertyEditor(UiLib::CodeEditor::CodeType::JavaScript, parent);
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
// IdeCodePropertyEditor
//
IdeCodePropertyEditor::IdeCodePropertyEditor(UiLib::CodeEditor::CodeType codeType, QWidget* parent)
    :ExtWidgets::PropertyTextEditor(parent)
{
    m_textEdit = new UiLib::CodeEditor(codeType, this);

	connect(m_textEdit,
			&UiLib::CodeEditor::escapePressed,
			this,
			[this]()
			{
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
