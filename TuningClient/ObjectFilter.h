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
	ObjectFilter(FilterType filterType);

	bool load(QXmlStreamReader& reader);
	bool save(QXmlStreamWriter& writer);



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

	FilterType filterType() const;
	void setFilterType(FilterType value);

	SignalType signalType() const;
	void setSignalType(SignalType value);

public:
	bool isTree() const;
	bool isTab() const;
	bool isButton() const;
	bool isChild() const;

	std::vector<std::shared_ptr<ObjectFilter>> childFilters;

private:

	QString m_strID;
	QString m_caption;

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

	bool load(const QByteArray& data, QString *errorCode);

	bool load(const QString& fileName, QString *errorCode);
	bool save(const QString& fileName);

	int filterCount();
	const std::shared_ptr<ObjectFilter> filter_const(int index);

private:

	QMutex m_mutex;

	std::vector<std::shared_ptr<ObjectFilter>> m_filters;
};

extern ObjectFilterStorage theFilters;
extern ObjectFilterStorage theUserFilters;

#endif // OBJECTFILTER_H
