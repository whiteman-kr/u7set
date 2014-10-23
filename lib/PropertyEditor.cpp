#include "../include/PropertyEditor.h"
//#include "PropertyEditor.h"

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

//
// ---------QtMultiColorEdit----------
//

QtMultiColorEdit::QtMultiColorEdit(QWidget* parent):
	QWidget(parent)
{
	m_lineEdit = new QLineEdit(parent);

	QToolButton* button = new QToolButton(parent);
	button->setText("...");

	connect(m_lineEdit, &QLineEdit::editingFinished, this, &QtMultiColorEdit::onEditingFinished);

	connect(button, &QToolButton::clicked, this, &QtMultiColorEdit::onButtonPressed);

	QHBoxLayout* lt = new QHBoxLayout;
	lt->setContentsMargins(0, 0, 0, 0);
	lt->setSpacing(0);
	lt->addWidget(m_lineEdit);
	lt->addWidget(button, 0, Qt::AlignRight);

	setLayout(lt);

	m_lineEdit->installEventFilter(this);

	QRegExp regexp("\\[([1,2]?[0-9]{0,2};){3}[1,2]?[0-9]{0,2}\\]");
	QRegExpValidator *validator = new QRegExpValidator(regexp, this);
	m_lineEdit->setValidator(validator);
}

bool QtMultiColorEdit::eventFilter(QObject* watched, QEvent* event)
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

void QtMultiColorEdit::onButtonPressed()
{
	QString t = m_lineEdit->text();
	QColor color = colorFromText(t);

	QColorDialog dialog(color, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		setValue(dialog.selectedColor());
		emit valueChanged(dialog.selectedColor());
	}
}

void QtMultiColorEdit::setValue(QVariant value)
{
	if (m_lineEdit == nullptr)
	{
		Q_ASSERT(m_lineEdit);
		return;
	}

	QColor color = value.value<QColor>();
	QString val = QString("[%1;%2;%3;%4]").
				  arg(color.red()).
				  arg(color.green()).
				  arg(color.blue()).
				  arg(color.alpha());

	m_lineEdit->setText(val);
}

void QtMultiColorEdit::onEditingFinished()
{
	if (m_escape == false)
	{
		if (m_editingFinished == false)
		{

			QString t = m_lineEdit->text();

			QColor color = colorFromText(t);

			emit valueChanged(color);

			m_editingFinished = true;	//a "static" value. On "Enter", onEditingFinished comes twice???
		}
	}
}

