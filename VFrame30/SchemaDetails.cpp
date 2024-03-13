#include "SchemaDetails.h"
#include "IMatsSchemaItemAssociations.h"
#include "./SchemaItems/SchemaItemAfb.h"
#include "./SchemaItems/SchemaItemIndicator.h"
#include "LogicSchema.h"
#include "PropertyNames.h"
#include "Schema.h"
#include "UfbSchema.h"

namespace VFrame30
{
	//
	//
	//				SchemaDetails
	//
	//
	SchemaDetails::SchemaDetails(const QString& details)
	{
		parseDetails(details);
	}

	bool SchemaDetails::operator< (const SchemaDetails& b) const
	{
		return this->m_schemaId < b.m_schemaId;
	}

	QString SchemaDetails::getDetailsString(const Schema* schema, const QString& path)
	{
		if (schema == nullptr)
		{
			assert(schema);
			return QString();
		}

		// form details JSON object (signal list)
		//
		QJsonObject jsonObject;

		// Get signalIds
		//
		QStringList signalIdsList = schema->getSignalList();
		QSet<QString> signalIds{signalIdsList.begin(), signalIdsList.end()};

		// Get labels for AFBs
		//
		QStringList labels = schema->getLabels();

		// Get list of receivers/transmitters and loopbacks
		//
		QSet<QString> connections;
		QSet<QString> loopbacks;
		QSet<QString> packedLogicIds;
		std::vector<TrendIndicatorSchemaItems> realTimeTrends;

		for (const std::shared_ptr<SchemaLayer>& layer : schema->layers())
		{
			if (layer->compile() == true)
			{
				for (const auto& item : layer->items())
				{
					// Items on LogicSchemas
					//
					if (const auto* associations = dynamic_cast<const IMatsSchemaItemAssociations*>(item.get());
						associations != nullptr)
					{
						for (const auto& connectionId : associations->associatedConnectionIds())
						{
							connections.insert(connectionId);
						}

						for (const auto& loopbackId : associations->associatedLoopbackIds())
						{
							loopbacks.insert(loopbackId);
						}
					}

					if (const SchemaItemAfb* afb = item->toType<SchemaItemAfb>();
						afb != nullptr && afb->isPackedLogic() == true)
					{
						packedLogicIds << afb->packedLogicId();
					}

					// Items on all other schemas
					//
					if (const SchemaItemIndicator* ii = item->toType<SchemaItemIndicator>();
						ii != nullptr &&
						ii->isTrend() == true)
					{
						realTimeTrends.emplace_back(ii->guid(),
													ii->trendSamplePeriod(),
													ii->trendTimeType(),
													ii->trendDurationSeconds(),
													ii->signalIds());
					}
				}

				break;
			}
		}

		// Get schema tags, kept in lowercase
		//
		QStringList schemaTags = schema->tagsAsList();
		for (QString& tag : schemaTags)
		{
			tag = tag.toLower();
		}

		// Get item tags, kept in lowercase
		//
		QStringList itemTags = schema->itemTags().toList();
		for (QString& tag : itemTags)
		{
			tag = tag.toLower();
		}

		// Get a list of guids
		//
		std::vector<QUuid> guids = schema->getGuids();

		QStringList guidsStringList;
		guidsStringList.reserve(static_cast<int>(guids.size()));

		for (const QUuid uuid : guids)
		{
			guidsStringList.push_back(uuid.toString());
		}

		// Form JSon object
		//
		QVariant signaListVariant(signalIds.values());
		QVariant labelsVariant(labels);
		QVariant connectionsVariant(connections.values());
		QVariant loopbacksVariant(loopbacks.values());
		QVariant tagsVariant(schemaTags);
		QVariant packedLogicIdsVariant(packedLogicIds.values());
		QVariant itemTagsVariant(itemTags);
		QVariant guidsVariant(guidsStringList);

		jsonObject.insert("Version", QJsonValue(1));
		jsonObject.insert("SchemaID", QJsonValue(schema->schemaId()));
		jsonObject.insert("Caption", QJsonValue(schema->caption()));
		jsonObject.insert("ExcludedFromBuild", QJsonValue(schema->excludeFromBuild()));

		if (schema->isLogicSchema() == true)
		{
			const LogicSchema* logicSchema = schema->toLogicSchema();
			assert(logicSchema);

			QString equipIds = logicSchema->equipmentIds();
			equipIds = equipIds.replace('\n', ' ');
			jsonObject.insert("EquipmentID", QJsonValue(equipIds));

			jsonObject.insert(PropertyNames::lmDescriptionFile, QJsonValue(logicSchema->lmDescriptionFile()));
		}

		if (schema->isUfbSchema() == true)
		{
			const UfbSchema* ufbSchema = schema->toUfbSchema();
			assert(ufbSchema);

			jsonObject.insert(PropertyNames::lmDescriptionFile, QJsonValue(ufbSchema->lmDescriptionFile()));
		}

		jsonObject.insert("Path", QJsonValue(path));
		jsonObject.insert("Signals", QJsonValue::fromVariant(signaListVariant));
		jsonObject.insert("Labels", QJsonValue::fromVariant(labelsVariant));
		jsonObject.insert("Connections", QJsonValue::fromVariant(connectionsVariant));
		jsonObject.insert("Loopbacks", QJsonValue::fromVariant(loopbacksVariant));
		jsonObject.insert("Tags", QJsonValue::fromVariant(tagsVariant));
		jsonObject.insert("PackedLogicIds", QJsonValue::fromVariant(packedLogicIdsVariant));
		jsonObject.insert("ItemTags", QJsonValue::fromVariant(itemTagsVariant));
		jsonObject.insert("ItemGuids", QJsonValue::fromVariant(guidsVariant));

		QJsonArray jsontrendsIndicators;
		for (const TrendIndicatorSchemaItems& trendItem : realTimeTrends)
		{
			jsontrendsIndicators.push_back(trendItem.toJsonObject());
		}
		jsonObject["RealTimeTrendItems"] = jsontrendsIndicators;

		// Convert json to string and return it
		//
		QJsonDocument jsonDoc(jsonObject);

		QByteArray data = jsonDoc.toJson(QJsonDocument::JsonFormat::Indented);
		QString result = QString::fromUtf8(data);

		return result;
	}

