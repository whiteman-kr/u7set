#ifndef DIALOGALERT_H
#define DIALOGALERT_H


class DialogAlert : public QDialog
{
	Q_OBJECT
public:
	DialogAlert(QWidget* parent);
	virtual ~DialogAlert();

public slots:
	void onAlertArrived(QString text);

private:
	QTextEdit* m_textEdit = nullptr;

	QString m_text;
};

#endif // DIALOGALERT_H
