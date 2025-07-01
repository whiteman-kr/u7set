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
	QLineEdit* m_signalIdEdit = nullptr;
	QLineEdit* m_writeValueEdit = nullptr;
	QPushButton* m_readButton = nullptr;
	QPushButton* m_writeButton = nullptr;

	QCompleter* m_signalIdCompleter = nullptr;
	QStringListModel* m_signalIdModel = nullptr;
};
