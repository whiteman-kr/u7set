#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"
#include "Settings.h"
#include "TuningValue.h"

#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtStringPropertyManager>
#include <QtEnumPropertyManager>
#include <QtIntPropertyManager>
#include <QtDoublePropertyManager>
#include <QtBoolPropertyManager>
#include <QList>
#include <QMetaProperty>
#include <QDebug>
#include <QMap>
#include <QStringList>
#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QStyleOptionButton>
#include <QPainter>
#include <QApplication>
#include <QToolButton>
#include <QPushButton>
#include <QTextEdit>
#include <QDialog>
#include <QRegExpValidator>
#include <QColorDialog>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QTextBrowser>
#include <QTreeWidget>

namespace ExtWidgets
{

	QString propertyVectorText(QVariant& value)
	{
		// PropertyVector
		//
		if (variantIsPropertyVector(value) == true)
		{
			auto pv = variantToPropertyVector(value);
			if (pv == nullptr)
			{
				Q_ASSERT(pv);
				return QString();
			}

			return QObject::tr("PropertyVector [%1 items]").arg(static_cast<int>(pv->size()));
		}

		// PropertyList
		//

		if (variantIsPropertyList(value) == true)
		{
			auto pv = variantToPropertyList(value);
			if (pv == nullptr)
			{
				Q_ASSERT(pv);
				return QString();
			}

			return QObject::tr("PropertyList [%1 items]").arg(static_cast<int>(pv->size()));
		}

		Q_ASSERT(false);
		return QString();
	}