	bool SchemaDetails::parseDetails(const QString& details)
	{
		if (details.trimmed().isEmpty() == true)
		{
			*this = {};
			return true;
		}

		// parse details section (from DB), result is signal list
		//
		QByteArray data = details.toUtf8();

		QJsonParseError parseError;
		QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

		if (parseError.error != QJsonParseError::NoError)
		{
			qDebug() << "Schema details parsing error: " << parseError.errorString();
			qDebug() << "JSON document: " << details;

			*this = {};
			return false;
		}

		if (jsonDoc.isObject() == false)
		{
			assert(jsonDoc.isObject());		// have a look at json doc, it is supposed to be an object
			qDebug() << Q_FUNC_INFO << " json is supposed to be object";

			*this = {};
			return false;
		}

		QJsonObject jsonObject = jsonDoc.object();

		QJsonValue version = jsonObject.value(QLatin1String("Version"));
		int versionInt = version.toInt(-1);

		if (versionInt == -1 ||
			version.type() != QJsonValue::Double)
		{
			*this = {};
			return false;
		}

		switch (versionInt)
		{
		case 1:
			{
				// SchemaID
				//
				m_schemaId = jsonObject.value(QLatin1String("SchemaID")).toString();

				// Caption
				//
				m_caption = jsonObject.value(QLatin1String("Caption")).toString();

				// ExcludedFromBuild
				//
				QJsonValue excFromBuild = jsonObject.value(QLatin1String("ExcludedFromBuild"));
				if (excFromBuild.isUndefined() == false && excFromBuild.isBool() == true)
				{
					m_excludedFromBuild = excFromBuild.toBool();
				}
				else
				{
					m_excludedFromBuild = false;
				}

				// EquipmentID
				//
				QJsonValue eqidValue = jsonObject.value(QLatin1String("EquipmentID")).toString();
				if (eqidValue.isUndefined() == false)
				{
					m_equipmentId = eqidValue.toString();
				}
				else
				{
					m_equipmentId.clear();
				}

				// LmDescriptionFile
				//
				QJsonValue lmdescrValue = jsonObject.value(PropertyNames::lmDescriptionFile).toString();
				if (lmdescrValue.isUndefined() == false)
				{
					m_lmDescriptionFile = lmdescrValue.toString();
				}
				else
				{
					m_lmDescriptionFile.clear();
				}

				// Path
				//
				m_path = jsonObject.value(QLatin1String("Path")).toString();

				// Signals
				//
				m_signals.clear();
				QStringList signalsStrings = jsonObject.value(QLatin1String("Signals")).toVariant().toStringList();

				for (const QString& str : signalsStrings)
				{
					m_signals.insert(str);
				};

				// Labels
				//
				m_labels.clear();
				QStringList labelList = jsonObject.value(QLatin1String("Labels")).toVariant().toStringList();

				for (const QString& str : labelList)
				{
					m_labels.insert(str);
				}

				// Connections
				//
				m_connections.clear();
				QStringList connList = jsonObject.value(QLatin1String("Connections")).toVariant().toStringList();

				for (const QString& str : connList)
				{
					m_connections.insert(str);
				}

				// Loopbacks
				//
				m_loopbacks.clear();
				QStringList loopbackList = jsonObject.value(QLatin1String("Loopbacks")).toVariant().toStringList();

				for (const QString& str : loopbackList)
				{
					m_loopbacks.insert(str);
				}

				// Schema Tags
				//
				{
					m_schemaTags.clear();
					QStringList tagsList = jsonObject.value(QLatin1String("Tags")).toVariant().toStringList();

					for (const QString& str : tagsList)
					{
						m_schemaTags.insert(str);
					}
				}

				// SchemaItems Tags
				//
				{
					m_itemTags.clear();
					QStringList itemTagsList = jsonObject.value(QLatin1String("ItemTags")).toVariant().toStringList();

					for (const QString& str : itemTagsList)
					{
						m_itemTags.insert(str.toLower());
					}
				}

				// m_packedLogicIds
				//
				{
					m_packedLogicIds.clear();
					QStringList packedIdList = jsonObject.value(QLatin1String("PackedLogicIds")).toVariant().toStringList();

					for (const QString& str : packedIdList)
					{
						m_packedLogicIds.insert(str);
					}
				}

				// ItemGuids
				//
				{
					m_guids.clear();

					QStringList guidList = jsonObject.value(QLatin1String("ItemGuids")).toVariant().toStringList();
					std::for_each(guidList.begin(), guidList.end(), [this](const QString& str) {	m_guids.insert(QUuid(str)); });
				}

				// m_trendsIndicators
				//
				{
					m_trendsIndicators.clear();
					QJsonArray jsa = jsonObject.value(QLatin1String("RealTimeTrendItems")).toArray();

					for (QJsonValueRef v : jsa)
					{
						Q_ASSERT(v.isObject() == true);
						m_trendsIndicators.emplace_back().fromJsonObject(v.toObject());
					}
				}
			}
			break;
		default:
			assert(false);
			*this = {};
			return false;
		}

		return true;
	}

