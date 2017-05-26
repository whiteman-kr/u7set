#include "../lib/PropertyObject.h"
#include "../lib/PropertyEditor.h"
#include "Settings.h"

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


namespace ExtWidgets
{
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

	bool PropertyTextEditor::modified()
	{
		return m_modified;
	}

	void PropertyTextEditor::textChanged()
	{
		m_modified = true;
	}

	//
	// ------------ PropertyPlainTextEditor ------------
	//
	PropertyPlainTextEditor::PropertyPlainTextEditor(QWidget* parent) :
		PropertyTextEditor(parent)
	{
		m_textEdit = new QPlainTextEdit();

		QHBoxLayout* l = new QHBoxLayout(this);
		l->addWidget(m_textEdit);

		m_textEdit->setTabChangesFocus(false);
		m_textEdit->setFont(QFont("Courier", font().pointSize() + 2));

		QFontMetrics metrics(m_textEdit->font());

		const int tabStop = 4;  // 4 characters
		QString spaces;
		for (int i = 0; i < tabStop; ++i)
		{
			spaces += " ";
		}

		m_textEdit->setTabStopWidth(metrics.width(spaces));

		connect(m_textEdit, &QPlainTextEdit::textChanged, this, &PropertyPlainTextEditor::textChanged);

	}

	void PropertyPlainTextEditor::setText(const QString& text)
	{
		m_textEdit->blockSignals(true);

		m_textEdit->setPlainText(text);

		m_textEdit->blockSignals(false);
	}

	QString PropertyPlainTextEditor::text()
	{
		return m_textEdit->toPlainText();
	}

	void PropertyPlainTextEditor::setReadOnly(bool value)
	{
		m_textEdit->setReadOnly(value);
	}

