#include "Stable.h"
#include "../lib/PropertyEditor.h"
#include "../lib/Ui/UiTools.h"
#include "TuningValue.h"

#include <QToolButton>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QColorDialog>
#include <QTextBrowser>
#include <QPlainTextEdit>
#include <QDesktopServices>

namespace ExtWidgets
{

	QString PropertyEditorBase::m_commonCategoryName = "Common";

	//
	// PropertyTools
	//

	QString PropertyTools::propertyVectorText(QVariant& value)
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

	QString PropertyTools::stringListText(const QVariant& value)
	{
		if (value.userType() == QVariant::StringList)
		{
			const int TextMaxLength = 128;

			QStringList strings = value.toStringList();

			QString val = strings.join(' ');

			if (val.length() > TextMaxLength)
			{
				val = QObject::tr("QStringList [%1 items]").arg(static_cast<int>(strings.size()));
			}

			val.remove(QChar::LineFeed);
			val.remove(QChar::CarriageReturn);

			return val;
		}

		Q_ASSERT(false);
		return QString();
	}

	QString PropertyTools::colorVectorText(QVariant& value)
	{
		QVector<QColor> v = value.value<QVector<QColor>>();
		return QString("QVector<QColor> [%1 items]").arg(static_cast<int>(v.size()));
	}

	QString PropertyTools::propertyValueText(Property* p, int row)
	{
		QVariant value = p->value();

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
			return t.toString();
		}

		if (type == qMetaTypeId<QVector<QColor>>())
		{
			return colorVectorText(value);
		}

		if (type == qMetaTypeId<QDateTime>())
		{
			return value.toString();
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
				val = QObject::tr("<%1 bytes>").arg(val.length());
			}

			val.replace("\n", " ");

