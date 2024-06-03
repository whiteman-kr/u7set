#include "./include/AppSignalLists/SignalListChecker.h"
#include "SignalListCheckerPrivate.h"
#include "../UtilsLib/LogFile.h"

namespace AppSignalLists
{
	//
	// DialogCheckAppSignalLists
	//
	DialogCheckAppSignalLists::DialogCheckAppSignalLists(std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters,
														 QWidget* parent) :
		QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout();

		QTextEdit* edit = new QTextEdit();

		QString text = tr("<font size=\"4\">Errors have been occured while loading signal lists:<br><br>");

		for (const std::pair<QString, QString>& p : notFoundSignalsAndFilters)
		{
			QString msg =
				tr("Signal with id <font color=\"red\">'%1'</font> was not found in the list '%2'.").arg(p.first).arg(p.second);

			text += msg + "<br>";
		}

		text += tr("<br>Do you wish to remove these signals from lists?</font>");

		edit->setText(text);

		edit->setReadOnly(true);


		QPushButton* yesButton = new QPushButton(tr("Yes"));
		yesButton->setAutoDefault(false);

		QPushButton* noButton = new QPushButton(tr("No"));
		noButton->setDefault(true);

		QDialogButtonBox* buttonBox = new QDialogButtonBox();

		buttonBox->addButton(yesButton, QDialogButtonBox::YesRole);
		buttonBox->addButton(noButton, QDialogButtonBox::NoRole);

		connect(yesButton, &QPushButton::clicked, this, &QDialog::accept);
		connect(noButton, &QPushButton::clicked, this, &QDialog::reject);

		buttonBox->setFocus();

		mainLayout->addWidget(edit);
		mainLayout->addWidget(buttonBox);

		setLayout(mainLayout);

		resize(800, 400);
	}
} // namespace
