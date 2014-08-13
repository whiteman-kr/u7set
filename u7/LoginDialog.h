#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
	class LoginDialog;
}

class LoginDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit LoginDialog(QWidget* parent);
	~LoginDialog();

	const QString& username() const;
	const QString& password() const;
	
private slots:
	void on_buttonBox_accepted();

private:
	Ui::LoginDialog *ui;

	QString m_username;
	QString m_password;
};

#endif // LOGINDIALOG_H
