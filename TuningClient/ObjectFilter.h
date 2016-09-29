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

struct ObjectFilterValue
{
	QString appSignalId;
	QString caption;
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
	ObjectFilter(FilterType filterType);
	ObjectFilter(const ObjectFilter& That);
	~ObjectFilter();

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

	std::vector <ObjectFilterValue> signalValues() const;
	void setValues(const std::vector <ObjectFilterValue>& values);

	FilterType filterType() const;
	void setFilterType(FilterType value);

	SignalType signalType() const;
	void setSignalType(SignalType value);

	ObjectFilter* parentFilter() const;

	bool allowAll() const;
	void setAllowAll(bool value);

	bool folder() const;
	void setFolder(bool value);

public:
	bool isTree() const;
	bool isTab() const;
	bool isButton() const;

	void addChild(std::shared_ptr<ObjectFilter> child);

	void removeChild(std::shared_ptr<ObjectFilter> child);

	int childFiltersCount();
	std::shared_ptr<ObjectFilter> childFilter(int index);

private:

	QString m_strID;
	QString m_caption;

	bool m_allowAll = false;
	bool m_folder = false;

	// Filters
	//
	QStringList m_customAppSignalIDMasks;
	QStringList m_equipmentIDMasks;
	QStringList m_appSignalIDMasks;

	std::vector <ObjectFilterValue> m_signalValues;

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

	int topFilterCount() const;
	std::shared_ptr<ObjectFilter> topFilter(int index) const;

	bool addTopFilter(const std::shared_ptr<ObjectFilter>& filter);

	bool removeFilter(const std::shared_ptr<ObjectFilter>& filter);

	int schemaDetailsCount();
	SchemaDetails schemaDetails(int index);

	void createAutomaticFilters();

private:

	std::vector<std::shared_ptr<ObjectFilter>> m_topFilters;

	std::vector<SchemaDetails> m_schemasDetails;
};

Q_DECLARE_METATYPE(std::shared_ptr<ObjectFilter>)
Q_DECLARE_METATYPE(ObjectFilterValue)


extern ObjectFilterStorage theFilters;
extern ObjectFilterStorage theUserFilters;

#endif // OBJECTFILTER_H
