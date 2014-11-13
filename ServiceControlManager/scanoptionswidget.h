#ifndef SCANOPTIONSWIDGET_H
#define SCANOPTIONSWIDGET_H

#include <QDialog>
#include <QSet>

class QLineEdit;

class ScanOptionsWidget : public QDialog
{
    Q_OBJECT
public:
    explicit ScanOptionsWidget(QWidget *parent = 0);

    QString getSelectedAddress();
	bool broadcast(quint32 ip) { return m_broadcastSet.contains(ip); }

signals:

public slots:

private:
    QLineEdit* m_addressEdit;
	QSet<quint32> m_broadcastSet;
};

#endif // SCANOPTIONSWIDGET_H
