#pragma once
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/PropertyObject.h"
#include "../lib/Hash.h"
#include "../VFrame30/Schema.h"
#include <QColor>

const int MAX_VALUES_COLUMN_COUNT = 6;

struct TuningCounters
{
	int errorCounter = 0;
	int sorCounter = 0;
	bool sorActive = false;
	bool sorValid = false;
	int controlEnabledCounter = 0;
	int discreteCounter = 0;
};

class TuningFilterSignal
{
public:
	TuningFilterSignal();

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
	Hash m_appSignalHash = UNDEFINED_HASH;

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
	void setEquipmentHashes(std::vector<Hash> signalValue);

	const std::vector<Hash>& signalsHashes() const;
	void setSignalsHashes(std::vector<Hash> signalValue);

public:
	// Properties
	//
	QString ID() const;
	void setID(const QString& signalValue);

	QString customID() const;
	void setCustomID(const QString& signalValue);

	QString caption() const;
	void setCaption(const QString& signalValue);

	bool isSourceProject() const;
	bool isSourceEquipment() const;
	bool isSourceSchema() const;
	bool isSourceUser() const;

	Source source() const;
	void setSource(Source signalValue);

	InterfaceType interfaceType() const;
	void setInterfaceType(InterfaceType signalValue);

	SignalType signalType() const;
	void setSignalType(SignalType signalValue);

	QColor backColor() const;
	void setBackColor(const QColor& signalValue);

	QColor textColor() const;
	void setTextColor(const QColor& signalValue);

	QColor backSelectedColor() const;
	void setBackSelectedColor(const QColor& signalValue);

	QColor textSelectedColor() const;
	void setTextSelectedColor(const QColor& signalValue);

	bool hasDiscreteCounter() const;
	void setHasDiscreteCounter(bool signalValue);

	// Filters
	//
	QString customAppSignalIDMask() const;
	void setCustomAppSignalIDMask(const QString& signalValue);

	QString equipmentIDMask() const;
	void setEquipmentIDMask(const QString& signalValue);

	QString appSignalIDMask() const;
	void setAppSignalIDMask(const QString& signalValue);

	// FilterSignals
	//
	std::vector <TuningFilterSignal> getFilterSignals() const;

	int filterSignalsCount() const;
	bool filterSignalExists(Hash hash) const;

	void addFilterSignal(const TuningFilterSignal& fs);
	bool removeFilterSignal(Hash hash);

	bool filterSignal(Hash hash, TuningFilterSignal& fs);

	// Counters
	//
	TuningCounters counters() const;
	void setCounters(TuningCounters signalValue);

	// Tab appearance
	//
	int valuesColumnCount() const;
	void setValuesColumnCount(int signalValue);

	std::vector<QString> valueColumnsAppSignalIdSuffixes() const;

	bool columnCustomAppId() const;
	void setColumnCustomAppId(bool signalValue);

	bool columnAppId() const;
	void setColumnAppId(bool signalValue);

	bool columnEquipmentId() const;
	void setColumnEquipmentId(bool signalValue);

	bool columnCaption() const;
	void setColumnCaption(bool signalValue);

	bool columnUnits() const;
	void setColumnUnits(bool signalValue);

	bool columnType() const;
	void setColumnType(bool signalValue);

	bool columnLimits() const;
	void setColumnLimits(bool signalValue);

	bool columnDefault() const;
	void setColumnDefault(bool signalValue);

	bool columnValid() const;
	void setColumnValid(bool signalValue);

	bool columnOutOfRange() const;
	void setColumnOutOfRange(bool signalValue);


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
	std::shared_ptr<TuningFilter> childFilter(const QString& caption) const;

	std::shared_ptr<TuningFilter> findFilterById(const QString& id) const;	// Recursive search

	void updateOptionalProperties();

private:
	void copy(const TuningFilter& That);

	bool processMaskList(const QString& s, const QStringList& masks) const;

	void setPropertyVisible(const QLatin1String& name, bool visible);

private:

	//
	// Properties
	//
	QString m_ID = "ID";
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

	// Tab appearance
	//
	int m_valueColumnsCount = 0;
	std::vector<QString> m_valueColumnsAppSignalIdSuffixes;

	bool m_columnCustomAppId = true;
	bool m_columnAppId = false;
	bool m_columnEquipmentId = true;
	bool m_columnCaption = true;
	bool m_columnUnits = true;
	bool m_columnType = true;
	bool m_columnLimits = true;
	bool m_columnDefault = true;
	bool m_columnValid = false;
	bool m_columnOutOfRange = false;

private:

	//
	// Run-time data
	//

	// Values
	//
	std::map <Hash, TuningFilterSignal> m_signalValuesMap;

	// Parent and child
	//
	TuningFilter* m_parentFilter = nullptr;
	std::vector<std::shared_ptr<TuningFilter>> m_childFilters;

	// Hashes of equipment and filtered signals, used in client applicaton
	//
	std::vector<Hash> m_equipmentHashes;
	std::vector<Hash> m_signalsHashes;

	// Counters
	//
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
	//
	bool load(const QByteArray& data, QString* errorCode);
	bool load(const QString& fileName, QString* errorCode);

	bool save(QByteArray& data);
	bool save(QByteArray& data, TuningFilter::Source saveSourceType);
	bool save(const QString& fileName, QString* errorMsg, TuningFilter::Source saveSourceType);

	bool copyToClipboard(std::vector<std::shared_ptr<TuningFilter>> filters);
	std::shared_ptr<TuningFilter> pasteFromClipboard();

	// Operation
	//
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

Q_DECLARE_METATYPE(TuningFilterSignal)