	bool SchemaDetails::saveData(Proto::SchemaDetails* message) const
	{
		if (message == nullptr)
		{
			assert(message);
			return false;
		}

		message->set_version(m_version);
		message->set_schemaid(m_schemaId.toStdString());
		message->set_caption(m_caption.toStdString());
		message->set_excludedfrombuild(m_excludedFromBuild);
		message->set_equipmentid(m_equipmentId.toStdString());
		message->set_lmdescriptionfile(m_lmDescriptionFile.toStdString());
		message->set_path(m_path.toStdString());

		for (const QString& s : m_signals)
		{
			message->add_signalids(s.toStdString());
		}

		for (const QString& l : m_labels)
		{
			message->add_labels(l.toStdString());
		}

		for (const QString& c : m_connections)
		{
			message->add_connections(c.toStdString());
		}

		for (const QString& l : m_loopbacks)
		{
			message->add_loopbacks(l.toStdString());
		}

		for (const QString& t : m_schemaTags)
		{
			message->add_schematags(t.toStdString());
		}

		for (const QString& t : m_itemTags)
		{
			message->add_itemtags(t.toStdString());
		}

		for (const QString& t : m_packedLogicIds)
		{
			message->add_packedlogicids(t.toStdString());
		}

		for (const QUuid& u : m_guids)
		{
			::Proto::Uuid* uuidMesage = message->add_guids();
			assert(uuidMesage);

			uuidMesage->set_uuid(&u, sizeof(u));
		}

		for (const auto& ti : m_trendsIndicators)
		{
			::Proto::TrendIndicatorSchemaItems* trendsIndicators = message->add_trendindicators();
			ti.saveData(trendsIndicators);
		}

		return true;
	}