			return val;
		}
			break;

		case QVariant::StringList:
		{
			if (row != -1)
			{
				QStringList strings = value.toStringList();

				if (row < 0 || row >= static_cast<int>(strings.size()))
				{
					// No string exists in this row, it is ok - other property can have more rows
					//
					return QString();
				}

				return strings[row];
			}

			return stringListText(value);
		}
			break;

		case QVariant::Color:
		{
			QColor color = value.value<QColor>();

			return PropertyEditorBase::colorToText(color);
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
			return QObject::tr("Data <%1 bytes>").arg(array.size());
		}
			break;

		case QVariant::Image:
		{
			QImage image = value.value<QImage>();
			return QObject::tr("Image <Width = %1 Height = %2>").arg(image.width()).arg(image.height());
		}
			break;

		default:
			Q_ASSERT(false);
		}

		return QString();
	}

	//
	// ------------ PropertyEditorBase ------------
	//

	PropertyEditorBase::PropertyEditorBase()
	{
		QSettings s;
		thePropertyEditorSettings.restore(s);
	}

	PropertyEditorBase::~PropertyEditorBase()
	{
		QSettings s;
		thePropertyEditorSettings.store(s);
	}

	PropertyEditor* PropertyEditorBase::createChildPropertyEditor(QWidget* parent)
	{
		return new PropertyEditor(parent);
	}

	PropertyTextEditor* PropertyEditorBase::createPropertyTextEditor(std::shared_ptr<Property> propertyPtr, QWidget* parent)
	{
		Q_UNUSED(propertyPtr);
		return new PropertyPlainTextEditor(parent);
	}

	bool PropertyEditorBase::expertMode() const
	{
		return m_expertMode;
	}

	void PropertyEditorBase::setExpertMode(bool expertMode)
	{
		m_expertMode = expertMode;
	}

	bool PropertyEditorBase::isReadOnly() const
	{
		return m_readOnly;
	}

	void PropertyEditorBase::setReadOnly(bool readOnly)
	{
		m_readOnly = readOnly;
	}



	QString PropertyEditorBase::colorToText(QColor color)
	{
		return QString("[%1;%2;%3;%4]").
				arg(color.red()).
				arg(color.green()).
				arg(color.blue()).
				arg(color.alpha());
	}

	QColor PropertyEditorBase::colorFromText(const QString& t)
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

	QIcon PropertyEditorBase::drawCheckBox(int state, bool enabled)
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

	QIcon PropertyEditorBase::drawImage(const QImage& image)
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

	QIcon PropertyEditorBase::propertyIcon(Property* p, bool sameValue, bool enabled)
	{
		QVariant value = p->value();

		switch (value.userType())
		{
			case QVariant::Bool:
				{
					if (sameValue == true)
					{
						Qt::CheckState checkState = value.toBool() == true ? Qt::Checked : Qt::Unchecked;
						return drawCheckBox(checkState, enabled);
					}
					else
					{
						return drawCheckBox(Qt::PartiallyChecked, enabled);
					}
				}
				break;
			case QVariant::Color:
				{
					if (sameValue == true)
					{
						QColor color = value.value<QColor>();
						return UiTools::drawColorBox(color);
					}
				}
				break;
		case QVariant::Image:
			{
				if (sameValue == true)
				{
					QImage image = value.value<QImage>();
					return drawImage(image);
				}
			}
			break;
		}
		return QIcon();
	}

	PropertyEditCellWidget* PropertyEditorBase::createCellEditor(std::shared_ptr<Property> propertyPtr, bool sameValue, bool readOnly, QWidget* parent)
	{
		return createCellRowEditor(propertyPtr, -1, sameValue, readOnly, parent);
	}

	PropertyEditCellWidget* PropertyEditorBase::createCellRowEditor(std::shared_ptr<Property> propertyPtr, int row, bool sameValue, bool readOnly, QWidget* parent)
	{
		if (propertyPtr == nullptr)
		{
			Q_ASSERT(propertyPtr);
			return new PropertyEditCellWidget(parent);
		}

		PropertyEditCellWidget* editor = nullptr;

		while (true)
		{

			if (variantIsPropertyList(propertyPtr->value()) == true || variantIsPropertyVector(propertyPtr->value()))
			{
				MultiArrayEdit* m_editor = new MultiArrayEdit(this, parent, propertyPtr, readOnly);
				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

				break;
			}

			if (propertyPtr->isEnum())
			{
				MultiEnumEdit* m_editor = new MultiEnumEdit(parent, propertyPtr, readOnly);
				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

				break;
			}

			if (propertyPtr->value().userType() == FilePathPropertyType::filePathTypeId())
			{
				MultiFilePathEdit* m_editor = new MultiFilePathEdit(parent, readOnly);
				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

				break;
			}

			if (propertyPtr->value().userType() == QVariant::StringList)
			{
				if (row == -1)
				{
					MultiArrayEdit* m_editor = new MultiArrayEdit(this, parent, propertyPtr, readOnly);
					editor = m_editor;

					if (sameValue == true)
					{
						m_editor->setValue(propertyPtr, readOnly);
					}

				}
				else
				{
					MultiTextEdit* m_editor = new MultiTextEdit(this, propertyPtr, row, readOnly, parent);

					editor = m_editor;

					if (sameValue == true)
					{
						m_editor->setValue(propertyPtr, readOnly);
					}
				}

				break;
			}

			if (propertyPtr->value().userType() == TuningValue::tuningValueTypeId())
			{
				MultiTextEdit* m_editor = new MultiTextEdit(this, propertyPtr, readOnly, parent);

				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

				break;
			}

			if (propertyPtr->value().userType() == qMetaTypeId<QVector<QColor>>())
			{
				MultiArrayEdit* m_editor = new MultiArrayEdit(this, parent, propertyPtr, readOnly);
				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

				break;
			}

			switch(propertyPtr->value().userType())
			{
			case QVariant::Bool:
			{
				MultiCheckBox* m_editor = new MultiCheckBox(parent, readOnly);

				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

				QTimer::singleShot(10, m_editor, &MultiCheckBox::changeValueOnButtonClick);

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
				MultiTextEdit* m_editor = new MultiTextEdit(this, propertyPtr, readOnly, parent);

				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

			}
				break;

			case QVariant::Color:
			{
				MultiColorEdit* m_editor = new MultiColorEdit(parent, readOnly);

				editor = m_editor;

				if (sameValue == true)
				{
					m_editor->setValue(propertyPtr, readOnly);
				}

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
			return new PropertyEditCellWidget(parent);
		}

		return editor;
	}

	void PropertyEditorBase::setScriptHelpFile(const QString& scriptHelpFile)
	{
		m_scriptHelpFile = scriptHelpFile;
	}

	QString PropertyEditorBase::scriptHelpFile() const
	{
		return m_scriptHelpFile;
	}

	//
	// ------------ PropertyArrayEditorDialog ------------
	//
	PropertyArrayEditorDialog::PropertyArrayEditorDialog(PropertyEditorBase* propertyEditorBase, QWidget* parent, const QString& propertyName, const QVariant& value):
		QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint),
		m_value(value),
		m_propertyEditorBase(propertyEditorBase)
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

		m_childPropertyEditor = m_propertyEditorBase->createChildPropertyEditor(this);
		connect(m_childPropertyEditor, &PropertyEditor::propertiesChanged, this, &PropertyArrayEditorDialog::onPropertiesChanged);

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

		QVBoxLayout* treeAddRemoveLayout = new QVBoxLayout();
		treeAddRemoveLayout->addWidget(m_treeWidget);
		treeAddRemoveLayout->addLayout(addRemoveLayout);

		// Top Layout

		QWidget* leftWidget = new QWidget();

		QHBoxLayout* leftLayout = new QHBoxLayout(leftWidget);
		leftLayout->setContentsMargins(0, 0, 0, 0);
		leftLayout->addLayout(treeAddRemoveLayout);
		leftLayout->addLayout(moveLayout);


		// Splitter

		m_splitter = new QSplitter();

		m_splitter->addWidget(leftWidget);
		m_splitter->addWidget(m_childPropertyEditor);

		// Ok/Cancel

		QHBoxLayout* buttonsLayout = new QHBoxLayout();
		buttonsLayout->addStretch();

		b = new QPushButton(tr("OK"), this);
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::accept);
		buttonsLayout->addWidget(b);

		b = new QPushButton(tr("Cancel"), this);
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::reject);
		buttonsLayout->addWidget(b);

		mainLayout->addWidget(m_splitter);

		//

		mainLayout->addLayout(buttonsLayout);

		setLayout(mainLayout);

		updateDescriptions();

		if (m_treeWidget->topLevelItemCount() > 0)
		{
			m_treeWidget->topLevelItem(0)->setSelected(true);
		}

		if (thePropertyEditorSettings.m_arrayPropertyEditorSize != QSize(-1, -1))
		{
			resize(thePropertyEditorSettings.m_arrayPropertyEditorSize);
		}

		if (thePropertyEditorSettings.m_arrayPropertyEditorSplitterState.isEmpty() == false)
		{
			m_splitter->restoreState(thePropertyEditorSettings.m_arrayPropertyEditorSplitterState);
		}
	}

	PropertyArrayEditorDialog::~PropertyArrayEditorDialog()
	{
		thePropertyEditorSettings.m_arrayPropertyEditorSplitterState = m_splitter->saveState();
		thePropertyEditorSettings.m_arrayPropertyEditorSize = size();
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
		if (m_treeWidget == nullptr || m_childPropertyEditor == nullptr)
		{
			Q_ASSERT(m_treeWidget);
			Q_ASSERT(m_childPropertyEditor);
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

		m_childPropertyEditor->setObjects(propertyObjectList);

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
	// ------------ VectorEditorDialog ------------
	//
	VectorEditorDialog::VectorEditorDialog(QWidget* parent, const QString& propertyName, const QVariant& value):
		QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint),
		m_valueType(value.userType())
	{
		if (isValueColorVector() == false && isValueStringList() == false)
		{
			Q_ASSERT(false);
			return;
		}

		if (isValueStringList())
		{
			m_strings = value.toStringList();
		}
		if (isValueColorVector())
		{
			m_colors = value.value<QVector<QColor>>();
		}

		setWindowTitle(propertyName);

		setMinimumSize(320, 480);

		QVBoxLayout* mainLayout = new QVBoxLayout();

		// Create Editor

		m_treeWidget = new QTreeWidget();

		QStringList headerLabels;

		if (isValueStringList())
		{
			headerLabels << tr("Strings");
		}
		if (isValueColorVector())
		{
			headerLabels << tr("Colors");
		}

		m_treeWidget->setColumnCount(headerLabels.size());
		m_treeWidget->setHeaderLabels(headerLabels);
		m_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_treeWidget->setRootIsDecorated(false);

		m_treeWidget->header()->hide();

		connect (m_treeWidget, &QTreeWidget::itemChanged, this, &VectorEditorDialog::itemChanged);

		// Buttons

		QVBoxLayout* buttonsLayout = new QVBoxLayout();

		QPushButton* b = new QPushButton(tr("Add"));
		connect(b, &QPushButton::clicked, this, &VectorEditorDialog::onAdd);
		buttonsLayout->addWidget(b);

		b = new QPushButton(tr("Remove"));
		connect(b, &QPushButton::clicked, this, &VectorEditorDialog::onRemove);
		buttonsLayout->addWidget(b);

		b = new QPushButton(tr("Up"));
		connect(b, &QPushButton::clicked, this, &VectorEditorDialog::onMoveUp);
		buttonsLayout->addWidget(b);

		b = new QPushButton(tr("Down"));
		connect(b, &QPushButton::clicked, this, &VectorEditorDialog::onMoveDown);
		buttonsLayout->addWidget(b);

		buttonsLayout->addStretch();

		// Top Layout

		QWidget* leftWidget = new QWidget();

		QHBoxLayout* leftLayout = new QHBoxLayout(leftWidget);
		leftLayout->setContentsMargins(0, 0, 0, 0);
		leftLayout->addWidget(m_treeWidget);
		leftLayout->addLayout(buttonsLayout);

		// Ok/Cancel

		QHBoxLayout* okCancelButtonsLayout = new QHBoxLayout();
		okCancelButtonsLayout->addStretch();

		b = new QPushButton(tr("OK"), this);
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::accept);
		okCancelButtonsLayout->addWidget(b);

		b = new QPushButton(tr("Cancel"), this);
		connect(b, &QPushButton::clicked, this, &PropertyArrayEditorDialog::reject);
		okCancelButtonsLayout->addWidget(b);

		mainLayout->addWidget(leftWidget);

		//

		mainLayout->addLayout(okCancelButtonsLayout);

		setLayout(mainLayout);

		updateVectorData();

		if (m_treeWidget->topLevelItemCount() > 0)
		{
			m_treeWidget->topLevelItem(0)->setSelected(true);
		}

		if (thePropertyEditorSettings.m_vectorEditorSize != QSize(-1, -1))
		{
			resize(thePropertyEditorSettings.m_vectorEditorSize);
		}

	}

	VectorEditorDialog::~VectorEditorDialog()
	{
		thePropertyEditorSettings.m_vectorEditorSize = size();
	}

	QVariant VectorEditorDialog::value()
	{
		if (isValueStringList())
		{
			return m_strings;
		}
		if (isValueColorVector())
		{
			return QVariant::fromValue(m_colors);
		}

		Q_ASSERT(false);
		return QVariant();
	}

	void VectorEditorDialog::onMoveUp()
	{
		moveItems(false);
	}

	void VectorEditorDialog::onMoveDown()
	{
		moveItems(true);
	}

	void VectorEditorDialog::onAdd()
	{
		if (m_treeWidget == nullptr)
		{
			Q_ASSERT(m_treeWidget);
			return;
		}

		// Create new item

		if (isValueStringList())
		{
			m_strings.push_back(tr("String - %1").arg(m_strings.size() + 1));
		}

		if (isValueColorVector())
		{
			QColor color = Qt::black;
			m_colors.push_back(color);
		}

		m_treeWidget->clearSelection();

		updateVectorData();

		QTreeWidgetItem* item = m_treeWidget->topLevelItem(m_treeWidget->topLevelItemCount() - 1);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		item->setSelected(true);

		return;
	}

	void VectorEditorDialog::onRemove()
	{
		if (m_treeWidget == nullptr)
		{
			Q_ASSERT(m_treeWidget);
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

			if (isValueStringList())
			{
				if (row < 0 || row >= static_cast<int>(m_strings.size()))
				{
					Q_ASSERT(false);
					return;
				}

				m_strings.removeAt(row);
			}
			if (isValueColorVector())
			{
				if (row < 0 || row >= static_cast<int>(m_colors.size()))
				{
					Q_ASSERT(false);
					return;
				}

				m_colors.removeAt(row);
			}
		}

		updateVectorData();

		return;
	}

	void VectorEditorDialog::itemChanged(QTreeWidgetItem *item, int column)
	{
		Q_UNUSED(column);

		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		int index = m_treeWidget->indexOfTopLevelItem(item);

		if (isValueStringList())
		{
			if (index < 0 || index >= static_cast<int>(m_strings.size()))
			{
				Q_ASSERT(false);
				return;
			}

			m_strings[index] = item->text(0);
		}
		if (isValueColorVector())
		{
			if (index < 0 || index >= static_cast<int>(m_colors.size()))
			{
				Q_ASSERT(false);
				return;
			}

			QColor color = PropertyEditorBase::colorFromText(item->text(0));
			if (color.isValid() == false)
			{
				return;
			}

			m_colors[index] = color;

			m_treeWidget->blockSignals(true);
			item->setText(0, PropertyEditorBase::colorToText(color));
			item->setIcon(0, UiTools::drawColorBox(color));
			m_treeWidget->blockSignals(false);
		}

		return;
	}

	void VectorEditorDialog::updateVectorData()
	{
		m_treeWidget->blockSignals(true);

		int count = 0;
		if (isValueStringList())
		{
			count = static_cast<int>(m_strings.size());
		}
		if (isValueColorVector())
		{
			count = static_cast<int>(m_colors.size());
		}

		for (int i = 0; i < count; i++)
		{
			QTreeWidgetItem* twi = nullptr;

			if (m_treeWidget->topLevelItemCount() > i)
			{
				twi = m_treeWidget->topLevelItem(i);
			}
			else
			{
				twi = new QTreeWidgetItem();
				twi->setFlags(twi->flags() | Qt::ItemIsEditable);
				m_treeWidget->addTopLevelItem(twi);

			}

			if (isValueStringList())
			{
				twi->setText(0, m_strings[i]);
			}
			if (isValueColorVector())
			{
				QColor color = m_colors[i];

				twi->setText(0, PropertyEditorBase::colorToText(color));
				twi->setIcon(0, UiTools::drawColorBox(color));
			}
		}

		m_treeWidget->blockSignals(false);

		return;
	}

	void VectorEditorDialog::moveItems(bool forward)
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

		int count = 0;
		if (isValueStringList())
		{
			count = static_cast<int>(m_strings.size());
		}
		if (isValueColorVector())
		{
			count = static_cast<int>(m_colors.size());
		}

		for (int row : selectedRows)
		{
			int row2 = row + direction;
			if (row2 < 0 || row2 >= m_treeWidget->topLevelItemCount())
			{
				break;
			}

			if (row < 0 || row >= count ||
					row2 < 0 || row2 >= count)
			{
				Q_ASSERT(false);
				return;
			}

			if (isValueStringList())
			{
				std::swap(m_strings[row], m_strings[row2]);
			}
			if (isValueColorVector())
			{
				std::swap(m_colors[row], m_colors[row2]);
			}

			QTreeWidgetItem* item1 = m_treeWidget->topLevelItem(row);
			item1->setSelected(false);

			QTreeWidgetItem* item2 = m_treeWidget->topLevelItem(row2);
			item2->setSelected(true);
		}

		updateVectorData();

		return;
	}

	bool VectorEditorDialog::isValueStringList() const
	{
		return m_valueType == QVariant::StringList;
	}

	bool VectorEditorDialog::isValueColorVector() const
	{
		return m_valueType == qMetaTypeId<QVector<QColor>>();
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

	PropertyEditorHelpDialog::PropertyEditorHelpDialog(const QString& caption, const QString& text, QWidget* parent):
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

	PropertyEditorHelpDialog::~PropertyEditorHelpDialog()
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

		m_plainTextEdit->setTabStopDistance(metrics.horizontalAdvance(spaces));

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
	// ------------ PropertyEditCellWidget ------------
	//

	PropertyEditCellWidget::PropertyEditCellWidget(QWidget* parent)
		:QWidget(parent)
	{
	}

	PropertyEditCellWidget::~PropertyEditCellWidget()
	{
	}

	void PropertyEditCellWidget::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		Q_UNUSED(property);
		Q_UNUSED(readOnly);

		Q_ASSERT(false);
		return;
	}

	void PropertyEditCellWidget::setInitialText(const QString& text)
	{
		Q_UNUSED(text);
		return;
	}

	//
	// ------------ MultiFilePathEdit ------------
	//

	MultiFilePathEdit::MultiFilePathEdit(QWidget* parent, bool readOnly):
		PropertyEditCellWidget(parent)
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
		PropertyEditCellWidget(parent)
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

		QTimer::singleShot(0, m_combo, SLOT(setFocus()));
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
		PropertyEditCellWidget(parent)
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
		QColor color = PropertyEditorBase::colorFromText(t);

		QColorDialog dialog(color, this);
		if (dialog.exec() == QDialog::Accepted)
		{
			QColor selectedColor = dialog.selectedColor();

			QString str = PropertyEditorBase::colorToText(selectedColor);

			if (selectedColor != m_oldColor)
            {
				m_oldColor = selectedColor;
                m_lineEdit->setText(str);

				emit valueChanged(selectedColor);
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

		QString str = PropertyEditorBase::colorToText(color);

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
			QColor color = PropertyEditorBase::colorFromText(t);

            if (color != m_oldColor)
			{
                m_oldColor = color;
                emit valueChanged(color);
			}
		}
	}

	//
	// ---------MultiTextEditorDialog----------
	//
	MultiTextEditorDialog::MultiTextEditorDialog(PropertyEditorBase* propertyEditorBase, QWidget* parent, const QString &text, std::shared_ptr<Property> p):
		QDialog(parent, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
		m_propertyEditorBase(propertyEditorBase),
		m_property(p)
	{
		setWindowTitle(p->caption());

		if (thePropertyEditorSettings.m_multiLinePropertyEditorWindowPos.x() != -1 && thePropertyEditorSettings.m_multiLinePropertyEditorWindowPos.y() != -1)
		{
			move(thePropertyEditorSettings.m_multiLinePropertyEditorWindowPos);
			restoreGeometry(thePropertyEditorSettings.m_multiLinePropertyEditorGeometry);
		}
		else
		{
			resize(1024, 768);
		}

		QVBoxLayout* vl = new QVBoxLayout();

		// Create Editor

		m_editor = m_propertyEditorBase->createPropertyTextEditor(m_property, this);

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

		if (p->isScript() && m_propertyEditorBase->scriptHelpFile().isEmpty() == false)
		{
			QPushButton* helpButton = new QPushButton("?", this);

			hl->addWidget(helpButton);

			connect(helpButton, &QPushButton::clicked, [this] ()
			{
				UiTools::openHelp(m_propertyEditorBase->scriptHelpFile(), this);
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

		thePropertyEditorSettings.m_multiLinePropertyEditorWindowPos = pos();
		thePropertyEditorSettings.m_multiLinePropertyEditorGeometry = saveGeometry();

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

	MultiTextEdit::MultiTextEdit(PropertyEditorBase* propertyEditorBase, std::shared_ptr<Property> p, bool readOnly, QWidget* parent):
		MultiTextEdit(propertyEditorBase, p, -1, readOnly, parent)

	{
	}

	MultiTextEdit::MultiTextEdit(PropertyEditorBase* propertyEditorBase, std::shared_ptr<Property> p, int row, bool readOnly, QWidget* parent):
		PropertyEditCellWidget(parent),
		m_property(p),
		m_row(row),
		m_userType(p->value().userType()),
		m_propertyEditorBase(propertyEditorBase)
	{
		if (p == nullptr || m_propertyEditorBase == nullptr)
		{
			assert(p);
			assert(m_propertyEditorBase);
			return;
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
				(m_userType == QVariant::String && p->password() == false) ||
				(m_userType == QVariant::StringList && m_row != -1))
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

			MultiTextEditorDialog multlLineEdit(m_propertyEditorBase, this, m_oldValue.toString(), m_property);
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
		case QVariant::StringList:
			{
				QStringList l = property->value().toStringList();
				if (m_row < 0 || m_row >= static_cast<int>(l.size()))
				{
					Q_ASSERT(false);
					return;
				}

				QString editText = l[m_row];
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
				m_lineEdit->setText(t.toString());
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

	void MultiTextEdit::setInitialText(const QString& text)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

		m_lineEdit->setText(text);
		m_textEdited = true;
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
		case QVariant::StringList:
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
	// ---------MultiArrayEdit----------
	//
	MultiArrayEdit::MultiArrayEdit(PropertyEditorBase* propertyEditorBase, QWidget* parent, std::shared_ptr<Property> p, bool readOnly):
		PropertyEditCellWidget(parent),
		m_property(p),
		m_currentValue(p->value()),
		m_propertyEditorBase(propertyEditorBase)
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

		if (m_currentValue.userType() == QVariant::StringList)
		{
			m_lineEdit->setText("<StringList>");
		}

		if (m_currentValue.userType() == qMetaTypeId<QVector<QColor>>())
		{
			m_lineEdit->setText("QColor [0 items]");
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

	void MultiArrayEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		m_button->setEnabled(readOnly == false);

		m_currentValue = property->value();

		if (m_currentValue.userType() == QVariant::StringList)
		{
			m_lineEdit->setText(PropertyTools::stringListText(m_currentValue));
			return;
		}

		if (m_currentValue.userType() == qMetaTypeId<QVector<QColor>>())
		{
			m_lineEdit->setText(PropertyTools::colorVectorText(m_currentValue));
			return;
		}

		if (variantIsPropertyVector(m_currentValue) == true || variantIsPropertyList(m_currentValue) == true)
		{
			m_lineEdit->setText(PropertyTools::propertyVectorText(m_currentValue));
			return;
		}

		Q_ASSERT(false);
		return;
	}

	void MultiArrayEdit::onButtonPressed()
	{
		if (variantIsPropertyVector(m_currentValue) == false &&
				variantIsPropertyList(m_currentValue) == false &&
				m_currentValue.userType() != QVariant::StringList &&
				m_currentValue.userType() != qMetaTypeId<QVector<QColor>>())
		{
			Q_ASSERT(false);
			return;
		}

		QVariant newValue;

		if (variantIsPropertyVector(m_currentValue) == true || variantIsPropertyList(m_currentValue) == true)
		{
			PropertyArrayEditorDialog d(m_propertyEditorBase, this, m_property->caption(), m_currentValue);
			if (d.exec() != QDialog::Accepted)
			{
				return;
			}
			newValue = d.value();
		}

		if (m_currentValue.userType() == QVariant::StringList || m_currentValue.userType() == qMetaTypeId<QVector<QColor>>())
		{
			VectorEditorDialog d(this, m_property->caption(), m_currentValue);
			if (d.exec() != QDialog::Accepted)
			{
				return;
			}
			newValue = d.value();
		}

		if (newValue != m_currentValue)
		{
			 emit valueChanged(newValue);
		}

		m_currentValue = newValue;

		if (variantIsPropertyVector(m_currentValue) == true || variantIsPropertyList(m_currentValue) == true)
		{
			m_lineEdit->setText(PropertyTools::propertyVectorText(m_currentValue));
		}
		else
		{
			if (m_currentValue.userType() == QVariant::StringList)
			{
				m_lineEdit->setText(PropertyTools::stringListText(m_currentValue));
			}
			else
			{
				if ( m_currentValue.userType() == qMetaTypeId<QVector<QColor>>())
				{
					m_lineEdit->setText(PropertyTools::colorVectorText(m_currentValue));
				}
			}
		}

		return;
	}

	//
	// ---------QtMultiCheckBox----------
	//


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

	MultiCheckBox::MultiCheckBox(QWidget* parent, bool readOnly):
		PropertyEditCellWidget(parent)
	{
		m_checkBox = new PropertyEditorCheckBox(parent);

		m_checkBox->setEnabled(readOnly == false);

		m_checkBox->setCheckState(Qt::PartiallyChecked);
		updateText();

		connect(m_checkBox, &QCheckBox::stateChanged, this, &MultiCheckBox::onStateChanged);

		QHBoxLayout*lt = new QHBoxLayout;
		lt->setContentsMargins(3, 1, 0, 0);
		lt->addWidget(m_checkBox);
		setLayout(lt);
	}

	void MultiCheckBox::setValue(std::shared_ptr<Property> propertyPtr, bool readOnly)
	{
		if (m_checkBox == nullptr)
		{
			Q_ASSERT(m_checkBox);
			return;
		}

		m_checkBox->setTristate(false);

		Qt::CheckState state = propertyPtr->value().toBool() ? Qt::Checked : Qt::Unchecked;

		m_checkBox->blockSignals(true);
		m_checkBox->setCheckState(state);

		updateText();

		m_checkBox->setEnabled(readOnly == false);
		m_checkBox->blockSignals(false);

		QTimer::singleShot(0, m_checkBox, SLOT(setFocus()));
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
	// ------- Property Editor Settings ----------
	//

	void PropertyEditorSettings::restore(QSettings& s)
	{
		m_arrayPropertyEditorSplitterState = s.value("PropertyEditor/arrayPropertyEditorSplitterState").toByteArray();
		m_arrayPropertyEditorSize = s.value("PropertyEditor/arrayPropertyEditorSize").toSize();

		m_vectorEditorSize = s.value("PropertyEditor/m_vectorEditorSize").toSize();

		m_multiLinePropertyEditorWindowPos = s.value("PropertyEditor/multiLinePropertyEditorWindowPos", QPoint(-1, -1)).toPoint();
		m_multiLinePropertyEditorGeometry = s.value("PropertyEditor/multiLinePropertyEditorGeometry").toByteArray();

		m_propertyEditorFontScaleFactor = s.value("PropertyEditor/fontScaleFactor").toDouble();
		if (m_propertyEditorFontScaleFactor < 1.0 || m_propertyEditorFontScaleFactor > 3.0)
		{
			m_propertyEditorFontScaleFactor = 1.0;
		}

	}

	void PropertyEditorSettings::store(QSettings& s)
	{
		s.setValue("PropertyEditor/arrayPropertyEditorSplitterState", m_arrayPropertyEditorSplitterState);
		s.setValue("PropertyEditor/arrayPropertyEditorSize", m_arrayPropertyEditorSize);

		s.setValue("PropertyEditor/m_vectorEditorSize", m_vectorEditorSize);

		s.setValue("PropertyEditor/multiLinePropertyEditorWindowPos", m_multiLinePropertyEditorWindowPos);
		s.setValue("PropertyEditor/multiLinePropertyEditorGeometry", m_multiLinePropertyEditorGeometry);

		s.setValue("PropertyEditor/fontScaleFactor", m_propertyEditorFontScaleFactor);
	}


	//
	// PropertyTableItemDelegate
	//

	PropertyEditorDelegate::PropertyEditorDelegate(PropertyTreeWidget* treeWidget, PropertyEditor* propretyEditor) :
		QItemDelegate(treeWidget),
		m_treeWidget(treeWidget),
		m_propertyEditor(propretyEditor)
	{
	}

	QWidget* PropertyEditorDelegate::createEditor(QWidget *parent,
													 const QStyleOptionViewItem &option,
													 const QModelIndex &index) const
	{
		Q_UNUSED(option);

		if (m_treeWidget == nullptr || m_propertyEditor == nullptr)
		{
			Q_ASSERT(m_treeWidget);
			Q_ASSERT(m_propertyEditor);
			return new QWidget(parent);
		}

		int row = -1;

		std::shared_ptr<Property> p = m_propertyEditor->propertyFromIndex(index);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return new QWidget(parent);
		}

		m_cellEditor = m_propertyEditor->createCellRowEditor(p, row, true, p->readOnly() == true || m_propertyEditor->isReadOnly() == true, parent);

		connect(m_cellEditor, &PropertyEditCellWidget::valueChanged, this, &PropertyEditorDelegate::onValueChanged);

		m_editIndex = index;

		return m_cellEditor;
	}

	void PropertyEditorDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
	{
		Q_UNUSED(editor);

		if (m_editIndex == index)
		{
			m_editIndex = QModelIndex();
		}

		QItemDelegate::destroyEditor(editor, index);
	}

	void PropertyEditorDelegate::setEditorData(QWidget *editor,
												  const QModelIndex &index) const
	{
		if (m_treeWidget == nullptr || m_propertyEditor == nullptr)
		{
			Q_ASSERT(m_treeWidget);
			Q_ASSERT(m_propertyEditor);
			return;
		}

		std::shared_ptr<Property> p = m_propertyEditor->propertyFromIndex(index);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return;
		}

		PropertyEditCellWidget *cellEditor = dynamic_cast<PropertyEditCellWidget*>(editor);
		if (cellEditor == nullptr)
		{
			Q_ASSERT(cellEditor);
			return;
		}
		cellEditor->setValue(p, p->readOnly() == true || m_propertyEditor->isReadOnly() == true);

		return;
	}

	void PropertyEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
	{
		Q_UNUSED(editor);
		Q_UNUSED(model);
		Q_UNUSED(index);

		// This function is called when user press Enter or changes selection
	}

	void PropertyEditorDelegate::updateEditorGeometry(QWidget *editor,
														 const QStyleOptionViewItem &option,
														 const QModelIndex &index) const
	{
		Q_UNUSED(index);
		editor->setGeometry(option.rect);
	}

	QSize PropertyEditorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		return QItemDelegate::sizeHint(option, index) + QSize(3, 4);
	}

	void PropertyEditorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
				const QModelIndex &index) const
	{

		// Don't draw if editor is active for this index
		//
		if (index == m_editIndex)
		{
			//painter->fillRect(option.rect, Qt::red);
		}
		else
		{
			QItemDelegate::paint(painter, option, index);
		}

		// Draw vertical grid line
		//
		QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
		painter->save();
		painter->setPen(QPen(color));
		int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
		painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
		painter->restore();
	}

	void PropertyEditorDelegate::onValueChanged(QVariant value)
	{
		emit valueChanged(value);
	}

	// PropertyTreeWidget
	//

	QString PropertyTreeWidget::propertyCaption(const QModelIndex& mi)
	{
		QTreeWidgetItem* item = itemFromIndex(mi);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return QString();
		}

		return item->text(static_cast<int>(PropertyEditorColumns::Caption));
	}

	void PropertyTreeWidget::closeCurrentEditorIfOpen()
	{
		QWidget* editWidget = indexWidget(currentIndex());
		if (editWidget != nullptr)
		{
			closeEditor(editWidget, QAbstractItemDelegate::RevertModelCache);
		}

		return;
	}

	void PropertyTreeWidget::mousePressEvent(QMouseEvent *event)
	{
		QTreeWidget::mousePressEvent(event);

		if (event->button() == Qt::MouseButton::LeftButton)
		{
			QModelIndex mi = indexAt(event->pos());

			if (mi.column() == static_cast<int>(PropertyEditorColumns::Value))
			{
				emit mousePressed();
			}
		}
	}

	void PropertyTreeWidget::keyPressEvent(QKeyEvent *event)
	{
		if (event->key() == Qt::Key_F2 || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
		{
			if (selectedIndexes().size() > 0)
			{
				emit editKeyPressed();
				return;
			}
		}

		if (event->key() == Qt::Key_Space)
		{
			if (selectedIndexes().size() > 0)
			{
				emit spaceKeyPressed();
				return;
			}
		}

		QTreeWidget::keyPressEvent(event);
	}

	void PropertyTreeWidget::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyleOptionViewItem opt = option;

		QTreeWidget::drawRow(painter, opt, index);
		QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
		painter->save();
		painter->setPen(QPen(color));
		painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
		painter->restore();
	}

	//
	// ------- Property Editor ----------
	//

	PropertyEditor::PropertyEditor(QWidget* parent) :
		QWidget(parent),
		PropertyEditorBase()
	{
		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->setContentsMargins(1, 1, 1, 1);
		mainLayout->setSpacing(2);

		// Table View

		m_treeWidget = new PropertyTreeWidget();
		mainLayout->addWidget(m_treeWidget);

		QStringList labels;
		labels << tr("Property");
		labels << tr("Value");
		m_treeWidget->setHeaderLabels(labels);
		m_treeWidget->setColumnCount(labels.size());
		m_treeWidget->setAutoFillBackground(true);
		m_treeWidget->setSelectionBehavior(QTreeWidget::SelectRows);
		m_treeWidget->setSelectionMode(QTreeWidget::SingleSelection);

		connect(m_treeWidget, &PropertyTreeWidget::mousePressed, this, &PropertyEditor::onCellClicked);
		connect(m_treeWidget, &PropertyTreeWidget::editKeyPressed, this, &PropertyEditor::onCellEditKeyPressed);
		connect(m_treeWidget, &PropertyTreeWidget::spaceKeyPressed, this, &PropertyEditor::onCellToggleKeyPressed);

		m_treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

		// Edit Delegate

		m_itemDelegate = new PropertyEditorDelegate(m_treeWidget, this);
		connect(m_itemDelegate, &PropertyEditorDelegate::valueChanged, this, &PropertyEditor::onValueChanged);
		connect(m_itemDelegate, &QItemDelegate::closeEditor, this, &PropertyEditor::onCellEditorClosed);
		m_treeWidget->setItemDelegate(m_itemDelegate);

		connect(this, &PropertyEditor::showErrorMessage, this, &PropertyEditor::onShowErrorMessage, Qt::QueuedConnection);

		//

		/*if (thePropertyEditorSettings.m_propertyEditorFontScaleFactor != 1.0)
		{
			setFontSizeF(fontSizeF() * thePropertyEditorSettings.m_propertyEditorFontScaleFactor);
		}*/

		return;
	}

	void PropertyEditor::clear()
	{
		m_treeWidget->clear();

		m_treeObjects.clear();
	}

	const QList<std::shared_ptr<PropertyObject>>& PropertyEditor::objects() const
	{
		return m_objects;
	}

	void PropertyEditor::setObjects(const std::vector<std::shared_ptr<PropertyObject>>& objects)
	{
		QList<std::shared_ptr<PropertyObject>> list =
		        QList<std::shared_ptr<PropertyObject>>::fromVector(QVector<std::shared_ptr<PropertyObject>>{objects.begin(), objects.end()});

		return setObjects(list);
	}

	void PropertyEditor::setObjects(const QList<std::shared_ptr<PropertyObject>>& objects)
	{
		bool wasVisible = isVisible();

		if (wasVisible == true)
		{
			setVisible(false);
		}

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

		if (wasVisible == true)
		{
			setVisible(true);
		}

		return;
	}

	int PropertyEditor::splitterPosition() const
	{
		return m_treeWidget->columnWidth(static_cast<int>(PropertyEditorColumns::Caption));
	}

	void PropertyEditor::setSplitterPosition(int pos)
	{
		m_treeWidget->setColumnWidth(static_cast<int>(PropertyEditorColumns::Caption), pos);
	}

	void PropertyEditor::autoAdjustSplitterPosition()
	{
		m_treeWidget->resizeColumnToContents(static_cast<int>(PropertyEditorColumns::Caption));
	}

	bool PropertyEditor::isPropertyExists(const QString& propertyName) const
	{
		return m_treeObjects.find(propertyName) != m_treeObjects.end();
	}

	void PropertyEditor::updatePropertiesValues()
	{
		for (auto it : m_treeObjects)
		{
			updatePropertyValue(it.first);
		}
	}

	void PropertyEditor::updatePropertyValue(const QString& propertyName)
	{
		auto it = m_treeObjects.find(propertyName);
		if (it == m_treeObjects.end())
		{
			Q_ASSERT(false);
			return;
		}

		const PropertyEditorObject& po = it->second;

		if (po.item == nullptr)
		{
			Q_ASSERT(po.item);
			return;
		}

		if (po.property == nullptr)
		{
			Q_ASSERT(po.property);
			return;
		}

		// Output the text
		//
		if (po.property->password() == true)
		{
			po.item->setText(static_cast<int>(PropertyEditorColumns::Value), QString());
			return;
		}

		QString text;

		if (po.sameValue == true)
		{
			text = PropertyTools::propertyValueText(po.property.get(), -1);
		}
		else
		{
			// Get the property value from Property object
			//
			QVariant value = po.property->value();

			// PropertyVector, PropertyList
			//
			if (variantIsPropertyVector(value) == true)
			{
				text = tr("<PropertyVector>");
			}

			if (variantIsPropertyList(value) == true)
			{
				text = tr("<PropertyList>");
			}

			// Bool

			if (value.type() == QVariant::Bool)
			{
				text = "<Different values>";
			}
		}

		po.item->setText(static_cast<int>(PropertyEditorColumns::Value), text);
		po.item->setIcon(static_cast<int>(PropertyEditorColumns::Value), propertyIcon(po.property.get(), po.sameValue, po.readOnly == false));

		return;
	}

	void PropertyEditor::valueChanged(QString propertyName, QVariant value)
	{
		// Set the new property value in all objects
		//
		QList<std::shared_ptr<PropertyObject>> modifiedObjects;

		QString errorString;

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

	void PropertyEditor::showEvent(QShowEvent* event)
	{
		if (event->type() == QEvent::Show)
		{
			updatePropertiesValues();
		}
	}

	void PropertyEditor::hideEvent(QHideEvent* event)
	{
		if (event->type() == QEvent::Hide)
		{
			m_treeWidget->closeCurrentEditorIfOpen();
		}
	}

	void PropertyEditor::updatePropertiesList()
	{
		bool wasVisible = isVisible();

		if (wasVisible == true)
		{
			setVisible(false);
		}

		fillProperties();

		if (wasVisible == true)
		{
			setVisible(true);
		}

		return;
	}

	void PropertyEditor::onCellClicked()
	{
		startEditing();
	}

	void PropertyEditor::onCellEditKeyPressed()
	{
		if (getSelectionType() != QVariant::Bool)
		{
			startEditing();
		}
	}

	void PropertyEditor::onCellToggleKeyPressed()
	{
		if (getSelectionType() == QVariant::Bool)
		{
			toggleSelected();
		}
	}

	void PropertyEditor::onValueChanged(QVariant value)
	{
		QModelIndexList selectedRows = m_treeWidget->selectionModel()->selectedRows();
		if (selectedRows.size() != 1)
		{
			Q_ASSERT(false);
			return;
		}

		const QModelIndex& mi = selectedRows[0];

		std::shared_ptr<Property> p = propertyFromIndex(mi);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return;
		}

		valueChanged(p->caption(), value);

		// Value is now the same for all objects

		auto it = m_treeObjects.find(p->caption());
		if (it == m_treeObjects.end())
		{
			Q_ASSERT(false);
			return;
		}

		PropertyEditorObject& propertyEditorObject = it->second;
		propertyEditorObject.sameValue = true;

		// Force redraw

		updatePropertyValue(p->caption());

		return;
	}

	void PropertyEditor::onCellEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
	{
		Q_UNUSED(editor);
		Q_UNUSED(hint);

		m_treeWidget->setFocus();
	}

	void PropertyEditor::onShowErrorMessage(QString message)
	{
		QMessageBox::warning(this, "Error", message);
	}

	void PropertyEditor::fillProperties()
	{
		clear();

		QMap<QString, std::shared_ptr<Property>> propertyItems;
		QList<QString> propertyNames;

		std::vector<PropertyEditorObject> propertyEditorObjects;

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

				if (p->expert() && expertMode() == false)
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
			int type = 0;
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

			if (p->readOnly() == true || isReadOnly() == true)
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

			propertyEditorObjects.emplace_back(p, sameValue, readOnly);
		}

		// Sort here

		std::sort(propertyEditorObjects.begin(), propertyEditorObjects.end(), [](const PropertyEditorObject& o1, const PropertyEditorObject& o2){
			return std::make_tuple(o1.property->category(), o1.property->viewOrder(), o1.property->caption()) <
					std::make_tuple(o2.property->category(), o2.property->viewOrder(), o2.property->caption());

		});

		// Sort

		for (const PropertyEditorObject& poe : propertyEditorObjects)
		{
			createProperty(poe);
		}

		return;
	}

	void PropertyEditor::createProperty(const PropertyEditorObject& poe)
	{
		QTreeWidgetItem* groupItem = nullptr;

		const QString& caption = poe.property->caption();
		const QString& description = poe.property->description();
		QString category = poe.property->category();
		if (category.isEmpty() == true)
		{
			category = m_commonCategoryName;
		}

		for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* item = m_treeWidget->topLevelItem(i);
			if (item == nullptr)
			{
				Q_ASSERT(item);
				return;
			}

			if (item->text(static_cast<int>(PropertyEditorColumns::Caption)) == category)
			{
				groupItem = item;
				break;
			}
		}

		if (groupItem == nullptr)
		{
			groupItem = new QTreeWidgetItem(QStringList() << category);
			m_treeWidget->addTopLevelItem(groupItem);
		}

		// Add the property now
		//
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << caption);
		item->setToolTip(static_cast<int>(PropertyEditorColumns::Caption), description);
		item->setFlags(item->flags() | Qt::ItemIsEditable);

		if (poe.property->essential() == true)
		{
			item->setBackground(static_cast<int>(PropertyEditorColumns::Caption), QColor(0xf0, 0xf0, 0xf0));
			item->setBackground(static_cast<int>(PropertyEditorColumns::Value), QColor(0xf0, 0xf0, 0xf0));
		}

		// Add object to the map
		//
		m_treeObjects[caption] = poe;
		m_treeObjects[caption].setTreeWidgetItem(item);

		// Fill property value
		//
		updatePropertyValue(poe.property->caption());

		// Add item to group
		//
		groupItem->addChild(item);

		if (groupItem->isExpanded() == false)
		{
			groupItem->setExpanded(true);
		}

		return;
	}

	std::shared_ptr<Property> PropertyEditor::propertyFromIndex(QModelIndex index) const
	{
		QString propertyName = m_treeWidget->propertyCaption(index);

		auto it = m_treeObjects.find(propertyName);
		if (it == m_treeObjects.end())
		{
			return nullptr;
		}

		const PropertyEditorObject& po = it->second;
		return po.property;
	}

	// returns -1 if no type is selected or they are different
	//
	int PropertyEditor::getSelectionType()
	{
		QModelIndexList selectedRows = m_treeWidget->selectionModel()->selectedRows();
		if (selectedRows.size() != 1)
		{
			return -1;
		}

		const QModelIndex& mi = selectedRows[0];

		std::shared_ptr<Property> p = propertyFromIndex(mi);
		if (p == nullptr)
		{
			return -1;
		}

		return p->value().userType();
	}

	void PropertyEditor::startEditing()
	{
		if (getSelectionType() == -1)
		{
			return;
		}

		QModelIndex index = m_treeWidget->currentIndex();

		if (index.column() != static_cast<int>(PropertyEditorColumns::Value))
		{
			index = index.siblingAtColumn(static_cast<int>(PropertyEditorColumns::Value));
		}

		m_treeWidget->edit(index);
	}

	void PropertyEditor::toggleSelected()
	{
		QModelIndexList selectedRows = m_treeWidget->selectionModel()->selectedRows();
		if (selectedRows.size() != 1)
		{
			Q_ASSERT(false);
			return;
		}

		const QModelIndex& mi = selectedRows[0];

		std::shared_ptr<Property> p = propertyFromIndex(mi);
		if (p == nullptr)
		{
			Q_ASSERT(p);
			return;
		}

		if (p->value().userType() == QVariant::Bool)
		{
			bool b = p->value().toBool();

			valueChanged(p->caption(), !b);

			updatePropertyValue(p->caption());
		}

		return;
	}

	PropertyEditorSettings thePropertyEditorSettings;
}
