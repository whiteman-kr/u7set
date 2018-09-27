#include "DialogAlert.h"

#include <QDialog>
#include <QDateTime>
#include <QDesktopWidget>

#include "../lib/Ui/UiTools.h"

DialogAlert::DialogAlert(QWidget* parent, QString title):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	m_textEdit = new QTextEdit();
	mainLayout->addWidget(m_textEdit);

	m_textEdit->setReadOnly(true);

	QHBoxLayout* buttonLayout = new QHBoxLayout();

	QPushButton* clearButton = new QPushButton("Clear");
	connect(clearButton, &QPushButton::clicked, this, &DialogAlert::onClear);
	buttonLayout->addWidget(clearButton);

	buttonLayout->addStretch();

	QPushButton* closeButton = new QPushButton("Close");
	connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
	buttonLayout->addWidget(closeButton);

	mainLayout->addLayout(buttonLayout);

	//

	if (title.isEmpty() == true)
	{
		setWindowTitle("Alert Window");
	}
	else
	{
		setWindowTitle(title);
	}

	QSize defaultSize = QSize(1024, 300);

	setMinimumSize(defaultSize);

	// Restore settings

	QSettings s;

	QPoint windowPos = s.value("DialogAlert/windowPos", QPoint(-1, -1)).toPoint();
	QSize windowSize = s.value("DialogAlert/windowSize", QSize(-1, -1)).toSize();

	if (windowPos.x() != -1 && windowPos.y() != -1)
	{
		move(windowPos);
	}
	else
	{
		QRect desktopRect = QApplication::desktop()->availableGeometry(this);
		int x = (desktopRect.width() - width()) / 2;
		int y = (desktopRect.height() - height()) / 2;
		move(QPoint(x, y));
	}

	if (windowSize.width() != -1 && windowSize.height() != -1)
	{
		resize(windowSize);
	}
	else
	{
		resize(defaultSize);
	}
}

DialogAlert::~DialogAlert()
{
	QSettings s;

	QPoint windowPos = pos();
	QSize windowSize = size();

	s.setValue("DialogAlert/windowPos", windowPos);
	s.setValue("DialogAlert/windowSize", windowSize);
}

void DialogAlert::onAlertArrived(QString text)
{
	if (isVisible() == false)
	{
		m_text.clear();

		UiTools::adjustDialogPlacement(this);

		show();
	}
	else
	{
		raise();
	}

	if (m_text.length() > 16384)
	{
		m_text.clear();
	}

	QDateTime time = QDateTime::currentDateTime();

	text = text.toHtmlEscaped();

	m_text += tr("<font size=\"4\" color=\"black\">%1</font><font size=\"4\" color=\"red\"> %2</font><br>").arg(time.toString("dd.MM.yyyy hh:mm:ss.zzz")).arg(text);

	m_textEdit->setHtml(m_text);

	QTextCursor textCursor = m_textEdit->textCursor();
	textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	m_textEdit->setTextCursor(textCursor);
}

void DialogAlert::onClear()
{
	m_text.clear();
	m_textEdit->clear();
}

