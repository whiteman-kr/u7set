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
    explicit DialogInputValue(bool analog, float value, bool sameValue, float lowLimit, float highLimit, int decimalPlaces, QWidget *parent = 0);
	~DialogInputValue();

private:
	Ui::DialogInputValue *ui;

    float m_value = 0;
    float m_lowLimit = 0;
    float m_highLimit = 0;
	bool m_analog = true;

	virtual void accept();

public:
    float value() { return m_value; }

private slots:
	void on_m_checkBox_clicked(bool checked);
};

#endif // DIALOGINPUTVALUE_H
