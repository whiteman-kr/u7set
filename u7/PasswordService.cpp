#include "PasswordService.h"
#include <QString>
#include <QWidget>
#include <QMessageBox>

PasswordService::PasswordService()
{
}

bool PasswordService::checkPassword(const QString& password, const QString& passwordConfirmation, bool showMessageBox, QWidget* parent)
{
	if (password.size() < 6)
	{
		if (showMessageBox == true)
		{
			QMessageBox mb(parent);

			mb.setText(QObject::tr("Password is too simple."));
			mb.setInformativeText(QObject::tr("Password must contain at least 6 characters digits or symbols."));
			mb.setIcon(QMessageBox::Warning);

			mb.exec();
		}

		return false;
	}

	if (password != passwordConfirmation)
	{
		if (showMessageBox == true)
		{
			QMessageBox mb(parent);

			mb.setText(QObject::tr("The password was not correctly confirmed"));
			mb.setInformativeText(QObject::tr("Be sure that the confirmation password exactly matches the entered password."));
			mb.setIcon(QMessageBox::Warning);

			mb.exec();
		}

		return false;
	}

static const QString standardPassword[] = {
		"123456", "1234567", "12345678", "123456789", "1234567890", "1234567890-", "12345567890-=", "1234567890-=\"", "123123", "windows", "kirovograd",
		"0987654", "09876543", "098765432", "0987654321", "password", "`12345'", "`123456", "`1234567", "`12345678", "`123456789",
		"qwerty", "qwertyu", "qwertyui", "qwertyuio", "qwertyuiop", "qwertyuiop[", "qwertyuiop[]", "admin123", "123admin",
		"asdfgh", "asdfghj", "asdfghjk", "asdfghjkl;", "asdfghjkl;'", "administrator", "Administartor"
		"zxcvbn", "zxcvbnm", "zxcvbnm,", "zxcvbnm,.", "zxcvbnm,./",
		"dragon", "baseball", "football", "letmein", "monkey", "696969", "superman",
		"abc123", "qwe123", "zxc123", "123qwe","123asd", "123zxc"
		};

	for (unsigned int i = 0; i < sizeof(standardPassword) / sizeof(standardPassword[0]); i++)
	{
		if (password == standardPassword[i])
		{
			if (showMessageBox == true)
			{
				QMessageBox mb(parent);

				mb.setText(QObject::tr("Password is too simple."));
				mb.setInformativeText(QObject::tr("Please choose another password."));
				mb.setIcon(QMessageBox::Warning);

				mb.exec();
			}

			return false;
		}
	}

	// Check if the password symbols are same
	//
	bool sameChars = true;
	QChar firstChar = password[0];

	for (int i = 0; i < password.size(); i++)
	{
		if (password[i] != firstChar)
		{
			sameChars = false;
			break;
		}
	}

	if (sameChars == true)
	{
		QMessageBox mb(parent);

		mb.setText(QObject::tr("Password is too simple."));
		mb.setInformativeText(QObject::tr("Please choose another password."));
		mb.setIcon(QMessageBox::Warning);

		mb.exec();
		return false;
	}

	// Password is correct
	//

	return true;
}