	bool PropertyPlainTextEditor::eventFilter(QObject* obj, QEvent* event)
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
			}
		}

		// pass the event on to the parent class
		return PropertyTextEditor::eventFilter(obj, event);
	}

	//
	// ------------ QtMultiFilePathEdit ------------
	//
	QtMultiFilePathEdit::QtMultiFilePathEdit(QWidget* parent):
		QWidget(parent)
	{
		m_lineEdit = new QLineEdit(parent);

        m_button = new QToolButton(parent);
        m_button->setText("...");

		connect(m_lineEdit, &QLineEdit::editingFinished, this, &QtMultiFilePathEdit::onEditingFinished);

        connect(m_button, &QToolButton::clicked, this, &QtMultiFilePathEdit::onButtonPressed);

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_lineEdit);
        lt->addWidget(m_button, 0, Qt::AlignRight);

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

        m_oldPath = QVariant::fromValue(f);
        m_lineEdit->setText(f.filePath);

		emit valueChanged(QVariant::fromValue(f));
	}

	void QtMultiFilePathEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

        m_oldPath = property->value();

        FilePathPropertyType f = property->value().value<FilePathPropertyType>();
		m_lineEdit->setText(f.filePath);
		m_lineEdit->setReadOnly(readOnly);
		m_button->setEnabled(readOnly == false);
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

	void QtMultiEnumEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
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

        m_oldValue = property->value().toInt();

        if (m_combo->count() == 0)
        {
            m_combo->blockSignals(true);
            for (std::pair<int, QString> i : property->enumValues())
            {
                m_combo->addItem(i.second, i.first);
            }
            m_combo->blockSignals(false);
        }

        // select an item with a value
		//
		bool found =  false;
		for (int i = 0; i < m_combo->count(); i++)
		{
            if (m_combo->itemData(i).toInt() == m_oldValue)
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

		m_combo->setEnabled(readOnly == false);
	}

	void QtMultiEnumEdit::indexChanged(int index)
	{
		int value = m_combo->itemData(index).toInt();
        if (m_oldValue != value)
		{
            m_oldValue = value;
			emit valueChanged(m_combo->itemText(index));
        }
	}


	//
	// ---------QtMultiColorEdit----------
	//

	QtMultiColorEdit::QtMultiColorEdit(QWidget* parent):
		QWidget(parent)
	{
		m_lineEdit = new QLineEdit(parent);

        m_button = new QToolButton(parent);
        m_button->setText("...");

		connect(m_lineEdit, &QLineEdit::editingFinished, this, &QtMultiColorEdit::onEditingFinished);

        connect(m_button, &QToolButton::clicked, this, &QtMultiColorEdit::onButtonPressed);

		QHBoxLayout* lt = new QHBoxLayout;
		lt->setContentsMargins(0, 0, 0, 0);
		lt->setSpacing(0);
		lt->addWidget(m_lineEdit);
        lt->addWidget(m_button, 0, Qt::AlignRight);

		setLayout(lt);

		m_lineEdit->installEventFilter(this);

		QRegExp regexp("\\[([1,2]?[0-9]{0,2};){3}[1,2]?[0-9]{0,2}\\]");
		QRegExpValidator* validator = new QRegExpValidator(regexp, this);
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

	void QtMultiColorEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
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
		m_lineEdit->setReadOnly(readOnly);
		m_button->setEnabled(readOnly == false);
	}

	void QtMultiColorEdit::onEditingFinished()
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
	MultiLineEdit::MultiLineEdit(QWidget* parent, PropertyEditor* propertyEditor, const QString &text, std::shared_ptr<Property> p):
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

		QVBoxLayout* vl = new QVBoxLayout();

		// Create Editor

		m_editor = m_propertyEditor->createCodeEditor(m_property->isScript() == true, this);

		m_editor->setText(text);

		connect(m_editor, &PropertyTextEditor::escapePressed, this, &MultiLineEdit::reject);

		// Buttons

		QPushButton* okButton = new QPushButton(tr("OK"), this);
		QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);

		okButton->setDefault(true);

		connect(okButton, &QPushButton::clicked, this, &MultiLineEdit::accept);
		connect(cancelButton, &QPushButton::clicked, this, &MultiLineEdit::reject);

		connect(this, &QDialog::finished, this, &MultiLineEdit::finished);

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
		hl->addWidget(okButton);
		hl->addWidget(cancelButton);


		vl->addWidget(m_editor);
		vl->addLayout(hl);

		setLayout(vl);
	}

	QString MultiLineEdit::text()
	{
		return m_text;
	}

	void MultiLineEdit::finished(int result)
	{
		Q_UNUSED(result);

		theSettings.m_multiLinePropertyEditorWindowPos = pos();
		theSettings.m_multiLinePropertyEditorGeometry = saveGeometry();

	}

	void MultiLineEdit::accept()
	{
		if (m_editor == nullptr)
		{
			Q_ASSERT(m_editor);
			return;
		}

		m_text = m_editor->text();

		QDialog::accept();
	}

	void MultiLineEdit::reject()
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
	// ---------QtMultiTextEdit----------
	//

	QtMultiTextEdit::QtMultiTextEdit(QWidget* parent, PropertyEditor* propertyEditor, std::shared_ptr<Property> p):
		QWidget(parent),
		m_propertyEditor(propertyEditor),
		m_property(p),
		m_userType(p->value().userType())
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

		m_lineEdit = new QLineEdit(parent);
		connect(m_lineEdit, &QLineEdit::editingFinished, this, &QtMultiTextEdit::onEditingFinished);

		if (m_userType == QVariant::String && p->validator().isEmpty() == true && p->password() == false)
		{
			m_button = new QToolButton(parent);
			m_button->setText("...");

			connect(m_button, &QToolButton::clicked, this, &QtMultiTextEdit::onButtonPressed);
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

		if (p->validator().isEmpty() == false)
        {
			QRegExp regexp(p->validator());
			QRegExpValidator* v = new QRegExpValidator(regexp, this);
            m_lineEdit->setValidator(v);
        }

		if (p->password() == true)
        {
            m_lineEdit->setEchoMode(QLineEdit::Password);
        }


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
		MultiLineEdit* multlLineEdit = new MultiLineEdit(this, m_propertyEditor, m_lineEdit->text(), m_property);
		if (multlLineEdit->exec() == QDialog::Accepted)
		{
            m_lineEdit->blockSignals(true);

            m_lineEdit->setText(multlLineEdit->text());
            if (m_lineEdit->text() != m_oldValue)
            {
                 emit valueChanged(m_lineEdit->text());
            }

            m_oldValue = m_lineEdit->text();

            m_lineEdit->blockSignals(false);
        }
		delete multlLineEdit;
	}

	void QtMultiTextEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

		switch (m_userType)
		{
		case QVariant::String:
			{
				m_oldValue = property->value().toString();
				m_lineEdit->setText(m_oldValue.toString());
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
		case QVariant::Double:
		{
			m_oldValue = property->value().toDouble();
			m_lineEdit->setText(QString::number(m_oldValue.toDouble(), 'f', property->precision()));
		}
			break;
		default:
			assert(false);
			return;
		}

		m_lineEdit->setReadOnly(readOnly);

		if (m_button != nullptr)
		{
			m_button->setEnabled(readOnly == false);
		}
	}

	void QtMultiTextEdit::onEditingFinished()
	{
		if (m_escape == true)
		{
			return;
		}

		m_lineEdit->blockSignals(true);

/*		if (m_lineEdit->text() != m_oldValue)
		{
			   emit valueChanged(m_lineEdit->text());
		}*/
		switch (m_userType)
		{
		case QVariant::String:
			{
				if (m_lineEdit->text() != m_oldValue.toString())
				{
					m_oldValue = m_lineEdit->text();
					emit valueChanged(m_lineEdit->text());
				}
			}
			break;
		case QVariant::Int:
			{
				bool ok = false;
				int value = m_lineEdit->text().toInt(&ok);
				if (ok == true && value != m_oldValue.toInt())
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
				if (ok == true && value != m_oldValue.toUInt())
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
				if (ok == true && value != m_oldValue.toDouble())
				{
					m_oldValue = value;
					emit valueChanged(value);
				}
			}
			break;
		default:
			assert(false);
			return;
		}

        m_lineEdit->blockSignals(false);
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

	void QtMultiCheckBox::setValue(bool value, bool readOnly)
	{
		if (m_checkBox == nullptr)
		{
			Q_ASSERT(m_checkBox);
			return;
		}

		m_checkBox->blockSignals(true);
		m_checkBox->setCheckState(value ? Qt::Checked : Qt::Unchecked);

		updateText();
		m_checkBox->setEnabled(readOnly == false);
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

	QtMultiVariantFactory::QtMultiVariantFactory(PropertyEditor* propertyEditor):
		QtAbstractEditorFactory<QtMultiVariantPropertyManager>(propertyEditor),
		m_propertyEditor(propertyEditor)
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

        std::shared_ptr<Property> p = manager->value(property);
        if (p == nullptr)
        {
            assert(p);
            return nullptr;
        }

        if (p->isEnum())
        {
            QtMultiEnumEdit* m_editor = new QtMultiEnumEdit(parent);
            editor = m_editor;
			m_editor->setValue(p, m_property->isEnabled() == false);

            connect(m_editor, &QtMultiEnumEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
            connect(m_editor, &QtMultiEnumEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
        }
        else
        {

            if (p->value().userType() == FilePathPropertyType::filePathTypeId())
            {
                QtMultiFilePathEdit* m_editor = new QtMultiFilePathEdit(parent);
                editor = m_editor;
				m_editor->setValue(p, m_property->isEnabled() == false);

                connect(m_editor, &QtMultiFilePathEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
                connect(m_editor, &QtMultiFilePathEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
            }
            else
            {
                switch(p->value().userType())
				{
/*					case QVariant::Int:
						{
							QtMultiIntSpinBox* m_editor = new QtMultiIntSpinBox(parent);
							editor = m_editor;
                            m_editor->setValue(p);

							connect(m_editor, &QtMultiIntSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiIntSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
						}
						break;
					case QVariant::UInt:
						{
							QtMultiUIntSpinBox* m_editor = new QtMultiUIntSpinBox(parent);
							editor = m_editor;
                            m_editor->setValue(p);

							connect(m_editor, &QtMultiUIntSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiUIntSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
						}
						break;
					case QVariant::Double:
						{
							QtMultiDoubleSpinBox* m_editor = new QtMultiDoubleSpinBox(parent);
							editor = m_editor;
                            m_editor->setValue(p);

							connect(m_editor, &QtMultiDoubleSpinBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiDoubleSpinBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
						}
						break;
						*/
					case QVariant::Bool:
						{
							QtMultiCheckBox* m_editor = new QtMultiCheckBox(parent);
							editor = m_editor;

							connect(m_editor, &QtMultiCheckBox::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiCheckBox::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);

							if (m_property->isEnabled() == false)
							{
								m_editor->setValue(p->value().toBool(), m_property->isEnabled() == false);
							}
							else
							{
								// change value on first click
								//
								bool newValue = p->value().toBool();

								if (manager->sameValue(property) == false)
								{
									newValue = true;
								}
								else
								{
									newValue = !newValue;
								}

								m_editor->setValue(newValue, m_property->isEnabled() == false);

								m_valueSetOnTimer = newValue;
								QTimer::singleShot(10, this, &QtMultiVariantFactory::slotSetValueTimer);
							}
						}
						break;
					case QVariant::String:
					case QVariant::Int:
					case QVariant::UInt:
					case QVariant::Double:
						{
							QtMultiTextEdit* m_editor = new QtMultiTextEdit(parent, m_propertyEditor, p);

							editor = m_editor;
							m_editor->setValue(p, m_property->isEnabled() == false);

							connect(m_editor, &QtMultiTextEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiTextEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
						}
						break;

					case QVariant::Color:
						{
							QtMultiColorEdit* m_editor = new QtMultiColorEdit(parent);
							editor = m_editor;
							m_editor->setValue(p, m_property->isEnabled() == false);

							connect(m_editor, &QtMultiColorEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiColorEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
						}
						break;

					case QVariant::Uuid:
						{
							QtMultiTextEdit* m_editor = new QtMultiTextEdit(parent, m_propertyEditor, p);
							editor = m_editor;
							m_editor->setValue(p, m_property->isEnabled() == false);

							connect(m_editor, &QtMultiTextEdit::valueChanged, this, &QtMultiVariantFactory::slotSetValue);
							connect(m_editor, &QtMultiTextEdit::destroyed, this, &QtMultiVariantFactory::slotEditorDestroyed);
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

	void QtMultiVariantFactory::slotSetValueTimer()
	{
		emit slotSetValue(m_valueSetOnTimer);
	}

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


    std::shared_ptr<Property> QtMultiVariantPropertyManager::value(const QtProperty* property) const
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

        return value(property)->value().type();
	}

    void QtMultiVariantPropertyManager::setProperty(const QtProperty* property, std::shared_ptr<Property> propertyValue)
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

        it->p->setValue(value);

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

        emit propertyChanged(property);
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
						return drawCheckBox(checkState, property->isEnabled());
					}
					else
					{
						return drawCheckBox(Qt::PartiallyChecked, property->isEnabled());
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
            // enum is special
            //
            if (p->isEnum())
            {
                int v = p->value().toInt();
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

				case QVariant::Double:
					{
                        double val = value.toDouble();
                        int precision = p->precision();
                        if (precision == 0)
                        {
                            precision = 1;
                        }
                        return QString::number(val, 'f', precision);
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

				default:
					Q_ASSERT(false);
			}
		}
		else
		{
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

	QString QtMultiVariantPropertyManager::displayText(const QtProperty* property) const
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

		m_propertyGroupManager = new QtGroupPropertyManager(this);
		m_propertyVariantManager = new QtMultiVariantPropertyManager(this);

		QtMultiVariantFactory* editFactory = new QtMultiVariantFactory(this);

		setFactoryForManager(m_propertyVariantManager, editFactory);

		connect(m_propertyVariantManager, &QtMultiVariantPropertyManager::valueChanged, this, &PropertyEditor::onValueChanged);

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


	void PropertyEditor::setObjects(const QList<std::shared_ptr<PropertyObject>>& objects)
	{
        setVisible(false);

		clearProperties();

        QMap<QString, std::shared_ptr<Property>> propertyItems;
		QList<QString> propertyNames;

		// Create a map with all properties
		//
        m_objects.clear();

        for (auto pobject : objects)
		{
            m_objects.push_back(pobject);

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
            if (propsByName.size() != objects.size() || propsByName.size() == 0)
			{
				continue;   // this property is not in all objects
			}

			// now check if all properties have the same type and values
			//
            int type;
            QVariant value;

			bool sameType = true;
			bool sameValue = true;

            for (auto p = propsByName.begin(); p != propsByName.end(); p++)
			{
                if (p == propsByName.begin())
				{
                    Property* _p = p->get();

					// remember the first item params
					//
                    type = _p->value().userType();
                    value = _p->value();
				}
				else
				{
                    Property* _p = p->get();

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

            createProperty(nullptr, p->caption(), category, description, p, sameValue);
		}

        sortItems(0, Qt::AscendingOrder);

        setVisible(true);
		return;
	}

	const QList<std::shared_ptr<PropertyObject>>& PropertyEditor::objects() const
	{
		return m_objects;
	}

	QtProperty* PropertyEditor::createProperty(QtProperty* parentProperty, const QString& caption, const QString& category, const QString& description, const std::shared_ptr<Property> value, bool sameValue)
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

            QtProperty* property = createProperty(subGroup, caption, category, description, value, sameValue);

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
			subProperty->setEnabled(m_readOnly == false && value->readOnly() == false);
            m_propertyVariantManager->setProperty(subProperty, value);
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
            QVariant value = vals.value(p).first;
            bool sameValue = vals.value(p).second;

            m_propertyVariantManager->setAttribute(p, "@propertyEditor@sameValue", sameValue);

            if (sameValue == true)
            {
                m_propertyVariantManager->setValue(p, value);
            }
        }
	}

	void PropertyEditor::updateProperties()
	{
		updateProperty(QString());
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

	void PropertyEditor::clearProperties()
	{
		m_propertyVariantManager->clear();
		m_propertyGroupManager->clear();
		clear();
        m_objects.clear();
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

        for (auto i : m_objects)
		{
            PropertyObject* pObject = i.get();

			// Do not set property, if it has same value
            if (pObject->propertyValue(property->propertyName()) == value)
			{
				continue;
			}

            QVariant oldValue = pObject->propertyValue(property->propertyName());

            pObject->setPropertyValue(property->propertyName(), value);

            QVariant newValue = pObject->propertyValue(property->propertyName());

			if (oldValue == newValue && errorString.isEmpty() == true)
			{
				errorString = QString("Property: %1 - incorrect input value")
							  .arg(property->propertyName());
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

	PropertyTextEditor* PropertyEditor::createCodeEditor(bool script, QWidget* parent)
	{
		Q_UNUSED(script);
		return new PropertyPlainTextEditor(parent);
	}

	void PropertyEditor::onShowErrorMessage(QString message)
	{
		QMessageBox::warning(this, "Error", message);
	}

}
