#include "TextEditCompleter.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>

QTextEditCompleter::QTextEditCompleter(QWidget *parent)
	: QTextEdit(parent)
{
}

QTextEditCompleter::~QTextEditCompleter()
{
}

void QTextEditCompleter::setCompleter(QCompleter* completer)
{
	if (m_completer)
	{
		QObject::disconnect(m_completer, nullptr, this, nullptr);
	}

	m_completer = completer;

	if (!m_completer)
        return;

	m_completer->setWidget(this);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCaseSensitivity(Qt::CaseSensitive);
	m_completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
	m_completer->setFilterMode(Qt::MatchFlag::MatchStartsWith);

	QObject::connect(m_completer, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

QCompleter *QTextEditCompleter::completer() const
{
	return m_completer;
}

void QTextEditCompleter::insertCompletion(const QString& completion)
{
	if (m_completer->widget() != this)
		return;
	QTextCursor tc = textCursor();
	int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString QTextEditCompleter::textUnderCursor() const
{
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void QTextEditCompleter::focusInEvent(QFocusEvent *e)
{
	if (m_completer)
		m_completer->setWidget(this);
    QTextEdit::focusInEvent(e);
}

void QTextEditCompleter::keyPressEvent(QKeyEvent *e)
{
	if (m_completer && m_completer->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
	if (!m_completer || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
	if (!m_completer || (ctrlOrShift && e->text().isEmpty()))
        return;

	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

	if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 4
                      || eow.contains(e->text().right(1)))) {
		m_completer->popup()->hide();
        return;
    }

	if (completionPrefix != m_completer->completionPrefix()) {
		m_completer->setCompletionPrefix(completionPrefix);
		m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
	cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
				+ m_completer->popup()->verticalScrollBar()->sizeHint().width());
	m_completer->complete(cr); // popup it up!
}