	bool SchemaDetails::loadData(const Proto::SchemaDetails& message)
	{
		m_version = message.version();
		m_schemaId = QString::fromStdString(message.schemaid());
		m_caption = QString::fromStdString(message.caption());
		m_excludedFromBuild = message.excludedfrombuild();
		m_equipmentId = QString::fromStdString(message.equipmentid());
		m_lmDescriptionFile = QString::fromStdString(message.lmdescriptionfile());
		m_path = QString::fromStdString(message.path());

		m_signals.clear();
		int signalsCount = message.signalids_size();
		for (int i = 0; i < signalsCount; i++)
		{
			QString signalId = QString::fromStdString(message.signalids(i));
			m_signals.insert(signalId);
		}

		m_labels.clear();
		int labelCount = message.labels_size();
		for (int i = 0; i < labelCount; i++)
		{
			QString label = QString::fromStdString(message.labels(i));
			m_labels.insert(label);
		}

		m_connections.clear();
		int connectionCount = message.connections_size();
		for (int i = 0; i < connectionCount; i++)
		{
			QString conn = QString::fromStdString(message.connections(i));
			m_connections.insert(conn);
		}

		m_loopbacks.clear();
		int loopbackCount = message.loopbacks_size();
		for (int i = 0; i < loopbackCount; i++)
		{
			QString lb = QString::fromStdString(message.loopbacks(i));
			m_loopbacks.insert(lb);
		}

		m_schemaTags.clear();
		for (int i = 0, tagCount = message.schematags_size(); i < tagCount; i++)
		{
			QString tag = QString::fromStdString(message.schematags(i));
			m_schemaTags.insert(tag);
		}

		m_itemTags.clear();
		for (int i = 0, tagCount = message.itemtags_size(); i < tagCount; i++)
		{
			QString tag = QString::fromStdString(message.itemtags(i)).toLower();
			m_itemTags.insert(tag);
		}

		m_packedLogicIds.clear();
		for (int i = 0, size = message.packedlogicids_size(); i < size; i++)
		{
			m_packedLogicIds.insert(QString::fromStdString(message.packedlogicids(i)));
		}

		m_guids.clear();
		int guidCount = message.guids_size();
		for (int i = 0; i < guidCount; i++)
		{
			QUuid guid = Proto::Read(message.guids(i));
			m_guids.insert(guid);
		}

		{
			m_trendsIndicators.clear();
			int tiCount = message.trendindicators_size();
			for (int i = 0; i < tiCount; i++)
			{
				const auto& tiMessage = message.trendindicators(i);

				TrendIndicatorSchemaItems& trendSchemaItem = m_trendsIndicators.emplace_back();
				trendSchemaItem.loadData(tiMessage);
			}
		}

		return true;
	}

