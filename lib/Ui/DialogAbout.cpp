#include "../lib/Ui/DialogAbout.h"
#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

void DialogAbout::show(QWidget* parent, const QString& description)
{
	QDialog aboutDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	QHBoxLayout* hl = new QHBoxLayout;

	QLabel* logo = new QLabel(&aboutDialog);
	logo->setPixmap(QPixmap(":/Images/Images/logo.png"));

	hl->addWidget(logo);

	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);

	QString text = "<h3>" + qApp->applicationName() +" v" + qApp->applicationVersion() + "</h3>";

#ifndef Q_DEBUG
	text += "Build: Release";
#else
	text += "Build: Debug";
#endif

#ifdef CI_PIPELINE_IID
	text += "<br>Commit date: " GITLAB_CI_BUILD;
	text += "<br>Commit SHA1: " CI_COMMIT_SHA;
#else
	text += "<br>Commit date: LOCALBUILD";
	text += "<br>Commit SHA1: LOCALBUILD";
#endif

	QLabel* label = new QLabel(text, &aboutDialog);
	label->setIndent(10);
	label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	vl->addWidget(label);

	label = new QLabel(&aboutDialog);
	label->setIndent(10);
	label->setText(description);
	label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	label->setWordWrap(true);
	vl->addWidget(label);

	QPushButton* copyCommitSHA1Button = new QPushButton("Copy commit SHA1");
	connect(copyCommitSHA1Button, &QPushButton::clicked, [](){
#ifdef CI_PIPELINE_IID
		qApp->clipboard()->setText(CI_COMMIT_SHA);
#endif
	});

	QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(copyCommitSHA1Button, QDialogButtonBox::ActionRole);
	buttonBox->addButton(QDialogButtonBox::Ok);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(hl);
	mainLayout->addWidget(buttonBox);
	aboutDialog.setLayout(mainLayout);

	connect(buttonBox, &QDialogButtonBox::accepted, &aboutDialog, &QDialog::accept);

	aboutDialog.exec();

	return;
}
