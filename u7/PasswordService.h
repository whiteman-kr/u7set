#ifndef PASSWORDSERVICE_H
#define PASSWORDSERVICE_H

class QString;
class QWidget;

class PasswordService
{
public:
	PasswordService();

public:
	static bool checkPassword(const QString& password, const QString& passwordConfirmation, bool showMessageBox, QWidget* parent);
};

#endif // PASSWORDSERVICE_H