	bool SchemaDetails::searchForString(const QString& searchText) const
	{
		if (m_schemaId.contains(searchText, Qt::CaseInsensitive) == true)
		{
			return true;
		}

		if (m_caption.contains(searchText, Qt::CaseInsensitive) == true)
		{
			return true;
		}

		if (m_equipmentId.contains(searchText, Qt::CaseInsensitive) == true)
		{
			return true;
		}

		if (m_signals.find(searchText) != m_signals.end())
		{
			return true;
		}

		if (m_labels.find(searchText) != m_labels.end())
		{
			return true;
		}

		if (m_connections.find(searchText) != m_connections.end())
		{
			return true;
		}

		if (m_loopbacks.find(searchText) != m_loopbacks.end())
		{
			return true;
		}

		if (m_itemTags.contains(searchText.toLower()) == true)
		{
			return true;
		}

		if (m_packedLogicIds.contains(searchText) == true)
		{
			return true;
		}

		QUuid textAsUuid(searchText);

		if (textAsUuid.isNull() == false &&
			m_guids.find(textAsUuid) != m_guids.end())
		{
			return true;
		}

		return false;
	}

	bool SchemaDetails::isNull() const
	{
		return m_schemaId.isEmpty();
	}

	bool SchemaDetails::hasSchemaTag(const QString& tag) const
	{
		return m_schemaTags.find(tag.trimmed().toLower()) != m_schemaTags.end();
	}

	bool SchemaDetails::hasSchemaTag(const QStringList& tags) const
	{
		for (const QString& tag : tags)
		{
			if (m_schemaTags.find(tag.trimmed().toLower()) != m_schemaTags.end())
			{
				return true;
			}
		}

		return false;
	}

	const std::set<QString>& SchemaDetails::schemaTags() const
	{
		return m_schemaTags;
	}

	bool SchemaDetails::hasEquipmentId(const QString& equipmentId) const
	{
		QStringList eqs = m_equipmentId.split(' ', Qt::SkipEmptyParts);

		bool result = eqs.contains(equipmentId, Qt::CaseInsensitive);
		return result;
	}

	bool SchemaDetails::hasSignal(const QString& signalId) const
	{
		return m_signals.contains(signalId);
	}

	bool SchemaDetails::hasLoopback(const QString& loopbackId) const
	{
		return m_loopbacks.contains(loopbackId);
	}

	bool SchemaDetails::hasConnection(const QString& connectionId) const
	{
		return m_connections.contains(connectionId);
	}

	bool SchemaDetails::hasLabel(const QString& label) const
	{
		return m_labels.contains(label);
	}

	bool SchemaDetails::hasGuid(QUuid guid) const
	{
		return m_guids.contains(guid);
	}

	SchemaDetails::TrendIndicatorSchemaItems::TrendIndicatorSchemaItems(QUuid _itemUuid,
																		E::RtTrendsSamplePeriod _samplePeriod,
																		E::TimeType _timeType,
																		int _durationSeconds,
																		const QStringList& _appSignalIds) :
		itemUuid(_itemUuid),
		samplePeriod(_samplePeriod),
		timeType(_timeType),
		durationSeconds(_durationSeconds),
		appSignalIds(_appSignalIds)
	{
	}

	QJsonObject SchemaDetails::TrendIndicatorSchemaItems::toJsonObject() const
	{
		QJsonObject jsonObject;

		jsonObject.insert(QLatin1String{"itemUuid"}, QJsonValue{itemUuid.toString()});
		jsonObject.insert(QLatin1String{"samplePeriod"}, QJsonValue{static_cast<int>(samplePeriod)});
		jsonObject.insert(QLatin1String{"timeType"}, QJsonValue{static_cast<int>(timeType)});
		jsonObject.insert(QLatin1String{"appSignalIds"}, QJsonValue{appSignalIds.join(", ")});
		jsonObject.insert(QLatin1String{"durationSeconds"}, QJsonValue{durationSeconds});

		return jsonObject;
	}

	bool SchemaDetails::TrendIndicatorSchemaItems::fromJsonObject(const QJsonObject& jsonObject)
	{
		itemUuid = QUuid::fromString(jsonObject.value(QLatin1String{"itemUuid"}).toString());
		samplePeriod = static_cast<E::RtTrendsSamplePeriod>(jsonObject.value(QLatin1String{"samplePeriod"}).toInt());
		timeType = static_cast<E::TimeType>(jsonObject.value(QLatin1String{"timeType"}).toInt());
		durationSeconds = jsonObject.value(QLatin1String{"durationSeconds"}).toInt();

		appSignalIds = jsonObject.value(QLatin1String{"appSignalIds"}).toString().split(',', Qt::SkipEmptyParts);
		for (QString& appSignal : appSignalIds)
		{
			appSignal = appSignal.trimmed();
		}

		return true;
	}

