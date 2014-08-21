#pragma once

class QString;
class QWidget;

class PasswordService
{
public:
	PasswordService();

public:
	static bool checkPassword(const QString& password, const QString& passwordConfirmation, bool showMessageBox, QWidget* parent);
};

