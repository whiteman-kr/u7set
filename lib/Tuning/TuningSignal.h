#ifndef TUNINGSIGNAL_H
#define TUNINGSIGNAL_H

#include "Stable.h"
#include "../lib/Hash.h"

union TuningSignalStateFlags
{
	struct
	{
		quint32	m_valid : 1;
		quint32	m_overflow : 1;
		quint32	m_underflow : 1;
		quint32	m_userModified : 1;
		quint32	m_writing : 1;
		quint32	m_needRedraw : 1;
	};

	quint32 all;

};

struct TuningSignalState
{

	friend class TuningSignal;

private:

	TuningSignalStateFlags m_flags;

	float m_value = 0;
	float m_editValue = 0;

	int m_writingCounter = 0;

public:

	//

	bool valid() const;
	void setValid(bool value);

	bool underflow() const;

	bool overflow() const;

	bool needRedraw();

	bool userModified() const;
	void clearUserModified();

	bool writing() const;
	void setWriting(bool value);
};


class TuningSignal
{
public:

	TuningSignal();

	// Properties

	QString customAppSignalID() const;
	void setCustomAppSignalID(const QString& value);

	QString equipmentID() const;
	void setEquipmentID(const QString& value);

	QString appSignalID() const;
	void setAppSignalID(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	QString units() const;
	void setUnits(const QString& value);

	bool analog() const;
	void setAnalog(bool value);

    float defaultValue() const;
    void setDefaultValue(float value);

    float lowLimit() const;
    void setLowLimit(float value);

    float highLimit() const;
    void setHighLimit(float value);

    float readLowLimit() const;
    void setReadLowLimit(float value);

    float readHighLimit() const;
    void setReadHighLimit(float value);

    int decimalPlaces() const;
	void setDecimalPlaces(int value);

    Hash appSignalHash() const;

	// Change state methods

	float value() const;
	void setValue(float value);

	void onReceiveValue(float value, bool &writingFailed);

	float editValue() const;
	void onEditValue(float value);

	void onSendValue(float value);

	bool limitsUnbalance() const;

public:

	TuningSignalState state;

private:

	// Params

	QString m_customAppSignalID;
	QString m_equipmentID;
	QString m_appSignalID;
	QString m_caption;
	QString m_units;

	bool m_analog = false;

    float m_defaultValue = 0;

    float m_lowLimit = 0;
    float m_highLimit = 0;

    float m_readLowLimit = 0;
    float m_readHighLimit = 0;

    int m_decimalPlaces = 0;

	Hash m_appSignalHash = 0;
};


class TuningSignalStorage
{

public:

	TuningSignalStorage();

    bool loadSignals(const QByteArray& data, QString *errorCode);

    void invalidateSignals();

    // Object accessing


    int objectCount() const;

    bool objectExists(Hash hash) const;

	TuningSignal *objectPtr(int index) const;

	TuningSignal *objectPtrByHash(Hash hash) const;

private:

    // Objects
    //

    std::map<Hash, int> m_objectsMap;

	std::vector<std::shared_ptr<TuningSignal>> m_objects;

};

#endif // TUNINGSIGNAL_H
