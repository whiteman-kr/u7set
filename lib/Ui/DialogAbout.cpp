#include "DialogAbout.h"
#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QClipboard>

#if __has_include("../../gitlabci_version.h")
#	include "../../gitlabci_version.h"
#endif

void DialogAbout::show(QWidget* parent, const QString& description, const QString& imagePath)
{
	QDialog aboutDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	QHBoxLayout* hl = new QHBoxLayout;

	QLabel* logo = new QLabel(&aboutDialog);
	logo->setPixmap(QPixmap(imagePath));
	logo->setScaledContents(true);

	// Set logo size that it will be not too big.
	//
	QSize logoSize = logo->sizeHint();
	if (logoSize.width() > 200)
	{
		logoSize *= 200.0 / logoSize.width();
		logo->setFixedSize(logoSize);
	}

	hl->addWidget(logo);

	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);

	QString text = "<h3>" + qApp->applicationName() +" v" + qApp->applicationVersion() + "</h3>";

#ifndef QT_DEBUG
	text += tr("Build: Release");
#else
	text += tr("Build: Debug");
#endif

#ifdef GITLAB_CI_BUILD
	text += tr("<br>Commit SHA: %1").arg(CI_COMMIT_SHA);
	text += tr("<br>Branch: %1").arg(CI_COMMIT_REF_SLUG);
	text += tr("<br>Build Date: %1").arg(BUILD_DATE);
#else
	text += tr("<br>Commit SHA1: No data");
	text += tr("<br>Branch: No data");
	text += tr("<br>Date: No data");
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

	QPushButton* copyCommitSHA1Button = new QPushButton(tr("Copy commit SHA1"));
	connect(copyCommitSHA1Button, &QPushButton::clicked, [](bool){
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
