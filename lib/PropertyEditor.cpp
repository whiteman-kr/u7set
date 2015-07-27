#include "../include/PropertyEditor.h"
#include "Settings.h"
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
#include <QFileDialog>

namespace ExtWidgets
{

	int FilePathPropertyType::filePathTypeId()
	{
		return qMetaTypeId<FilePathPropertyType>();
	}

	int EnumPropertyType::enumTypeId()
	{
		return qMetaTypeId<EnumPropertyType>();
	}

	//
	// ------------ QtMultiFilePathEdit ------------
	//
	QtMultiFilePathEdit::QtMultiFilePathEdit(QWidget* parent):
		QWidget(parent)
	{
		m_lineEdit = new QLineEdit(parent);

		QToolButton* button = new QToolButton(parent);
		button->setText("...");

		connect(m_lineEdit, &QLineEdit::editingFinished, this, &QtMultiFilePathEdit::onEditingFinished);

		connect(button, &QToolButton::clicked, this, &QtMultiFilePathEdit::onButtonPressed);

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_lineEdit);
		lt->addWidget(button, 0, Qt::AlignRight);

		setLayout(lt);

		m_lineEdit->installEventFilter(this);

		QTimer::singleShot(0, m_lineEdit, SLOT(setFocus()));
	}

	bool QtMultiFilePathEdit::eventFilter(QObject* watched, QEvent* event)
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

	void QtMultiFilePathEdit::onButtonPressed()
	{
		FilePathPropertyType f = m_oldPath.value<FilePathPropertyType>();

		QString filePath = QFileDialog::getOpenFileName(this, tr("Select file"), f.filePath, f.filter);
		if (filePath.isEmpty() == true)
		{
			return;
		}

		f.filePath = QDir::toNativeSeparators(filePath);

		setValue(QVariant::fromValue(f));
		emit valueChanged(QVariant::fromValue(f));
	}

	void QtMultiFilePathEdit::setValue(QVariant value)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

		m_oldPath = value;

		FilePathPropertyType f = value.value<FilePathPropertyType>();
		m_lineEdit->setText(f.filePath);
	}

	void QtMultiFilePathEdit::onEditingFinished()
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
	QtMultiEnumEdit::QtMultiEnumEdit(QWidget* parent):
		QWidget(parent)
	{
		m_combo = new QComboBox(parent);

		connect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_combo);

		setLayout(lt);
	}

	void QtMultiEnumEdit::setItems(QVariant value)
	{
		if (m_combo == nullptr)
		{
			Q_ASSERT(m_combo);
			return;
		}

		m_oldValue = value;

		EnumPropertyType e = value.value<EnumPropertyType>();

		m_combo->blockSignals(true);

		for (std::pair<QString, int>& i : e.items)
		{
			m_combo->addItem(i.first, i.second);
		}

		m_combo->blockSignals(false);

	}

	void QtMultiEnumEdit::setValue(QVariant value)
	{
		if (m_combo == nullptr)
		{
			Q_ASSERT(m_combo);
			return;
		}

		m_oldValue = value;

		EnumPropertyType e = value.value<EnumPropertyType>();

		// select an item with a value
		//

		bool found =  false;
		for (int i = 0; i < m_combo->count(); i++)
		{
			if (m_combo->itemData(i).toInt() == e.value)
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
	}

	void QtMultiEnumEdit::indexChanged(int index)
	{
		EnumPropertyType e = m_oldValue.value<EnumPropertyType>();

		int value = m_combo->itemData(index).toInt();

		if (e.value != value)
		{
			e.value = value;
			m_oldValue = QVariant::fromValue(e);

			/*QVariant v = value;
		bool result = v.convert(e.typeVariant.userType());
		if (result == false)
		{
			assert(false);
		}*/

			/*QObject o;
		o.setProperty("Fake", e.typeValue);
		o.setProperty("Fake", value);

		qDebug()<<value;

		QVariant v = o.property("Fake");*/

			//QVariant iValue = value;

			emit valueChanged(m_oldValue);
		}
	}


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

		QTimer::singleShot(0, m_lineEdit, SLOT(setFocus()));
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

		m_oldColor = color;

		m_lineEdit->setText(val);
	}

	void QtMultiColorEdit::onEditingFinished()
	{
		if (m_escape == false)
		{
			QString t = m_lineEdit->text();

			QColor color = colorFromText(t);

			if (color != m_oldColor)
			{
				emit valueChanged(color);
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
		QDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint)
	{
		setWindowTitle("Text Editor");

		if (theSettings.m_multiLinePropertyEditorWindowPos.x() != -1 && theSettings.m_multiLinePropertyEditorWindowPos.y() != -1)
		{
			move(theSettings.m_multiLinePropertyEditorWindowPos);
			restoreGeometry(theSettings.m_multiLinePropertyEditorGeometry);
		}

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

	void MultiLineEdit::closeEvent(QCloseEvent *event)
	{
		Q_UNUSED(event);
		theSettings.m_multiLinePropertyEditorWindowPos = pos();
		theSettings.m_multiLinePropertyEditorGeometry = saveGeometry();
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
		QTimer::singleShot(0, m_lineEdit, SLOT(setFocus()));
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

		m_oldValue = value;
		m_lineEdit->setText(value);
	}

	void QtMultiTextEdit::onEditingFinished()
	{
		if (m_escape == false)
		{
			if (m_lineEdit->text() != m_oldValue)
			{
				emit valueChanged(m_lineEdit->text());
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
	// ---------QtMultiUIntSpinBox----------
	//

	QtMultiUIntSpinBox::QtMultiUIntSpinBox(QWidget* parent):
		QWidget(parent)
	{
		m_spinBox = new QSpinBox(parent);
		m_spinBox->setKeyboardTracking(false);
		// warning! a problem that QSpinBox::setRange needs "ints", and we have uint type...
		m_spinBox->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

		connect(m_spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
				this, &QtMultiUIntSpinBox::onValueChanged);

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->addWidget(m_spinBox);
		setLayout(lt);

		m_spinBox->installEventFilter(this);
	}

	bool QtMultiUIntSpinBox::eventFilter(QObject* watched, QEvent* event)
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

	void QtMultiUIntSpinBox::setValue(quint32 value)
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

	void QtMultiUIntSpinBox::onValueChanged(quint32 value)
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

		emit valueChanged(state == Qt::Checked ? true : false);
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

		if (manager->value(property).userType() == FilePathPropertyType::filePathTypeId())
		{
			QtMultiFilePathEdit* m_editor = new QtMultiFilePathEdit(parent);
			editor = m_editor;
			m_editor->setValue(manager->value(property));

			connect(m_editor, &QtMultiFilePathEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
			connect(m_editor, &QtMultiFilePathEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
		}
		else
		{
			if (manager->value(property).userType() == EnumPropertyType::enumTypeId())
			{
				QtMultiEnumEdit* m_editor = new QtMultiEnumEdit(parent);
				editor = m_editor;
				m_editor->setItems(manager->value(property));
				m_editor->setValue(manager->value(property));

				connect(m_editor, &QtMultiEnumEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
				connect(m_editor, &QtMultiEnumEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
			}
			else
			{
				switch(manager->value(property).userType())
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
					case QVariant::UInt:
						{
							QtMultiUIntSpinBox* m_editor = new QtMultiUIntSpinBox(parent);
							editor = m_editor;
							m_editor->setValue(manager->value(property).toUInt());

							connect(m_editor, &QtMultiUIntSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiUIntSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
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

							if (manager->sameValue(property) == true)
							{
								m_editor->setCheckState(manager->value(property).toBool() == true ? Qt::Checked : Qt::Unchecked);
							}
							else
							{
								m_editor->setCheckState(Qt::PartiallyChecked);
							}

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
			}
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
		m_manager->setAttribute(m_property, "@propertyEditor@sameValue", true);
		m_manager->emitSetValue(m_property, value);
	}

	void QtMultiVariantFactory::slotEditorDestroyed(QObject* object)
	{
		Q_UNUSED(object);
	}

	//
	// ---------QtMultiVariantPropertyManager----------
	//

	QtMultiVariantPropertyManager::QtMultiVariantPropertyManager(QObject* parent) :
		QtAbstractPropertyManager(parent)
	{

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

	QVariant QtMultiVariantPropertyManager::attribute(const QtProperty* property, const QString& attribute) const
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

	bool QtMultiVariantPropertyManager::hasAttribute(const QtProperty* property, const QString& attribute) const
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

	int QtMultiVariantPropertyManager::valueType(const QtProperty* property) const
	{
		if (property == nullptr)
		{
			Q_ASSERT(property);
			return QVariant::Invalid;
		}

		return value(property).type();
	}

	bool QtMultiVariantPropertyManager::sameValue(const QtProperty* property) const
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

		it.value().value = value;

		emit propertyChanged(property);
	}

	void QtMultiVariantPropertyManager::setAttribute (QtProperty* property, const QString& attribute, const QVariant& value)
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

		if (value(property).userType() == FilePathPropertyType::filePathTypeId())
		{
			return QIcon();
		}

		switch (value(property).userType())
		{
			case QVariant::Bool:
				{
					if (sameValue(property) == true)
					{
						Qt::CheckState checkState = value(property).toBool() == true ? Qt::Checked : Qt::Unchecked;
						return drawCheckBox(checkState);
					}
					else
					{
						return drawCheckBox(Qt::PartiallyChecked);
					}
				}
				break;
			case QVariant::Color:
				{
					if (sameValue(property) == true)
					{
						QColor color = value(property).value<QColor>();
						return drawColorBox(color);
					}
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

		if (sameValue(property) == true)
		{
			int type = value(property).userType();

			if (type == FilePathPropertyType::filePathTypeId())
			{
				FilePathPropertyType f = value(property).value<FilePathPropertyType>();
				return f.filePath;
			}

			if (type == EnumPropertyType::enumTypeId())
			{
				EnumPropertyType e = value(property).value<EnumPropertyType>();
				for (std::pair<QString, int>& i : e.items)
				{
					if (i.second == e.value)
					{
						return i.first;
					}
				}
				return QString();
			}

			switch (type)
			{
				case QVariant::Int:
					{
						int val = value(property).toInt();
						return QString::number(val);
					}
					break;

				case QVariant::UInt:
					{
						quint32 val = value(property).toUInt();
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
						return value(property).toBool() == true ? "True" : "False";
					}
					break;
				case QVariant::String:
					{
						QString val = value(property).toString();

						if (val.length() > 32)
						{
							val = val.left(32) + "...";
						}

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
		else
		{
			switch (value(property).type())
			{
				case QVariant::Bool:
					return "<Different values>";
				default:
					return QString();
			}
		}

		return QString();
	}

	QString QtMultiVariantPropertyManager::displayText(const QtProperty *property) const
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
		m_propertyGroupManager = new QtGroupPropertyManager(this);
		m_propertyVariantManager = new QtMultiVariantPropertyManager(this);

		QtMultiVariantFactory* spinBoxFactory = new QtMultiVariantFactory(this);
		QtMultiVariantFactory* doubleSpinBoxFactory = new QtMultiVariantFactory(this);
		QtMultiVariantFactory* lineEditFactory = new QtMultiVariantFactory(this);
		QtMultiVariantFactory *checkBoxFactory = new QtMultiVariantFactory(this);
		QtMultiVariantFactory *colorFactory = new QtMultiVariantFactory(this);

		setFactoryForManager(m_propertyVariantManager, lineEditFactory);
		setFactoryForManager(m_propertyVariantManager, spinBoxFactory);
		setFactoryForManager(m_propertyVariantManager, doubleSpinBoxFactory);
		setFactoryForManager(m_propertyVariantManager, checkBoxFactory);
		setFactoryForManager(m_propertyVariantManager, colorFactory);

		connect(m_propertyVariantManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);

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
		setVisible(false);

		clearProperties();

		QMap<QString, PropertyItem> propertyItems;
		QList<QString> propertyNames;

		// Create a map with all properties
		//
		for (auto pobject = objects.begin(); pobject != objects.end(); pobject++)
		{
			QObject* object = pobject->get();

			// Add static properties declared by Q_PROPERTY
			//
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

				if (metaProperty.isEnumType())
				{
					// Fill enum's keys and set special value
					//
					EnumPropertyType ept;
					ept.value = object->property(name).toInt();
					ept.typeVariant = object->property(name);
					for (int i = 0; i < metaProperty.enumerator().keyCount(); i++)
					{
						ept.items.push_back(std::make_pair(metaProperty.enumerator().key(i), metaProperty.enumerator().value(i)));
					}
					pi.value = QVariant::fromValue(ept);

				}
				else
				{
					// Set the real value
					//
					pi.value = object->property(name);
				}
				pi.type = pi.value.userType();

				propertyItems.insertMulti(name, pi);

				if (propertyNames.indexOf(name) == -1)
				{
					propertyNames.append(name);
				}
			}

			//Add dynamic properties added by SetProperty
			//
			QList<QByteArray> dynamicPropNames = object->dynamicPropertyNames();


			// Sort dynamic properties by name
			//
			QStringList dynamicPropSortedNames;
			for (auto name : dynamicPropNames)
			{
				dynamicPropSortedNames.append(name);
			}
			dynamicPropSortedNames.sort();

			for (auto name : dynamicPropSortedNames)
			{

				static PropertyItem pi;

				pi.object = *pobject;
				pi.value = object->property(name.toStdString().c_str());
				pi.type = pi.value.userType();

				propertyItems.insertMulti(name, pi);

				if (propertyNames.indexOf(name) == -1)
				{
					propertyNames.append(name);
				}
			}
		}

		// add only common properties with same type
		//
		for (auto name = propertyNames.begin(); name != propertyNames.end(); name++)
		{
			// take all properties witn the same name
			//
			QList<PropertyItem> items = propertyItems.values(*name);

			if (items.size() != objects.size())
			{
				continue;   // this property is not in all objects
			}

			// now check if all properties have the same type and values
			//
			static int type;
			static QVariant value;

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

					if (pi.type == EnumPropertyType::enumTypeId())
					{
						EnumPropertyType v1 = pi.value.value<EnumPropertyType>();
						EnumPropertyType v2 = value.value<EnumPropertyType>();

						if (v1.value != v2.value)
						{
							sameValue = false;
						}
					}
					else
					{
						if (pi.value != value)
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

			QtProperty* property = createProperty(nullptr, *name, *name, value, type, sameValue);

			for (auto p = items.begin(); p != items.end(); p++)
			{
				PropertyItem& pi = *p;
				m_propToClassMap.insertMulti(property, pi.object);
			}
		}

		setVisible(true);
		return;
	}

	QtProperty* PropertyEditor::createProperty(QtProperty *parentProperty, const QString& name, const QString& fullName, const QVariant& value, int type, bool sameValue)
	{
		int slashPos = name.indexOf("\\");
		if (parentProperty == nullptr || slashPos != -1)
		{
			QString groupName;
			QString propName;

			if (slashPos == -1)
			{
				propName = name;
				groupName = "Common";
			}
			else
			{
				propName = name.right(name.length() - slashPos - 1);
				groupName = name.left(slashPos);
			}

			// Add the property now
			//
			QtProperty* subGroup = nullptr;
			QList<QtProperty*> propList;

			if (parentProperty != nullptr)
			{
				// Find, if group already exists
				//
				propList = parentProperty->subProperties();
			}
			else
			{
				propList = properties();
			}

			for (QtProperty* p : propList)
			{
				if (p->propertyName() == groupName)
				{
					subGroup = p;
					break;
				}
			}

			if (subGroup == nullptr)
			{
				subGroup = m_propertyGroupManager->addProperty(groupName);
			}

			QtProperty* property = createProperty(subGroup, propName, fullName, value, type, sameValue);

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

			if (type == FilePathPropertyType::filePathTypeId())
			{
				subProperty = m_propertyVariantManager->addProperty(fullName);
				if (sameValue == true)
				{
					m_propertyVariantManager->setValue(subProperty, value);
				}
				else
				{
					FilePathPropertyType f;
					m_propertyVariantManager->setValue(subProperty, QVariant::fromValue(f));
				}
			}
			else
			{
				if (type == EnumPropertyType::enumTypeId())
				{
					subProperty = m_propertyVariantManager->addProperty(fullName);
					if (sameValue == true)
					{
						m_propertyVariantManager->setValue(subProperty, value);
					}
					else
					{
						EnumPropertyType e;

						EnumPropertyType v = value.value<EnumPropertyType>();
						for (size_t i = 0; i < v.items.size(); i++)
						{
							e.items.push_back(v.items[i]);
						}

						m_propertyVariantManager->setValue(subProperty, QVariant::fromValue(e));
					}
				}
				else
				{

					switch (type)
					{
						case QVariant::Int:
							subProperty = m_propertyVariantManager->addProperty(fullName);
							if (sameValue == true)
							{
								m_propertyVariantManager->setValue(subProperty, value.toInt());
							}
							else
							{
								m_propertyVariantManager->setValue(subProperty, (int)0);
							}
							break;

						case QVariant::UInt:
							subProperty = m_propertyVariantManager->addProperty(fullName);
							if (sameValue == true)
							{
								m_propertyVariantManager->setValue(subProperty, value.toUInt());
							}
							else
							{
								m_propertyVariantManager->setValue(subProperty, (quint32)0);
							}
							break;

						case QVariant::String:
							subProperty = m_propertyVariantManager->addProperty(fullName);
							if (sameValue == true)
							{
								m_propertyVariantManager->setValue(subProperty, value.toString());
							}
							else
							{
								m_propertyVariantManager->setValue(subProperty, QString());
							}
							break;

						case QVariant::Double:
							subProperty = m_propertyVariantManager->addProperty(fullName);
							if (sameValue == true)
							{
								m_propertyVariantManager->setValue(subProperty, value.toDouble());
							}
							else
							{
								m_propertyVariantManager->setValue(subProperty, (double)0);
							}
							break;

						case QVariant::Bool:
							subProperty = m_propertyVariantManager->addProperty(fullName);
							if (sameValue == true)
							{
								m_propertyVariantManager->setValue(subProperty, value.toBool());
							}
							else
							{
								m_propertyVariantManager->setValue(subProperty, false);
							}
							break;

						case QVariant::Color:
							subProperty = m_propertyVariantManager->addProperty(fullName);
							if (sameValue == true)
							{
								m_propertyVariantManager->setValue(subProperty, value);
							}
							else
							{
								m_propertyVariantManager->setValue(subProperty, QColor(Qt::black));
							}
							break;

						default:
							Q_ASSERT(false);
							return nullptr;
					}
				}
			}

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

	void PropertyEditor::updateProperty(const QString& propertyName)
	{
		QSet<QtProperty*> props;
		QMap<QtProperty*, QVariant> vals;

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
			m_propertyVariantManager->setValue(p, vals.value(p));
		}
	}

	void PropertyEditor::updateProperties()
	{
		updateProperty(QString());
	}

	void PropertyEditor::createValuesMap(const QSet<QtProperty*>& props, QMap<QtProperty*, QVariant>& values)
	{
		values.clear();

		for (auto p = props.begin(); p != props.end(); p++)
		{
			QtProperty* property = *p;

			bool sameValue = true;
			QVariant value;

			QList<std::shared_ptr<QObject>> objects = m_propToClassMap.values(property);
			for (auto i = objects.begin(); i != objects.end(); i++)
			{
				QObject* pObject = (*i).get();

				QVariant val = pObject->property(property->propertyName().toStdString().c_str());

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
		m_propertyVariantManager->clear();
		m_propertyGroupManager->clear();
		clear();
		m_propToClassMap.clear();
	}

	void PropertyEditor::onValueChanged(QtProperty* property, QVariant value)
	{

		// If the value has internal enumerated type, convert it to a native type
		// using previously saved old value with its type
		//
		QVariant newValue = value;

		if (newValue.userType() == EnumPropertyType::enumTypeId())
		{
			EnumPropertyType e = newValue.value<EnumPropertyType>();

			newValue = e.value;

			int oldType = e.typeVariant.userType();

			bool result = newValue.convert(oldType);
			if (result == false)
			{
				assert(false);	//maybe call QMetaType::registerConverter<int, enumname>(IntToEnum<enumname>);

				return;
			}
		}

		valueChanged(property, newValue);
	}

	void PropertyEditor::valueChanged(QtProperty* property, QVariant value)
	{
		// Set the new property value in all objects
		//
		QList<std::shared_ptr<QObject>> objects = m_propToClassMap.values(property);
		QList<std::shared_ptr<QObject>> modifiedObjects;

		if (objects.size() == 0)
		{
			Q_ASSERT(objects.size() > 0);
			return;
		}

		QString errorString;

		for (auto i = objects.begin(); i != objects.end(); i++)
		{
			QObject* pObject = i->get();

			if (value.userType() == EnumPropertyType::enumTypeId())
			{
				assert(false);
				continue;
			}

			// Do not set property, if it has same value
			if (pObject->property(property->propertyName().toStdString().c_str()) == value)
			{
				continue;
			}

			QVariant oldValue = pObject->property(property->propertyName().toStdString().c_str());

			pObject->setProperty(property->propertyName().toStdString().c_str(), value);

			QVariant newValue = pObject->property(property->propertyName().toStdString().c_str());

			if (oldValue == newValue && errorString.isEmpty() == true)
			{
				errorString = QString("Property: %1 - incorrect input value")
							  .arg(property->propertyName());
			}

			modifiedObjects.append(*i);
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

	void PropertyEditor::onShowErrorMessage(QString message)
	{
		QMessageBox::warning(this, "Error", message);
	}

}
