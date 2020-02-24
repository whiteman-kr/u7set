#pragma once

class UiTools
{
public:

	static QIcon drawColorBox(QColor color);
	
	static void adjustDialogPlacement(QDialog* dialog);

	static void openHelp(const QString& file, QWidget* parent);
};
