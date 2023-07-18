#pragma once

#include <QDialog>

class UiTools
{
public:

	static QIcon drawColorBox(QColor color);
	
	static void adjustDialogPlacement(QDialog* dialog);

	static void openPdf(const QString& file, QWidget* parent);
};


