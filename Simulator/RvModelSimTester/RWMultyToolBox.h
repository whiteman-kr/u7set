#pragma once

#include <QWidget>
#include "../RvModelSimShared/SimModelPackets.h"

class QTableWidget;

class RWMultyToolBox : public QWidget
{
	Q_OBJECT
public:
	explicit RWMultyToolBox(QWidget* parent = nullptr);
	void setValueType(SignalType type);
	SignalType valueType() const { return m_valueType; }

signals:
	void requestRead(const QString& signalId);
	void requestWrite(const QString& signalId, const QString& value);

private:
	QTableWidget* tableWidget;
	SignalType m_valueType = SignalType::AnalogFloat;
};