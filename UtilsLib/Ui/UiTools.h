#pragma once

#include <QDialog>

class UiTools
{
public:

	static QIcon drawColorBox(QColor color);
	
	static void adjustDialogPlacement(QDialog* dialog);

	static void openHelp(const QString& file, QWidget* parent);

    static int m_lastPaintEventCode;    // This is for debugging QRasterPaintEngine::brushOriginChanged crash
};


