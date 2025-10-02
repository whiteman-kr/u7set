#include <UiLib/DialogAbout.h>
#include "version.h"

namespace UiLib
{
	void DialogAbout::show(QWidget* parent,
						   QString description,
						   QString imagePath,
						   QString organization,
						   QString person,
						   QDate licenseEndDate,
						   QUuid licenseId,
						   QString workplaceId)
	{
		QDialog aboutDialog{parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint};

		auto hl = new QHBoxLayout;
		hl->setSpacing(10);

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

		QDate date = QDateTime::fromSecsSinceEpoch(U7SET_BUILD_DATE_SECONDS, Qt::LocalTime).date();
		text += tr("<br>Build Date: %1").arg(DateTimeToString::date(date));

		auto label = new QLabel(text, &aboutDialog);
		label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		vl->addWidget(label);

		label = new QLabel(&aboutDialog);
		label->setText(description);
		label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		label->setWordWrap(true);
		vl->addWidget(label);

		if (workplaceId.isEmpty() == false)
		{
			QString licenseInfo = tr("License organization: %1").arg(organization);
			licenseInfo += tr("<br>Issued for: %1").arg(person);
			licenseInfo += tr("<br>License end date: %1").arg(DateTimeToString::date(licenseEndDate));
			licenseInfo += tr("<br>LicenseID: %1").arg(licenseId.toString());

			auto labelLicenseInfo = new QLabel(&aboutDialog);
			labelLicenseInfo->setText(licenseInfo);
			labelLicenseInfo->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
			labelLicenseInfo->setWordWrap(true);
			vl->addWidget(labelLicenseInfo);
		}

		auto copyCommitSHA1Button = new QPushButton(tr("Copy commit SHA1"));
		connect(copyCommitSHA1Button,
				&QPushButton::clicked,
				[](bool)
				{
					qApp->clipboard()->setText(U7SET_COMMIT_HASH);
				});

		QPushButton* copyWorkplaceIdButton = nullptr;

		if (workplaceId.isEmpty() == false)
		{
			vl->addWidget(new QLabel{"WorkplaceID: "});

			auto labelWorkplaceId = new QTextEdit(&aboutDialog);
			labelWorkplaceId->setText(workplaceId);
			labelWorkplaceId->setReadOnly(true);
			labelWorkplaceId->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
			vl->addWidget(labelWorkplaceId);

			copyWorkplaceIdButton = new QPushButton(tr("Copy WorkplaceID"));
			connect(copyWorkplaceIdButton,
					&QPushButton::clicked,
					[&workplaceId](bool)
					{
						qApp->clipboard()->setText(workplaceId);
					});
		}

		auto buttonBox = new QDialogButtonBox(Qt::Horizontal);
		buttonBox->addButton(QDialogButtonBox::Ok);
		buttonBox->addButton(copyCommitSHA1Button, QDialogButtonBox::ActionRole);

		if (copyWorkplaceIdButton != nullptr)
		{
			buttonBox->addButton(copyWorkplaceIdButton, QDialogButtonBox::ActionRole);
		}

		auto mainLayout = new QVBoxLayout;
		mainLayout->addLayout(hl);
		mainLayout->addWidget(buttonBox);
		aboutDialog.setLayout(mainLayout);

		connect(buttonBox, &QDialogButtonBox::accepted, &aboutDialog, &QDialog::accept);

		aboutDialog.exec();

		return;
	}
} // namespace UiLib
