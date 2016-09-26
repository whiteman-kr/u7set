#ifndef OBJECTFILTER_H
#define OBJECTFILTER_H

#include "Stable.h"
#include "TuningObject.h"
#include "../lib/Hash.h"

struct SchemaDetails
{
	QString m_caption;
	QString m_strId;
	QStringList m_appSignals;

};

class ObjectFilter : public QObject
{
	Q_OBJECT

public:
	//Enums
	//
	enum class FilterType
	{
		Tree,
		Tab,
		Button,
		Child

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
	ObjectFilter(FilterType filterType);

	bool load(QXmlStreamReader& reader, std::map<Hash, std::shared_ptr<ObjectFilter>> &filtersMap);
	bool save(QXmlStreamWriter& writer);

	bool match(const TuningObject &object);



public:
	// Properties
	//
	QString strID() const;
	void setStrID(const QString& value);

	Hash hash() const;

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

	QStringList appSignalIds() const;
	void setAppSignalIds(const QStringList& value);

	FilterType filterType() const;
	void setFilterType(FilterType value);

	SignalType signalType() const;
	void setSignalType(SignalType value);

	ObjectFilter* parent() const;
	void setParent(ObjectFilter* value);

	bool allowAll() const;
	void setAllowAll(bool value);

	bool denyAll() const;
	void setDenyAll(bool value);

public:
	bool isTree() const;
	bool isTab() const;
	bool isButton() const;
	bool isChild() const;

	void addChild(std::shared_ptr<ObjectFilter> child);

	int childFiltersCount();
	ObjectFilter* childFilter(int index);

private:

	QString m_strID;
	QString m_caption;

	bool m_allowAll = false;
	bool m_denyAll = false;

	Hash m_hash = 0;

	// Filters
	//

	QStringList m_customAppSignalIDMasks;
	QStringList m_equipmentIDMasks;
	QStringList m_appSignalIDMasks;
	QStringList m_appSignalIds;

	FilterType m_filterType = FilterType::Tree;
	SignalType m_signalType = SignalType::All;

	std::vector<std::shared_ptr<ObjectFilter>> m_childFilters;

	ObjectFilter* m_parent = nullptr;

};



class ObjectFilterStorage
{
public:
	ObjectFilterStorage();

	bool load(const QByteArray& data, QString *errorCode);
	bool loadSchemasDetails(const QByteArray& data, QString *errorCode);

	bool load(const QString& fileName, QString *errorCode);
	bool save(const QString& fileName);

	int topFilterCount();
	ObjectFilter *topFilter(int index);

	ObjectFilter *filter(Hash hash);

	int schemaDetailsCount();
	SchemaDetails schemaDetails(int index);

	void createAutomaticFilters();

private:

	std::map<Hash, std::shared_ptr<ObjectFilter>> m_filtersMap;
	std::vector<Hash> m_topFilters;
	std::vector<SchemaDetails> m_schemasDetails;
};

extern ObjectFilterStorage theFilters;
extern ObjectFilterStorage theUserFilters;

#endif // OBJECTFILTER_H