	//
	// ------------ PropertyArrayEditorDialog ------------
	//
	PropertyArrayEditorDialog::PropertyArrayEditorDialog(QWidget* parent, const QString& propertyName, const QVariant& value):
		QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint),
		m_value(value)
	{
		if (variantIsPropertyVector(m_value) == false && variantIsPropertyList(m_value) == false)
		{
			Q_ASSERT(false);
			return;
		}

		setWindowTitle(propertyName);

		setMinimumSize(640, 480);

		QVBoxLayout* mainLayout = new QVBoxLayout();

		// Create Editor

		m_treeWidget = new QTreeWidget();
		QStringList headerLabels;
		headerLabels << tr("Property");

		m_treeWidget->setColumnCount(headerLabels.size());
		m_treeWidget->setHeaderLabels(headerLabels);
		m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_treeWidget->setRootIsDecorated(false);

		connect(m_treeWidget, &QTreeWidget::itemSelectionChanged, this, &PropertyArrayEditorDialog::onSelectionChanged);

		m_propertyEditor = new PropertyEditor(this);
		connect(m_propertyEditor, &PropertyEditor::propertiesChanged, this, &PropertyArrayEditorDialog::onPropertiesChanged);

		// Move Up/Down

		QVBoxLayout* moveLayout = new QVBoxLayout();

		QPushButton* b = new QPushButton(tr("Up"));
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::onMoveUp);
		moveLayout->addWidget(b);

		b = new QPushButton(tr("Down"));
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::onMoveDown);
		moveLayout->addWidget(b);

		moveLayout->addStretch();

		// Add/Remove

		QHBoxLayout* addRemoveLayout = new QHBoxLayout();

		b = new QPushButton(tr("Add"));
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::onAdd);
		addRemoveLayout->addWidget(b);

		b = new QPushButton(tr("Remove"));
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::onRemove);
		addRemoveLayout->addWidget(b);

		addRemoveLayout->addStretch();

		// Left Layout

		QVBoxLayout* leftLayout = new QVBoxLayout();
		leftLayout->addWidget(m_treeWidget);
		leftLayout->addLayout(addRemoveLayout);

		// Top Layout


		QHBoxLayout* horzLayout = new QHBoxLayout();
		horzLayout->addLayout(leftLayout);
		horzLayout->addLayout(moveLayout);
		horzLayout->addWidget(m_propertyEditor);

		// Ok/Cancel

		QHBoxLayout* buttonsLayout = new QHBoxLayout();
		buttonsLayout->addStretch();

		b = new QPushButton(tr("OK"), this);
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::accept);
		buttonsLayout->addWidget(b);

		b = new QPushButton(tr("Cancel"), this);
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::reject);
		buttonsLayout->addWidget(b);

		mainLayout->addLayout(horzLayout);

		//

		mainLayout->addLayout(buttonsLayout);

		setLayout(mainLayout);

		updateDescriptions();

		if (m_treeWidget->topLevelItemCount() > 0)
		{
			m_treeWidget->topLevelItem(0)->setSelected(true);
		}
	}

	QVariant PropertyArrayEditorDialog::value()
	{
		return m_value;
	}

	void PropertyArrayEditorDialog::onMoveUp()
	{
		moveItems(false);
	}

	void PropertyArrayEditorDialog::onMoveDown()
	{
		moveItems(true);
	}

	void PropertyArrayEditorDialog::onAdd()
	{
		if (m_treeWidget == nullptr)
		{
			Q_ASSERT(m_treeWidget);
			return;
		}

		if (variantIsPropertyVector(m_value) == false && variantIsPropertyList(m_value) == false)
		{
			Q_ASSERT(false);
			return;
		}

		// Create new object

		std::shared_ptr<PropertyObject> newObject = nullptr;

		if (variantIsPropertyVector(m_value) == true)
		{
			auto propertyVector = variantToPropertyVector(m_value);
			if (propertyVector == nullptr)
			{
				Q_ASSERT(propertyVector);
				return;
			}

			newObject = propertyVector->createItem();
			propertyVector->push_back(newObject);
		}

		if (variantIsPropertyList(m_value) == true)
		{
			auto propertyList = variantToPropertyList(m_value);
			if (propertyList == nullptr)
			{
				Q_ASSERT(propertyList);
				return;
			}

			newObject = propertyList->createItem();
			propertyList->push_back(newObject);
		}

		if (newObject == nullptr)
		{
			Q_ASSERT(newObject);
			return;
		}

		// Add item to the list and select it

		updateDescriptions();

		m_treeWidget->clearSelection();

		QTreeWidgetItem* item = m_treeWidget->topLevelItem(m_treeWidget->topLevelItemCount() - 1);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		item->setSelected(true);

		return;
	}

	void PropertyArrayEditorDialog::onRemove()
	{
		if (m_treeWidget == nullptr)
		{
			Q_ASSERT(m_treeWidget);
			return;
		}

		if (variantIsPropertyVector(m_value) == false && variantIsPropertyList(m_value) == false)
		{
			Q_ASSERT(false);
			return;
		}

		// Find selected indexes and sort in descending order

		std::vector<int> selectedRows;

		auto selectedIndexes = m_treeWidget->selectionModel()->selectedIndexes();
		for (auto si : selectedIndexes)
		{
			selectedRows.push_back(si.row());
		}

		std::sort(selectedRows.begin(), selectedRows.end(), std::greater<int>());

		// Delete tree items and objects

		for (int row : selectedRows)
		{
			QTreeWidgetItem* item = m_treeWidget->takeTopLevelItem(row);
			if (item == nullptr)
			{
				Q_ASSERT(item);
				return;
			}
			delete item;

			if (variantIsPropertyVector(m_value) == true)
			{
				auto propertyVector = variantToPropertyVector(m_value);
				if (propertyVector == nullptr)
				{
					Q_ASSERT(propertyVector);
					return;
				}

				if (row < 0 || row >= static_cast<int>(propertyVector->size()))
				{
					Q_ASSERT(false);
					return;
				}

				propertyVector->erase(propertyVector->begin() + row);
			}

			if (variantIsPropertyList(m_value) == true)
			{
				auto propertyList = variantToPropertyList(m_value);
				if (propertyList == nullptr)
				{
					Q_ASSERT(propertyList);
					return;
				}

				if (row < 0 || row >= static_cast<int>(propertyList->size()))
				{
					Q_ASSERT(false);
					return;
				}

				auto it = propertyList->begin();
				std::advance(it, row);

				propertyList->erase(it);
			}
		}

		updateDescriptions();

		return;
	}

	void PropertyArrayEditorDialog::onSelectionChanged()
	{
		if (m_treeWidget == nullptr || m_propertyEditor == nullptr)
		{
			Q_ASSERT(m_treeWidget);
			Q_ASSERT(m_propertyEditor);
			return;
		}

		std::vector<std::shared_ptr<PropertyObject>> propertyObjectList;

		auto selectedIndexes = m_treeWidget->selectionModel()->selectedIndexes();

		if (variantIsPropertyVector(m_value) == true)
		{
			auto propertyVector = variantToPropertyVector(m_value);
			if (propertyVector == nullptr)
			{
				Q_ASSERT(propertyVector);
				return;
			}

			for (QModelIndex& mi : selectedIndexes)
			{
				if (mi.row() < 0 || mi.row() >= static_cast<int>(propertyVector->size()))
				{
					Q_ASSERT(false);
					return;
				}

				propertyObjectList.push_back((*propertyVector)[mi.row()]);
			}
		}

		if (variantIsPropertyList(m_value) == true)
		{
			auto propertyList = variantToPropertyList(m_value);
			if (propertyList == nullptr)
			{
				Q_ASSERT(propertyList);
				return;
			}

			for (QModelIndex& mi : selectedIndexes)
			{
				if (mi.row() < 0 || mi.row() >= static_cast<int>(propertyList->size()))
				{
					Q_ASSERT(false);
					return;
				}

				auto it = propertyList->begin();
				std::advance(it, mi.row());

				auto p = *it;
				if (p == nullptr)
				{
					Q_ASSERT(p);
					return;
				}

				propertyObjectList.push_back(p);
			}
		}

		m_propertyEditor->setObjects(propertyObjectList);
		m_propertyEditor->resizeColumnToContents(0);

		return;
	}

	void PropertyArrayEditorDialog::onPropertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
	{
		updateDescriptions();
	}

	QString PropertyArrayEditorDialog::getObjectDescription(int objectIndex, PropertyObject* object)
	{
		std::vector<std::shared_ptr<Property>> props = object->properties();

		for (auto prop : props)
		{
			if (prop->category().isEmpty() == true &&
					prop->value().userType() == QVariant::String)
			{
				return tr("%1 - %2").arg(objectIndex).arg(prop->value().toString());
			}
		}

		return QString("%1 - PropertyObject").arg(objectIndex);
	}

	void PropertyArrayEditorDialog::updateDescriptions()
	{
		if (variantIsPropertyVector(m_value) == true)
		{
			std::vector<std::shared_ptr<PropertyObject>>* pv = variantToPropertyVector(m_value);

			int index = 0;
			for (auto prop : *pv)
			{
				QTreeWidgetItem* twi = nullptr;

				if (m_treeWidget->topLevelItemCount() > index)
				{
					twi = m_treeWidget->topLevelItem(index);
				}
				else
				{
					twi = new QTreeWidgetItem();
					m_treeWidget->addTopLevelItem(twi);
				}

				twi->setText(0, getObjectDescription(index, prop.get()));
				index++;
			}
		}

		if (variantIsPropertyList(m_value) == true)
		{
			std::list<std::shared_ptr<PropertyObject>>* pl = variantToPropertyList(m_value);

			int index = 0;
			for (auto prop : *pl)
			{
				QTreeWidgetItem* twi = nullptr;

				if (m_treeWidget->topLevelItemCount() > index)
				{
					twi = m_treeWidget->topLevelItem(index);
				}
				else
				{
					twi = new QTreeWidgetItem();
					m_treeWidget->addTopLevelItem(twi);
				}

				twi->setText(0, getObjectDescription(index, prop.get()));
				index++;
			}
		}

		return;
	}

	void PropertyArrayEditorDialog::moveItems(bool forward)
	{
		int direction = forward ? 1 : -1;

		auto selectedIndexes = m_treeWidget->selectionModel()->selectedIndexes();
		if (selectedIndexes.isEmpty() == true)
		{
			return;
		}

		std::vector<int> selectedRows;
		for (auto si : selectedIndexes)
		{
			selectedRows.push_back(si.row());
		}

		// Sort indexes
		//
		if (direction < 0)
		{
			std::sort(selectedRows.begin(), selectedRows.end(), std::less<int>());
		}
		else
		{
			std::sort(selectedRows.begin(), selectedRows.end(), std::greater<int>());
		}

		//m_treeWidget->clearSelection();

		for (int row : selectedRows)
		{
			int row2 = row + direction;
			if (row2 < 0 || row2 >= m_treeWidget->topLevelItemCount())
			{
				break;
			}

			if (variantIsPropertyVector(m_value) == true)
			{
				auto pv = variantToPropertyVector(m_value);
				if (pv == nullptr)
				{
					Q_ASSERT(pv);
					return;
				}

				if (row < 0 || row >= static_cast<int>(pv->size()) ||
					row2 < 0 || row2 >= static_cast<int>(pv->size()))
				{
					Q_ASSERT(false);
					return;
				}

				auto v1 = (*pv)[row];
				auto v2 = (*pv)[row2];

				if (v1 == nullptr || v2 == nullptr)
				{
					Q_ASSERT(v1);
					Q_ASSERT(v2);
					return;
				}

				(*pv)[row] = v2;
				(*pv)[row2] = v1;
			}

			if (variantIsPropertyList(m_value) == true)
			{
				auto pl = variantToPropertyList(m_value);
				if (pl == nullptr)
				{
					Q_ASSERT(pl);
					return;
				}

				if (row < 0 || row >= static_cast<int>(pl->size()) ||
					row2 < 0 || row2 >= static_cast<int>(pl->size()))
				{
					Q_ASSERT(false);
					return;
				}

				auto it1 = pl->begin();
				auto it2 = pl->begin();
				std::advance(it1, row);
				std::advance(it2, row2);

				std::swap(*it1, *it2);
			}

			QTreeWidgetItem* item1 = m_treeWidget->topLevelItem(row);
			item1->setSelected(false);

			QTreeWidgetItem* item2 = m_treeWidget->topLevelItem(row2);
			item2->setSelected(true);
		}

		updateDescriptions();

		return;
	}


	//
	// ------------ FilePathPropertyType ------------
	//

	int FilePathPropertyType::filePathTypeId()
	{
		return qMetaTypeId<FilePathPropertyType>();
	}

	//
	// ------------ PropertyEditorHelp ------------
	//

	PropertyEditorHelp::PropertyEditorHelp(const QString& caption, const QString& text, QWidget* parent):
		QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint)
	{
		setWindowTitle(caption);

		setAttribute(Qt::WA_DeleteOnClose);

		QTextBrowser* textEdit = new QTextBrowser();

		textEdit->setHtml(text);

		textEdit->setReadOnly(true);

		textEdit->setFont(QFont("Courier", font().pointSize() + 2));

		QHBoxLayout* l = new QHBoxLayout(this);

		l->addWidget(textEdit);

	}

	PropertyEditorHelp::~PropertyEditorHelp()
	{

	}

	//
	// ------------ PropertyTextEditor ------------
	//
	PropertyTextEditor::PropertyTextEditor(QWidget* parent) :
		QWidget(parent)
	{

	}

    PropertyTextEditor::~PropertyTextEditor()
    {
		if (m_regExpValidator != nullptr)
		{
			delete m_regExpValidator;
			m_regExpValidator = nullptr;
		}
	}

	void PropertyTextEditor::setValidator(const QString& validator)
	{
		if (m_regExpValidator != nullptr)
		{
			delete m_regExpValidator;
			m_regExpValidator = nullptr;
		}

		if (validator.isEmpty() == false)
		{
			m_regExpValidator = new QRegExpValidator(QRegExp(validator), this);
		}
	}

	bool PropertyTextEditor::modified()
	{
		return m_modified;
	}

	bool PropertyTextEditor::hasOkCancelButtons()
	{
		return m_hasOkCancelButtons;
	}

	void PropertyTextEditor::setHasOkCancelButtons(bool value)
	{
		m_hasOkCancelButtons = value;
	}

	void PropertyTextEditor::textChanged()
	{
		m_modified = true;
	}

	void PropertyTextEditor::okButtonPressed()
	{
		emit okPressed();
	}

	void PropertyTextEditor::cancelButtonPressed()
	{
		emit cancelPressed();
	}

	//
	// ------------ PropertyPlainTextEditor ------------
	//
	PropertyPlainTextEditor::PropertyPlainTextEditor(QWidget* parent) :
		PropertyTextEditor(parent)
	{
		m_plainTextEdit = new QPlainTextEdit();

		QHBoxLayout* l = new QHBoxLayout(this);
		l->addWidget(m_plainTextEdit);
		l->setContentsMargins(0, 0, 0, 0);

		m_plainTextEdit->setTabChangesFocus(false);
		m_plainTextEdit->setFont(QFont("Courier", font().pointSize() + 2));

		QFontMetrics metrics(m_plainTextEdit->font());

		const int tabStop = 4;  // 4 characters
		QString spaces;
		for (int i = 0; i < tabStop; ++i)
		{
			spaces += " ";
		}

		m_plainTextEdit->setTabStopWidth(metrics.width(spaces));

		connect(m_plainTextEdit, &QPlainTextEdit::textChanged, this, &PropertyPlainTextEditor::textChanged);
		connect(m_plainTextEdit->document(), &QTextDocument::contentsChange, this, &PropertyPlainTextEditor::onPlainTextContentsChange);
	}

	void PropertyPlainTextEditor::setText(const QString& text)
	{
		m_plainTextEdit->blockSignals(true);

		m_plainTextEdit->setPlainText(text);

		m_plainTextEdit->blockSignals(false);

		m_prevPlainText = text;
	}

	QString PropertyPlainTextEditor::text()
	{
		return m_plainTextEdit->toPlainText();
	}

	void PropertyPlainTextEditor::setReadOnly(bool value)
	{
		m_plainTextEdit->setReadOnly(value);
	}

	bool PropertyPlainTextEditor::eventFilter(QObject* obj, QEvent* event)
	{
		if (obj == m_plainTextEdit)
		{
			if (event->type() == QEvent::KeyPress)
			{
				QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

				if (keyEvent->key() == Qt::Key_Escape)
				{
					emit escapePressed();
					return true;
				}
			}
		}

		// pass the event on to the parent class
		return PropertyTextEditor::eventFilter(obj, event);
	}

	void PropertyPlainTextEditor::onPlainTextContentsChange(int position, int charsRemoved, int charsAdded)
	{
		Q_UNUSED(charsAdded);

		if (m_plainTextEdit == nullptr)
		{
			assert(m_plainTextEdit);
			return;
		}

		if (m_regExpValidator == nullptr)
		{
			return;
		}

		QString newText = text();

		int pos = 0;

		if (m_regExpValidator->validate(newText, pos) == QValidator::Invalid)
		{
			// Restore text

			bool oldBlockState = m_plainTextEdit->document()->blockSignals(true);

			m_plainTextEdit->setPlainText(m_prevPlainText);

			m_plainTextEdit->document()->blockSignals(oldBlockState);

			// Restore cursor position

			QTextCursor c = m_plainTextEdit->textCursor();

			if (charsRemoved > 0)
			{
				c.setPosition(position + charsRemoved);
			}
			else
			{
				c.setPosition(position);
			}

			m_plainTextEdit->setTextCursor(c);

			return;
		}

		m_prevPlainText = newText;
	}


	//
	// ------------ QtMultiFilePathEdit ------------
	//
	MultiFilePathEdit::MultiFilePathEdit(QWidget* parent, bool readOnly):
		QWidget(parent)
	{
		m_lineEdit = new QLineEdit(parent);

        m_button = new QToolButton(parent);
        m_button->setText("...");

		connect(m_lineEdit, &QLineEdit::editingFinished, this, &MultiFilePathEdit::onEditingFinished);

		connect(m_button, &QToolButton::clicked, this, &MultiFilePathEdit::onButtonPressed);

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_lineEdit);
        lt->addWidget(m_button, 0, Qt::AlignRight);

		setLayout(lt);

		m_lineEdit->installEventFilter(this);
		m_lineEdit->setReadOnly(readOnly == true);

		m_button->setEnabled(readOnly == false);

		QTimer::singleShot(0, m_lineEdit, SLOT(setFocus()));
	}

	bool MultiFilePathEdit::eventFilter(QObject* watched, QEvent* event)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return QWidget::eventFilter(watched, event);
		}

		if (watched == m_lineEdit && event->type() == QEvent::KeyPress)
		{
			QKeyEvent* ke = static_cast<QKeyEvent*>(event);
			if (ke->key() == Qt::Key_Escape)
			{
				m_escape = true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}

	void MultiFilePathEdit::onButtonPressed()
	{
		FilePathPropertyType f = m_oldPath.value<FilePathPropertyType>();

		QString filePath = QFileDialog::getOpenFileName(this, tr("Select file"), f.filePath, f.filter);
		if (filePath.isEmpty() == true)
		{
			return;
		}

		f.filePath = QDir::toNativeSeparators(filePath);

        m_oldPath = QVariant::fromValue(f);
        m_lineEdit->setText(f.filePath);

		emit valueChanged(QVariant::fromValue(f));
	}

	void MultiFilePathEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

        m_oldPath = property->value();

        FilePathPropertyType f = property->value().value<FilePathPropertyType>();
		m_lineEdit->setText(f.filePath);
		m_lineEdit->setReadOnly(readOnly == true);

		m_button->setEnabled(readOnly == false);
	}

	void MultiFilePathEdit::onEditingFinished()
	{
		if (m_escape == false)
		{
			QString t = m_lineEdit->text();

			FilePathPropertyType f = m_oldPath.value<FilePathPropertyType>();

			if (f.filePath != t)
			{
				f.filePath = t;
				emit valueChanged(QVariant::fromValue(f));
			}
		}
	}

	// ------------ QtMultiEnumEdit ------------
	//
	MultiEnumEdit::MultiEnumEdit(QWidget* parent, std::shared_ptr<Property> p, bool readOnly):
		QWidget(parent)
	{
		if (p == nullptr)
		{
			assert(p);
			return;
		}

		if (p->isEnum() == false)
		{
			assert(false);
			return;
		}

		m_combo = new QComboBox(parent);

		for (std::pair<int, QString> i : p->enumValues())
		{
			m_combo->addItem(i.second, i.first);
		}
		m_combo->setCurrentIndex(-1);

		m_combo->setEnabled(readOnly == false);

		connect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_combo);

		setLayout(lt);
	}

	void MultiEnumEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		if (m_combo == nullptr)
		{
			Q_ASSERT(m_combo);
			return;
		}

        if (property->isEnum() == false)
        {
            assert(false);
            return;
        }

		m_combo->blockSignals(true);

		// select an item with a value
		//
		bool found =  false;
		for (int i = 0; i < m_combo->count(); i++)
		{
			if (m_combo->itemData(i).toInt() ==  property->value().toInt())
			{
				m_combo->setCurrentIndex(i);
				found = true;
				break;
			}
		}
		if (found == false)
		{
			m_combo->setCurrentIndex(-1);
        }

		m_combo->blockSignals(false);

		m_combo->setEnabled(readOnly == false);
	}

	void MultiEnumEdit::indexChanged(int index)
	{
		int value = m_combo->itemData(index).toInt();
		emit valueChanged(value);
	}


	//
	// ---------QtMultiColorEdit----------
	//

	MultiColorEdit::MultiColorEdit(QWidget* parent, bool readOnly):
		QWidget(parent)
	{
		m_lineEdit = new QLineEdit(parent);

        m_button = new QToolButton(parent);
        m_button->setText("...");

		connect(m_lineEdit, &QLineEdit::editingFinished, this, &MultiColorEdit::onEditingFinished);

		connect(m_button, &QToolButton::clicked, this, &MultiColorEdit::onButtonPressed);

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_lineEdit);
        lt->addWidget(m_button, 0, Qt::AlignRight);

		setLayout(lt);

		m_lineEdit->installEventFilter(this);
		m_lineEdit->setReadOnly(readOnly == true);

		m_button->setEnabled(readOnly == false);

		QRegExp regexp("\\[([1,2]?[0-9]{0,2};){3}[1,2]?[0-9]{0,2}\\]");
		QRegExpValidator* validator = new QRegExpValidator(regexp, this);
		m_lineEdit->setValidator(validator);

		QTimer::singleShot(0, m_lineEdit, SLOT(setFocus()));
	}

	bool MultiColorEdit::eventFilter(QObject* watched, QEvent* event)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return QWidget::eventFilter(watched, event);
		}

		if (watched == m_lineEdit && event->type() == QEvent::KeyPress)
		{
			QKeyEvent* ke = static_cast<QKeyEvent*>(event);
			if (ke->key() == Qt::Key_Escape)
			{
				m_escape = true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}

	void MultiColorEdit::onButtonPressed()
	{
		QString t = m_lineEdit->text();
		QColor color = colorFromText(t);

		QColorDialog dialog(color, this);
		if (dialog.exec() == QDialog::Accepted)
		{
            QColor color = dialog.selectedColor();
            QString str = QString("[%1;%2;%3;%4]").
                          arg(color.red()).
                          arg(color.green()).
                          arg(color.blue()).
                          arg(color.alpha());

            if (color != m_oldColor)
            {
                m_oldColor = color;
                m_lineEdit->setText(str);

                emit valueChanged(color);
            }
		}
	}

	void MultiColorEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

        QColor color = property->value().value<QColor>();
        QString str = QString("[%1;%2;%3;%4]").
					  arg(color.red()).
					  arg(color.green()).
					  arg(color.blue()).
					  arg(color.alpha());

		m_oldColor = color;

        m_lineEdit->setText(str);
		m_lineEdit->setReadOnly(readOnly == true);

		m_button->setEnabled(readOnly == false);
	}

	void MultiColorEdit::onEditingFinished()
	{
		if (m_escape == false)
		{
            QString t = m_lineEdit->text();
            QColor color = colorFromText(t);

            if (color != m_oldColor)
			{
                m_oldColor = color;
                emit valueChanged(color);
			}
		}
	}

	QColor MultiColorEdit::colorFromText(const QString& t)
	{
		if (t.isEmpty() == true)
		{
			return QColor();
		}

		QString text = t;
		text.remove(QRegExp("[\\[,\\]]"));

		QStringList l = text.split(";");
		if (l.count() != 4)
		{
			return QColor();
		}

		int r = l[0].toInt();
		int g = l[1].toInt();
		int b = l[2].toInt();
		int a = l[3].toInt();

		if (r < 0 || r > 255)
			r = 255;

		if (g < 0 || g > 255)
			g = 255;

		if (b < 0 || b > 255)
			b = 255;

		if (a < 0 || a > 255)
			a = 255;

		return QColor(r, g, b, a);
	}

	//
	// ---------MultiTextEditorDialog----------
	//
	MultiTextEditorDialog::MultiTextEditorDialog(QWidget* parent, PropertyEditor* propertyEditor, const QString &text, std::shared_ptr<Property> p):
		QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
		m_propertyEditor(propertyEditor),
		m_property(p)
	{
		setWindowTitle(p->caption());

		if (theSettings.m_multiLinePropertyEditorWindowPos.x() != -1 && theSettings.m_multiLinePropertyEditorWindowPos.y() != -1)
		{
			move(theSettings.m_multiLinePropertyEditorWindowPos);
			restoreGeometry(theSettings.m_multiLinePropertyEditorGeometry);
		}
		else
		{
			resize(1024, 768);
		}

		QVBoxLayout* vl = new QVBoxLayout();

		// Create Editor

		m_editor = m_propertyEditor->createPropertyTextEditor(m_property.get(), this);

		m_editor->setText(text);

		if (m_property->validator().isEmpty() == false)
		{
			m_editor->setValidator(m_property->validator());
		}

		connect(m_editor, &PropertyTextEditor::escapePressed, this, &MultiTextEditorDialog::reject);

		// Buttons

		QPushButton* okButton = nullptr;
		QPushButton* cancelButton = nullptr;

		if (m_editor->hasOkCancelButtons() == true)
		{
			okButton = new QPushButton(tr("OK"), this);
			cancelButton = new QPushButton(tr("Cancel"), this);

			okButton->setDefault(true);

			connect(okButton, &QPushButton::clicked, this, &MultiTextEditorDialog::accept);
			connect(cancelButton, &QPushButton::clicked, this, &MultiTextEditorDialog::reject);
		}
		else
		{
			connect(m_editor, &PropertyTextEditor::okPressed, this, &MultiTextEditorDialog::accept);
			connect(m_editor, &PropertyTextEditor::cancelPressed, this, &MultiTextEditorDialog::reject);
		}

		connect(this, &QDialog::finished, this, &MultiTextEditorDialog::finished);

		QHBoxLayout* hl = new QHBoxLayout();

		// Property Editor help

		if (p->isScript() && m_propertyEditor->scriptHelp().isEmpty() == false)
		{
			QPushButton* helpButton = new QPushButton("?", this);

			hl->addWidget(helpButton);

			connect(helpButton, &QPushButton::clicked, [this] ()
			{
				if (m_propertyEditorHelp == nullptr)
				{
					m_propertyEditorHelp = new PropertyEditorHelp(tr("Script Help"), m_propertyEditor->scriptHelp(), this);
					m_propertyEditorHelp->show();

					if (m_propertyEditor->scriptHelpWindowPos().x() != -1 && m_propertyEditor->scriptHelpWindowPos().y() != -1)
					{
						m_propertyEditorHelp->move(m_propertyEditor->scriptHelpWindowPos());
						m_propertyEditorHelp->restoreGeometry(m_propertyEditor->scriptHelpWindowGeometry());
					}
					else
					{
						// put the window at the right side of the screen

						QDesktopWidget* screen = QApplication::desktop();
						QRect screenGeometry = screen->availableGeometry();

						int screenHeight = screenGeometry.height();
						int screenWidth = screenGeometry.width();

						int defaultWidth = screenWidth / 4;

						// height must exclude the window header size

						int defaultHeight = screenHeight - (m_propertyEditorHelp->geometry().y() - m_propertyEditorHelp->y());

						m_propertyEditorHelp->move(screenWidth - defaultWidth, 0);
						m_propertyEditorHelp->resize(defaultWidth, defaultHeight);
					}

					connect(m_propertyEditorHelp, &PropertyEditorHelp::destroyed, [this] (QObject*)
					{
						m_propertyEditor->setScriptHelpWindowPos(m_propertyEditorHelp->pos());
						m_propertyEditor->setScriptHelpWindowGeometry(m_propertyEditorHelp->saveGeometry());
						m_propertyEditor->saveSettings();

						m_propertyEditorHelp = nullptr;

					});
				}
			});

			connect(this, &QDialog::finished, [this] (int)
			{
				if (m_propertyEditorHelp != nullptr)
				{
					m_propertyEditorHelp->accept();
				}
			});
		}

		hl->addStretch();
		if (okButton != nullptr)
		{
			hl->addWidget(okButton);
		}
		if (cancelButton != nullptr)
		{
			hl->addWidget(cancelButton);
		}


		vl->addWidget(m_editor);
		vl->addLayout(hl);

		setLayout(vl);
	}

	QString MultiTextEditorDialog::text()
	{
		return m_text;
	}

	void MultiTextEditorDialog::finished(int result)
	{
		Q_UNUSED(result);

		theSettings.m_multiLinePropertyEditorWindowPos = pos();
		theSettings.m_multiLinePropertyEditorGeometry = saveGeometry();

	}

	void MultiTextEditorDialog::accept()
	{
		if (m_editor == nullptr)
		{
			Q_ASSERT(m_editor);
			return;
		}

		m_text = m_editor->text();

		QDialog::accept();
	}

	void MultiTextEditorDialog::reject()
	{
		if (m_editor->modified() == true)
		{
			int result = QMessageBox::warning(this, qAppName(), tr("Do you want to save your changes?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

			if (result == QMessageBox::Yes)
			{
				accept();
				return;
			}

			if (result == QMessageBox::Cancel)
			{
				return;
			}
		}

		QDialog::reject();
	}

	//
	// ---------MultiTextEdit----------
	//

	MultiTextEdit::MultiTextEdit(QWidget* parent, std::shared_ptr<Property> p, bool readOnly, PropertyEditor* propertyEditor):
		QWidget(parent),
		m_property(p),
		m_userType(p->value().userType()),
		m_propertyEditor(propertyEditor)
	{

		if (p == nullptr || propertyEditor == nullptr)
		{
			assert(p);
			assert(propertyEditor);
		}

		if (m_userType == QVariant::Uuid)
		{
			m_userType = QVariant::String;
		}

		if (m_userType == TuningValue::tuningValueTypeId())
		{
			m_oldValue = p->value();	// Save type of Tuning Value for future setting
		}

		m_lineEdit = new QLineEdit(parent);
		connect(m_lineEdit, &QLineEdit::editingFinished, this, &MultiTextEdit::onEditingFinished);
		connect(m_lineEdit, &QLineEdit::textEdited, this, &MultiTextEdit::onTextEdited);

		if (m_property->specificEditor() == E::PropertySpecificEditor::LoadFileDialog ||
				m_userType == QVariant::ByteArray ||
				(m_userType == QVariant::String && p->password() == false))
		{
			m_button = new QToolButton(parent);
			m_button->setText("...");

			connect(m_button, &QToolButton::clicked, this, &MultiTextEdit::onButtonPressed);
		}

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_lineEdit);
		if (m_button != nullptr)
		{
			lt->addWidget(m_button, 0, Qt::AlignRight);
		}

		setLayout(lt);

		m_lineEdit->installEventFilter(this);

		if (m_property->specificEditor() != E::PropertySpecificEditor::LoadFileDialog &&	// for LoadFileDialog, Validator is used as mask
				p->validator().isEmpty() == false)
        {
			QRegExp regexp(p->validator());
			QRegExpValidator* v = new QRegExpValidator(regexp, this);
            m_lineEdit->setValidator(v);
        }

		if (m_userType == QVariant::String && p->password() == true)
        {
            m_lineEdit->setEchoMode(QLineEdit::Password);
        }

		m_lineEdit->setReadOnly(readOnly == true);

		if (m_userType == QVariant::ByteArray || m_userType == QVariant::Image)
		{
			m_lineEdit->setReadOnly(true);
		}

		if (m_button != nullptr)
		{
			m_button->setEnabled(readOnly == false);
		}

		QTimer::singleShot(0, m_lineEdit, SLOT(setFocus()));
	}

	bool MultiTextEdit::eventFilter(QObject* watched, QEvent* event)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return QWidget::eventFilter(watched, event);
		}

		if (watched == m_lineEdit && event->type() == QEvent::KeyPress)
		{
			QKeyEvent* ke = static_cast<QKeyEvent*>(event);
			if (ke->key() == Qt::Key_Escape)
			{
				m_escape = true;
			}
		}

		return QWidget::eventFilter(watched, event);
	}

	void MultiTextEdit::onButtonPressed()
	{
		QString editText;
		QByteArray editBytes;

		if (m_property->specificEditor() == E::PropertySpecificEditor::LoadFileDialog)
		{
			QString fileName = QFileDialog::getOpenFileName(this->parentWidget(), tr("Select File"), QString(), m_property->validator());
			if (fileName.isEmpty() == true)
			{
				return;
			}

			QFile f(fileName);
			if (f.open(QFile::ReadOnly) == false)
			{
				QMessageBox::critical(this, qAppName(), tr("File loading error!"));
				return;
			}

			editBytes = f.readAll();
			editText = editBytes.toStdString().c_str();
		}
		else
		{
			if (m_userType == QVariant::Image)
			{
				Q_ASSERT(false);	// Images should have E::PropertySpecificEditor::LoadFileDialog type
				return;
			}

			MultiTextEditorDialog multlLineEdit(this, m_propertyEditor, m_oldValue.toString(), m_property);
			if (multlLineEdit.exec() != QDialog::Accepted)
			{
				return;
			}

			editText = multlLineEdit.text();
			editBytes = editText.toUtf8();
		}

		// update LineEdit


		switch (m_userType)
		{
			case QVariant::ByteArray:
			{
				if (editBytes != m_oldValue)
				{
					 emit valueChanged(editBytes);
				}

				m_oldValue = editBytes;

				m_lineEdit->blockSignals(true);
				m_lineEdit->setText(tr("Data <%1 bytes>").arg(editBytes.size()));
				m_lineEdit->blockSignals(false);
			}
			break;

			case QVariant::Image:
			{
				QImage image;
				if (image.loadFromData(editBytes) == false)
				{
					QMessageBox::critical(this, qAppName(), tr("Image loading error!"));
				}

				if (image != m_oldValue)
				{
					 emit valueChanged(image);
				}

				m_oldValue = image;

				m_lineEdit->blockSignals(true);
				m_lineEdit->setText(tr("Image <Width = %1 Height = %2>").arg(image.width()).arg(image.height()));
				m_lineEdit->setReadOnly(true);
				m_lineEdit->blockSignals(false);
			}
			break;

			default:
			{
				if (editText != m_oldValue)
				{
					 emit valueChanged(editText);
				}

				editText = m_property->value().toString();

				m_oldValue = editText;

				bool longText = editText.length() > PropertyEditorTextMaxLength;

				m_lineEdit->blockSignals(true);

				if (longText == true)
				{
					m_lineEdit->setText(tr("<%1 bytes>").arg(editText.length()));
				}
				else
				{
					m_lineEdit->setText(editText);
				}
				m_lineEdit->setReadOnly(longText == true);

				m_lineEdit->blockSignals(false);
			}
		}

	}

	void MultiTextEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

		m_lineEdit->setReadOnly(readOnly == true);

		char numberFormat = property->precision() > 5 ? 'g' : 'f';

		switch (m_userType)
		{
		case QVariant::ByteArray:
			{
				QByteArray ba = property->value().toByteArray();
				m_oldValue = ba;

				m_lineEdit->setText(tr("Data <%1 bytes>").arg(ba.size()));
				m_lineEdit->setReadOnly(true);
			}
			break;
		case QVariant::Image:
			{
				QImage image = property->value().value<QImage>();
				m_oldValue = image;

				m_lineEdit->setText(tr("Image <Width = %1 Height = %2>").arg(image.width()).arg(image.height()));
				m_lineEdit->setReadOnly(true);
			}
			break;
		case QVariant::String:
			{
				QString editText = property->value().toString();
				m_oldValue = editText;

				bool longText = editText.length() > PropertyEditorTextMaxLength;

				if (longText == true)
				{
					m_lineEdit->setText(tr("<%1 bytes>").arg(editText.length()));
				}
				else
				{
					m_lineEdit->setText(editText);
				}
				m_lineEdit->setReadOnly(readOnly == true || longText == true);
			}
			break;
		case QVariant::Int:
		{
			m_oldValue = property->value().toInt();
			m_lineEdit->setText(QString::number(m_oldValue.toInt()));
		}
			break;
		case QVariant::UInt:
		{
			m_oldValue = property->value().toUInt();
			m_lineEdit->setText(QString::number(m_oldValue.toUInt()));
		}
			break;
		case QMetaType::Float:
		{
			m_oldValue = property->value().toFloat();
			m_lineEdit->setText(QString::number(m_oldValue.toFloat(), numberFormat, property->precision()));

		}
			break;
		case QVariant::Double:
		{
			m_oldValue = property->value().toDouble();
			m_lineEdit->setText(QString::number(m_oldValue.toDouble(), numberFormat, property->precision()));
		}
			break;
		default:
			if (m_userType == TuningValue::tuningValueTypeId())
			{
				TuningValue t = property->value().value<TuningValue>();
				m_oldValue = QVariant::fromValue(t);
				m_lineEdit->setText(t.toString(property->precision()));
			}
			else
			{
				assert(false);
				return;
			}
		}

		if (m_button != nullptr)
		{
			m_button->setEnabled(readOnly == false);
		}
	}

	void MultiTextEdit::onTextEdited(const QString &text)
	{
		Q_UNUSED(text);
		m_textEdited = true;

	}

	void MultiTextEdit::onEditingFinished()
	{
		if (m_escape == true)
		{
			return;
		}

		if (m_textEdited == false)
		{
			return;
		}

		m_lineEdit->blockSignals(true);

		switch (m_userType)
		{
		case QVariant::String:
		{
			if (m_lineEdit->text() != m_oldValue.toString() || m_oldValue.isNull() == true)
			{
				m_oldValue = m_lineEdit->text();
				emit valueChanged(m_oldValue);
			}
		}
		break;
		case QVariant::ByteArray:
			{
				Q_ASSERT(false);	// No editing allowed
			}
			break;
		case QVariant::Image:
			{
				Q_ASSERT(false);	// No editing allowed
			}
			break;
		case QVariant::Int:
			{
				bool ok = false;
				int value = m_lineEdit->text().toInt(&ok);
				if (ok == true &&
						(value != m_oldValue.toInt() || m_oldValue.isNull() == true))
				{
					m_oldValue = value;
					emit valueChanged(value);
				}
			}
			break;
		case QVariant::UInt:
			{
				bool ok = false;
				uint value = m_lineEdit->text().toUInt(&ok);
				if (ok == true &&
						(value != m_oldValue.toUInt() || m_oldValue.isNull() == true))
				{
					m_oldValue = value;
					emit valueChanged(value);
				}
			}
			break;
		case QMetaType::Float:
			{
				bool ok = false;
				float value = m_lineEdit->text().toFloat(&ok);
				if (ok == true &&
						(value != m_oldValue.toFloat() || m_oldValue.isNull() == true))
				{
					m_oldValue = value;
					emit valueChanged(value);
				}
			}
			break;
		case QVariant::Double:
			{
				bool ok = false;
				double value = m_lineEdit->text().toDouble(&ok);
				if (ok == true &&
						(value != m_oldValue.toDouble() || m_oldValue.isNull() == true))
				{
					m_oldValue = value;
					emit valueChanged(value);
				}
			}
			break;
		default:
			if (m_userType == TuningValue::tuningValueTypeId())
			{
				bool ok = false;
				TuningValue oldValue = m_oldValue.value<TuningValue>();
				TuningValue value(oldValue);
				value.fromString(m_lineEdit->text(), &ok);
				if (ok == true &&
						(value != oldValue || m_oldValue.isNull() == true))
				{
					m_oldValue.setValue(value);
					emit valueChanged(m_oldValue);
				}
			}
			else
			{
				assert(false);
				return;
			}
		}

        m_lineEdit->blockSignals(false);

		m_textEdited = false;
	}


	//
	// ---------QtMultiCheckBox----------
	//
	MultiArrayEdit::MultiArrayEdit(QWidget* parent, std::shared_ptr<Property> p, bool readOnly):
		QWidget(parent),
		m_property(p),
		m_currentValue(p->value())
	{

		m_lineEdit = new QLineEdit(parent);
		m_lineEdit->setReadOnly(true);

		// m_currentValue must have initialized type, all values must be deleted. They will be set by setValue

		if (variantIsPropertyVector(m_currentValue) == true)
		{
			auto pv = variantToPropertyVector(m_currentValue);
			pv->clear();

			m_lineEdit->setText("<PropertyVector>");
		}

		if (variantIsPropertyList(m_currentValue) == true)
		{
			auto pl = variantToPropertyList(m_currentValue);
			pl->clear();

			m_lineEdit->setText("<PropertyList>");
		}

		m_button = new QToolButton(parent);
		m_button->setText("...");

		connect(m_button, &QToolButton::clicked, this, &MultiArrayEdit::onButtonPressed);

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_lineEdit);
		lt->addWidget(m_button, 0, Qt::AlignRight);

		setLayout(lt);

		m_button->setEnabled(readOnly == false);
	}

	void MultiArrayEdit::setValue(QVariant value, bool readOnly)
	{
		m_button->setEnabled(readOnly == false);

		m_currentValue = value;

		m_lineEdit->setText(propertyVectorText(m_currentValue));

		if (variantIsPropertyVector(m_currentValue) == false && variantIsPropertyList(m_currentValue) == false)
		{
			Q_ASSERT(false);
			return;
		}
	}

	void MultiArrayEdit::onButtonPressed()
	{
		if (variantIsPropertyVector(m_currentValue) == false && variantIsPropertyList(m_currentValue) == false)
		{
			Q_ASSERT(false);
			return;
		}

		PropertyArrayEditorDialog d(this, m_property->caption(), m_currentValue);
		if (d.exec() != QDialog::Accepted)
		{
			return;
		}

		if (d.value() != m_currentValue)
		{
			 emit valueChanged(d.value());
		}

		m_currentValue = d.value();

		m_lineEdit->setText(propertyVectorText(m_currentValue));
	}

	//
	// ---------QtMultiCheckBox----------
	//
    static QIcon drawCheckBox(int state, bool enabled)
	{
		QStyleOptionButton opt;
		switch (state)
		{
			case Qt::Checked:
				opt.state |= QStyle::State_On;
				break;
			case Qt::Unchecked:
				opt.state |= QStyle::State_Off;
				break;
			case Qt::PartiallyChecked:
				opt.state |= QStyle::State_NoChange;
				break;
			default:
				Q_ASSERT(false);
		}

        if (enabled == false)
        {
            opt.state |= QStyle::State_ReadOnly;
        }
        else
        {
            opt.state |= QStyle::State_Enabled;
        }

		const QStyle* style = QApplication::style();

		const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
		const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);

		opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
		QPixmap pixmap = QPixmap(indicatorWidth, indicatorHeight);
		pixmap.fill(Qt::transparent);
		{
			QPainter painter(&pixmap);
			style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
		}
		return QIcon(pixmap);
	}

	static QIcon drawColorBox(QColor color)
	{
		const QStyle* style = QApplication::style();
		// Figure out size of an indicator and make sure it is not scaled down in a list view item
		// by making the pixmap as big as a list view icon and centering the indicator in it.
		// (if it is smaller, it can't be helped)
		const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth);
		const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight);
		const int listViewIconSize = indicatorWidth;
		const int pixmapWidth = indicatorWidth;
		const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

		QRect rect = QRect(0, 0, indicatorWidth - 1, indicatorHeight - 1);
		QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
		pixmap.fill(Qt::transparent);
		{
			// Center?
			const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
			const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
			QPainter painter(&pixmap);
			painter.translate(xoff, yoff);

			painter.setPen(QColor(Qt::black));
			painter.setBrush(QBrush(color));
			painter.drawRect(rect);
		}
		return QIcon(pixmap);
	}

	static QIcon drawImage(const QImage& image)
	{
		if (image.isNull() == false)
		{
			const QStyle* style = QApplication::style();
			const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth);
			const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight);
			const int listViewIconSize = indicatorWidth;
			const int pixmapWidth = indicatorWidth;
			const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

			QImage smallImage = image.scaled(pixmapWidth, pixmapHeight, Qt::KeepAspectRatio);

			return QIcon(QPixmap::fromImage(smallImage));
		}

		return QIcon();
	}

	void PropertyEditorCheckBox::paintEvent(QPaintEvent *e)
	{
		Q_UNUSED(e);

		QStyleOptionButton opt;
		switch (checkState())
		{
			case Qt::Checked:
				opt.state |= QStyle::State_On;
				break;
			case Qt::Unchecked:
				opt.state |= QStyle::State_Off;
				break;
			case Qt::PartiallyChecked:
				opt.state |= QStyle::State_NoChange;
				break;
			default:
				Q_ASSERT(false);
		}

		if (isEnabled() == false)
		{
			opt.state |= QStyle::State_ReadOnly;
		}
		else
		{
			opt.state |= QStyle::State_Enabled;
		}

		const QStyle* style = QApplication::style();

		const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
		const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);

		const int pixmapHeight = rect().height();

		// Center the image vertically

		const int xoff = 0;
		const int yoff = (pixmapHeight  > indicatorHeight)  ? (pixmapHeight  - indicatorHeight)  / 2 : 0;

		opt.rect = QRect(xoff, yoff, indicatorWidth, indicatorHeight);

		QPainter painter(this);
		style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);

		const int spacing = 6;
		QRect textRect = rect();
		textRect.setLeft(0 + indicatorWidth + spacing);

		painter.drawText(textRect, Qt::AlignVCenter,  text());
	}

	MultiCheckBox::MultiCheckBox(QWidget* parent):
		QWidget(parent)
	{
		m_checkBox = new PropertyEditorCheckBox(parent);
		connect(m_checkBox, &QCheckBox::stateChanged, this, &MultiCheckBox::onStateChanged);

		QHBoxLayout*lt = new QHBoxLayout;
		lt->setContentsMargins(3, 1, 0, 0);
		lt->addWidget(m_checkBox);
		setLayout(lt);
	}

	void MultiCheckBox::setValue(Qt::CheckState state, bool readOnly)
	{
		if (m_checkBox == nullptr)
		{
			Q_ASSERT(m_checkBox);
			return;
		}

		m_checkBox->blockSignals(true);
		m_checkBox->setCheckState(state);

		updateText();
		m_checkBox->setEnabled(readOnly == false);
		m_checkBox->blockSignals(false);
	}

	void MultiCheckBox::changeValueOnButtonClick()
	{
		QPoint pt = QCursor::pos();
		pt = m_checkBox->mapFromGlobal(pt);

		if (m_checkBox->hitOnButton(pt) == true)
		{
			m_checkBox->click();
		}
	}

	void MultiCheckBox::onStateChanged(int state)
	{
		if (m_checkBox == nullptr)
		{
			Q_ASSERT(m_checkBox);
			return;
		}

        updateText();
		m_checkBox->setTristate(false);
        emit valueChanged(state == Qt::Checked ? true : false);
	}

	void MultiCheckBox::updateText()
	{
		if (m_checkBox == nullptr)
		{
			Q_ASSERT(m_checkBox);
			return;
		}

		switch (m_checkBox->checkState())
		{
			case Qt::Checked:           m_checkBox->setText("True");                break;
			case Qt::Unchecked:         m_checkBox->setText("False");               break;
			case Qt::PartiallyChecked:  m_checkBox->setText("<Different values>");  break;
			default:
				Q_ASSERT(false);
		}
	}

	//
	// ---------QtMultiVariantFactory----------
	//

	MultiVariantFactory::MultiVariantFactory(PropertyEditor* propertyEditor):
		QtAbstractEditorFactory<MultiVariantPropertyManager>(propertyEditor),
		m_propertyEditor(propertyEditor)
	{
	}

	QWidget* MultiVariantFactory::createEditor(MultiVariantPropertyManager* manager, QtProperty* property, QWidget* parent)
	{
		if (manager == nullptr)
		{
			Q_ASSERT(manager);
			return new QWidget();
		}
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return new QWidget();
		}

		m_manager = manager;
		m_property = property;

		QWidget* editor = nullptr;

		std::shared_ptr<Property> propertyPtr = manager->value(property);
		if (propertyPtr == nullptr)
		{
			assert(propertyPtr);
			return nullptr;
		}

		while (true)
		{

			if (variantIsPropertyList(propertyPtr->value()) == true || variantIsPropertyVector(propertyPtr->value()))
			{
				MultiArrayEdit* m_editor = new MultiArrayEdit(parent, propertyPtr, property->isEnabled() == false);
				editor = m_editor;

				if (manager->sameValue(property) == true)
				{
					m_editor->setValue(propertyPtr->value(), property->isEnabled() == false);
				}

				connect(m_editor, &MultiArrayEdit::valueChanged, this, &MultiVariantFactory::slotSetValue);
				connect(m_editor, &MultiArrayEdit::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);

				break;
			}

			if (propertyPtr->isEnum())
			{
				MultiEnumEdit* m_editor = new MultiEnumEdit(parent, propertyPtr, property->isEnabled() == false);
				editor = m_editor;

				if (manager->sameValue(property) == true)
				{
					m_editor->setValue(propertyPtr, property->isEnabled() == false);
				}

				connect(m_editor, &MultiEnumEdit::valueChanged, this, &MultiVariantFactory::slotSetValue);
				connect(m_editor, &MultiEnumEdit::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);

				break;
			}

			if (propertyPtr->value().userType() == FilePathPropertyType::filePathTypeId())
			{
				MultiFilePathEdit* m_editor = new MultiFilePathEdit(parent, property->isEnabled() == false);
				editor = m_editor;

				if (manager->sameValue(property) == true)
				{
					m_editor->setValue(propertyPtr, property->isEnabled() == false);
				}

				connect(m_editor, &MultiFilePathEdit::valueChanged, this, &MultiVariantFactory::slotSetValue);
				connect(m_editor, &MultiFilePathEdit::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);

				break;
			}

			if (propertyPtr->value().userType() == TuningValue::tuningValueTypeId())
			{
				MultiTextEdit* m_editor = new MultiTextEdit(parent, propertyPtr, property->isEnabled() == false, m_propertyEditor);

				editor = m_editor;

				if (manager->sameValue(property) == true)
				{
					m_editor->setValue(propertyPtr, property->isEnabled() == false);
				}

				connect(m_editor, &MultiTextEdit::valueChanged, this, &MultiVariantFactory::slotSetValue);
				connect(m_editor, &MultiTextEdit::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);

				break;
			}

			switch(propertyPtr->value().userType())
			{
			case QVariant::Bool:
			{
				MultiCheckBox* m_editor = new MultiCheckBox(parent);

				editor = m_editor;

				connect(m_editor, &MultiCheckBox::valueChanged, this, &MultiVariantFactory::slotSetValue);
				connect(m_editor, &MultiCheckBox::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);

				Qt::CheckState state;

				if (manager->sameValue(property) == false)
				{
					state = Qt::PartiallyChecked;
				}
				else
				{
					state = propertyPtr->value().toBool() ? Qt::Checked : Qt::Unchecked;
				}

				m_editor->setValue(state, m_property->isEnabled() == false);

				QTimer::singleShot(10, m_editor, &MultiCheckBox::changeValueOnButtonClick);

				//m_editor->changeValueOnButtonClick();
			}
				break;
			case QVariant::String:
			case QVariant::Int:
			case QVariant::UInt:
			case QMetaType::Float:
			case QVariant::Double:
			case QVariant::Uuid:
			case QVariant::ByteArray:
			case QVariant::Image:
			{
				MultiTextEdit* m_editor = new MultiTextEdit(parent, propertyPtr, property->isEnabled() == false, m_propertyEditor);

				editor = m_editor;

				if (manager->sameValue(property) == true)
				{
					m_editor->setValue(propertyPtr, property->isEnabled() == false);
				}

				connect(m_editor, &MultiTextEdit::valueChanged, this, &MultiVariantFactory::slotSetValue);
				connect(m_editor, &MultiTextEdit::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);
			}
				break;

			case QVariant::Color:
			{
				MultiColorEdit* m_editor = new MultiColorEdit(parent, property->isEnabled() == false);

				editor = m_editor;

				if (manager->sameValue(property) == true)
				{
					m_editor->setValue(propertyPtr, property->isEnabled() == false);
				}

				connect(m_editor, &MultiColorEdit::valueChanged, this, &MultiVariantFactory::slotSetValue);
				connect(m_editor, &MultiColorEdit::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);
			}
				break;

			default:
				Q_ASSERT(false);
			}

			break;
		}

		if (editor == nullptr)
		{
			Q_ASSERT(editor);
			return new QWidget();
		}

		return editor;
	}

	void MultiVariantFactory::connectPropertyManager (MultiVariantPropertyManager* manager)
	{
		Q_UNUSED(manager);
	}

	void MultiVariantFactory::disconnectPropertyManager(MultiVariantPropertyManager* manager)
	{
		Q_UNUSED(manager);
	}

	void MultiVariantFactory::slotSetValue(QVariant value)
	{
		if (m_manager == nullptr)
		{
			Q_ASSERT(m_manager);
			return;
		}
		if (m_property == nullptr)
		{
			Q_ASSERT(m_property);
			return;
		}

        m_manager->setAttribute(m_property, "@propertyEditor@sameValue", true);
        m_manager->emitSetValue(m_property, value);
	}

	void MultiVariantFactory::slotEditorDestroyed(QObject* object)
	{
		Q_UNUSED(object);
	}

	//
	// ---------QtMultiVariantPropertyManager----------
	//

	MultiVariantPropertyManager::MultiVariantPropertyManager(QObject* parent) :
		QtAbstractPropertyManager(parent)
	{

	}

	QVariant MultiVariantPropertyManager::attribute(const QtProperty* property, const QString& attribute) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return QVariant();
		}

		const QMap<const QtProperty*, Data>::const_iterator it = values.constFind(property);
		if (it == values.end())
		{
			Q_ASSERT(false);
			return QVariant();
		}

		const QMap<QString, QVariant>::const_iterator attrit = it.value().attributes.constFind(attribute);
		if (attrit == it.value().attributes.end())
		{
			Q_ASSERT(false);
			return QVariant();
		}

		return attrit.value();
	}

	bool MultiVariantPropertyManager::hasAttribute(const QtProperty* property, const QString& attribute) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return false;
		}

		const QMap<const QtProperty*, Data>::const_iterator it = values.constFind(property);
		if (it == values.end())
		{
			Q_ASSERT(false);
			return false;
		}

		const QMap<QString, QVariant>::const_iterator attrit = it.value().attributes.constFind(attribute);
		return attrit != it.value().attributes.end();
	}

	std::shared_ptr<Property> MultiVariantPropertyManager::value(const QtProperty* property) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return nullptr;
		}

		const QMap<const QtProperty*, Data>::const_iterator it = values.constFind(property);
		if (it == values.end())
		{
			Q_ASSERT(false);
			return nullptr;
		}

		return it->p;
	}

	int MultiVariantPropertyManager::valueType(const QtProperty* property) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return QVariant::Invalid;
		}

        return value(property)->value().type();
	}

	void MultiVariantPropertyManager::setProperty(const QtProperty* property, std::shared_ptr<Property> propertyValue)
    {
        if (property == nullptr)
        {
            assert(property);
            return;
        }

        const QMap<const QtProperty*, Data>::iterator it = values.find(property);
        if (it == values.end())
        {
            Q_ASSERT(false);
            return;
        }

        it->p = propertyValue;
    }

	bool MultiVariantPropertyManager::sameValue(const QtProperty* property) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return false;
		}

		QVariant attr = attribute(property, "@propertyEditor@sameValue");
		if (attr.isValid() == false)
		{
			return false;
		}

		return attr.toBool();

	}

	QSet<QtProperty*> MultiVariantPropertyManager::propertyByName(const QString& propertyName)
	{
		QSet<QtProperty*> result;
		QSet<QtProperty*> allProps = properties();

		for (auto p : allProps)
		{
			if (p->propertyName() == propertyName)
			{
				result << p;
			}
		}
		return result;
	}

	void MultiVariantPropertyManager::updateProperty(QtProperty* property)
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return;
		}

		emit propertyChanged(property);
	}

	void MultiVariantPropertyManager::emitSetValue(QtProperty* property, const QVariant& value)
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return;
		}

		emit valueChanged(property, value);

		emit propertyChanged(property);
	}



	void MultiVariantPropertyManager::setAttribute (QtProperty* property, const QString& attribute, const QVariant& value)
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return;
		}

		const QMap<const QtProperty*, Data>::iterator it = values.find(property);
		if (it == values.end())
		{
			Q_ASSERT(false);
			return;
		}

		it.value().attributes[attribute] = value;
	}


	void MultiVariantPropertyManager::initializeProperty(QtProperty* property)
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return;
		}

		values[property] = MultiVariantPropertyManager::Data();
	}
	void MultiVariantPropertyManager::uninitializeProperty(QtProperty* property)
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return;
		}

		values.remove(property);
	}

	QIcon MultiVariantPropertyManager::valueIcon(const QtProperty* property) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return QIcon();
		}

        // Get the property value from Property object
        //
        std::shared_ptr<Property> p = value(property);
        if (p == nullptr)
        {
            assert(false);
            return QIcon();
        }

        QVariant value = p->value();

        switch (value.userType())
		{
			case QVariant::Bool:
				{
					if (sameValue(property) == true)
					{
                        Qt::CheckState checkState = value.toBool() == true ? Qt::Checked : Qt::Unchecked;
						return drawCheckBox(checkState, property->isEnabled() == true);
					}
					else
					{
						return drawCheckBox(Qt::PartiallyChecked, property->isEnabled() == true);
					}
				}
				break;
			case QVariant::Color:
				{
					if (sameValue(property) == true)
					{
                        QColor color = value.value<QColor>();
						return drawColorBox(color);
					}
				}
				break;
		case QVariant::Image:
			{
				if (sameValue(property) == true)
				{
					QImage image = value.value<QImage>();
					return drawImage(image);
				}
			}
			break;
		}
		return QIcon();
	}

	QString MultiVariantPropertyManager::valueText(const QtProperty* property) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return QString();
		}

        // Get the property value from Property object
        //
        std::shared_ptr<Property> p = value(property);
        if (p == nullptr)
        {
            assert(false);
            return QString();
        }

        if (p->password() == true)
        {
            return "";
        }

        QVariant value = p->value();

        // Output the text
        //
        if (sameValue(property) == true)
		{
			// PropertyVector, PropertyList
			//
			if (variantIsPropertyVector(value) == true || variantIsPropertyList(value) == true)
			{
				return propertyVectorText(value);
			}

            // enum is special
            //
            if (p->isEnum())
            {
				int v = value.toInt();
                for (std::pair<int, QString>& i : p->enumValues())
                {
                    if (i.first == v)
                    {
                        return i.second;
                    }
                }
                return QString();
            }

            // all other types
            //
            int type = value.userType();

            if (type == FilePathPropertyType::filePathTypeId())
			{
                FilePathPropertyType f = value.value<FilePathPropertyType>();
				return f.filePath;
			}

			if (type == TuningValue::tuningValueTypeId())
			{
				TuningValue t = value.value<TuningValue>();
				return t.toString(p->precision());
			}

			char numberFormat = p->precision() > 5 ? 'g' : 'f';

			switch (type)
			{
				case QVariant::Int:
					{
                        int val = value.toInt();
						return QString::number(val);
					}
					break;

				case QVariant::UInt:
					{
                        quint32 val = value.toUInt();
						return QString::number(val);
					}
					break;

				case QMetaType::Float:
					{
						float val = value.toFloat();
						return QString::number(val, numberFormat, p->precision());
					}
					break;
				case QVariant::Double:
					{
                        double val = value.toDouble();
						return QString::number(val, numberFormat, p->precision());
					}
					break;
				case QVariant::Bool:
					{
                        return value.toBool() == true ? "True" : "False";
					}
					break;
				case QVariant::String:
					{
                        QString val = value.toString();

						if (val.length() > PropertyEditorTextMaxLength)
						{
							val = tr("<%1 bytes>").arg(val.length());
						}

						val.replace("\n", " ");

						return val;
					}
					break;

				case QVariant::Color:
					{
                        QColor color = value.value<QColor>();
						QString val = QString("[%1;%2;%3;%4]").
									  arg(color.red()).
									  arg(color.green()).
									  arg(color.blue()).
									  arg(color.alpha());
						return val;
					}
					break;

				case QVariant::Uuid:
					{
                        QUuid uuid = value.value<QUuid>();
						return uuid.toString();
					}
					break;

				case QVariant::ByteArray:
					{
						QByteArray array = value.value<QByteArray>();
						return tr("Data <%1 bytes>").arg(array.size());
					}
					break;

				case QVariant::Image:
					{
						QImage image = value.value<QImage>();
						return tr("Image <Width = %1 Height = %2>").arg(image.width()).arg(image.height());
					}
					break;

			default:
					Q_ASSERT(false);
			}
		}
		else
		{
			// PropertyVector, PropertyList
			//
			if (variantIsPropertyVector(value) == true)
			{
				return tr("<PropertyVector>");
			}

			if (variantIsPropertyList(value) == true)
			{
				return tr("<PropertyList>");
			}

			switch (value.type())
			{
				case QVariant::Bool:
					return "<Different values>";
				default:
					return QString();
			}
		}

		return QString();
	}

	QString MultiVariantPropertyManager::displayText(const QtProperty* property) const
	{
		QString str = property->propertyName();

		int slashIndex = str.lastIndexOf("\\");

		if (slashIndex != -1)
		{
			str = str.right(str.length() - slashIndex - 1);
		}

		return str;
	}

	//
	// ------- Property Editor ----------
	//

	PropertyEditor::PropertyEditor(QWidget* parent) :
		QtTreePropertyBrowser(parent)
	{
		setResizeMode(ResizeMode::Interactive);

		setAlternatingRowColors(false);

		m_propertyGroupManager = new QtGroupPropertyManager(this);
		m_propertyVariantManager = new MultiVariantPropertyManager(this);

		MultiVariantFactory* editFactory = new MultiVariantFactory(this);

		setFactoryForManager(m_propertyVariantManager, editFactory);

		connect(m_propertyVariantManager, &MultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);

		connect(this, &PropertyEditor::showErrorMessage, this, &PropertyEditor::onShowErrorMessage, Qt::QueuedConnection);

		connect(this, &QtTreePropertyBrowser::currentItemChanged, this, &PropertyEditor::onCurrentItemChanged);

		setScriptHelp(tr("<h1>This is a sample script help!</h1>"));

		m_scriptHelpWindowPos = QPoint(-1, -1);

		return;
	}

	void PropertyEditor::saveSettings()
	{

	}

	void PropertyEditor::onCurrentItemChanged(QtBrowserItem* current)
	{
		if (current == nullptr)
		{
			return;
		}


		if (current->property() == nullptr)
		{
			Q_ASSERT(current->property());
			return;
		}
	}

	void PropertyEditor::setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects)
	{
		QList<std::shared_ptr<PropertyObject>> list =
				QList<std::shared_ptr<PropertyObject>>::fromVector(QVector<std::shared_ptr<PropertyObject>>::fromStdVector(objects));

		return setObjects(list);
	}

	void PropertyEditor::setObjects(const QList<std::shared_ptr<PropertyObject>>& objects)
	{
        setVisible(false);

		// Disconnect updatePropertiesList slot from previous objects

		for (std::shared_ptr<PropertyObject> po : m_objects)
		{
			bool ok = disconnect(po.get(), &PropertyObject::propertyListChanged, this, &PropertyEditor::updatePropertiesList);
			if (ok == false)
			{
				assert(false);
			}
		}

		m_objects = objects;

		fillProperties();

		// Connect updatePropertiesList slot to new objects

		for (std::shared_ptr<PropertyObject> po : m_objects)
		{
			bool ok =connect(po.get(), &PropertyObject::propertyListChanged, this, &PropertyEditor::updatePropertiesList);
			if (ok == false)
			{
				assert(false);
			}
		}

		setVisible(true);

		return;
	}

	const QList<std::shared_ptr<PropertyObject>>& PropertyEditor::objects() const
	{
		return m_objects;
	}

	QtProperty* PropertyEditor::createProperty(QtProperty* parentProperty, const QString& caption, const QString& category, const QString& description, const std::shared_ptr<Property> propertyPtr, bool sameValue, bool readOnly)
	{
        if (parentProperty == nullptr)
		{
            // Add the property now
			//
			QtProperty* subGroup = nullptr;

            QList<QtProperty*> propList = properties();
			for (QtProperty* p : propList)
			{
                if (p->propertyName() == category)
				{
					subGroup = p;
					break;
				}
			}

			if (subGroup == nullptr)
			{
                subGroup = m_propertyGroupManager->addProperty(category);
			}

			QtProperty* property = createProperty(subGroup, caption, category, description, propertyPtr, sameValue, readOnly);

			if (parentProperty == nullptr)
			{
				addProperty(subGroup);
			}
			else
			{
				parentProperty->addSubProperty(subGroup);
			}

			return property;
		}
		else
		{
			// Add the property now
			//
			QtProperty* subProperty = nullptr;

            subProperty = m_propertyVariantManager->addProperty(caption);
            subProperty->setToolTip(description);
			subProperty->setEnabled(m_readOnly == false && readOnly == false);


			if (propertyPtr->essential() == true)
			{
				//subProperty->setBackgroundColor(QColor(0xEA, 0xF0, 0xFF));
				subProperty->setBackgroundColor(QColor(0xf0, 0xf0, 0xf0));
			}

			m_propertyVariantManager->setProperty(subProperty, propertyPtr);
            m_propertyVariantManager->setAttribute(subProperty, "@propertyEditor@sameValue", sameValue);



			if (parentProperty == nullptr)
			{
				Q_ASSERT(parentProperty);
				return nullptr;
			}

			parentProperty->addSubProperty(subProperty);

			return subProperty;
		}

	}

	bool PropertyEditor::createPropertyStructsSortFunc(const CreatePropertyStruct& cs1, const CreatePropertyStruct& cs2)
	{
		return std::make_tuple(cs1.category, cs1.property->viewOrder(), cs1.caption)  < std::make_tuple(cs2.category, cs2.property->viewOrder(), cs2.caption);
	}

	void PropertyEditor::updatePropertyValues(const QString& propertyName)
	{
        QSet<QtProperty*> props;
        QMap<QtProperty*, std::pair<QVariant, bool>> vals;

		if (propertyName.isEmpty() == true)
		{
			props = m_propertyVariantManager->properties();
		}
		else
		{
			props = m_propertyVariantManager->propertyByName(propertyName);
		}

		createValuesMap(props, vals);

		for (auto p : props)
		{
            bool sameValue = vals.value(p).second;

            m_propertyVariantManager->setAttribute(p, "@propertyEditor@sameValue", sameValue);

			m_propertyVariantManager->updateProperty(p);
        }
	}

	void PropertyEditor::updatePropertiesList()
	{
		setVisible(false);

		fillProperties();

		setVisible(true);

		return;
	}

	void PropertyEditor::updatePropertiesValues()
	{
		updatePropertyValues(QString());
	}

	void PropertyEditor::fillProperties()
	{
		clearProperties();

		QMap<QString, std::shared_ptr<Property>> propertyItems;
		QList<QString> propertyNames;

		std::vector<CreatePropertyStruct> createPropertyStructs;

		// Create a map with all properties
		//

		for (std::shared_ptr<PropertyObject> pobject : m_objects)
		{
			PropertyObject* object = pobject.get();

			for (std::shared_ptr<Property> p : object->properties())
			{
				if (p->visible() == false)
				{
					continue;
				}

				if (p->expert() && m_expertMode == false)
				{
					continue;
				}

				propertyItems.insertMulti(p->caption(), p);

				if (propertyNames.indexOf(p->caption()) == -1)
				{
					propertyNames.append(p->caption());
				}
			}
		}

		// add only common properties with same type
		//
		for (auto name : propertyNames)
		{
			// take all properties witn the same name
			//
			QList<std::shared_ptr<Property>> propsByName = propertyItems.values(name);
			if (propsByName.size() != m_objects.size() || propsByName.size() == 0)
			{
				continue;   // this property is not in all objects
			}

			// now check if all properties have the same type and values
			//
			int type;
			QVariant value;

			bool sameType = true;
			bool sameValue = true;
			bool readOnly = false;

			for (auto p = propsByName.begin(); p != propsByName.end(); p++)
			{
				if (p == propsByName.begin())
				{
					Property* _p = p->get();

					// remember the first item params
					//
					type = _p->value().userType();
					value = _p->value();

					if (_p->readOnly() == true)
					{
						readOnly = true;
					}
				}
				else
				{
					Property* _p = p->get();

					if (_p->readOnly() == true)
					{
						readOnly = true;
					}

					// compare with next item params
					//
					if (_p->value().userType() != type)
					{
						sameType = false;
						break;
					}

					if (_p->isEnum())
					{
						if (value.toInt() != _p->value().toInt())
						{
							sameValue = false;
						}
					}
					else
					{
						if (value != _p->value())
						{
							sameValue = false;
						}
					}
				}
			}

			if (sameType == false)
			{
				continue;   // properties are not the same type
			}

			// add the property to the editor
			//
			std::shared_ptr<Property> p = propsByName[0];
			if (p == nullptr)
			{
				assert(p);
				continue;
			}

			// set the description and limits
			//
			QString description = p->description().isEmpty() ? p->caption() : p->description();

			if (p->readOnly() == true || m_readOnly == true)
			{
				description = QString("[ReadOnly] ") + description;
			}

			if (p->specific() && p->value().userType() == QMetaType::Float)
			{
				bool ok1 = false;
				bool ok2 = false;
				float l = p->lowLimit().toFloat(&ok1);
				float h = p->highLimit().toFloat(&ok2);

				if (ok1 == true && ok2 == true && l < h)
				{
					description = QString("%1 {%2 - %3}").arg(description).arg(l).arg(h);
				}
			}

			if (p->specific() && p->value().userType() == QVariant::Double)
			{
				bool ok1 = false;
				bool ok2 = false;
				double l = p->lowLimit().toDouble(&ok1);
				double h = p->highLimit().toDouble(&ok2);

				if (ok1 == true && ok2 == true && l < h)
				{
					description = QString("%1 {%2 - %3}").arg(description).arg(l).arg(h);
				}
			}

			if (p->specific() && p->value().userType() == QVariant::Int)
			{
				bool ok1 = false;
				bool ok2 = false;
				int l = p->lowLimit().toInt(&ok1);
				int h = p->highLimit().toInt(&ok2);

				if (ok1 == true && ok2 == true && l < h)
				{
					description = QString("%1 {%2 - %3}").arg(description).arg(l).arg(h);
				}
			}

			if (p->specific() && p->value().userType() == QVariant::UInt)
			{
				bool ok1 = false;
				bool ok2 = false;
				uint l = p->lowLimit().toUInt(&ok1);
				uint h = p->highLimit().toUInt(&ok2);

				if (ok1 == true && ok2 == true && l < h)
				{
					description = QString("%1 {%2 - %3}").arg(description).arg(l).arg(h);
				}
			}

			QString category = p->category();
			if (category.isEmpty())
			{
				category = "Common";
			}

			CreatePropertyStruct cs;
			cs.property = p;
			cs.caption = p->caption();
			cs.category = category;
			cs.description = description;
			cs.sameValue = sameValue;
			cs.readOnly = readOnly;

			createPropertyStructs.push_back(cs);
		}

		// Sort here

		std::sort(createPropertyStructs.begin(), createPropertyStructs.end(), createPropertyStructsSortFunc);

		// Sort

		for (const CreatePropertyStruct& cs : createPropertyStructs)
		{
			createProperty(nullptr, cs.caption, cs.category, cs.description, cs.property, cs.sameValue, cs.readOnly);
		}

		//sortItems(0, Qt::AscendingOrder);
	}

	void PropertyEditor::clearProperties()
	{
		m_propertyVariantManager->clear();
		m_propertyGroupManager->clear();
		clear();
	}

    void PropertyEditor::createValuesMap(const QSet<QtProperty*>& props, QMap<QtProperty*, std::pair<QVariant, bool>>& values)
	{
        values.clear();

		for (auto p = props.begin(); p != props.end(); p++)
		{
			QtProperty* property = *p;

			bool sameValue = true;
			QVariant value;

            for (auto i = m_objects.begin(); i != m_objects.end(); i++)
			{
                PropertyObject* pObject = i->get();

                QVariant val = pObject->propertyValue(property->propertyName());

                if (i == m_objects.begin())
				{
					value = val;
				}
				else
				{
					if (value != val)
					{
						sameValue = false;
						break;
					}
				}
			}

            values.insert(property, std::make_pair(value, sameValue));
        }
	}

	void PropertyEditor::setExpertMode(bool expertMode)
	{
		m_expertMode = expertMode;
	}

	bool PropertyEditor::readOnly() const
	{
		return m_readOnly;
	}

	void PropertyEditor::setReadOnly(bool readOnly)
	{
		m_readOnly = readOnly;
	}

	void PropertyEditor::setScriptHelp(const QString& text)
	{
		m_scriptHelp = text;
	}

	QString PropertyEditor::scriptHelp() const
	{
		return m_scriptHelp;
	}

	QPoint PropertyEditor::scriptHelpWindowPos() const
	{
		return m_scriptHelpWindowPos;
	}

	void PropertyEditor::setScriptHelpWindowPos(const QPoint& value)
	{
		m_scriptHelpWindowPos = value;
	}

	QByteArray PropertyEditor::scriptHelpWindowGeometry() const
	{
		return m_scriptHelpWindowGeometry;

	}
	void PropertyEditor::setScriptHelpWindowGeometry(const QByteArray& value)
	{
		m_scriptHelpWindowGeometry = value;
	}

	void PropertyEditor::onValueChanged(QtProperty* property, QVariant value)
	{
       valueChanged(property, value);
	}

	void PropertyEditor::valueChanged(QtProperty* property, QVariant value)
	{
		// Set the new property value in all objects
		//
        QList<std::shared_ptr<PropertyObject>> modifiedObjects;

		QString errorString;

		QString propertyName = property->propertyName();

		for (auto i : m_objects)
		{
            PropertyObject* pObject = i.get();

			// Do not set property, if it has same value

			QVariant oldValue = pObject->propertyValue(propertyName);

			if (oldValue == value)
			{
				continue;
			}

			// Warning!!! If property changing changes the list of properties (e.g. SpecificProperties),
			// property pointer becomes unusable! So next calls to property-> will cause crash

			pObject->setPropertyValue(propertyName, value);

			QVariant newValue = pObject->propertyValue(propertyName);

			if (oldValue == newValue && errorString.isEmpty() == true)
			{
				errorString = QString("Property: %1 - incorrect input value")
							  .arg(propertyName);
			}

			modifiedObjects.append(i);
		}

		if (errorString.isEmpty() == false)
		{
			emit showErrorMessage(errorString);
		}

		if (modifiedObjects.count() > 0)
		{
            emit propertiesChanged(modifiedObjects);
		}
		return;
	}

	PropertyTextEditor* PropertyEditor::createPropertyTextEditor(Property* property, QWidget* parent)
	{
        Q_UNUSED(property);
		return new PropertyPlainTextEditor(parent);
	}

	void PropertyEditor::onShowErrorMessage(QString message)
	{
		QMessageBox::warning(this, "Error", message);
	}

}
