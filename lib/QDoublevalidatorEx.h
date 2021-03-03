#pragma once
#include <QDoubleValidator>

// Validate for double values which aacepts both . and ,
//
class QDoubleValidatorEx : public QDoubleValidator
{
	Q_OBJECT

public:
	explicit QDoubleValidatorEx(QObject* parent = nullptr) :
		QDoubleValidator(parent)
	{
	}

	QDoubleValidatorEx(double bottom, double top, int decimals, QObject* parent = nullptr) :
		QDoubleValidator(bottom, top, decimals, parent)
	{
	}

	virtual QValidator::State validate(QString& str, int& npos) const override
	{
		str.replace('.', QLocale{}.decimalPoint());
		str.replace('E', QLocale{}.exponential(), Qt::CaseInsensitive);

		return QDoubleValidator::validate(str, npos);
	}
};
