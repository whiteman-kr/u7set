#ifndef SWITCHPRESETSPAGEOPTIONS_H
#define SWITCHPRESETSPAGEOPTIONS_H

#include <QDialog>

namespace Ui {
	class SwitchPresetsPageOptions;
}

class SwitchFiltersPageOptions : public QDialog
{
	Q_OBJECT

public:
	explicit SwitchFiltersPageOptions(QWidget *parent, int defaultColCount, int defaultRowCount, int defaultWidth, int defaultHeight);
	~SwitchFiltersPageOptions();

	int buttonsColCount() const;
	int buttonsRowCount() const;
	int buttonsWidth() const;
	int buttonsHeight() const;


private:
	Ui::SwitchPresetsPageOptions *ui;

};

#endif // SWITCHPRESETSPAGEOPTIONS_H
