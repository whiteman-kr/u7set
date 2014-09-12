#ifndef SIGNAL_H
#define SIGNAL_H

#include <QString>

enum SignalType
{
	analog = 0,
	discrete = 1
};

enum SignalInOutType
{
	input = 0,
	output = 1,
	internal = 2
};

class Signal
{
public:
	Signal();

	int ID() const {return m_ID;}

	QString strId() const {return m_strId;}
	void setStrId(QString& newStrId) {m_strId = newStrId;}

	QString extStrId() const {return m_extStrId;}
	void setExtStrId(QString& newExtStrId) {m_extStrId = newExtStrId;}

	QString name() const {return m_name;}
	void setName(QString& newName) {m_name = newName;}

	int channel() const {return m_channel;}

	SignalType type() const {return m_type;}

	int dataFormat() const {return m_dataFormat;}
	void setDataFormat(int newDataFormat) {m_dataFormat = newDataFormat;}

	// Data
	//
private:
	int m_ID;
	QString m_strId;
	QString m_extStrId;
	QString m_name;
	int m_channel;
	SignalType m_type;
	int m_dataFormat;
	int m_inputUnit;
	int m_outputUnit;

	//Should be used only by friends
	void setID(int signalID) {m_ID = signalID;}
	void setChannel(int newChannel) {m_channel = newChannel;}
	void setType(SignalType type) {m_type = type;}
};


class SignalSet
{
private:
	QHash<int, Signal*> m_signalSet;

public:
	SignalSet();
	virtual ~SignalSet();

	void addSignal(const Signal& signal);
	Signal* getSignal(int signalID);

	void removeAll();
};


#endif // SIGNAL_H
