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

		if (m_combo->count() == 0)
		{
			m_combo->blockSignals(true);
			for (std::pair<int, QString> i : p->enumValues())
			{
				m_combo->addItem(i.second, i.first);
			}
			m_combo->blockSignals(false);
		}

		connect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));

		m_combo->setEnabled(readOnly == false);

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

		m_oldValue = property->value().toInt();

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

	void MultiEnumEdit::indexChanged(int index)
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
	// ---------MultiLineEdit----------
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
	// ---------QtMultiTextEdit----------
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

		if (m_userType == QVariant::String && p->password() == false)
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

		m_lineEdit->setReadOnly(readOnly == true);

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
		if (m_userType == QVariant::String)
		{
			editText = m_oldValue.toString();	// Take string from m_oldValue, because m_lineEdit has length limit
		}
		else
		{
			editText = editText = m_lineEdit->text();
		}

		MultiTextEditorDialog* multlLineEdit = new MultiTextEditorDialog(this, m_propertyEditor, editText, m_property);
		if (multlLineEdit->exec() == QDialog::Accepted)
		{
			editText = multlLineEdit->text();

			if (editText != m_oldValue)
			{
				 emit valueChanged(editText);
			}

			editText = m_property->value().toString();

			m_oldValue = editText;

			// update LineEdit

			m_lineEdit->blockSignals(true);

			bool longText = editText.length() > PropertyEditorTextMaxLength;

			m_lineEdit->setReadOnly(longText == true);

			if (longText == true)
			{
				m_lineEdit->setText(tr("<%1 bytes>").arg(editText.length()));
			}
			else
			{
				m_lineEdit->setText(editText);
			}

            m_lineEdit->blockSignals(false);
        }
		delete multlLineEdit;
	}

	void MultiTextEdit::setValue(std::shared_ptr<Property> property, bool readOnly)
	{
		if (m_lineEdit == nullptr)
		{
			Q_ASSERT(m_lineEdit);
			return;
		}

		m_lineEdit->setReadOnly(readOnly == true);

		switch (m_userType)
		{
		case QVariant::String:
			{
				m_oldValue = property->value().toString();

				QString editText = property->value().toString();

				bool longText = editText.length() > PropertyEditorTextMaxLength;

				m_lineEdit->setReadOnly(readOnly == true || longText == true);

				if (longText == true)
				{
					m_lineEdit->setText(tr("<%1 bytes>").arg(editText.length()));
				}
				else
				{
					m_lineEdit->setText(editText);
				}
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
			m_lineEdit->setText(QString::number(m_oldValue.toFloat(), 'g', property->precision()));
		}
			break;
		case QVariant::Double:
		{
			m_oldValue = property->value().toDouble();
			m_lineEdit->setText(QString::number(m_oldValue.toDouble(), 'g', property->precision()));
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
					emit valueChanged(m_lineEdit->text());
				}
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

	MultiCheckBox::MultiCheckBox(QWidget* parent):
		QWidget(parent)
	{
		m_checkBox = new QCheckBox(parent);

		connect(m_checkBox, &QCheckBox::stateChanged, this, &MultiCheckBox::onStateChanged);

		QHBoxLayout*lt = new QHBoxLayout;
		lt->setContentsMargins(3, 1, 0, 0);
		lt->addWidget(m_checkBox);
		setLayout(lt);
	}

	void MultiCheckBox::setValue(bool value, bool readOnly)
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
        }
        else
        {

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
			}
			else
			{
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
				}
				else
				{
					switch(propertyPtr->value().userType())
					{
						case QVariant::Bool:
							{
								MultiCheckBox* m_editor = new MultiCheckBox(parent);

								editor = m_editor;

								connect(m_editor, &MultiCheckBox::valueChanged, this, &MultiVariantFactory::slotSetValue);
								connect(m_editor, &MultiCheckBox::destroyed, this, &MultiVariantFactory::slotEditorDestroyed);

								if (m_property->isEnabled() == false)
								{
									m_editor->setValue(propertyPtr->value().toBool(), m_property->isEnabled() == false);
								}
								else
								{
									// change value on first click
									//
									bool newValue = propertyPtr->value().toBool();

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
									QTimer::singleShot(10, this, &MultiVariantFactory::slotSetValueTimer);
								}
							}
							break;
						case QVariant::String:
						case QVariant::Int:
						case QVariant::UInt:
						case QMetaType::Float:
						case QVariant::Double:
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

						case QVariant::Uuid:
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

						default:
							Q_ASSERT(false);
					}
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

	void MultiVariantFactory::connectPropertyManager (MultiVariantPropertyManager* manager)
	{
		Q_UNUSED(manager);
		//connect(manager, &QtMultiVariantPropertyManager::valueChanged, this, &QtMultiVariantFactory::slotPropertyChanged);
	}

	void MultiVariantFactory::disconnectPropertyManager(MultiVariantPropertyManager* manager)
	{
		Q_UNUSED(manager);
		//disconnect(manager, &QtMultiVariantPropertyManager::valueChanged, this, &QtMultiVariantFactory::slotPropertyChanged);
	}

	/*void QtMultiVariantFactory::slotPropertyChanged(QtProperty* property, QVariant value)
{
	Q_UNUSED(property);
	Q_UNUSED(value);
}*/

	void MultiVariantFactory::slotSetValueTimer()
	{
		emit slotSetValue(m_valueSetOnTimer);
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

			if (type == TuningValue::tuningValueTypeId())
			{
				TuningValue t = value.value<TuningValue>();
				return t.toString(p->precision());
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

				case QMetaType::Float:
					{
						float val = value.toFloat();
						return QString::number(val, 'g', p->precision());
					}
					break;
				case QVariant::Double:
					{
                        double val = value.toDouble();
						return QString::number(val, 'g', p->precision());
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

			if (pObject->propertyValue(propertyName) == value)
			{
				continue;
			}

			QVariant oldValue = pObject->propertyValue(propertyName);

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
