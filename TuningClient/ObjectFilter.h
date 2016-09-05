#ifndef OBJECTFILTER_H
#define OBJECTFILTER_H

#include "Stable.h"

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
	ObjectFilter();

	bool load(QXmlStreamReader& reader,
				std::map<QString, std::shared_ptr<ObjectFilter>>& filtersMap,
				std::vector<std::shared_ptr<ObjectFilter>>& filtersVector);
	bool save(QXmlStreamWriter& writer);



public:
	// Properties
	//
	QString strID() const;
	void setStrID(const QString& value);

	QString parentStrID() const;
	void setParentStrID(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	bool user() const;
	void setUser(bool value);

	// Filters
	//
	QString customAppSignalIDMask() const;
	void setCustomAppSignalIDMask(const QString& value);

	QString equipmentIDMask() const;
	void setEquipmentIDMask(const QString& value);

	QString appSignalIDMask() const;
	void setAppSignalIDMask(const QString& value);

	FilterType filterType() const;
	void setFilterType(FilterType value);

	SignalType signalType() const;
	void setSignalType(SignalType value);

public:
	bool isTree() const;
	bool isTab() const;
	bool isButton() const;
	bool isChild() const;

private:

	QString m_strID;
	QString m_parentStrID;
	QString m_caption;

	bool m_user = false;

	// Filters
	//

	QString m_customAppSignalIDMask;
	QString m_equipmentIDMask;
	QString m_appSignalIDMask;

	FilterType m_filterType = FilterType::Tree;
	SignalType m_signalType = SignalType::All;
};



class ObjectFilterStorage
{
public:
	ObjectFilterStorage();

	bool load(const QString& fileName);
	bool save(const QString& fileName, bool user);

	QString errorCode();

	int filtersCount() const;
	ObjectFilter* filter(int index);
	ObjectFilter* filter(const QString& filterId);

private:
	std::map<QString, std::shared_ptr<ObjectFilter>> m_filtersMap;
	std::vector<std::shared_ptr<ObjectFilter>> m_filtersVector;

	QString m_errorCode;
};

extern ObjectFilterStorage theFilters;

#endif // OBJECTFILTER_H
