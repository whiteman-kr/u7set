#pragma once

#include "../../../CommonLib/Types.h"

namespace AppSignalLists
{
	// Column names

	static const QString col_CustomAppSignalId = "CustomAppSignalID";
	static const QString col_AppSignalId = "AppSignalId";
	static const QString col_EquipmentID = "EquipmentID";
	static const QString col_Caption = "Caption";
	static const QString col_Units = "Units";
	static const QString col_Type = "Type";
	static const QString col_LowLimit = "LowLimit";
	static const QString col_HighLimit= "HighLimit";
	static const QString col_Default= "Default";
	static const QString col_Value = "Value";

	// Property names

	static const QLatin1String prop_Caption = QLatin1String("Caption");
	static const QLatin1String prop_SignalType = QLatin1String("SignalType");
	static const QLatin1String prop_ID = QLatin1String("ID");
	static const QLatin1String prop_Uuid = QLatin1String("Uuid");
	static const QLatin1String prop_Tags = QLatin1String("Tags");
	static const QLatin1String prop_CustomAppSignalMasks = QLatin1String("CustomAppSignalMasks");
	static const QLatin1String prop_AppSignalMasks = QLatin1String("AppSignalMasks");
	static const QLatin1String prop_EquipmentIDMasks = QLatin1String("EquipmentIDMasks");
	static const QLatin1String prop_AppSignalTags = QLatin1String("AppSignalTags");

	class AppSignalListItem
	{
	public:
		AppSignalListItem() = default;
		explicit AppSignalListItem(const QString& appSignalId);

		const QString& appSignalId() const;
		void setAppSignalId(const QString& appSignalId);

		Hash appSignalHash() const;

		TuningValue value() const;
		void setValue(TuningValue value);

		bool hasValue() const;
		void removeValue();

	private:
		QString m_appSignalId;
		Hash m_appSignalHash = UNDEFINED_HASH;
		std::optional<TuningValue> m_value;
	};

	class AppSignalList : public PropertyObject
	{
		Q_OBJECT

	public:
		// Enums
		//
		enum class SignalType
		{
			All,
			Analog,
			Discrete,
		};
		Q_ENUM(SignalType)

	public:
		AppSignalList();

		void SaveData(Proto::Envelope* message) const;
		bool LoadData(const Proto::Envelope& message);

	public:
		// Main Properties

		QString id() const;
		void setId(const QString& value);

		QUuid uuid() const;
		void setUuid(const QUuid& uuid);

		QString caption() const;
		void setCaption(const QString& value);

		SignalType signalType() const;
		void setSignalType(SignalType value);

		QString tags() const;
		void setTags(const QString& value);

		const QStringList& tagsList() const;
		bool hasAnyTag(const QStringList& tags) const;

		// Masks opreations

		QString customAppSignalIDMask() const;
		void setCustomAppSignalIDMask(QString value);

		QString equipmentIDMask() const;
		void setEquipmentIDMask(QString value);

		QString appSignalIDMask() const;
		void setAppSignalIDMask(QString value);

		QString appSignalTags() const;
		void setAppSignalTags(QString value);

		// Items operations

		int count() const;

		const AppSignalListItem& operator[](int index) const;
		AppSignalListItem& operator[](int index);

		const AppSignalListItem& operator[](Hash hash) const;
		AppSignalListItem& operator[](Hash hash);

		bool itemExists(Hash hash) const;
		int itemIndex(Hash hash) const;

		void add(const AppSignalListItem& item);
		[[nodiscard]] bool remove(int index);
		[[nodiscard]] bool remove(Hash hash);

		/*
		void checkSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters);
		void removeNotExistingSignals(const std::vector<Hash>& signalHashes, int& removedCounter);
		*/

		//

		bool match(const AppSignalParam& param) const;

	private:
		static bool processMaskList(const QString& s, const QStringList& masks);

	private:
		//
		// Properties
		//
		QUuid m_uuid;
		QString m_id = "ID";
		QString m_caption;

		SignalType m_signalType = SignalType::All;

		QStringList m_tags;

		// Filters
		//
		QString m_customAppSignalIDMasks;
		QString m_equipmentIDMasks;
		QString m_appSignalIDMasks;
		QString m_appSignalTags;

		// Cached filters splitted to string list, \n replaced to ';'
		//
		QStringList m_cachedCustomAppSignalIDMasks;
		QStringList m_cachedEquipmentIDMasks;
		QStringList m_cachedAppSignalIDMasks;
		QStringList m_cachedAppSignalTags;

		// Items
		//
		std::vector<AppSignalListItem> m_items;

	public:
		static const char* mimeType; // = "application/x-radiyappsignallist";
	};
}