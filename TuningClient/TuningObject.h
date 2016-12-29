#ifndef TUNINGOBJECT_H
#define TUNINGOBJECT_H

#include "Stable.h"
#include "../lib/Hash.h"

class TuningObject
{
public:

	TuningObject();

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

    float value() const;
    void setValue(float value);

    void onReceiveValue(float value, bool &writingFailed);

    float editValue() const;
    void onEditValue(float value);

    void onSendValue(float value);

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

	bool valid() const;
	void setValid(bool value);

    bool underflow() const;

	bool overflow() const;

    Hash appSignalHash() const;

    bool redraw();

    bool userModified() const;
    void clearUserModified();

    bool writing() const;
    void setWriting(bool value);

    bool limitsUnbalance() const;

private:

	QString m_customAppSignalID;
	QString m_equipmentID;
	QString m_appSignalID;
	QString m_caption;
	QString m_units;

	bool m_analog = false;

    float m_defaultValue = 0;
    float m_value = 0;
    float m_editValue = 0;

    float m_lowLimit = 0;
    float m_highLimit = 0;

    float m_readLowLimit = 0;
    float m_readHighLimit = 0;

    int m_decimalPlaces = 0;

    bool m_valid = false;
    bool m_underflow = false;
    bool m_overflow = false;

	Hash m_appSignalHash = 0;

    bool m_redraw = false;
    bool m_userModified = false;

    bool m_writing = false;
    int m_writingCounter = 0;

};


class TuningObjectStorage
{

public:

    TuningObjectStorage();

    bool loadSignals(const QByteArray& data, QString *errorCode);

    void invalidateSignals();

    // Object accessing


    int objectCount() const;

    bool objectExists(Hash hash) const;

    TuningObject *objectPtr(int index) const;

    TuningObject *objectPtrByHash(Hash hash) const;

private:

    // Objects
    //

    std::map<Hash, int> m_objectsMap;

    std::vector<std::shared_ptr<TuningObject>> m_objects;

};

#endif // TUNINGOBJECT_H
