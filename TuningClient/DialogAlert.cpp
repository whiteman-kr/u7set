#include "DialogAlert.h"
#include "MainWindow.h"

DialogAlert::DialogAlert(QWidget* parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();
	setLayout(mainLayout);

	m_textEdit = new QTextEdit();
	mainLayout->addWidget(m_textEdit);

	m_textEdit->setReadOnly(true);

	QHBoxLayout* buttonLayout = new QHBoxLayout();

	buttonLayout->addStretch();

	QPushButton* closeButton = new QPushButton("Close");
	connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
	buttonLayout->addWidget(closeButton);

	mainLayout->addLayout(buttonLayout);

	setMinimumSize(1024, 300);
	setWindowTitle("Alert Window");
}

DialogAlert::~DialogAlert()
{

}

void DialogAlert::onAlertArrived(QString text)
{
	if (isVisible() == false)
	{
		m_text.clear();
		show();
	}
	else
	{
		activateWindow();
	}

	QDateTime time = QDateTime::currentDateTime();

	m_text += tr("<font size=\"4\" color=\"black\">%1</font><font size=\"4\" color=\"red\"> %2</font><br>").arg(time.toString("dd.MM.yyyy hh:mm:ss.zzz")).arg(text);

	m_textEdit->setHtml(m_text);

	QTextCursor textCursor = m_textEdit->textCursor();
	textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	m_textEdit->setTextCursor(textCursor);
}
