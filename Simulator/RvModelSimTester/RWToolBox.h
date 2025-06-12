#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class QCompleter;
class QStringListModel;

class RWToolBox : public QWidget
{
	Q_OBJECT
public:
	explicit RWToolBox(QWidget* parent = nullptr);

signals:
	void requestRead(const QString& sygnalID);
	void requestWrite(const QString& sygnalID, const QString& value);

private slots:
	void onReadClicked();
	void onWriteClicked();

private:
	QLineEdit* signalIdEdit;
	QLineEdit* writeValueEdit;
	QPushButton* readButton;
	QPushButton* writeButton;

	QCompleter* signalIdCompleter;
	QStringListModel* signalIdModel;
};
