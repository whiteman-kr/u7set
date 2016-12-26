#pragma once

#include <QDialog>
#include <QCompleter>

namespace Ui {
	class LoginDialog;
}

class LoginDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit LoginDialog(const QStringList& loginCompleterList, QWidget* parent);
	~LoginDialog();

	QString username() const;
	QString password() const;

protected:
	virtual void showEvent(QShowEvent* event) override;
	
private slots:
	void on_buttonBox_accepted();

private:
	Ui::LoginDialog *ui;
	QCompleter* m_completer;

	QString m_username;
	QString m_password;
};

