#include "DialogAbout.h"
#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

#if __has_include("../../gitlabci_version.h")
#	include "../../gitlabci_version.h"
#endif

void DialogAbout::show(QWidget* parent, const QString& description, const QString& imagePath)
{
	QDialog aboutDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	QHBoxLayout* hl = new QHBoxLayout;

	QLabel* logo = new QLabel(&aboutDialog);
	logo->setPixmap(QPixmap(imagePath));

	hl->addWidget(logo);

	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);

	QString text = "<h3>" + qApp->applicationName() +" v" + qApp->applicationVersion() + "</h3>";

#ifndef Q_DEBUG
	text += "Build: Release";
#else
	text += "Build: Debug";
#endif

#ifdef GITLAB_CI_BUILD
	text += "<br>Commit SHA: "	CI_COMMIT_SHA;
	text += "<br>Branch: "		CI_BUILD_REF_SLUG;
	text += "<br>Build Date: "	BUILD_DATE;
	text += "<br>Build Host: "	COMPUTERNAME;
#else
	text += "<br>Commit SHA1: No data";
	text += "<br>Branch: No data";
	text += "<br>Date: No data";
	text += "<br>Host: No data";
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
#ifdef CI_PIPELINE_ID
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
