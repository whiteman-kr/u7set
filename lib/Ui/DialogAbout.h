#pragma once
#include <QObject>

class DialogAbout : public QObject
{
	Q_OBJECT

public:
	static void show(QWidget* parent, const QString& description, const QString& imagePath);
};
