#ifndef OBJECTFILTER_H
#define OBJECTFILTER_H

#include "Stable.h"
#include "../lib/Tuning/TuningObject.h"
#include "../lib/PropertyObject.h"
#include "../lib/Hash.h"

struct SchemaDetails
{
	QString m_caption;
    QString m_Id;
	QStringList m_appSignalIDs;

};

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

    DialogCheckFilterSignals(QStringList& errorLog, QWidget* parent);

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

    bool load(QXmlStreamReader& reader, bool automatic);
	bool save(QXmlStreamWriter& writer) const;

    bool match(const TuningObject &object, bool checkValues) const;

    void checkSignals(const TuningObjectStorage *objects, QStringList& errorLog, int &notFoundCounter);

    void removeNotExistingSignals(const TuningObjectStorage *objects, int &removedCounter);

public:
	// Properties
	//
	QString ID() const;
	void setID(const QString& value);

	QString caption() const;
	void setCaption(const QString& value);

    bool automatic() const;
    void setAutomatic(bool value);

    FilterType filterType() const;
    void setFilterType(FilterType value);

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

	std::vector <TuningFilterValue> signalValues() const;
	void setValues(const std::vector <TuningFilterValue>& values);

    int valuesCount() const;
    bool valueExists(Hash hash) const;

    void addValue(const TuningFilterValue& value);
    void removeValue(Hash hash);

    void setValue(const TuningFilterValue &value);

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
    void removeAutomaticChildren();

	int childFiltersCount() const;
	std::shared_ptr<TuningFilter> childFilter(int index) const;

private:
	void copy(const TuningFilter& That);

private:

    // Properties
    //

	QString m_ID;
	QString m_caption;

    bool m_automatic = false;

    FilterType m_filterType = FilterType::Tree;
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

		return *this;

	}

    // Serialization

    bool load(const QByteArray& data, QString *errorCode, bool automatic);

    bool load(const QString& fileName, QString *errorCode, bool automatic);
	bool save(const QString& fileName, QString *errorMsg);

    bool copyToClipboard(std::vector<std::shared_ptr<TuningFilter>> filters);
    std::shared_ptr<TuningFilter> pasteFromClipboard();

    // Schemas loading

    bool loadSchemasDetails(const QByteArray& data, QString *errorCode);
    int schemaDetailsCount();
	SchemaDetails schemaDetails(int index);

    // Operations

    void createAutomaticFilters(const TuningObjectStorage *objects, bool bySchemas, bool byEquipment, const QStringList &tuningSourcesEquipmentIds);

    void removeAutomaticFilters();

    void checkSignals(const TuningObjectStorage *objects, bool &removedNotFound, QWidget* parentWidget);

protected:

	virtual void writeLogError(const QString& message);
	virtual void writeLogWarning(const QString& message);
	virtual void writeLogMessage(const QString& message);

public slots:
	void slot_filtersUpdated(QByteArray data);
	void slot_schemasDetailsUpdated(QByteArray data);


public:

	std::shared_ptr<TuningFilter> m_root = nullptr;

private:

    void checkFilterSignals(TuningFilter* filter, const std::vector<TuningObject> &tuningObjects, QStringList& errorLog, int &notFoundCounter);

private:

	std::vector<SchemaDetails> m_schemasDetails;
};

Q_DECLARE_METATYPE(std::shared_ptr<TuningFilter>)

Q_DECLARE_METATYPE(TuningFilterValue)

#endif // OBJECTFILTER_H
