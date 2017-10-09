#pragma once

class QString;
class QWidget;

class PasswordService
{
public:
	PasswordService();

public:
	static bool checkPassword(QString password, QString passwordConfirmation, QString username, bool showMessageBox, QWidget* parent);
};