	bool SchemaDetails::TrendIndicatorSchemaItems::saveData(::Proto::TrendIndicatorSchemaItems* message) const
	{
		Proto::Write(message->mutable_itemuuid(), itemUuid);
		message->set_sampleperiod(static_cast<int>(samplePeriod));
		message->set_timetype(static_cast<int>(timeType));
		message->set_durationseconds(durationSeconds);

		for (const QString& s : appSignalIds)
		{
			message->add_appsignalids(s.toStdString());
		}

		return true;
	}

	bool SchemaDetails::TrendIndicatorSchemaItems::loadData(const ::Proto::TrendIndicatorSchemaItems& message)
	{
		itemUuid = Proto::Read(message.itemuuid());
		samplePeriod = static_cast<E::RtTrendsSamplePeriod>(message.sampleperiod());
		timeType = static_cast<E::TimeType>(message.timetype());
		durationSeconds = message.durationseconds();

		appSignalIds.clear();
		for (const auto& sm : message.appsignalids())
		{
			appSignalIds.push_back(QString::fromStdString(sm));
		}

		return true;
	}

	SchemaDetailsSet::SchemaDetailsSet() :
		Proto::ObjectSerialization<SchemaDetailsSet>(Proto::ProtoCompress::Auto)
	{
	}

	bool SchemaDetailsSet::SaveData(Proto::Envelope* envelopeMessage) const
	{
		std::string className = {"SchemaDetailsSet"};
		quint32 classNameHash = ::ClassNameHashCode(className);
		envelopeMessage->set_classnamehash(classNameHash);

		::Proto::SchemaDetailsSet* setMessage = envelopeMessage->MutableExtension(Proto::schemaDetailsSet);

		for (auto detailsPair : m_details)
		{
			::Proto::SchemaDetails* detailsMessage = setMessage->add_schemasdetails();
			detailsPair.second->saveData(detailsMessage);
		}

		return true;
	}

	bool SchemaDetailsSet::LoadData(const Proto::Envelope& message)
	{
		clear();

		if (message.HasExtension(Proto::schemaDetailsSet) == false)
		{
			assert(message.HasExtension(Proto::schemaDetailsSet));
			return false;
		}

		const Proto::SchemaDetailsSet& setMessage = message.GetExtension(Proto::schemaDetailsSet);

		int detailsCount = setMessage.schemasdetails_size();
		for (int i = 0; i < detailsCount; i++)
		{
			std::shared_ptr<SchemaDetails> details = std::make_shared<SchemaDetails>();
			bool loadOk = details->loadData(setMessage.schemasdetails(i));

			if (loadOk == true)
			{
				add(details);
			}
		}

		return true;
	}

	std::shared_ptr<SchemaDetailsSet> SchemaDetailsSet::CreateObject(const Proto::Envelope& message)
	{
		std::shared_ptr<SchemaDetailsSet> object = std::make_shared<SchemaDetailsSet>();
		object->LoadData(message);

		return object;
	}

	void SchemaDetailsSet::clear()
	{
		m_details.clear();
	}

	void SchemaDetailsSet::add(const QString& details)
	{
		return add(std::make_shared<SchemaDetails>(details));
	}

	void SchemaDetailsSet::add(const SchemaDetails& details)
	{
		return add(std::make_shared<SchemaDetails>(details));
	}

	void SchemaDetailsSet::add(SchemaDetails&& details)
	{
		return add(std::make_shared<SchemaDetails>(std::move(details)));
	}

	void SchemaDetailsSet::add(std::shared_ptr<SchemaDetails> details)
	{
		assert(details);

		m_details[details->m_schemaId] = std::move(details);

		return;
	}

