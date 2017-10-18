#ifndef OBJECTFILTER_H
#define OBJECTFILTER_H

#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/Tuning/TuningSignalState.h"
#include "../lib/PropertyObject.h"
#include "../lib/Hash.h"
#include "../VFrame30/Schema.h"

class TuningFilterValue
{
public:
	TuningFilterValue();

	QString appSignalId() const;
	void setAppSignalId(const QString& value);

	bool useValue() const;
	void setUseValue(bool value);

	float value() const;
	void setValue(float value);

	Hash appSignalHash() const;

	bool load(QXmlStreamReader& reader);
	bool save(QXmlStreamWriter& writer) const;

private:

	QString m_appSignalId;
	Hash m_appSignalHash = 0;

	bool m_useValue = false;
	float m_value = 0;

};

class DialogCheckFilterSignals : public QDialog
{
	Q_OBJECT

public:

	DialogCheckFilterSignals(std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parent);

private slots:

	void buttonClicked(QAbstractButton* button);

private:

	QDialogButtonBox* m_buttonBox = nullptr;
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
	bool save(QXmlStreamWriter& writer) const;

	bool match(const AppSignalParam& object, bool checkValues) const;

	void checkSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters);

	void removeNotExistingSignals(const std::vector<Hash>& signalHashes, int& removedCounter);

public:
	// Properties
	//
	QString ID() const;
	void setID(const QString& value);

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

	void setValue(const TuningFilterValue& value);

	bool value(Hash hash, TuningFilterValue& value);

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
	QString m_caption;

	Source m_source = Source::User;

	InterfaceType m_interfaceType = InterfaceType::Tree;

	SignalType m_signalType = SignalType::All;

	QColor m_backColor = Qt::GlobalColor::lightGray;
	QColor m_textColor = Qt::GlobalColor::lightGray;


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

	std::vector<std::shared_ptr<TuningFilter>> m_childFilters;

	TuningFilter* m_parentFilter = nullptr;

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

	// Serialization

	bool load(const QByteArray& data, QString* errorCode);
	bool load(const QString& fileName, QString* errorCode);

    bool save(QByteArray& data);
	bool save(const QString& fileName, QString* errorMsg);

	bool copyToClipboard(std::vector<std::shared_ptr<TuningFilter>> filters);
	std::shared_ptr<TuningFilter> pasteFromClipboard();

	// Schemas loading

	bool loadSchemasDetails(const QByteArray& data, QString* errorCode);
	int schemaDetailsCount();
	VFrame30::SchemaDetails schemaDetails(int index);

	// Operations

	void createAutomaticFilters(const TuningSignalStorage* objects, bool bySchemas, bool byEquipment, const QStringList& tuningSourcesEquipmentIds);

	void removeFilters(TuningFilter::Source sourceType);

	void checkFilterSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString> >& notFoundSignalsAndFilters);
	void checkAndRemoveFilterSignals(const std::vector<Hash>& signalHashes, bool& removedNotFound, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters, QWidget* parentWidget);

protected:

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);


public:

	std::shared_ptr<TuningFilter> m_root = nullptr;

private:

	//void checkFilterSignals(TuningFilter* filter, const std::vector<TuningSignal>& tuningSignals, QStringList& errorLog, int& notFoundCounter);

private:

	std::vector<VFrame30::SchemaDetails> m_schemasDetails;
};

Q_DECLARE_METATYPE(std::shared_ptr<TuningFilter>)

Q_DECLARE_METATYPE(TuningFilterValue)

#endif // OBJECTFILTER_H
