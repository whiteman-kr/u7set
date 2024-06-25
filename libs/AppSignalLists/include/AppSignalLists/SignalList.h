#pragma once

#include "../../../CommonLib/Types.h"
#include "../../../CommonLib/Hash.h"
#include "../../../AppSignalLib/TuningValue.h"

#include <map>
#include <memory>
#include <optional>
#include <vector>
#include <set>

class AppSignalParam;

class ILogFile;

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
		AppSignalList(const AppSignalList& that);
		AppSignalList(AppSignalList&& that) = delete;

		AppSignalList& operator=(const AppSignalList& that);
		AppSignalList& operator=(AppSignalList&& that) = delete;

		~AppSignalList() = default;

	public:
		void SaveData(Proto::Envelope* message) const;
		bool LoadData(const Proto::Envelope& message);

	public:
		// Main Properties

		QString id() const;
		void setId(const QString& value);

		QUuid uuid() const;
		QString uuidString() const;
		void setUuid(const QUuid& uuid);

		QString caption() const;
		void setCaption(const QString& value);

		SignalType signalType() const;
		void setSignalType(SignalType value);

		// System, Tags operations

		QString systemTags() const;
		void setSystemTags(const QString& value);

		const QStringList& systemTagsList() const;
		QStringList& systemTagsList();

		bool hasAnySystemTag(const QStringList& tags) const;

		// User Tags operations

		QString userTags() const;
		void setUserTags(const QString& value);

		const QStringList& userTagsList() const;
		QStringList& userTagsList();

		bool hasAnyUserTag(const QStringList& tags) const;

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
		[[nodiscard]] std::set<Hash> itemsHashes() const;

		[[nodiscard]] const AppSignalListItem& itemByHash(Hash hash) const;
		[[nodiscard]] AppSignalListItem& itemByHash(Hash hash);

		[[nodiscard]] bool itemExists(Hash hash) const;

		void add(const AppSignalListItem& item);
		void remove(Hash hash);

		[[nodiscard]] const std::set<Hash>& appListHashesCache() const;
		[[nodiscard]] std::set<Hash>& mutableAppListHashesCache();

		[[nodiscard]] const std::set<Hash>& tuningListHashesCache() const;
		[[nodiscard]] std::set<Hash>& mutableTuningListHashesCache();

		// Returns true if application signal matches to this list (mask or items list)
		//
		bool appSignalMatch(const AppSignalParam& asp) const;

		// Returns true if list matches to set of IDs, masks or tags
		//
		bool listMatch(const QStringList& appSignalListIds, const QStringList& appSignalListMasks, const QStringList& appSignalListTags);

	public:
		static inline const QString tagIde = "ide";
		static inline const QString tagUi = "ui";
		static inline const QString tagEquipment = "eqp";
		static inline const QString tagSchema = "schema";
		static inline const QString tagTcAuto = "tcauto";

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

		QStringList m_systemTags;
		QStringList m_userTags;

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
		std::map<Hash, AppSignalListItem> m_items;	// Signals added by user are stored here
		
		std::set<Hash> m_appListHashesCache;			// All signals hashes that match this filter are stored here. They are filled on build and after local lists editing
		std::set<Hash> m_tuningListHashesCache;		// Only Tuning signals hashes that match this filter are stored here for IDE-created filters. On loading by TuningClient, they replace cache.

	public:
		static const char* mimeType; // = "application/x-radiyappsignallist";
	};

	class AppSignalListSet : public QObject
	{
		Q_OBJECT

	public:
		AppSignalListSet() = default;
		
		AppSignalListSet(const AppSignalListSet& that);
		AppSignalListSet(AppSignalListSet&& that) noexcept;

		AppSignalListSet& operator= (const AppSignalListSet& that);
		AppSignalListSet& operator= (AppSignalListSet&& that) noexcept;

		~AppSignalListSet() = default;

	public:
		void clear();
		[[nodiscard]] int count() const;
		
		bool add(const QByteArray& ba);
		bool add(std::shared_ptr<AppSignalList> list);
		bool add(const AppSignalListSet& appSignalListSet);

		[[nodiscard]] std::shared_ptr<AppSignalList> get(int index) const;
		[[nodiscard]] std::shared_ptr<AppSignalList> get(const QString& id) const;
		[[nodiscard]] std::shared_ptr<AppSignalList> get(const QUuid& uuid) const;

		void remove(const QUuid& uuid);
		void remove(const QString& systemTag);

		std::vector<std::shared_ptr<AppSignalList>> lists() const;

		virtual bool load(QString* errorMessage);
		virtual bool save(QString* errorMessage) const;
		
		std::vector<std::pair<QString, QString>> checkForSameIds() const;

		void fireUpdatePerformed();

	signals:
		void updatePerformed();

	private:
		std::vector<std::shared_ptr<AppSignalList>> m_lists;
	};
} // namespace AppSignalLists
