#pragma once

namespace Proto
{
	class SchemaDetails;
	class TrendIndicatorSchemaItems;
	class Envelope;
}

namespace VFrame30
{
	class Schema;

	// SchemaDetails is a class to parse to/from JSON doc
	// Format:
	//		Version : 1
	//		SchemaID : "SCHMEAID"
	//		Signals: ["id", "id", "id", ...]
	//		Labels: ["Label1", "Label2", "Label3", ...]
	//		ItemGuids: ["guid1", "guid2", "guid3", ...]
	//
	class SchemaDetails
	{
	public:
		SchemaDetails() noexcept = default;
		explicit SchemaDetails(const QString& details); // Parse details from string, isNull() will be true if parsing failed.

		bool operator<(const SchemaDetails& b) const;

	public:
		[[nodiscard]] static QString getDetailsString(const Schema* schema, const QString& path);

		bool parseDetails(const QString& details);

		bool saveData(Proto::SchemaDetails* message) const;
		bool loadData(const Proto::SchemaDetails& message);

		[[nodiscard]] bool searchForString(const QString& searchText) const;

		[[nodiscard]] bool isNull() const;

		[[nodiscard]] bool hasSchemaTag(const QString& tag) const;
		[[nodiscard]] bool hasSchemaTag(const QStringList& tags) const;
		[[nodiscard]] const std::set<QString>& schemaTags() const;

		[[nodiscard]] bool hasEquipmentId(const QString& equipmentId) const;
		[[nodiscard]] bool hasSignal(const QString& signalId) const;
		[[nodiscard]] bool hasLoopback(const QString& loopbackId) const;
		[[nodiscard]] bool hasLabel(const QString& label) const;
		[[nodiscard]] bool hasConnection(const QString& connectionId) const;
		[[nodiscard]] bool hasGuid(QUuid guid) const;

	public:
		int m_version = 0;
		QString m_schemaId;
		QString m_caption;
		bool m_excludedFromBuild = false;
		QString m_equipmentId;			// Valid for LogicSchemas
		QString m_lmDescriptionFile;	// Valid for LogicSchemas and UfbSchemas
		QString m_path;					// Path in terms of SchemaEditor ("/ABC/DEF", "/")
		std::set<QString> m_signals;
		std::set<QString> m_labels;
		std::set<QString> m_connections;
		std::set<QString> m_loopbacks;
		std::set<QString> m_schemaTags;		// All tags are kept in lowercase
		std::set<QString> m_itemTags;		// All tags are kept in lowercase
		std::set<QString> m_packedLogicIds;
		std::set<QUuid> m_guids;

		// SchemaItemIndicator, type trend
		//
		struct TrendIndicatorSchemaItems
		{
			TrendIndicatorSchemaItems() = default;
			TrendIndicatorSchemaItems(QUuid _itemUuid,
									  E::RtTrendsSamplePeriod _samplePeriod,
									  E::TimeType _timeType,
									  int _durationSeconds,
									  const QStringList& _appSignalIds);

			QUuid itemUuid;			// SchemaItemIndicator id
			E::RtTrendsSamplePeriod samplePeriod = E::RtTrendsSamplePeriod::sp_1s;
			E::TimeType timeType = E::TimeType::Local;
			int durationSeconds = 0;
			QStringList appSignalIds;

			QJsonObject toJsonObject() const;
			bool fromJsonObject(const QJsonObject& jsonObject);

			bool saveData(::Proto::TrendIndicatorSchemaItems* message) const;
			bool loadData(const ::Proto::TrendIndicatorSchemaItems& message);
		};

		std::vector<TrendIndicatorSchemaItems> m_trendsIndicators;
	};


	class SchemaDetailsSet : public Proto::ObjectSerialization<SchemaDetailsSet>
	{
	public:
		SchemaDetailsSet();
		SchemaDetailsSet(const SchemaDetailsSet&) = default;
		SchemaDetailsSet(SchemaDetailsSet&&) = default;
		SchemaDetailsSet& operator=(const SchemaDetailsSet&) = default;
		SchemaDetailsSet& operator=(SchemaDetailsSet&&) noexcept = default;

		// Serialization implementation of Proto::ObjectSerialization<>
		//
		friend Proto::ObjectSerialization<SchemaDetailsSet>;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		static std::shared_ptr<SchemaDetailsSet> CreateObject(const Proto::Envelope& message);

		// Properties and functions
		//
	public:
		void clear();

		void add(const QString& details);
		void add(const SchemaDetails& details);
		void add(SchemaDetails&& details);
		void add(std::shared_ptr<SchemaDetails> details);

		std::vector<SchemaDetails> schemasDetails() const;
		std::vector<SchemaDetails> schemasDetails(QString equipmentId) const;

		std::shared_ptr<SchemaDetails> schemaDetails(QString schemaId) const;
		std::shared_ptr<SchemaDetails> schemaDetails(int index) const;

		QStringList schemasByAppSignalId(const QString& appSignalId) const;
		QStringList schemasByConnectionId(const QString& connectionId) const;
		QStringList schemasByLoopbackId(const QString& loopbackId) const;

		int schemaCount() const;
		QString schemaCaptionById(const QString& schemaId) const;
		QString schemaCaptionByIndex(int schemaIndex) const;
		QString schemaIdByIndex(int schemaIndex) const;

		std::vector<SchemaDetails::TrendIndicatorSchemaItems> trendIndicators() const;

	private:
		std::map<QString, std::shared_ptr<SchemaDetails>> m_details;	// Key is schemaId
	};

} // namespace VFrame30