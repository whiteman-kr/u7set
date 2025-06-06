#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;

class RWToolBox : public QWidget
{
    Q_OBJECT
public:
    explicit RWToolBox(QWidget* parent = nullptr);

signals:
	void readAction(const QString& action);
	void writeAction(const QString& action);
private slots:
    void onReadClicked();
    void onWriteClicked();

private:
    QLineEdit* signalIdEdit;
    QLineEdit* writeValueEdit;
    QPushButton* readButton;
    QPushButton* writeButton;
};
