#pragma once

class QWidget;
class QString;

namespace UiLib
{
	class DialogAbout : public QObject
	{
		Q_OBJECT

	public:
		static void show(QWidget* parent,
						 QString description,
						 QString imagePath,
						 QString organization = "Not applicable",
						 QString person = "Not applicable",
						 QDate licenseEndDate = QDate{2299, 1, 1},
						 QUuid licenseId = QUuid{},
						 QString workplaceId = "Not applicable");
	};
}