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
	QStringList m_appSignals;

};

struct ObjectValue
{
	QString appSignalId;
	QString caption;
	bool useValue = false;
	bool analog = false;
	int decimalPlaces = 0;
	double value = 0;
};

class ObjectFilter : public PropertyObject
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
	ObjectFilter();
	ObjectFilter(const ObjectFilter& That);
	ObjectFilter(FilterType filterType);
	~ObjectFilter();

	ObjectFilter& operator= (const ObjectFilter& That);

	bool load(QXmlStreamReader& reader);
	bool save(QXmlStreamWriter& writer);

	bool match(const TuningObject &object);



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

	std::vector <ObjectValue> signalValues() const;
	void setValues(const std::vector <ObjectValue>& values);

	void setValue(const QString& appSignalId, double value);

	bool valueExists(const QString& appSignalId);
	void addValue(const ObjectValue& value);

	void removeValue(const QString& appSignalId);

	FilterType filterType() const;
	void setFilterType(FilterType value);

	SignalType signalType() const;
	void setSignalType(SignalType value);

	ObjectFilter* parentFilter() const;

	bool allowAll() const;
	void setAllowAll(bool value);

	bool isEmpty() const;

public:
	bool isRoot() const;
	bool isTree() const;
	bool isTab() const;
	bool isButton() const;

	void addTopChild(std::shared_ptr<ObjectFilter> child);
	void addChild(std::shared_ptr<ObjectFilter> child);

	void removeChild(std::shared_ptr<ObjectFilter> child);

	void removeAllChildren();

	int childFiltersCount() const;
	std::shared_ptr<ObjectFilter> childFilter(int index) const;

private:
	void copy(const ObjectFilter& That);

private:

	QString m_strID;
	QString m_caption;

	bool m_allowAll = false;

	// Filters
	//
	QStringList m_customAppSignalIDMasks;
	QStringList m_equipmentIDMasks;
	QStringList m_appSignalIDMasks;

	std::vector <ObjectValue> m_signalValues;

	FilterType m_filterType = FilterType::Tree;
	SignalType m_signalType = SignalType::All;

	std::vector<std::shared_ptr<ObjectFilter>> m_childFilters;

	ObjectFilter* m_parentFilter = nullptr;

};



class ObjectFilterStorage
{
public:
	ObjectFilterStorage();
	ObjectFilterStorage(const ObjectFilterStorage& That);

	bool load(const QByteArray& data, QString *errorCode);
	bool loadSchemasDetails(const QByteArray& data, QString *errorCode);

	bool load(const QString& fileName, QString *errorCode);
	bool save(const QString& fileName);

	int schemaDetailsCount();
	SchemaDetails schemaDetails(int index);

	void createAutomaticFilters();

	std::shared_ptr<ObjectFilter> m_root = nullptr;

private:

	std::vector<SchemaDetails> m_schemasDetails;
};

Q_DECLARE_METATYPE(std::shared_ptr<ObjectFilter>)
Q_DECLARE_METATYPE(ObjectValue)


extern ObjectFilterStorage theFilters;
extern ObjectFilterStorage theUserFilters;

#endif // OBJECTFILTER_H
