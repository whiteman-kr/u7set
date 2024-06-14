#pragma once

namespace VFrame30
{
	class SchemaDetails;
};

namespace TuningLib
{
	// Structs
	//
	struct TuningCounters
	{
		int errorCounter = 0;
		int sorCounter = 0;
		bool sorActive = false;
		bool sorValid = false;
		int discreteCounter = 0;
	};

	class TuningUiItem : public PropertyObject
	{
		Q_OBJECT

	public:
		// Enums
		//
		enum class InterfaceType
		{
			Root,
			Generic,
			Tab,
			Button,
			Counter,
			SchemasTab
		};
		Q_ENUM(InterfaceType)

		enum class TabType
		{
			Generic,
			FiltersSwitch,
		};
		Q_ENUM(TabType)

		enum class CounterType
		{
			StatusBar,
			FilterTree,
		};
		Q_ENUM(CounterType)

	public:
		TuningUiItem();
		TuningUiItem(const TuningUiItem& That) = delete;
		TuningUiItem& operator=(const TuningUiItem& That) = delete;

		bool load(QXmlStreamReader& reader);
		bool save(QXmlStreamWriter& writer) const;

	public:
		// Main Properties

		QUuid uuid() const;
		QString uuidString() const;
		void setUuid(const QUuid& uuid);

		QString ID() const;
		void setID(const QString& value);

		QString caption() const;
		void setCaption(const QString& value);

		// Interface Type

		bool isRoot() const;
		bool isGeneric() const;
		bool isTab() const;
		bool isButton() const;
		bool isCounter() const;
		bool isSchemasTab() const;

		InterfaceType interfaceType() const;
		void setInterfaceType(InterfaceType value);

		// Colors

		bool useColors() const;
		void setUseColors(bool value);

		QColor backColor() const;
		void setBackColor(const QColor& value);

		QColor textColor() const;
		void setTextColor(const QColor& value);

		QColor backSelectedColor() const;
		void setBackSelectedColor(const QColor& value);

		QColor textSelectedColor() const;
		void setTextSelectedColor(const QColor& value);

		QColor backAlertedColor() const;
		void setBackAlertedColor(const QColor& value);

		QColor textAlertedColor() const;
		void setTextAlertedColor(const QColor& value);

		bool hasDiscreteCounter() const;
		void setHasDiscreteCounter(bool value);

		// Counters

		TuningCounters counters() const;
		void setCounters(TuningCounters value);

		CounterType counterType() const;
		void setCounterType(CounterType type);

		// Tags

		QString tags() const;
		void setTags(const QString& value);

		const QStringList& tagsList() const;
		QStringList& tagsList();

		// Filters

		QString filters() const;
		void setFilters(const QString& value);

		const QStringList& filtersList() const;
		QStringList& filtersList();

		// Schema tab properties

		QString startSchemaId() const;
		void setStartSchemaId(const QString& id);

		// Tab appearance

		TabType tabType() const;
		void setTabType(TabType type);

		int valuesColumnCount() const;
		void setValuesColumnCount(int value);

		std::vector<QString> valueColumnsAppSignalIdSuffixes() const;
		void setValueColumnsAppSignalIdSuffixes(const std::vector<QString>& suffixes);

		bool columnCustomAppId() const;
		void setColumnCustomAppId(bool value);

		bool columnAppId() const;
		void setColumnAppId(bool value);

		bool columnEquipmentId() const;
		void setColumnEquipmentId(bool value);

		bool columnCaption() const;
		void setColumnCaption(bool value);

		bool columnUnits() const;
		void setColumnUnits(bool value);

		bool columnType() const;
		void setColumnType(bool value);

		bool columnLimits() const;
		void setColumnLimits(bool value);

		bool columnDefault() const;
		void setColumnDefault(bool value);

		bool columnValid() const;
		void setColumnValid(bool value);

		bool columnOutOfRange() const;
		void setColumnOutOfRange(bool value);

	public:
		// Operations

		TuningUiItem* parentItem() const;

		void addChild(const std::shared_ptr<TuningUiItem>& child);
		void insertChild(int index, const std::shared_ptr<TuningUiItem>& child);

		bool removeChild(const QUuid& uuid);
		bool removeChild(const QString& ID);
		bool removeChild(int index);
		void removeAllChildren();

		int childCount() const;
		std::shared_ptr<TuningUiItem> child(int index) const;

		std::shared_ptr<TuningUiItem> find(const QString& id) const; // Recursive search
		std::shared_ptr<TuningUiItem> find(const QUuid& uuid) const; // Recursive search

		void updateOptionalProperties();

		std::vector<TuningUiItem*> childernToVector() const;

	private:
		void setPropertyVisible(const QLatin1String& name, bool visible);

	private:
		//
		// Properties
		//
		QUuid m_uuid;
		QString m_ID = "ID";
		QString m_caption;

		InterfaceType m_interfaceType = InterfaceType::Generic;

		bool m_useColors = false;

		QColor m_backColor = Qt::GlobalColor::lightGray;
		QColor m_textColor = Qt::GlobalColor::black;

		QColor m_backSelectedColor = Qt::GlobalColor::darkGray;
		QColor m_textSelectedColor = Qt::GlobalColor::black;

		QColor m_backAlertedColor = Qt::GlobalColor::red;
		QColor m_textAlertedColor = Qt::GlobalColor::white;

		bool m_hasDiscreteCounter = false;

		QStringList m_tags;
		QStringList m_filters;

		QString m_startSchemaId;

		// Tab appearance
		//
		int m_valueColumnsCount = 0;
		std::vector<QString> m_valueColumnsAppSignalIdSuffixes;

		TabType m_tabType = TabType::Generic;
		CounterType m_counterType = CounterType::StatusBar;

		// Visible columns
		//
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

		//
		// Run-time data
		//
	private:
		// Parent and child
		//
		TuningUiItem* m_parentItem = nullptr;
		std::vector<std::shared_ptr<TuningUiItem>> m_children;

		// Counters
		//
		TuningCounters m_counters;
	};

	class TuningUiStorage : public QObject
	{
		Q_OBJECT
	public:
		TuningUiStorage();

		// Access
		//
		const TuningUiItem* root() const;
		TuningUiItem* root();

		// Serialization
		//
		bool load(const QByteArray& data, QString* errorCode);
		bool save(QByteArray& data) const;

		// Operation
		//
		void add(std::shared_ptr<TuningUiItem> filter, bool moveToTop);

		// Validate
		// 
		std::vector<std::pair<QString, QString>> checkForSameIds() const;
		std::vector<std::tuple<QString, QString, QString>> checkFilters(const QStringList& appSignalLists);

	protected:
		std::unique_ptr<TuningUiItem> m_root;
		// std::vector<VFrame30::SchemaDetails> m_schemasDetails;
	};

} // namespace TuningLib
