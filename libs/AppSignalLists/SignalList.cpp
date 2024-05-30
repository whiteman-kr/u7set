#include "./include/AppSignalLists/SignalList.h"
#include "TextResource.h"
#include <ProtoCommonHelper.h>

namespace AppSignalLists
{
	const char* AppSignalList::mimeType = "application/x-radiyappsignallist";

	AppSignalListItem::AppSignalListItem(const QString& appSignalId) :
		m_appSignalId(appSignalId),
		m_appSignalHash(::calcHash(appSignalId))
	{
	}

	const QString& AppSignalListItem::appSignalId() const
	{
		return m_appSignalId;
	}

	void AppSignalListItem::setAppSignalId(const QString& appSignalId)
	{
		m_appSignalId = appSignalId;
		m_appSignalHash = ::calcHash(appSignalId);
	}

	Hash AppSignalListItem::appSignalHash() const
	{
		return m_appSignalHash;
	}

	TuningValue AppSignalListItem::value() const
	{
		Q_ASSERT(m_value.has_value());
		return m_value.value_or(TuningValue());
	}

	void AppSignalListItem::setValue(TuningValue value)
	{
		m_value = value;
	}

	bool AppSignalListItem::hasValue() const
	{
		return m_value.has_value();
	}

	void AppSignalListItem::removeValue()
	{
		m_value.reset();
	}

	//
	// AppSignalList
	//
	AppSignalList::AppSignalList()
	{
		QUuid uuid = QUuid::createUuid();
		setUuid(uuid);

		ADD_PROPERTY_GETTER_SETTER(QString, AppSignalLists::prop_Caption, true, AppSignalList::caption, AppSignalList::setCaption);
		ADD_PROPERTY_GETTER_SETTER(SignalType,
								   AppSignalLists::prop_SignalType,
								   true,
								   AppSignalList::signalType,
								   AppSignalList::setSignalType);
		ADD_PROPERTY_GETTER_SETTER(QString, AppSignalLists::prop_ID, true, AppSignalList::id, AppSignalList::setId);
		ADD_PROPERTY_GETTER_SETTER(QString, AppSignalLists::prop_Tags, true, AppSignalList::userTags, AppSignalList::setUserTags)
			->setSpecificEditor(E::PropertySpecificEditor::Tags);

		ADD_PROPERTY_GETTER_SETTER(QString,
								   AppSignalLists::prop_CustomAppSignalMasks,
								   true,
								   AppSignalList::customAppSignalIDMask,
								   AppSignalList::setCustomAppSignalIDMask)
			->setCategory("Masks");

		ADD_PROPERTY_GETTER_SETTER(QString,
								   AppSignalLists::prop_AppSignalMasks,
								   true,
								   AppSignalList::appSignalIDMask,
								   AppSignalList::setAppSignalIDMask)
			->setCategory("Masks");

		ADD_PROPERTY_GETTER_SETTER(QString,
								   AppSignalLists::prop_EquipmentIDMasks,
								   true,
								   AppSignalList::equipmentIDMask,
								   AppSignalList::setEquipmentIDMask)
			->setCategory("Masks");

		auto tagsProperty = ADD_PROPERTY_GETTER_SETTER(QString,
													   AppSignalLists::prop_AppSignalTags,
													   true,
													   AppSignalList::appSignalTags,
													   AppSignalList::setAppSignalTags);
		tagsProperty->setCategory("Masks");
		tagsProperty->setSpecificEditor(E::PropertySpecificEditor::Tags);


		return;
	}

	void AppSignalList::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = ::ClassNameHashCode(className);
		message->set_classnamehash(classnamehash);

		::Proto::AppSignalList* appSignalList = message->MutableExtension(::Proto::appSignalList);

		Proto::Write(appSignalList->mutable_uuid(), uuid());
		appSignalList->set_id(id().toUtf8());
		appSignalList->set_caption(caption().toUtf8());
		appSignalList->set_signaltype(static_cast<int>(signalType()));
		appSignalList->set_systemtags(systemTags().toUtf8());
		appSignalList->set_usertags(userTags().toUtf8());

		appSignalList->set_customappsignalidmasks(customAppSignalIDMask().toUtf8());
		appSignalList->set_equipmentidmasks(equipmentIDMask().toUtf8());
		appSignalList->set_appsignalidmasks(appSignalIDMask().toUtf8());
		appSignalList->set_appsignaltags(appSignalTags().toUtf8());

		for (const auto& [hash, signal] : m_items)
		{
			::Proto::AppSignalListSignal* lsi = appSignalList->add_listsignals();

			lsi->set_appsignalid(signal.appSignalId().toUtf8());
			if (signal.hasValue() == true)
			{
				::Proto::TuningValue* tv = lsi->mutable_value();
				signal.value().save(tv);
			}
		}

