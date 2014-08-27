#ifndef SCANOPTIONSWIDGET_H
#define SCANOPTIONSWIDGET_H

#include <QDialog>

class QLineEdit;

class ScanOptionsWidget : public QDialog
{
    Q_OBJECT
public:
    explicit ScanOptionsWidget(QWidget *parent = 0);

    QString getSelectedAddress();

signals:

public slots:

private:
    QLineEdit* addressEdit;
};

#endif // SCANOPTIONSWIDGET_H
