#pragma once
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/PropertyObject.h"
#include "../lib/Hash.h"
#include "../VFrame30/Schema.h"

struct TuningCounters
{
	int errorCounter = 0;
	int sorCounter = 0;
	int controlEnabledCounter = 0;
	int discreteCounter = 0;

	TuningCounters& operator += (const TuningCounters& That)
	{
		this->errorCounter += That.errorCounter;
		this->sorCounter += That.sorCounter;
		this->controlEnabledCounter += That.controlEnabledCounter;
		this->discreteCounter += That.discreteCounter;

		return *this;
	}
};

class TuningFilterValue
{
public:
	TuningFilterValue();

	QString appSignalId() const;
	void setAppSignalId(const QString& value);

	bool useValue() const;
	void setUseValue(bool value);

	TuningValue value() const;
	void setValue(TuningValue value);

	Hash appSignalHash() const;

	bool load(QXmlStreamReader& reader);
	bool save(QXmlStreamWriter& writer) const;

private:

	QString m_appSignalId;
	Hash m_appSignalHash = 0;

	bool m_useValue = false;
	TuningValue m_value;

};

class TuningFilter : public PropertyObject
{
	Q_OBJECT

public:
	//Enums
	//
	enum class InterfaceType
	{
		Root,
		Tree,
		Tab,
		Button
	};
	Q_ENUM(InterfaceType)

	enum class Source
	{
		Project,
		Schema,
		Equipment,
		User
	};
	Q_ENUM(Source)

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
	TuningFilter(InterfaceType interfaceType);
	~TuningFilter();

	TuningFilter& operator= (const TuningFilter& That);

	bool load(QXmlStreamReader& reader);
	bool save(QXmlStreamWriter& writer, bool filterByInterfaceType, TuningFilter::Source saveSourceType) const;

	bool match(const AppSignalParam& object) const;

	void checkSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters);

	void removeNotExistingSignals(const std::vector<Hash>& signalHashes, int& removedCounter);

	const std::vector<Hash>& equipmentHashes() const;
	void setEquipmentHashes(std::vector<Hash> value);

	const std::vector<Hash>& signalsHashes() const;
	void setSignalsHashes(std::vector<Hash> value);

public:
	// Properties
	//
	QString ID() const;
	void setID(const QString& value);

	QString customID() const;
	void setCustomID(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

	bool isSourceProject() const;
	bool isSourceEquipment() const;
	bool isSourceSchema() const;
	bool isSourceUser() const;

	Source source() const;
	void setSource(Source value);

	InterfaceType interfaceType() const;
	void setInterfaceType(InterfaceType value);

	SignalType signalType() const;
	void setSignalType(SignalType value);

	QColor backColor() const;
	void setBackColor(const QColor& value);

	QColor textColor() const;
	void setTextColor(const QColor& value);

	QColor backSelectedColor() const;
	void setBackSelectedColor(const QColor& value);

	QColor textSelectedColor() const;
	void setTextSelectedColor(const QColor& value);

	bool hasDiscreteCounter() const;
	void setHasDiscreteCounter(bool value);

	// Filters
	//
	QString customAppSignalIDMask() const;
	void setCustomAppSignalIDMask(const QString& value);

	QString equipmentIDMask() const;
	void setEquipmentIDMask(const QString& value);

	QString appSignalIDMask() const;
	void setAppSignalIDMask(const QString& value);

	// Values
	//

	std::vector <TuningFilterValue> getValues() const;
	void setValues(const std::vector <TuningFilterValue>& getValues);

	int valuesCount() const;
	bool valueExists(Hash hash) const;

	void addValue(const TuningFilterValue& value);
	void removeValue(Hash hash);

	bool value(Hash hash, TuningFilterValue& value);
	void setValue(const TuningFilterValue& value);

	TuningCounters counters() const;
	void setCounters(TuningCounters value);

public:
	// Operations
	//

	TuningFilter* parentFilter() const;

	bool isEmpty() const;

	bool isRoot() const;
	bool isTree() const;
	bool isTab() const;
	bool isButton() const;

	void addTopChild(const std::shared_ptr<TuningFilter>& child);
	void addChild(const std::shared_ptr<TuningFilter>& child);

	void removeChild(const std::shared_ptr<TuningFilter>& child);
	bool removeChild(const QString& ID);

	void removeAllChildren();
	void removeChildren(Source source);

	int childFiltersCount() const;
	std::shared_ptr<TuningFilter> childFilter(int index) const;

private:
	void copy(const TuningFilter& That);

private:

	// Properties
	//

	QString m_ID;
	QString m_customID;
	QString m_caption;

	Source m_source = Source::User;

	InterfaceType m_interfaceType = InterfaceType::Tree;

	SignalType m_signalType = SignalType::All;

	QColor m_backColor = Qt::GlobalColor::lightGray;
	QColor m_textColor = Qt::GlobalColor::lightGray;

	QColor m_backSelectedColor = Qt::GlobalColor::darkGray;
	QColor m_textSelectedColor = Qt::GlobalColor::darkGray;

	bool m_hasDiscreteCounter = false;

	// Filters
	//
	QStringList m_customAppSignalIDMasks;
	QStringList m_equipmentIDMasks;
	QStringList m_appSignalIDMasks;

	// Values
	//

	std::vector<Hash> m_signalValuesVec;
	std::map <Hash, TuningFilterValue> m_signalValuesMap;

	// Parent and child
	//

	TuningFilter* m_parentFilter = nullptr;
	std::vector<std::shared_ptr<TuningFilter>> m_childFilters;

	// Hashes of equipment and filtered signals, used in client applicaton

	std::vector<Hash> m_equipmentHashes;
	std::vector<Hash> m_signalsHashes;

	// Counters

	TuningCounters m_counters;

};

class TuningFilterStorage : public QObject
{
	Q_OBJECT
public:
	TuningFilterStorage();
	TuningFilterStorage(const TuningFilterStorage& That);

	TuningFilterStorage& operator = (const TuningFilterStorage& That)
	{
		m_root = std::make_shared<TuningFilter>(*That.m_root.get());
		m_schemasDetails = That.m_schemasDetails;

		return* this;

	}

	std::shared_ptr<TuningFilter> root();

	// Serialization

	bool load(const QByteArray& data, QString* errorCode);
	bool load(const QString& fileName, QString* errorCode);

	bool save(QByteArray& data);
	bool save(QByteArray& data, TuningFilter::Source saveSourceType);
	bool save(const QString& fileName, QString* errorMsg, TuningFilter::Source saveSourceType);

	bool copyToClipboard(std::vector<std::shared_ptr<TuningFilter>> filters);
	std::shared_ptr<TuningFilter> pasteFromClipboard();

	// Operation

	void add(std::shared_ptr<TuningFilter> filter, bool moveToTop);

	void checkFilterSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters);

protected:

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

protected:

	std::shared_ptr<TuningFilter> m_root = nullptr;

	std::vector<VFrame30::SchemaDetails> m_schemasDetails;
};

Q_DECLARE_METATYPE(std::shared_ptr<TuningFilter>)

Q_DECLARE_METATYPE(TuningFilterValue)


