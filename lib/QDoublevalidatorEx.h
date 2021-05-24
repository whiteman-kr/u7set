#pragma once
#include <QDoubleValidator>

// Validate for double values which aacepts both . and ,
//
class QDoubleValidatorEx : public QDoubleValidator
{
	Q_OBJECT

public:
	explicit QDoubleValidatorEx(bool allowReferences, QObject* parent = nullptr) :
		QDoubleValidator(parent),
		m_allowReferences(allowReferences)
	{
	}

	QDoubleValidatorEx(double bottom, double top, int decimals, bool allowReferences, QObject* parent = nullptr) :
		QDoubleValidator(bottom, top, decimals, parent),
		m_allowReferences(allowReferences)
	{
	}

	virtual QValidator::State validate(QString& str, int& npos) const override
	{
		str.replace('.', QLocale{}.decimalPoint());
		str.replace('E', QLocale{}.exponential(), Qt::CaseInsensitive);

		QValidator::State state = QDoubleValidator::validate(str, npos);
		if (state == QValidator::State::Invalid && m_allowReferences == true)
		{
			state = QValidator::State::Acceptable;
		}

		return state;
	}

private:
	bool m_allowReferences = false;		// Allow $(abc.efg)
};

// Validate for double values which aacepts both . and ,
//
class QIntValidatorEx : public QIntValidator
{
	Q_OBJECT

public:
	explicit QIntValidatorEx(bool allowReferences, QObject* parent = nullptr) :
		QIntValidator(parent),
		m_allowReferences(allowReferences)
	{
	}

	QIntValidatorEx(int minimum, int maximum, bool allowReferences, QObject* parent = nullptr) :
		QIntValidator(minimum, maximum, parent),
		m_allowReferences(allowReferences)
	{
	}

	virtual QValidator::State validate(QString& str, int& npos) const override
	{
		QValidator::State state = QIntValidator::validate(str, npos);
		if (state == QValidator::State::Invalid && m_allowReferences == true)
		{
			state = QValidator::State::Acceptable;
		}

		return state;
	}

private:
	bool m_allowReferences = false;		// Allow $(abc.efg)
};