	std::vector<SchemaDetails> SchemaDetailsSet::schemasDetails() const
	{
		std::vector<SchemaDetails> result;
		result.reserve(m_details.size());

		for (const auto& schemaPair : m_details)
		{
			result.push_back(*schemaPair.second);
		}

		return result;
	}

	std::vector<SchemaDetails> SchemaDetailsSet::schemasDetails(QString equipmentId) const
	{
		std::vector<SchemaDetails> result;
		result.reserve(m_details.size());

		for (const auto& schemaPair : m_details)
		{
			const SchemaDetails* ptr = schemaPair.second.get();

			if (ptr->hasEquipmentId(equipmentId) == true)
			{
				result.push_back(*ptr);
			}
		}

		return result;
	}

	std::shared_ptr<SchemaDetails> SchemaDetailsSet::schemaDetails(QString schemaId) const
	{
		std::shared_ptr<SchemaDetails> result;

		if (auto it = m_details.find(schemaId);
			it != m_details.end())
		{
			result = it->second;
		}

		return result;
	}

	std::shared_ptr<SchemaDetails> SchemaDetailsSet::schemaDetails(int index) const
	{
		std::shared_ptr<SchemaDetails> result;

		if (index >= 0 && index < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, index);

			if (it != m_details.end())
			{
				result = it->second;
			}
		}

		return result;
	}

	QStringList SchemaDetailsSet::schemasByAppSignalId(const QString& appSignalId) const
	{
		QStringList result;
		result.reserve(16);

		for (const auto& [schemaId, schemaDetails] : m_details)
		{
			Q_ASSERT(schemaDetails);

			if (schemaDetails->hasSignal(appSignalId) == true)
			{
				result.push_back(schemaId);
			}
		}

		return result;
	}

	QStringList SchemaDetailsSet::schemasByConnectionId(const QString& connectionId) const
	{
		QStringList result;
		result.reserve(16);

		for (const auto& [schemaId, schemaDetails] : m_details)
		{
			Q_ASSERT(schemaDetails);

			if (schemaDetails->hasConnection(connectionId) == true)
			{
				result.push_back(schemaId);
			}
		}

		return result;
	}

	QStringList SchemaDetailsSet::schemasByLoopbackId(const QString& loopbackId) const
	{
		QStringList result;
		result.reserve(16);

		for (const auto& [schemaId, schemaDetails] : m_details)
		{
			Q_ASSERT(schemaDetails);

			if (schemaDetails->hasLoopback(loopbackId) == true)
			{
				result.push_back(schemaId);
			}
		}

		return result;
	}

	int SchemaDetailsSet::schemaCount() const
	{
		return static_cast<int>(m_details.size());
	}

	QString SchemaDetailsSet::schemaCaptionById(const QString& schemaId) const
	{
		auto it = m_details.find(schemaId);
		return (it == m_details.end()) ? QString{} : it->second->m_caption;
	}

	QString SchemaDetailsSet::schemaCaptionByIndex(int schemaIndex) const
	{
		if (schemaIndex >= 0 && schemaIndex < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, schemaIndex);

			return (it == m_details.end()) ? QString{} : it->second->m_caption;
		}

		return {};
	}

	QString SchemaDetailsSet::schemaIdByIndex(int schemaIndex) const
	{
		if (schemaIndex >= 0 && schemaIndex < static_cast<int>(m_details.size()))
		{
			auto it = m_details.begin();
			std::advance(it, schemaIndex);

			return (it == m_details.end()) ? QString{} : it->second->m_schemaId;
		}

		return {};
	}

	std::vector<SchemaDetails::TrendIndicatorSchemaItems> SchemaDetailsSet::trendIndicators() const
	{
		std::vector<SchemaDetails::TrendIndicatorSchemaItems> result;
		result.reserve(128);

		for (const auto& [schemaId, schemaDetails] : m_details)
		{
			result.insert(result.end(), schemaDetails->m_trendsIndicators.begin(), schemaDetails->m_trendsIndicators.end());
		}

		return result;
	}

} // namespace VFrame30