QColor QtMultiColorEdit::colorFromText(const QString& t)
{
	QString text = t;
	text.remove(QRegExp("[\\[,\\]]"));

	QStringList l = text.split(";");
	if (l.count() != 4)
	{
		Q_ASSERT(l.count() == 4);
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
// ---------MultiLineEdit----------
//
MultiLineEdit::MultiLineEdit(QWidget *parent, const QString &text):
	QDialog(parent)
{
	setWindowTitle("Text Editor");

	QString value = text;

	QVBoxLayout* vl = new QVBoxLayout();

	m_textEdit = new QTextEdit(this);
	m_textEdit->setTabChangesFocus(true);
	m_textEdit->setPlainText(value);

	QPushButton* okButton = new QPushButton("OK", this);
	QPushButton* cancelButton = new QPushButton("Cancel", this);

	okButton->setDefault(true);

	connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	QHBoxLayout *hl = new QHBoxLayout();
	hl->addStretch();
	hl->addWidget(okButton);
	hl->addWidget(cancelButton);


	vl->addWidget(m_textEdit);
	vl->addLayout(hl);

	setLayout(vl);
}

QString MultiLineEdit::text()
{
	return m_text;
}

void MultiLineEdit::accept()
{
	if (m_textEdit == nullptr)
	{
		Q_ASSERT(m_textEdit);
		return;
	}

	m_text = m_textEdit->toPlainText();

	QDialog::accept();
}

//
// ---------QtMultiTextEdit----------
//

QtMultiTextEdit::QtMultiTextEdit(QWidget* parent):
	QWidget(parent)
{
	m_lineEdit = new QLineEdit(parent);

	QToolButton* button = new QToolButton(parent);
	button->setText("...");

	connect(m_lineEdit, &QLineEdit::editingFinished, this, &QtMultiTextEdit::onEditingFinished);

	connect(button, &QToolButton::clicked, this, &QtMultiTextEdit::onButtonPressed);

	QHBoxLayout* lt = new QHBoxLayout;
	lt->setContentsMargins(0, 0, 0, 0);
	lt->setSpacing(0);
	lt->addWidget(m_lineEdit);
	lt->addWidget(button, 0, Qt::AlignRight);

	setLayout(lt);

	m_lineEdit->installEventFilter(this);
}

bool QtMultiTextEdit::eventFilter(QObject* watched, QEvent* event)
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

void QtMultiTextEdit::onButtonPressed()
{
	MultiLineEdit* multlLineEdit = new MultiLineEdit(this, m_lineEdit->text());
	if (multlLineEdit->exec() == QDialog::Accepted)
	{
		QString value = multlLineEdit->text();

		setValue(value);
		emit valueChanged(value);
	}
}

void QtMultiTextEdit::setValue(QString value)
{
	if (m_lineEdit == nullptr)
	{
		Q_ASSERT(m_lineEdit);
		return;
	}

	m_lineEdit->setText(value);
}

void QtMultiTextEdit::onEditingFinished()
{
	if (m_escape == false)
	{
		if (m_editingFinished == false)
		{
			emit valueChanged(m_lineEdit->text());
			m_editingFinished = true;	//a "static" value. On "Enter", onEditingFinished comes twice???
		}
	}
}

//
// ---------QtMultiDoubleSpinBox----------
//

QtMultiDoubleSpinBox::QtMultiDoubleSpinBox(QWidget* parent):
	QWidget(parent)
{
	m_spinBox = new QDoubleSpinBox(parent);
	m_spinBox->setKeyboardTracking(false);
	m_spinBox->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
	m_spinBox->setDecimals(2);

	connect(m_spinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			this, &QtMultiDoubleSpinBox::onValueChanged);

	QHBoxLayout* lt = new QHBoxLayout;
	lt->setContentsMargins(0, 0, 0, 0);
	lt->addWidget(m_spinBox);
	setLayout(lt);

	m_spinBox->installEventFilter(this);
}

bool QtMultiDoubleSpinBox::eventFilter(QObject* watched, QEvent* event)
{
	if (m_spinBox == nullptr)
	{
		Q_ASSERT(m_spinBox);
		return QWidget::eventFilter(watched, event);
	}

	if (watched == m_spinBox && event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		if (ke->key() == Qt::Key_Escape)
		{
			m_escape = true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

void QtMultiDoubleSpinBox::setValue(double value)
{
	if (m_spinBox == nullptr)
	{
		Q_ASSERT(m_spinBox);
		return;
	}

	m_spinBox->blockSignals(true);
	m_spinBox->setValue(value);
	m_spinBox->blockSignals(false);
}

void QtMultiDoubleSpinBox::onValueChanged(double value)
{
	if (m_escape == false)
	{
		emit valueChanged(value);
	}
}

//
// ---------QtMultiIntSpinBox----------
//

QtMultiIntSpinBox::QtMultiIntSpinBox(QWidget* parent):
	QWidget(parent)
{
	m_spinBox = new QSpinBox(parent);
	m_spinBox->setKeyboardTracking(false);
	m_spinBox->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

	connect(m_spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &QtMultiIntSpinBox::onValueChanged);

	QHBoxLayout *lt = new QHBoxLayout;
	lt->setContentsMargins(0, 0, 0, 0);
	lt->addWidget(m_spinBox);
	setLayout(lt);

	m_spinBox->installEventFilter(this);
}

bool QtMultiIntSpinBox::eventFilter(QObject * watched, QEvent * event)
{
	if (m_spinBox == nullptr)
	{
		Q_ASSERT(m_spinBox);
		return false;
	}

	if (watched == m_spinBox && event->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(event);
		if (ke->key() == Qt::Key_Escape)
		{
			m_escape = true;
		}
	}

	return QWidget::eventFilter(watched, event);
}

void QtMultiIntSpinBox::setValue(int value)
{
	if (m_spinBox == nullptr)
	{
		Q_ASSERT(m_spinBox);
		return;
	}

	m_spinBox->blockSignals(true);
	m_spinBox->setValue(value);
	m_spinBox->blockSignals(false);
}

void QtMultiIntSpinBox::onValueChanged(int value)
{
	if (m_escape == false)
	{
		emit valueChanged(value);
	}
}

//
// ---------QtMultiCheckBox----------
//
static QIcon drawCheckBox(int state)
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

	opt.state |= QStyle::State_Enabled;
	const QStyle *style = QApplication::style();
	// Figure out size of an indicator and make sure it is not scaled down in a list view item
	// by making the pixmap as big as a list view icon and centering the indicator in it.
	// (if it is smaller, it can't be helped)
	const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
	const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
	const int listViewIconSize = indicatorWidth;
	const int pixmapWidth = indicatorWidth;
	const int pixmapHeight = qMax(indicatorHeight, listViewIconSize);

	opt.rect = QRect(0, 0, indicatorWidth, indicatorHeight);
	QPixmap pixmap = QPixmap(pixmapWidth, pixmapHeight);
	pixmap.fill(Qt::transparent);
	{
		// Center?
		const int xoff = (pixmapWidth  > indicatorWidth)  ? (pixmapWidth  - indicatorWidth)  / 2 : 0;
		const int yoff = (pixmapHeight > indicatorHeight) ? (pixmapHeight - indicatorHeight) / 2 : 0;
		QPainter painter(&pixmap);
		painter.translate(xoff, yoff);
		style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &painter);
	}
	return QIcon(pixmap);
}

static QIcon drawColorBox(QColor color)
{
	const QStyle *style = QApplication::style();
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

QtMultiCheckBox::QtMultiCheckBox(QWidget* parent):
	QWidget(parent)
{
	m_checkBox = new QCheckBox(parent);

	connect(m_checkBox, &QCheckBox::stateChanged, this, &QtMultiCheckBox::onStateChanged);

	QHBoxLayout*lt = new QHBoxLayout;
	lt->setContentsMargins(3, 1, 0, 0);
	lt->addWidget(m_checkBox);
	setLayout(lt);
}

void QtMultiCheckBox::setCheckState(Qt::CheckState state)
{
	if (m_checkBox == nullptr)
	{
		Q_ASSERT(m_checkBox);
		return;
	}

	m_checkBox->blockSignals(true);
	m_checkBox->setCheckState(state);
	updateText();
	m_checkBox->blockSignals(false);
}

void QtMultiCheckBox::onStateChanged(int state)
{
	if (m_checkBox == nullptr)
	{
		Q_ASSERT(m_checkBox);
		return;
	}

	updateText();

	m_checkBox->setTristate(false);

	emit valueChanged(state);
}

void QtMultiCheckBox::updateText()
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

QtMultiVariantFactory::QtMultiVariantFactory(QObject* parent):
	QtAbstractEditorFactory<QtMultiVariantPropertyManager>(parent)
{
}

QWidget* QtMultiVariantFactory::createEditor(QtMultiVariantPropertyManager* manager, QtProperty* property, QWidget* parent)
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

	switch(manager->type())
	{
		case QVariant::Int:
			{
				QtMultiIntSpinBox* m_editor = new QtMultiIntSpinBox(parent);
				editor = m_editor;
				m_editor->setValue(manager->value(property).toInt());

				connect(m_editor, &QtMultiIntSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiIntSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;
		case QVariant::Double:
			{
				QtMultiDoubleSpinBox* m_editor = new QtMultiDoubleSpinBox(parent);
				editor = m_editor;
				m_editor->setValue(manager->value(property).toDouble());

				connect(m_editor, &QtMultiDoubleSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiDoubleSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;
		case QVariant::Bool:
			{
				QtMultiCheckBox* m_editor = new QtMultiCheckBox(parent);
				editor = m_editor;
				m_editor->setCheckState((Qt::CheckState)manager->value(property).toInt());

				connect(m_editor, &QtMultiCheckBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiCheckBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;

		case QVariant::String:
			{
				QtMultiTextEdit* m_editor = new QtMultiTextEdit(parent);
				editor = m_editor;
				m_editor->setValue(manager->value(property).toString());

				connect(m_editor, &QtMultiTextEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiTextEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;

		case QVariant::Color:
			{
				QtMultiColorEdit* m_editor = new QtMultiColorEdit(parent);
				editor = m_editor;
				m_editor->setValue(manager->value(property));

				connect(m_editor, &QtMultiColorEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiColorEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			break;

		default:
			Q_ASSERT(false);
	}

	if (editor == nullptr)
	{
		Q_ASSERT(editor);
		return new QWidget();
	}

	return editor;
}

void QtMultiVariantFactory::connectPropertyManager (QtMultiVariantPropertyManager* manager)
{
	Q_UNUSED(manager);
	//connect(manager, &QtMultiVariantPropertyManager::valueChanged, this, &QtMultiVariantFactory::slotPropertyChanged);
}

void QtMultiVariantFactory::disconnectPropertyManager(QtMultiVariantPropertyManager* manager)
{
	Q_UNUSED(manager);
	//disconnect(manager, &QtMultiVariantPropertyManager::valueChanged, this, &QtMultiVariantFactory::slotPropertyChanged);
}

/*void QtMultiVariantFactory::slotPropertyChanged(QtProperty* property, QVariant value)
{
	Q_UNUSED(property);
	Q_UNUSED(value);
}*/

void QtMultiVariantFactory::slotSetValue(QVariant value)
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

	m_manager->setValue(m_property, value);
	m_manager->emitSetValue(m_property, value);
}

void QtMultiVariantFactory::slotEditorDestroyed(QObject* object)
{
	Q_UNUSED(object);
}

//
// ---------QtMultiVariantPropertyManager----------
//

QtMultiVariantPropertyManager::QtMultiVariantPropertyManager(QObject* parent, QVariant::Type type) :
	QtAbstractPropertyManager(parent),
	m_type(type)
{

}

const QVariant::Type QtMultiVariantPropertyManager::type() const
{
	return m_type;
}

QVariant QtMultiVariantPropertyManager::value(const QtProperty* property) const
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
		return 0;
	}
	return it.value().value;
}

QSet<QtProperty*> QtMultiVariantPropertyManager::propertyByName(const QString& propertyName)
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

void QtMultiVariantPropertyManager::setValue(QtProperty* property, const QVariant& value)
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

	if (value.isValid() == false)
	{
		it.value().value = QVariant();
	}
	else
	{
		QVariant newValue = value;
		it.value().value = newValue;
	}


	emit propertyChanged(property);
}

void QtMultiVariantPropertyManager::emitSetValue(QtProperty* property, const QVariant& value)
{
	if (property == nullptr)
	{
		Q_ASSERT(property);
		return;
	}

	emit valueChanged(property, value);
}

void QtMultiVariantPropertyManager::initializeProperty(QtProperty* property)
{
	if (property == nullptr)
	{
		Q_ASSERT(property);
		return;
	}

	values[property] = QtMultiVariantPropertyManager::Data();
}
void QtMultiVariantPropertyManager::uninitializeProperty(QtProperty* property)
{
	if (property == nullptr)
	{
		Q_ASSERT(property);
		return;
	}

	values.remove(property);
}

QIcon QtMultiVariantPropertyManager::valueIcon(const QtProperty* property) const
{
	if (property == nullptr)
	{
		Q_ASSERT(property);
		return QIcon();
	}

	if (value(property).isValid() == false)
	{
		return QIcon();
	}

	switch (type())
	{
		case QVariant::Bool:
			{
				Qt::CheckState checkState = (Qt::CheckState)value(property).toInt();
				return drawCheckBox(checkState);
			}
			break;
		case QVariant::Color:
			{
				QColor color = value(property).value<QColor>();
				return drawColorBox(color);
			}
			break;
	}
	return QIcon();
}

QString QtMultiVariantPropertyManager::valueText(const QtProperty* property) const
{
	if (property == nullptr)
	{
		Q_ASSERT(property);
		return QString();
	}

	if (value(property).isValid() == true)
	{
		switch (type())
		{
			case QVariant::Int:
				{
					int val = value(property).toInt();
					return QString::number(val);
				}
				break;

			case QVariant::Double:
				{
					double val = value(property).toDouble();
					return QString::number(val);
				}
				break;
			case QVariant::Bool:
				{
					Qt::CheckState checkState = (Qt::CheckState)value(property).toInt();

					switch (checkState)
					{
						case Qt::Checked:           return("True");
						case Qt::Unchecked:         return("False");
						case Qt::PartiallyChecked:  return("<Different values>");
					}

					return "???";

				}
				break;
			case QVariant::String:
				{
					QString val = value(property).toString();
					val.replace("\n", " ");
					return val;
				}
				break;

			case QVariant::Color:
				{
					QColor color = value(property).value<QColor>();
					QString val = QString("[%1;%2;%3;%4]").
								  arg(color.red()).
								  arg(color.green()).
								  arg(color.blue()).
								  arg(color.alpha());
					return val;
				}
				break;

			default:
				Q_ASSERT(false);
		}
	}

	return QString("");
}

//
// ------- Property Editor ----------
//

PropertyEditor::PropertyEditor(QWidget* parent) :
	QtTreePropertyBrowser(parent)
{
	m_propertyGroupManager = new QtGroupPropertyManager(this);
	m_propertyStringManager = new QtMultiVariantPropertyManager(this, QVariant::String);
	m_propertyIntManager = new QtMultiVariantPropertyManager(this, QVariant::Int);
	m_propertyDoubleManager = new QtMultiVariantPropertyManager(this, QVariant::Double);
	m_propertyBoolManager = new QtMultiVariantPropertyManager(this, QVariant::Bool);
	m_propertyColorManager = new QtMultiVariantPropertyManager(this, QVariant::Color);

	QtMultiVariantFactory* spinBoxFactory = new QtMultiVariantFactory(this);
	QtMultiVariantFactory* doubleSpinBoxFactory = new QtMultiVariantFactory(this);
	QtMultiVariantFactory* lineEditFactory = new QtMultiVariantFactory(this);
	QtMultiVariantFactory *checkBoxFactory = new QtMultiVariantFactory(this);
	QtMultiVariantFactory *colorFactory = new QtMultiVariantFactory(this);

	setFactoryForManager(m_propertyStringManager, lineEditFactory);
	setFactoryForManager(m_propertyIntManager, spinBoxFactory);
	setFactoryForManager(m_propertyDoubleManager, doubleSpinBoxFactory);
	setFactoryForManager(m_propertyBoolManager, checkBoxFactory);
	setFactoryForManager(m_propertyColorManager, colorFactory);

	connect(m_propertyIntManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);
	connect(m_propertyStringManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);
	connect(m_propertyDoubleManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);
	connect(m_propertyBoolManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);
	connect(m_propertyColorManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);

	connect(this, &PropertyEditor::showErrorMessage, this, &PropertyEditor::onShowErrorMessage, Qt::QueuedConnection);

	connect(this, &QtTreePropertyBrowser::currentItemChanged, this, &PropertyEditor::onCurrentItemChanged);

	return;
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


void PropertyEditor::setObjects(QList<std::shared_ptr<QObject> >& objects)
{
	clearProperties();

	QMap<QString, PropertyItem> propertyItems;
	QList<QString> propertyNames;

	// Create a map with all properties
	//
	for (auto pobject = objects.begin(); pobject != objects.end(); pobject++)
	{
		QObject* object = pobject->get();

		const QMetaObject* metaObject = object->metaObject();

		for (int i = 0; i < metaObject->propertyCount(); ++i)
		{
			QMetaProperty metaProperty = metaObject->property(i);

			const char* name = metaProperty.name();
			if (QString(name) == "objectName")
			{
				continue;
			}

			PropertyItem pi;

			pi.object = *pobject;
			pi.type = metaProperty.type();
			pi.value = object->property(name);

			propertyItems.insertMulti(name, pi);

			if (propertyNames.indexOf(name) == -1)
			{
				propertyNames.append(name);
			}
		}
	}

	QtProperty* commonProperty = m_propertyGroupManager->addProperty(tr("Common"));

	// add only common properties with same type
	//
	for (auto n = propertyNames.begin(); n != propertyNames.end(); n++)
	{
		// take all properties witn the same name
		//
		QString name = *n;
		QList<PropertyItem> items = propertyItems.values(name);

		if (items.size() != objects.size())
		{
			continue;   // this property is not in all objects
		}

		// now check if all properties have the same type and values
		//
		QVariant::Type type;
		QVariant value;

		bool sameType = true;
		bool sameValue = true;

		for (auto p = items.begin(); p != items.end(); p++)
		{
			PropertyItem& pi = *p;

			if (p == items.begin())
			{
				// remember the first item params
				//
				type = pi.type;
				value = pi.value;
			}
			else
			{
				// compare with next item params
				//
				if (pi.type != type)
				{
					sameType = false;
					break;
				}

				if (pi.value != value)
				{
					sameValue = false;
				}
			}

			m_propToClassMap.insertMulti(name, pi.object);
		}

		if (sameType == false)
		{
			continue;   // properties are not the same type
		}

		// Add the property now
		//
		QtProperty* subProperty = nullptr;

		switch (type)
		{
			case QVariant::Int:
				subProperty = m_propertyIntManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyIntManager->setValue(subProperty, value.toInt());
				}
				break;

			case QVariant::String:
				subProperty = m_propertyStringManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyStringManager->setValue(subProperty, value.toString());
				}
				break;

			case QVariant::Double:
				subProperty = m_propertyDoubleManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyDoubleManager->setValue(subProperty, value.toDouble());
				}
				break;

			case QVariant::Bool:
				subProperty = m_propertyBoolManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyBoolManager->setValue(subProperty, value.toBool() == true ? Qt::Checked : Qt::Unchecked);
				}
				else
				{
					m_propertyBoolManager->setValue(subProperty, Qt::PartiallyChecked);
				}
				break;

			case QVariant::Color:
				subProperty = m_propertyColorManager->addProperty(name);
				if (sameValue == true)
				{
					m_propertyColorManager->setValue(subProperty, value);
				}
				break;

			default:
				Q_ASSERT(false);
				continue;
		}

		if (subProperty == nullptr)
		{
			Q_ASSERT(subProperty);
			continue;
		}

		commonProperty->addSubProperty(subProperty);
	}

	addProperty(commonProperty);

	return;
}

void PropertyEditor::updateProperties(const QString& propertyName)
{
	QSet<QtProperty*> props;
	QMap<QtProperty*, QVariant> vals;

	//m_propertyIntManager

	if (propertyName.isEmpty() == true)
	{
		props = m_propertyIntManager->properties();
	}
	else
	{
		props = m_propertyIntManager->propertyByName(propertyName);
	}

	createValuesMap(props, vals);

	for (auto p : props)
	{
		m_propertyIntManager->setValue(p, vals.value(p));
	}

	//m_propertyBoolManager

	if (propertyName.isEmpty() == true)
	{
		props = m_propertyBoolManager->properties();
	}
	else
	{
		props = m_propertyBoolManager->propertyByName(propertyName);
	}

	createValuesMap(props, vals);

	for (auto p : props)
	{
		m_propertyBoolManager->setValue(p, vals.value(p));
	}

	//m_propertyDoubleManager

	if (propertyName.isEmpty() == true)
	{
		props = m_propertyDoubleManager->properties();
	}
	else
	{
		props = m_propertyDoubleManager->propertyByName(propertyName);
	}

	createValuesMap(props, vals);

	for (auto p : props)
	{
		m_propertyDoubleManager->setValue(p, vals.value(p));
	}

	//m_propertyStringManager

	if (propertyName.isEmpty() == true)
	{
		props = m_propertyStringManager->properties();
	}
	else
	{
		props = m_propertyStringManager->propertyByName(propertyName);
	}

	createValuesMap(props, vals);

	for (auto p : props)
	{
		m_propertyStringManager->setValue(p, vals.value(p));
	}

}

void PropertyEditor::updateProperties()
{
	updateProperties(QString());
}

void PropertyEditor::createValuesMap(const QSet<QtProperty*>& props, QMap<QtProperty*, QVariant>& values)
{
	values.clear();

	for (auto p = props.begin(); p != props.end(); p++)
	{
		QtProperty* property = *p;

		bool sameValue = true;
		QVariant value;

		QList<std::shared_ptr<QObject>> objects = m_propToClassMap.values(property->propertyName());
		for (auto i = objects.begin(); i != objects.end(); i++)
		{
			std::shared_ptr<QObject> pObject = *i;
			QMetaProperty metaProperty;
			if (propertyByName(pObject, property->propertyName(), metaProperty) == false)
			{
				Q_ASSERT(false);
				continue;
			}

			QVariant val = metaProperty.read(pObject.get());

			if (metaProperty.type() == QVariant::Bool)
			{
				if (val == true)
				{
					val = Qt::Checked;
				}
				else
				{
					val = Qt::Unchecked;
				}
			}

			if (i == objects.begin())
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

		if (sameValue == true)
		{
			values.insert(property, value);
		}
		else
		{
			values.insert(property, QVariant());
		}
	}
}

void PropertyEditor::clearProperties()
{
	m_propertyBoolManager->clear();
	m_propertyColorManager->clear();
	m_propertyDoubleManager->clear();
	m_propertyStringManager->clear();
	m_propertyIntManager->clear();
	m_propertyGroupManager->clear();
	clear();
	m_propToClassMap.clear();
}

void PropertyEditor::onValueChanged(QtProperty* property, QVariant value)
{
	valueChanged(property, value);
	updateProperties(property->propertyName());
}

void PropertyEditor::valueChanged(QtProperty* property, QVariant value)
{
	// Set the new property value in all objects
	//
	QList<std::shared_ptr<QObject>> objects = m_propToClassMap.values(property->propertyName());

	QString errorString;
	QMetaProperty writeProperty;

	for (auto i = objects.begin(); i != objects.end(); i++)
	{
		QObject* pObject = i->get();

		if (propertyByName(*i, property->propertyName(), writeProperty) == false)
		{
			Q_ASSERT(false);
			continue;
		}

		if (writeProperty.type() == QVariant::Bool)
		{
			if (value == Qt::Unchecked)
				value = false;
			else
				value = true;
		}

		writeProperty.write(pObject, value);

		if (writeProperty.read(pObject) != value && errorString.isEmpty() == true)
		{
			errorString = QString("Property: %1 - incorrect input value")
						  .arg(property->propertyName());
		}
	}

	if (errorString.isEmpty() == false)
	{
		updateProperties();
		emit showErrorMessage(errorString);
	}

	emit propertiesChanged(objects);

	return;
}

bool PropertyEditor::propertyByName(const std::shared_ptr<QObject>& object, const QString& name, QMetaProperty& metaProperty)
{
	const QMetaObject* metaObject = object->metaObject();

	int index = metaObject->indexOfProperty(name.toStdString().c_str());
	if (index == -1)
	{
		Q_ASSERT(false);
		return false;
	}
	metaProperty = metaObject->property(index);
	return true;
}

void PropertyEditor::onShowErrorMessage(QString message)
{
	QMessageBox::warning(this, "Error", message);
}