		for (Hash hash: m_listHashesCache) 
		{
			appSignalList->add_listhashescache(hash);
		}

		return;
	}

	bool AppSignalList::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::appSignalList) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::appSignalList));
			return false;
		}

		const ::Proto::AppSignalList& appSignalList = message.GetExtension(::Proto::appSignalList);

		setUuid(Proto::Read(appSignalList.uuid()));
		setId(QString::fromStdString(appSignalList.id()));
		setCaption(QString::fromStdString(appSignalList.caption()));
		setSignalType(static_cast<SignalType>(appSignalList.signaltype()));
		setSystemTags(QString::fromStdString(appSignalList.systemtags()));
		setUserTags(QString::fromStdString(appSignalList.usertags()));

		setCustomAppSignalIDMask(QString::fromStdString(appSignalList.customappsignalidmasks()));
		setEquipmentIDMask(QString::fromStdString(appSignalList.equipmentidmasks()));
		setAppSignalIDMask(QString::fromStdString(appSignalList.appsignalidmasks()));
		setAppSignalTags(QString::fromStdString(appSignalList.appsignaltags()));

		int count = appSignalList.listsignals_size();
		for (int i = 0; i < count; i++)
		{
			const ::Proto::AppSignalListSignal& signal = appSignalList.listsignals(i);

			AppSignalListItem item(QString::fromStdString(signal.appsignalid()));
			if (signal.has_value() == true)
			{
				const Proto::TuningValue& tv = signal.value();
				TuningValue v(tv);
				item.setValue(v);
			}
			m_items[item.appSignalHash()] = item;
		}

		count = appSignalList.listhashescache_size();
		m_listHashesCache.clear();
		m_listHashesCache.reserve(count);
		for (int i = 0; i < count; i++) 
		{
			m_listHashesCache.push_back(appSignalList.listhashescache(i));
		}

		return true;
	}

	QString AppSignalList::id() const
	{
		return m_id;
	}

	void AppSignalList::setId(const QString& value)
	{
		m_id = value;
	}

	QUuid AppSignalList::uuid() const
	{
		return m_uuid;
	}

	void AppSignalList::setUuid(const QUuid& uuid)
	{
		m_uuid = uuid;
	}

	QString AppSignalList::caption() const
	{
		return m_caption;
	}

	void AppSignalList::setCaption(const QString& value)
	{
		m_caption = value;
	}

	AppSignalList::SignalType AppSignalList::signalType() const
	{
		return m_signalType;
	}

	void AppSignalList::setSignalType(SignalType value)
	{
		m_signalType = value;
	}
		
	QString AppSignalList::systemTags() const
	{
		return m_systemTags.join(';');
	}

	void AppSignalList::setSystemTags(const QString& value)
	{
		if (value.isEmpty() == true)
		{
			m_systemTags.clear();
		}
		else
		{
			m_systemTags = value.split(';', Qt::SkipEmptyParts);
		}
	}

	const QStringList& AppSignalList::systemTagsList() const
	{
		return m_systemTags;
	}

	QStringList& AppSignalList::systemTagsList()
	{
		return m_systemTags;
	}

	QString AppSignalList::userTags() const
	{
		return m_userTags.join(';');
	}

	void AppSignalList::setUserTags(const QString& value)
	{
		if (value.isEmpty() == true)
		{
			m_userTags.clear();
		}
		else
		{
			m_userTags = value.split(';', Qt::SkipEmptyParts);
		}
	}

	const QStringList& AppSignalList::userTagsList() const
	{
		return m_userTags;
	}

	QStringList& AppSignalList::userTagsList()
	{
		return m_userTags;
	}

	bool AppSignalList::hasAnyUserTag(const QStringList& tags) const
	{
		for (const auto& tag : tags)
		{
			if (m_userTags.contains(tag) == true)
			{
				return true;
			}
		}

		return false;
	}

	QString AppSignalList::customAppSignalIDMask() const
	{
		return m_customAppSignalIDMasks;
	}

	void AppSignalList::setCustomAppSignalIDMask(QString value)
	{
		m_customAppSignalIDMasks = value;
		if (value.isEmpty() == true)
		{
			m_cachedCustomAppSignalIDMasks.clear();
		}
		else
		{
			value.replace('\n', ';');
			m_cachedCustomAppSignalIDMasks = value.split(';', Qt::SkipEmptyParts);
		}
	}

	QString AppSignalList::equipmentIDMask() const
	{
		return m_equipmentIDMasks;
	}

	void AppSignalList::setEquipmentIDMask(QString value)
	{
		m_equipmentIDMasks = value;
		if (value.isEmpty() == true)
		{
			m_cachedEquipmentIDMasks.clear();
		}
		else
		{
			value.replace('\n', ';');
			m_cachedEquipmentIDMasks = value.split(';', Qt::SkipEmptyParts);
		}
	}

	QString AppSignalList::appSignalIDMask() const
	{
		return m_appSignalIDMasks;
	}

	void AppSignalList::setAppSignalIDMask(QString value)
	{
		m_appSignalIDMasks = value;
		if (value.isEmpty() == true)
		{
			m_cachedAppSignalIDMasks.clear();
		}
		else
		{
			value.replace('\n', ';');
			m_cachedAppSignalIDMasks = value.split(';', Qt::SkipEmptyParts);
		}
	}

	QString AppSignalList::appSignalTags() const
	{
		return m_appSignalTags;
	}

	void AppSignalList::setAppSignalTags(QString value)
	{
		m_appSignalTags = value;
		if (value.isEmpty() == true)
		{
			m_cachedAppSignalTags.clear();
		}
		else
		{
			value.replace('\n', ';');
			m_cachedAppSignalTags = value.split(';', Qt::SkipEmptyParts);
		}
	}

	int AppSignalList::count() const
	{
		return static_cast<int>(m_items.size());
	}

	std::vector<Hash> AppSignalList::itemsHashes() const 
	{
		std::vector<Hash> result;
		result.reserve(m_items.size());
		for (auto& [hash, item]: m_items) 
		{
			result.push_back(hash);
		}
		return result;
	}

	const AppSignalListItem& AppSignalList::itemByHash(Hash hash) const
	{
		auto it = m_items.find(hash);
		if (it == m_items.end())
		{
			static AppSignalListItem err;
			Q_ASSERT(false);
			return err;
		}
		return it->second;
	}

	AppSignalListItem& AppSignalList::itemByHash(Hash hash)
	{
		auto it = m_items.find(hash);
		if (it == m_items.end())
		{
			static AppSignalListItem err;
			Q_ASSERT(false);
			return err;
		}
		return it->second;
	}

	bool AppSignalList::itemExists(Hash hash) const
	{
		return m_items.find(hash) != m_items.end();
	}

	/*int AppSignalList::itemIndex(Hash hash) const
	{
		auto it = std::find_if(m_items.begin(),
							   m_items.end(),
							   [hash](const auto& item)
							   {
								   return item.appSignalHash() == hash;
							   });
		if (it == m_items.end())
		{
			Q_ASSERT(false);
			return -1;
		}
		return std::distance(m_items.begin(), it);
	}*/

	void AppSignalList::add(const AppSignalListItem& item)
	{
		m_items[item.appSignalHash()] = item;
	}

	/*bool AppSignalList::remove(int index)
	{
		if (index >= m_items.size())
		{
			Q_ASSERT(false);
			return false;
		}
		m_items.erase(m_items.begin() + index);
		return true;
	}*/

	void AppSignalList::remove(Hash hash)
	{
		m_items.erase(hash);
	}

	const std::vector<Hash>& AppSignalList::listHashesCache() const 
	{
		return m_listHashesCache;
	}
	
	std::vector<Hash>& AppSignalList::listHashesCache() 
	{
		return m_listHashesCache;
	}

	bool AppSignalList::appSignalMatch(const AppSignalParam& asp) const
	{
		if (itemExists(asp.hash()) == true)
		{
			return true;
		}

		if (signalType() == AppSignalList::SignalType::Analog && asp.isAnalog() == false)
		{
			return false;
		}

		if (signalType() == AppSignalList::SignalType::Discrete && asp.isDiscrete() == false)
		{
			return false;
		}

		if (m_cachedEquipmentIDMasks.isEmpty() == false && processMaskList(asp.lmEquipmentId(), m_cachedEquipmentIDMasks) == true)
		{
			return true;
		}

		if (m_cachedAppSignalIDMasks.isEmpty() == false && processMaskList(asp.appSignalId(), m_cachedAppSignalIDMasks) == true)
		{
			return true;
		}

		if (m_cachedCustomAppSignalIDMasks.isEmpty() == false && processMaskList(asp.customSignalId(), m_cachedCustomAppSignalIDMasks) == true)
		{
			return true;
		}

		if (m_cachedAppSignalTags.isEmpty() == false)
		{
			bool tagsFound = false;

			const auto& objectTagsSet = asp.tags();

			for (const QString& tag : m_cachedAppSignalTags)
			{
				if (objectTagsSet.find(tag) != objectTagsSet.end())
				{
					tagsFound = true;
					break;
				}
			}

			if (tagsFound == false)
			{
				return false;
			}
		}

		return false;
	}

	bool AppSignalList::listMatch(const QStringList& appSignalListIds, const QStringList& appSignalListMasks, const QStringList& appSignalListTags) 
	{
		if (std::find(appSignalListIds.begin(), appSignalListIds.end(), id()) != appSignalListIds.end())
		{
			// Filter by lists
			//
			return true;
		}

		// Filter by masks
		//
		if (std::find_if(appSignalListMasks.begin(),
						 appSignalListMasks.end(),
						 [this](const QString& mask)
						 {
							 QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(mask));
							 auto matchResult = rx.match(id());
							 return matchResult.hasMatch() == true;
						 }) != appSignalListMasks.end())
		{
			return true;
		}

		// Filter by tags
		//
		if (hasAnyUserTag(appSignalListTags) == true)
		{
			return true;
		}

		return false;
	}	

	bool AppSignalList::processMaskList(const QString& s, const QStringList& masks)
	{
		if (masks.isEmpty() == true)
		{
			return true;
		}

		int directCount = 0;
		int directMatch = 0;
		int invertedCount = 0;
		int invertedMatch = 0;

		for (QString m : masks)
		{
			m = m.trimmed();

			if (m.isEmpty() == true)
			{
				continue;
			}

			bool invertMask = m.contains('!');
			m.remove('!');

			thread_local std::map<QString, QRegularExpression> cache;

			if (cache.find(m) == cache.end())
			{
				cache[m] = QRegularExpression(QRegularExpression::wildcardToRegularExpression(m));
			}

			const QRegularExpression& rx = cache[m];

			if (invertMask == false)
			{
				directCount++;

				auto matchResult = rx.match(s);
				if (matchResult.hasMatch() == true)
				{
					directMatch++;
				}
			}
			else
			{
				// invertMask == true
				//
				invertedCount++;

				auto matchResult = rx.match(s);
				if (matchResult.hasMatch() == false)
				{
					invertedMatch++;
				}
			}
		}

		bool result = true;

		if (directCount != 0 && directMatch == 0)
		{
			result = false;
		}

		if (invertedCount != 0 && invertedCount != invertedMatch)
		{
			result = false;
		}

		return result;
	}




	void AppSignalListSet::clear() 
	{
		m_lists.clear();
	}

	int AppSignalListSet::count() const
	{
		return static_cast<int>(m_lists.size());
	}

	bool AppSignalListSet::add(const QByteArray& ba) 
	{
		std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();

		Proto::Envelope envelope;
		if (envelope.ParseFromArray(ba.constData(), static_cast<int>(ba.size())) == false)
		{
			Q_ASSERT(false);
			return false;
		}

		bool ok = list->LoadData(envelope);
		if (ok == false)
		{
			Q_ASSERT(false);
			return false;
		}

		m_lists.push_back(list);

		return true;
	}

	bool AppSignalListSet::add(std::shared_ptr<AppSignalList> list)
	{
		m_lists.push_back(list);
		return true;
	}

	bool AppSignalListSet::add(const AppSignalListSet& appSignalListSet) 
	{
		for (const auto& list : appSignalListSet.m_lists) 
		{
			add(list);
		}
		return true;
	}

	std::shared_ptr<AppSignalList> AppSignalListSet::get(int index) const
	{
		if (index < 0 || index >= count()) 
		{
			Q_ASSERT(false);
			return nullptr;
		}
		return m_lists[index];
	}

	std::shared_ptr<AppSignalList> AppSignalListSet::get(const QString& id) const
	{
		auto it = std::find_if(m_lists.begin(),
							   m_lists.end(),
							   [id](const auto& list)
							   {
								   return list->id() == id;
							   });
		if (it == m_lists.end()) 
		{
			return nullptr;
		}
		return *it;
	}

	std::shared_ptr<AppSignalList> AppSignalListSet::get(const QUuid& uuid) const
	{
		auto it = std::find_if(m_lists.begin(),
							   m_lists.end(),
							   [uuid](const auto& list)
							   {
								   return list->uuid() == uuid;
							   });
		if (it == m_lists.end()) 
		{
			return nullptr;
		}
		return *it;
	}

	void AppSignalListSet::remove(const QUuid& uuid) 
	{
		std::erase_if(m_lists,
					  [uuid](const auto& it)
					  {
						  return it->uuid() == uuid;
					  });
	}

	void AppSignalListSet::remove(const QString& systemTag) 
	{
		std::erase_if(m_lists,
					  [systemTag](const auto& it)
					  {
						  return it->systemTagsList().contains(systemTag) == true;
					  });
	}

	std::vector<AppSignalList*> AppSignalListSet::lists() const 
	{
		std::vector<AppSignalList*> result;
		
		result.reserve(m_lists.size());
		for (const auto& list: m_lists) 
		{
			result.push_back(list.get());
		}

		return result;
	}

	bool AppSignalListSet::load(QString* errorMessage) 
	{
		Q_ASSERT(false);
		return false;
	}

	bool AppSignalListSet::save(QString* errorMessage) const 
	{
		Q_ASSERT(false);
		return false;
	}

	void AppSignalListSet::fireUpdatePerformed() 
	{
		emit updatePerformed();
	}

} // namespace AppSignalLists