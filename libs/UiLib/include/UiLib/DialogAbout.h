#pragma once

class QWidget;
class QString;

namespace UiLib
{
	class DialogAbout : public QObject
	{
		Q_OBJECT

	public:
		static void show(QWidget* parent, const QString& description, const QString& imagePath);
	};
}
