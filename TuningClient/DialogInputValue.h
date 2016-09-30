#ifndef DIALOGINPUTVALUE_H
#define DIALOGINPUTVALUE_H

#include <QDialog>

namespace Ui {
class DialogInputValue;
}

class DialogInputValue : public QDialog
{
	Q_OBJECT

public:
	explicit DialogInputValue(bool analog, double value, bool sameValue, int decimalPlaces, QWidget *parent = 0);
	~DialogInputValue();

private:
	Ui::DialogInputValue *ui;

	double m_value = 0;
	bool m_analog = true;

	virtual void accept();

public:
	double value() { return m_value; }

private slots:
	void on_m_checkBox_clicked(bool checked);
};

#endif // DIALOGINPUTVALUE_H
