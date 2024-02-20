#include "DialogAbout.h"
#include "version.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>


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

	QString text = "<h3>" + qApp->applicationName() + " v" + qApp->applicationVersion() + "</h3>";

#ifndef QT_DEBUG
	text += tr("Build: %1 Release").arg(U7SET_RELEASE_TYPE);
#else
	text += tr("Build: %1 Debug").arg(U7SET_RELEASE_TYPE);
#endif

	text += tr("<br>PipelineID: %1").arg(U7SET_PIPELINE_ID);

	text += tr("<br>Commit SHA: %1").arg(U7SET_COMMIT_HASH);
	text += tr("<br>Branch: %1").arg(U7SET_BRANCH_NAME);
	text += tr("<br>Build Date: %1").arg(U7SET_BUILD_DATE);

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
	connect(copyCommitSHA1Button, &QPushButton::clicked, [](bool)
			{
				qApp->clipboard()->setText(U7SET_COMMIT_HASH);
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
