#include "./include/AppSignalLists/SignalList.h"

namespace AppSignalLists
{
	const char* AppSignalList::mimeType = "application/x-radiyappsignallist";

	AppSignalListItem::AppSignalListItem(const QString& appSignalId):
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
		ADD_PROPERTY_GETTER_SETTER(SignalType, AppSignalLists::prop_SignalType, true, AppSignalList::signalType, AppSignalList::setSignalType);
		ADD_PROPERTY_GETTER_SETTER(QString, AppSignalLists::prop_ID, true, AppSignalList::id, AppSignalList::setId);
		ADD_PROPERTY_GETTER_SETTER(QString, AppSignalLists::prop_Tags, true, AppSignalList::tags, AppSignalList::setTags);

		auto propMask = ADD_PROPERTY_GETTER_SETTER(QString,
			AppSignalLists::prop_CustomAppSignalMasks,
			true,
			AppSignalList::customAppSignalIDMask,
			AppSignalList::setCustomAppSignalIDMask);
		propMask->setCategory("Masks");

		propMask = ADD_PROPERTY_GETTER_SETTER(QString,
			AppSignalLists::prop_AppSignalMasks,
			true,
			AppSignalList::appSignalIDMask,
			AppSignalList::setAppSignalIDMask);
		propMask->setCategory("Masks");

		propMask = ADD_PROPERTY_GETTER_SETTER(QString,
			AppSignalLists::prop_EquipmentIDMasks,
			true,
			AppSignalList::equipmentIDMask,
			AppSignalList::setEquipmentIDMask);
		propMask->setCategory("Masks");

		propMask = ADD_PROPERTY_GETTER_SETTER(QString,
			AppSignalLists::prop_AppSignalTags,
			true,
			AppSignalList::appSignalTags,
			AppSignalList::setAppSignalTags);
		propMask->setCategory("Masks");
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
		appSignalList->set_tags(tags().toUtf8());

		appSignalList->set_customappsignalidmasks(customAppSignalIDMask().toUtf8());
		appSignalList->set_equipmentidmasks(equipmentIDMask().toUtf8());
		appSignalList->set_appsignalidmasks(appSignalIDMask().toUtf8());
		appSignalList->set_appsignaltags(appSignalTags().toUtf8());

		for (const AppSignalListItem& signal: m_items)
		{
			::Proto::AppSignalListSignal* lsi = appSignalList->add_listsignals();

			lsi->set_appsignalid(signal.appSignalId().toUtf8());
			if (signal.hasValue() == true)
			{
				::Proto::TuningValue* tv = lsi->mutable_value();
				signal.value().save(tv);
			}
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
		setTags(QString::fromStdString(appSignalList.tags()));

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
			m_items.push_back(item);
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

	QString AppSignalList::tags() const
	{
		QString result;
		for (const auto& s : m_tags)
		{
			result += s + ';';
		}
		result.remove(result.length() - 1, 1);

		return result;
	}

	void AppSignalList::setTags(const QString& value)
	{
		if (value.isEmpty() == true)
		{
			m_tags.clear();
		}
		else
		{
			m_tags = value.split(';', Qt::SkipEmptyParts);
		}
	}

	const QStringList& AppSignalList::tagsList() const
	{
		return m_tags;
	}

	bool AppSignalList::hasAnyTag(const QStringList& tags) const
	{
		for (const auto& tag : tags)
		{
			if (m_tags.contains(tag) == true)
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

	const AppSignalListItem& AppSignalList::operator[](int index) const
	{
		static AppSignalListItem err;
		if (index < 0 || index >= m_items.size())
		{
			Q_ASSERT(false);
			return err;
		}
		return m_items[index];
	}

	AppSignalListItem& AppSignalList::operator[](int index)
	{
		static AppSignalListItem err;
		if (index < 0 || index >= m_items.size())
		{
			Q_ASSERT(false);
			return err;
		}
		return m_items[index];
	}

	const AppSignalListItem& AppSignalList::operator[](Hash hash) const
	{
		static AppSignalListItem err;
		auto it = std::find_if(m_items.begin(), m_items.end(), [hash](const auto& item) {return item.appSignalHash() == hash; });
		if (it == m_items.end())
		{
			Q_ASSERT(false);
			return err;
		}
		return *it;
	}

	AppSignalListItem& AppSignalList::operator[](Hash hash)
	{
		static AppSignalListItem err;
		auto it = std::find_if(m_items.begin(), m_items.end(), [hash](const auto& item) {return item.appSignalHash() == hash; });
		if (it == m_items.end())
		{
			Q_ASSERT(false);
			return err;
		}
		return *it;
	}

	bool AppSignalList::itemExists(Hash hash) const
	{
		auto it = std::find_if(m_items.begin(), m_items.end(), [hash](const auto& item) {return item.appSignalHash() == hash; });
		return it != m_items.end();
	}

	int AppSignalList::itemIndex(Hash hash) const
	{
		auto it = std::find_if(m_items.begin(), m_items.end(), [hash](const auto& item) {return item.appSignalHash() == hash; });
		if (it == m_items.end())
		{
			Q_ASSERT(false);
			return -1;
		}
		return std::distance(m_items.begin(), it);
	}

	void AppSignalList::add(const AppSignalListItem& item)
	{
		m_items.push_back(item);
	}

	bool AppSignalList::remove(int index)
	{
		if (index >= m_items.size())
		{
			Q_ASSERT(false);
			return false;
		}
		m_items.erase(m_items.begin() + index);
		return true;
	}

	bool AppSignalList::remove(Hash hash)
	{
		auto it = std::find_if(m_items.begin(), m_items.end(), [hash](const auto& item) {return item.appSignalHash() == hash; });
		if (it == m_items.end())
		{
			Q_ASSERT(false);
			return false;
		}
		m_items.erase(it);
		return true;
	}

	bool AppSignalList::match(const AppSignalParam& param) const
	{
		if (itemExists(param.hash()) == true)
		{
			return true;
		}

		if (signalType() == AppSignalList::SignalType::Analog && param.isAnalog() == false)
		{
			return false;
		}

		if (signalType() == AppSignalList::SignalType::Discrete && param.isAnalog() == true)
		{
			return false;
		}

		if (processMaskList(param.lmEquipmentId(), m_cachedEquipmentIDMasks) == false)
		{
			return false;
		}

		if (processMaskList(param.appSignalId(), m_cachedAppSignalIDMasks) == false)
		{
			return false;
		}

		if (processMaskList(param.customSignalId(), m_cachedCustomAppSignalIDMasks) == false)
		{
			return false;
		}

		if (m_cachedAppSignalTags.isEmpty() == false)
		{
			bool tagsFound = false;

			auto objectTags = param.tags();

			for (const QString& tag : m_cachedAppSignalTags)
			{
				if (objectTags.find(tag) != objectTags.end())
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

		return true;
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
			if (m.isEmpty() == true)
			{
				continue;
			}

			bool invertMask = m.contains('!');
			m.remove('!');

			QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(m.trimmed()));

			if (invertMask == false)
			{
				directCount++;

				auto matchResult = rx.match(s);
				if (matchResult.hasMatch() == true)
				{
					directMatch++;
				}
			}

			if (invertMask == true)
			{
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
}