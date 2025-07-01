#pragma once

#include "../RvModelSimShared/SimModelPackets.h"
#include <QWidget>

class QTableWidget;

class RWMultiToolBox : public QWidget
{
	Q_OBJECT
public:
	explicit RWMultiToolBox(QWidget* parent = nullptr);

	void setValueType(SignalType type);
	SignalType valueType() const;

	void updateSignalFromCSV(const QString& filename);


signals:
	void requestRead(const QString& signalId);
	void requestWrite(const QString& signalId, const QString& value);

private:
	int filledRowCount() const;
	void loadSignalFromCSV(const QString& filename);
	void setDefaultSignalDefinition();
	void saveSignalToCSV(const QString& filename) const;

	void onReadButtonClicked();
	void onWriteButtonClicked();
	void onAutoFillButtonClicked();

public:
	static inline const QString m_signalsFileName = "signals.csv";

private:
	QTableWidget* m_tableWidget = nullptr;
	SignalType m_valueType = SignalType::AnalogFloat;

	struct SignalDef
	{
		SignalType type;
		QString pattern;
		int start;
		int end;
	};

	std::vector<SignalDef> m_signalDefs;
};