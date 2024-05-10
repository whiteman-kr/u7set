#pragma once

#include "../../../CommonLib/Types.h"
#include "../../../CommonLib/Hash.h"
#include "../../../AppSignalLib/TuningValue.h"

#include <optional>
#include <vector>

class AppSignalParam;

namespace Proto
{
	class Envelope;
}

namespace AppSignalLists
{
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
		QStringList& tagsList();

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

		[[nodiscard]] int count() const;

		const AppSignalListItem& operator[](int index) const;
		AppSignalListItem& operator[](int index);

		const AppSignalListItem& operator[](Hash hash) const;
		AppSignalListItem& operator[](Hash hash);

		[[nodiscard]] bool itemExists(Hash hash) const;
		[[nodiscard]] int itemIndex(Hash hash) const;

		void add(const AppSignalListItem& item);
		[[nodiscard]] bool remove(int index);
		[[nodiscard]] bool remove(Hash hash);

		/*
		void checkSignals(const std::vector<Hash>& signalHashes, std::vector<std::pair<QString, QString>>& notFoundSignalsAndFilters);
		void removeNotExistingSignals(const std::vector<Hash>& signalHashes, int& removedCounter);
		*/

		//

		// Returns true if application signal matches to this list (mask or items list)
		//
		bool appSignalMatch(const AppSignalParam& asp) const;

		// Returns true if list matches to set of IDs, masks or tags
		//
		bool listMatch(const QStringList& appSignalListIds, const QStringList& appSignalListMasks, const QStringList& appSignalListTags);

	public:
		static inline const QString tagIde = "~ide~";
		static inline const QString tagEquipment = "~eqp~";
		static inline const QString tagSchema = "~schema~";

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

		// Cached filters split to string list, \n replaced to ';'
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

	class AppSignalListSet
	{
	public:
		void clear();
		[[nodiscard]] int count();
		
		bool add(const QByteArray& ba);
		bool add(const AppSignalListSet& appSignalListSet);

		void remove(const QString& tag);

	private:
		std::map<QUuid, std::shared_ptr<AppSignalList>> m_lists;

	};
}