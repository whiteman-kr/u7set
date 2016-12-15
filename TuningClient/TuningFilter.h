#ifndef OBJECTFILTER_H
#define OBJECTFILTER_H

#include "Stable.h"
#include "TuningObject.h"
#include "../lib/PropertyObject.h"
#include "../lib/Hash.h"

struct SchemaDetails
{
	QString m_caption;
	QString m_strId;
	QStringList m_appSignalIDs;

};

class TuningFilterValue
{
public:
	TuningFilterValue();

	QString appSignalId() const;
	void setAppSignalId(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	bool useValue() const;
	void setUseValue(bool value);

	bool analog()  const;
	void setAnalog(bool value);

	int decimalPlaces() const;
	void setDecimalPlaces(int value);

    float value() const;
    void setValue(float value);

    float lowLimit() const;
    void setLowLimit(float value);

    float highLimit() const;
    void setHighLimit(float value);

    Hash hash() const;
private:

	QString m_appSignalId;
	QString m_caption;
	bool m_useValue = false;
	bool m_analog = false;
	int m_decimalPlaces = 0;
    float m_value = 0;
    float m_lowLimit = 0;
    float m_highLimit = 0;

	Hash m_hash = 0;
};

class TuningFilter : public PropertyObject
{
	Q_OBJECT

public:
	//Enums
	//
	enum class FilterType
	{
		Root,
		Tree,
		Tab,
		Button
	};
	Q_ENUM(FilterType)

	enum class SignalType
	{
		All,
		Analog,
		Discrete,
	};
	Q_ENUM(SignalType)

public:
	TuningFilter();
	TuningFilter(const TuningFilter& That);
	TuningFilter(FilterType filterType);
	~TuningFilter();

	TuningFilter& operator= (const TuningFilter& That);

	bool load(QXmlStreamReader& reader);
	bool save(QXmlStreamWriter& writer) const;

	bool match(const TuningObject &object) const;

public:
	// Properties
	//
	QString strID() const;
	void setStrID(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	// Filters
	//
	QString customAppSignalIDMask() const;
	void setCustomAppSignalIDMask(const QString& value);

	QString equipmentIDMask() const;
	void setEquipmentIDMask(const QString& value);

	QString appSignalIDMask() const;
	void setAppSignalIDMask(const QString& value);

	std::vector <TuningFilterValue> signalValues() const;
	void setValues(const std::vector <TuningFilterValue>& values);

    void setValue(Hash hash, float value);

	bool valueExists(Hash hash) const;
	void addValue(const TuningFilterValue& value);

	void removeValue(Hash hash);

	FilterType filterType() const;
	void setFilterType(FilterType value);

	SignalType signalType() const;
	void setSignalType(SignalType value);

	TuningFilter* parentFilter() const;

	bool isEmpty() const;

public:
	bool isRoot() const;
	bool isTree() const;
	bool isTab() const;
	bool isButton() const;

	void addTopChild(const std::shared_ptr<TuningFilter>& child);
	void addChild(const std::shared_ptr<TuningFilter>& child);

	void removeChild(const std::shared_ptr<TuningFilter>& child);

	void removeAllChildren();

	int childFiltersCount() const;
	std::shared_ptr<TuningFilter> childFilter(int index) const;

private:
	void copy(const TuningFilter& That);

private:

	QString m_strID;
	QString m_caption;

	// Filters
	//
	QStringList m_customAppSignalIDMasks;
	QStringList m_equipmentIDMasks;
	QStringList m_appSignalIDMasks;

	std::vector<Hash> m_signalValuesVec;
	std::map <Hash, TuningFilterValue> m_signalValuesMap;

	FilterType m_filterType = FilterType::Tree;
	SignalType m_signalType = SignalType::All;

	std::vector<std::shared_ptr<TuningFilter>> m_childFilters;

	TuningFilter* m_parentFilter = nullptr;

};

class TuningFilterStorage
{
public:
	TuningFilterStorage();
	TuningFilterStorage(const TuningFilterStorage& That);

	bool load(const QByteArray& data, QString *errorCode);
	bool loadSchemasDetails(const QByteArray& data, QString *errorCode);

	bool load(const QString& fileName, QString *errorCode);
	bool save(const QString& fileName, QString *errorMsg);

	int schemaDetailsCount();
	SchemaDetails schemaDetails(int index);

	void createAutomaticFilters(bool bySchemas, bool byEquipment, const QStringList &tuningSourcesEquipmentIds);

	std::shared_ptr<TuningFilter> m_root = nullptr;

private:

	std::vector<SchemaDetails> m_schemasDetails;
};

Q_DECLARE_METATYPE(std::shared_ptr<TuningFilter>)
Q_DECLARE_METATYPE(TuningFilterValue)



#endif // OBJECTFILTER_H
