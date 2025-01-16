#include "WUtils.h"
#include "Context.h"
#include "VduAppSignalsInfoGenerator.h"

#include "ModuleLogicCompiler.h"

#include <HardwareLib/DeviceAppSignal.h>

#include "Crc.h"

namespace Builder
{
	VduAppSignalsInfoGenerator::VduAppSignalsInfoGenerator()
	{
		m_strings.reserve(32768);
		appendString(QString(), false);
	}

	VduAppSignalsInfoGenerator::~VduAppSignalsInfoGenerator()
	{
	}

	bool VduAppSignalsInfoGenerator::writeFiles(const ModuleLogicCompiler* mlc)
	{
		TEST_PTR_RETURN_FALSE(mlc);

		m_mlc = mlc;
		m_context = m_mlc->builderContext();

		TEST_PTR_RETURN_FALSE(m_context);

		m_log = m_context->m_log;

		TEST_PTR_RETURN_FALSE(m_log);

		m_vduOptoModule = m_context->m_opticModuleStorage->getOptoModule(m_mlc->lmEquipmentID());

		TEST_PTR_LOG_RETURN_FALSE(m_vduOptoModule, m_log);

		if (m_vduOptoModule->isVdu() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		m_signalSet = m_context->m_signalSet;

		TEST_PTR_RETURN_FALSE(m_signalSet);

		m_resultWriter = m_context->m_buildResultWriter;

		TEST_PTR_RETURN_FALSE(m_resultWriter);

		bool result = true;

		result &= fillAppSignalsInfo();

		RETURN_IF_FALSE(result);

		result &= fillOptoPortsInfo();

		result &= fillHeader();

		result &= writeBinFile();

		if (m_context->generateExtraDebugInfo() == true)
		{
			result &= writeTxtFile();
		}

		return result;
	}

	bool VduAppSignalsInfoGenerator::buildIoSignalsAddrMap()
	{
		const std::vector<AppSignal*>& ioSignals = m_mlc->ioSignals();

		for(const AppSignal* ioSignal : ioSignals)
		{
			TEST_PTR_CONTINUE(ioSignal);

			if (m_ioSignalsAddr.contains(ioSignal->appSignalID()))
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				continue;
			}

			Hardware::DeviceAppSignal* deviceAppSignal = nullptr;

			bool res = m_mlc->getDeviceAppSignal(*ioSignal, &deviceAppSignal);

			if (res == false)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("DeviceAppSignal not found for app signal %1").
											arg(ioSignal->appSignalID()));
				return false;
			}

			m_ioSignalsAddr.emplace(ioSignal->appSignalID(),
										Address16(deviceAppSignal->valueOffset(),
													deviceAppSignal->valueBit()));
		}

		return true;
	}

	bool VduAppSignalsInfoGenerator::fillAppSignalsInfo()
	{
		bool result = buildIoSignalsAddrMap();

		RETURN_IF_FALSE(result);

		// append VDU module native input/output/internal signals
		//
		const std::map<Hash, AppSignal*>& nativeVduSignals = m_mlc->moduleSignals();

		for(const auto& [h, s] : nativeVduSignals)
		{
			TEST_PTR_CONTINUE(s);

			Hash32 h32 = 0;

			result &= appendVduSignal(s->appSignalID(), false, NOT_VALID16, Address16(), &h32);

			Q_ASSERT(h32 != 0);
		}

		RETURN_IF_FALSE(result);

		// utf8Hash32(rxSignal appSignalID synonim) => utf8Hash32(rxSignal->appSignalIDs[0])
		//
		std::map<Hash32, Hash32> rxSignalsSynonims;

		// apped signals received by opto connections
		//
		uint16_t portIndex = 0;

		bool filterAutoIDs = true;

		for(const auto& [equipID, port] : m_vduOptoModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			for(const Hardware::TxRxSignalShared& rx : port->rxSignals())
			{
				QStringList ids = rx->appSignalIDs();

				if (ids.size() == 0)
				{
					Q_ASSERT(false);
					continue;
				}

				if (filterAutoIDs && ids.size() > 1)
				{
					QStringList tmpIds;

					for(const QString& id : ids)
					{
						if (id.startsWith("#AUTO_BUS_") == false)
						{
							tmpIds.append(id);
						}
					}

					if (tmpIds.size() > 0)
					{
						ids.swap(tmpIds);
					}
				}

				Hash32 signalHash = 0;

				bool res = appendVduSignal(ids[0], true, portIndex, rx->addrInBuf(), &signalHash);

				if (res == false)
				{
					result = false;
					continue;
				}

				Q_ASSERT(signalHash != 0);

				if (ids.size() > 1)
				{
					for(qsizetype i = 1; i < ids.size(); i++)
					{
						const QString& synonym = ids[i];

						res = appendHash32AppSignalID(synonym);

						if (res == false)
						{
							result = false;
							continue;
						}

						Hash32 synHash = utf8Hash32(synonym);

						auto it = rxSignalsSynonims.find(synHash);

						if (it != rxSignalsSynonims.end())
						{
							Q_ASSERT(it->second == signalHash);
							continue;
						}

						rxSignalsSynonims.emplace(synHash, signalHash);
					}
				}
			}

			portIndex++;
		}

		// set signalIndexes and build m_hash32ToSignalIndex map
		//
		uint16_t signalIndex = 0;

		for(auto& [id, vduSignal] : m_vduSignals)
		{
			vduSignal.signalIndex = signalIndex++;
			m_hash32ToSignalIndex.emplace(utf8Hash32(id), vduSignal.signalIndex);
		}

		RETURN_IF_FALSE(result);

		// append rxSignals AppSignalID synonims to m_hash32ToSignalIndex

		for(const auto& [synonymHash, signalHash] : rxSignalsSynonims)
		{
			auto it = m_hash32ToSignalIndex.find(signalHash);

			if (it != m_hash32ToSignalIndex.end())
			{
				m_hash32ToSignalIndex.emplace(synonymHash, it->second);
			}
			else
			{
				Q_ASSERT(false);
			}
		}

		// m_context->m_vduSignals filling
		//
		Q_ASSERT(m_context->m_vduSignals.contains(m_vduOptoModule->equipmentID()) == false);

		auto [newIt, b] = m_context->m_vduSignals.emplace(m_vduOptoModule->equipmentID(), std::map<Hash, int>{});

		std::map<Hash, int>& hash64ToSignalIndex = newIt->second;

		for(const auto& [h32, appSignalID] : m_hash32AppSignalID)
		{
			auto it = m_hash32ToSignalIndex.find(h32);

			if (it == m_hash32ToSignalIndex.end())
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Hash32 to VDU SignaIndex not found for '%1'").
											arg(appSignalID));
				result = false;
			}
			else
			{
				Hash h64 = calcHash(appSignalID);

				Q_ASSERT(hash64ToSignalIndex.contains(h64) == false);

				hash64ToSignalIndex.emplace(h64, it->second);
			}
		}

		RETURN_IF_FALSE(result);

		return result;
	}

	bool VduAppSignalsInfoGenerator::fillOptoPortsInfo()
	{
		bool result = true;

		uint16_t portIndex = 0;

		size_t txSignalsCount = 0;
		size_t rxSignalsCount = 0;

		for(const auto& [equipmentID, port] : m_vduOptoModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			VduOptoPort pi;

			pi.optoPortIndex = portIndex;
			portIndex++;

			pi.linkID = port->linkID();

			pi.rxDataSizeW = port->rxDataSizeW();
			pi.txDataSizeW = port->txDataSizeW();

			pi.rxDataUID = port->rxDataID();
			pi.txDataUID = port->txDataID();

			m_optoPorts.emplace_back(pi);

			//

			txSignalsCount += port->txSignalsCount();
			rxSignalsCount += port->rxSignalsCount();
		}

		m_txAppSignals.reserve(txSignalsCount);
//		m_rxAppSignals.reserve(rxSignalsCount);

		portIndex = 0;

		for(const auto& [equipmentID, port] : m_vduOptoModule->ports())
		{
			result &= fillTxRxSignalsInfo(portIndex, port->txSignals(), &m_txAppSignals);
//			result &= fillTxRxSignalsInfo(portIndex, port->rxSignals(), &m_rxAppSignals);

			portIndex++;
		}

		return result;
	}

	bool VduAppSignalsInfoGenerator::fillTxRxSignalsInfo(uint16_t portIndex,
														 const QVector<Hardware::TxRxSignalShared>& portTxRxSignals,
														 std::vector<VduTxRxAppSignal>* txRxSignals)
	{
		TEST_PTR_RETURN_FALSE(txRxSignals);

		bool result = true;

		for(const Hardware::TxRxSignalShared& ps : portTxRxSignals)
		{
			auto it = m_hash32ToSignalIndex.find(utf8Hash32(ps->appSignalID()));

			if (it == m_hash32ToSignalIndex.end())
			{
				result = false;
				LOG_INTERNAL_ERROR_MSG(m_log, QString("AppSignalID %1 not found").arg(ps->appSignalID()));
				continue;
			}

			uint16_t signalIndex = it->second;

			txRxSignals->emplace_back(VduTxRxAppSignal{	.optoPortIndex = portIndex,
														.signalIndex = signalIndex,
														.offsetW = static_cast<uint16_t>(ps->addrInBuf().offset()),
														.bitNo = static_cast<uint16_t>(ps->addrInBuf().bit())});
		}

		return result;
	}

	bool VduAppSignalsInfoGenerator::fillHeader()
	{
		// "VAS\0"
		//
		m_header.magic[0] = 'V';
		m_header.magic[1] = 'A';
		m_header.magic[2] = 'S';
		m_header.magic[3] = '\0';

		m_header.fileVersion = VAS_FILE_VERSION;

		m_header.appSignalsCount = static_cast<uint16_t>(m_vduSignals.size());
		m_header.hash32ToIndexCount = static_cast<uint16_t>(m_hash32ToSignalIndex.size());
		m_header.optoPortsCount = static_cast<uint16_t>(m_optoPorts.size());
//		m_header.rxAppSignalsCount = static_cast<uint16_t>(m_rxAppSignals.size());
		m_header.txAppSignalsCount = static_cast<uint16_t>(m_txAppSignals.size());

		m_header.reserv1 = 0;

		//Q_ASSERT(m_header.optoPortsCount == VDU_OPTO_PORTS_COUNT);

		m_header.refAppSignals = sizeof(m_header);

		m_header.refHash32ToIndex = m_header.refAppSignals + m_header.appSignalsCount * sizeof(VduAppSignal);

		m_header.refOptoPorts = m_header.refHash32ToIndex + m_header.hash32ToIndexCount * sizeof(VduHash32ToIndex);

//		m_header.refRxAppSignals = m_header.refOptoPorts + m_header.optoPortsCount * sizeof(VduOptoPort);

		m_header.refTxAppSignals = m_header.refOptoPorts + m_header.optoPortsCount * sizeof(VduOptoPort);

		m_header.refStrings = m_header.refTxAppSignals + m_header.txAppSignalsCount * sizeof(VduTxRxAppSignal);

		recalcStringsRefs(m_header.refStrings);

		return true;
	}

	bool VduAppSignalsInfoGenerator::appendVduSignal(const QString& appSignalID, bool isRxSignal,
													 uint16_t portIndex, const Address16& rxAddr, Hash32* h32)
	{
		TEST_PTR_RETURN_FALSE(h32);

		qDebug() << appSignalID;

		*h32 = 0;

		auto it = m_vduSignals.find(appSignalID);

		if (it != m_vduSignals.end())
		{
			const VduAppSignal& vs = it->second;

			if (isRxSignal &&
				(static_cast<VduSignalInOutType>(vs.vduSignalInOutType) == VduSignalInOutType::RxSignal))
			{
				QList<Hardware::OptoPortShared> ports;
				m_vduOptoModule->getOptoPorts(ports);

				if (vs.rxPortIndex >= ports.size() ||
					portIndex >= ports.size())
				{
					LOG_INTERNAL_ERROR(m_log);
					return false;
				}

				// App signal %1 received simultaneously via %2 and %3 opto ports of VDU %4
				//
				m_log->errCFG3054(appSignalID,
								  ports[vs.rxPortIndex]->equipmentID(),
								  ports[portIndex]->equipmentID(),
								  m_vduOptoModule->equipmentID());
				return false;
			}

			*h32 = utf8Hash32(appSignalID);
			return true;							// already appended
		}

		const AppSignal* appSignal = m_signalSet->getSignal(appSignalID);

		if (appSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("AppSignal %1 not found").arg(appSignalID));
			return false;
		}

		if (appendHash32AppSignalID(appSignalID) == false)
		{
			return false;
		}

		Hash32 hash = utf8Hash32(appSignalID);

		*h32 = hash;

		auto [it2, b] = m_vduSignals.emplace(appSignalID, VduAppSignal{});

		VduAppSignal& si = it2->second;

		si.signalIndex = NOT_VALID16;		// will set after m_vduSignals full filling

		VduSignalInOutType inOutType = VduSignalInOutType::Internal;

		si.rxPortIndex = NOT_VALID16;
		si.offsetW = NOT_VALID16;
		si.bitNo = NOT_VALID16;

		if (!isRxSignal)
		{
			inOutType = getVduSignalInOutType(appSignal);

			if (inOutType == VduSignalInOutType::Input ||
				inOutType == VduSignalInOutType::Output)
			{
				auto it3 = m_ioSignalsAddr.find(appSignalID);

				if (it3 != m_ioSignalsAddr.end())
				{
					si.offsetW = it3->second.offset();
					si.bitNo = it3->second.bit();
				}
				else
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString("AppSignal %1 not found in m_ioSignalsAddr map").
							arg(appSignalID));
					return false;
				}
			}
		}
		else
		{
			inOutType = VduSignalInOutType::RxSignal;

			si.rxPortIndex = portIndex;
			si.offsetW = rxAddr.offset();
			si.bitNo = rxAddr.bit();
		}

		si.vduSignalInOutType = static_cast<uint16_t>(inOutType);

		//

		VduSignalType vduSignalType = getVduSignalType(appSignal);

		si.vduSignalType = static_cast<uint16_t>(vduSignalType);

		//

		si.boolProps = 0;

		si.enableTuning = appSignal->enableTuning() ? 1 : 0;

		//

		// here this is offsets in m_string table, NOT in file!
		//
		si.refAppSignalID = appendString(appSignal->appSignalID(), false);
		si.refCustomAppSignalID = appendString(appSignal->customAppSignalID(), true);
		si.refCaption = appendString(appSignal->caption(), false);
		si.refUnit = appendString(appSignal->unit(), false);

		//

		if (appSignal->enableTuning())
		{
			si.tuningDefaultValue = appSignal->tuningDefaultValue().untypedUInt32Value();
			si.lowLimit = appSignal->tuningLowBound().untypedUInt32Value();
			si.highLimit = appSignal->tuningHighBound().untypedUInt32Value();
		}
		else
		{
			si.tuningDefaultValue = 0;
			si.lowLimit = vduSignalUntypedValue(vduSignalType, appSignal->lowEngineeringUnits());
			si.highLimit = vduSignalUntypedValue(vduSignalType, appSignal->highEngineeringUnits());
		}

		if (vduSignalType == VduSignalType::Discrete)
		{
			si.lowLimit = 0;
			si.highLimit = 1;
		}

		si.decimalPlaces = static_cast<uint16_t>(appSignal->decimalPlaces());

		return true;
	}

	bool VduAppSignalsInfoGenerator::appendHash32AppSignalID(const QString& appSignalID)
	{
		Hash32 h32 = utf8Hash32(appSignalID);

		auto it = m_hash32AppSignalID.find(h32);

		if (it != m_hash32AppSignalID.end())
		{
			if(it->second != appSignalID)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("Hash32 = %1 collision for '%2' and '%3' strings").
											  arg(hex32(h32), it->second, appSignalID));
				return false;
			}
			else
			{
				// duplicate append of same appSignalID, its Ok
				// may occurs when same appSignal received by different opto ports
			}
		}
		else
		{
			m_hash32AppSignalID.emplace(h32, appSignalID);
		}

		return true;
	}

	vdu_cstr VduAppSignalsInfoGenerator::appendString(const QString& str, bool checkExistString)
	{
		Q_UNUSED(checkExistString);

		// returns offset in m_strings NOT in file!

		QByteArray utf8Str = str.toUtf8();

		Hash32 hash = ::calcHash32(utf8Str);

		auto it = m_stringRefs.find(hash);

		if (it != m_stringRefs.end())
		{
			return it->second;
		}

		//

/*		if (checkExistString && str.length() > 1 && str[0] != '#')
		{
			QString str2 = '#' + str;
			QByteArray utf8Str2 = str2.toUtf8();

			Hash32 hash2 = ::calcHash32(utf8Str2);

			it = m_stringRefs.find(hash2);

			if (it != m_stringRefs.end())
			{
				return (it->second | 0x80000000);
			}
		} */

		//

		vdu_cstr ref = static_cast<vdu_cstr>(m_strings.size());

		m_stringRefs.emplace(hash, ref);

		size_t utf8StrSize = utf8Str.size();

		if (utf8StrSize > std::numeric_limits<uint16_t>::max())
		{
			Q_ASSERT(false);		// string too long
			return 0;
		}

		m_strings.push_back(utf8StrSize & 0xFF);
		m_strings.push_back((utf8StrSize >> 8) & 0xFF);

		for(char ch : utf8Str)
		{
			m_strings.push_back(ch);
		}

		m_strings.push_back(0);				// string null termination

		if ((m_strings.size() % 2) != 0)
		{
			m_strings.push_back(0);			// align string record on 2 bytes
		}

		return ref;
	}

	void VduAppSignalsInfoGenerator::recalcStringsRefs(uint32_t stringsOffsetInFile)
	{
		for(auto& [h, vs] : m_vduSignals)
		{
			vs.refAppSignalID += stringsOffsetInFile;
			vs.refCustomAppSignalID += stringsOffsetInFile;
			vs.refCaption += stringsOffsetInFile;
			vs.refUnit += stringsOffsetInFile;
		}
	}

	bool VduAppSignalsInfoGenerator::writeBinFile()
	{
		QByteArray data;

		data.append(reinterpret_cast<const char*>(&m_header),
					sizeof(m_header));

		//

		std::vector<VduAppSignal> vduSignals;

		vduSignals.reserve(m_vduSignals.size());

		for(const auto& [id, vs] : m_vduSignals)
		{
			vduSignals.emplace_back(vs);
		}

		data.append(reinterpret_cast<const char*>(vduSignals.data()),
					vduSignals.size() * sizeof(VduAppSignal));

		//

		std::vector<VduHash32ToIndex> h2i;

		h2i.reserve(m_hash32ToSignalIndex.size());

		for(const auto& [h32, index] : m_hash32ToSignalIndex)
		{
			h2i.emplace_back(VduHash32ToIndex{ .hash32 = h32, .signalIndex = index});
		}

		data.append(reinterpret_cast<const char*>(h2i.data()),
					h2i.size() * sizeof(VduHash32ToIndex));

		//

		data.append(reinterpret_cast<const char*>(m_optoPorts.data()),
					m_optoPorts.size() * sizeof(VduOptoPort));

		// data.append(reinterpret_cast<const char*>(m_rxAppSignals.data()),
		// 			m_rxAppSignals.size() * sizeof(VduTxRxAppSignal));

		data.append(reinterpret_cast<const char*>(m_txAppSignals.data()),
					m_txAppSignals.size() * sizeof(VduTxRxAppSignal));

		data.append(reinterpret_cast<const char*>(m_strings.data()),
					m_strings.size());

		//

		m_crc64Offset = data.size();
		m_crc64 = Crc64().add(data);

		data.append(reinterpret_cast<const char*>(&m_crc64), sizeof(m_crc64));

		return m_resultWriter->addFile(Directory::VDUs + Separator::DIR + m_vduOptoModule->equipmentID(),
									   File::VDU_APP_SIGNALS_BIN, data, false);
	}

	bool VduAppSignalsInfoGenerator::writeTxtFile()
	{
		QStringList file;

		printHeader(file);
		printAppSignals(file);
		printHashToIndex(file);
		printOptoPortsInfo(file);
//		printTxRxSignalsInfo(file, m_rxAppSignals);
		printTxRxSignalsInfo(file, m_txAppSignals);
		printStringsTable(file);
		printCrc64(file);
		printRefInfo(file);

		return m_resultWriter->addFile(Directory::VDUs + Separator::DIR + m_vduOptoModule->equipmentID(),
										File::VDU_APP_SIGNALS_TXT, file, false);
	}

	void VduAppSignalsInfoGenerator::printHeader(QStringList& file) const
	{
		file << QString(" VDU EquipmentID: %1\n").arg(m_vduOptoModule->equipmentID());

		file << LINE;
		file << "              VDU AppSignals info file header";
		file << LINE;
		file << "  Address   | Header field             | Value";
		file << LINE;

		file << addrStr(sizeof(m_header.magic),
						QString("signature                | '%1\\0'").arg(m_header.magic));

		file << addrStr(sizeof(m_header.fileVersion),
						QString("fileVersion              | %1").arg(m_header.fileVersion)) ;

		file << addrStr(sizeof(m_header.appSignalsCount),
						QString("appSignalsCount          | %1").arg(m_header.appSignalsCount)) ;

		file << addrStr(sizeof(m_header.hash32ToIndexCount),
						QString("hashToIndexCount         | %1").arg(m_header.hash32ToIndexCount)) ;

		file << addrStr(sizeof(m_header.optoPortsCount),
						QString("optoPortsCount           | %1").arg(m_header.optoPortsCount)) ;

		// file << addrStr(sizeof(m_header.rxAppSignalsCount),
		// 				QString("rxAppSignalsCount        | %1").arg(m_header.rxAppSignalsCount)) ;

		file << addrStr(sizeof(m_header.txAppSignalsCount),
						QString("txAppSignalsCount        | %1").arg(m_header.txAppSignalsCount)) ;

		file << addrStr(sizeof(m_header.reserv1),
						QString("reserv1                  | %1").arg(m_header.reserv1)) ;

		file << addrStr(sizeof(m_header.refAppSignals),
						QString("refAppSignals            | %1").arg(hex32(m_header.refAppSignals)));

		file << addrStr(sizeof(m_header.refHash32ToIndex),
						QString("refHashToSignalIndex     | %1").arg(hex32(m_header.refHash32ToIndex)));

		file << addrStr(sizeof(m_header.refOptoPorts),
						QString("refOptoPorts             | %1").arg(hex32(m_header.refOptoPorts)));

		// file << addrStr(sizeof(m_header.refRxAppSignals),
		// 				QString("refRxAppSignals          | %1").arg(hex32(m_header.refRxAppSignals)));

		file << addrStr(sizeof(m_header.refTxAppSignals),
						QString("refTxAppSignals          | %1").arg(hex32(m_header.refTxAppSignals)));

		file << addrStr(sizeof(m_header.refStrings),
						QString("refStrings               | %1").arg(hex32(m_header.refStrings)));
	}

	void VduAppSignalsInfoGenerator::printAppSignals(QStringList& file) const
	{
		file << LLINE;
		file << "              VDU AppSignals";
		file << LLINE;
		file << "  Address   | index  | inOutType | signalType | boolProps | refAppSignalID | refCustSignalID | refCaption | refUnit    | tunDefault | lowLimit   | highLimit  | decPlaces | rxPortIndex | offsetW | bitNo";
		file << LLINE;

		for(const auto& [id, vs] : m_vduSignals)
		{
			file << addrStr(sizeof(vs),
							QString("%1 | %2    | %3     | %4    | %5     | %6      | %7 | %8 | %9 | %10 | %11 | %12    | %13      | %14  | %15").
							arg(hex16(vs.signalIndex),					//	1
								hex16(vs.vduSignalInOutType),			//	2
								hex16(vs.vduSignalType),				//	3
								hex16(vs.boolProps),					//	4
								hex32(vs.refAppSignalID),				//	5
								hex32(vs.refCustomAppSignalID),			//	6
								hex32(vs.refCaption),					//	7
								hex32(vs.refUnit),						//	8
								hex32(vs.tuningDefaultValue),			//	9
								hex32(vs.lowLimit),						//	10
								hex32(vs.highLimit),					//	11
								hex16(vs.decimalPlaces),				//	12
								hex16(vs.rxPortIndex),					//	13
								hex16(vs.offsetW),						//	14
								hex16(vs.bitNo)));						//	15
		}
	}

	void VduAppSignalsInfoGenerator::printHashToIndex(QStringList& file) const
	{
		file << LLINE;
		file << "              Hash to SignalIndex";
		file << LLINE;
		file << "  Address   | hash32     | signalIndex | AppSignalID (for reference only, not included in VduAppSignals.bin )";
		file << LLINE;

		for(const auto& [h32, index] : m_hash32ToSignalIndex)
		{
			auto it = m_hash32AppSignalID.find(h32);

			if (it == m_hash32AppSignalID.end())
			{
				Q_ASSERT(false);
			}
			file << addrStr(sizeof(VduHash32ToIndex), QString("%1 | %2  | %3").arg(hex32(h32), hex32(index), it->second));
		}

		file << LLINE;
	}

	void VduAppSignalsInfoGenerator::printOptoPortsInfo(QStringList& file) const
	{
		file << "              Opto ports info table";
		file << LINE;
		file << "  Address   | portIndex | linkID    | rxDataSizeW | txDataSizeW | rxDataUID  | txDataUID";
		file << LINE;

		for(const VduOptoPort& pi : m_optoPorts)
		{
			file << addrStr(sizeof(pi),
							QString("%1    | %2    | %3      | %4      | %5 | %6").
								arg(hex16(pi.optoPortIndex)).
								arg(hex16(pi.linkID)).
								arg(hex16(pi.rxDataSizeW)).
								arg(hex16(pi.txDataSizeW)).
								arg(hex32(pi.rxDataUID)).
								arg(hex32(pi.txDataUID)));
		}
	}

	void VduAppSignalsInfoGenerator::printTxRxSignalsInfo(QStringList& file,
														  const std::vector<VduTxRxAppSignal>& txRxSignals) const
	{
		file << LINE;

		// if (&txRxSignals == &m_rxAppSignals)
		// {
		// 	file << "              Received app signals info table";
		// }
		// else
		// {
		// 	file << "              Transmitted app signals info table";
		// }

		file << "              Transmitted app signals info table";

		file << LINE;
		file << "  Address   | portIndex | signalIndex | valueOffsetW | valueBitNo";
		file << LINE;

		for(const VduTxRxAppSignal& s : txRxSignals)
		{
			file << addrStr(sizeof(s),
							QString("%1    | %2      | %3       | %4").
							arg(hex16(s.optoPortIndex)).
							arg(hex16(s.signalIndex)).
							arg(hex16(s.offsetW)).
							arg(hex16(s.bitNo)));
		}
	}

	void VduAppSignalsInfoGenerator::printStringsTable(QStringList& file) const
	{
		file << LINE;
		file << "              Strings table";
		file << LINE;
		file << "  Address   | strLen (bytes) | string";
		file << LINE;

		static const QString ZERO_CHAR("\\0");

		QString str;
		int index = 0;

		while(index < m_strings.size())
		{
			int strStartIndex = index;

			int strLen = m_strings[index + 1];	// high byte of len
			strLen <<= 8;
			strLen |= m_strings[index];			// low byte of len

			index += 2;

			str.clear();
			str.reserve(strLen + 8);

			str = QString::fromUtf8(reinterpret_cast<const char*>(m_strings.data() + index), strLen);

			index += strLen;

			Q_ASSERT(m_strings[index] == 0);
			str.append(ZERO_CHAR);
			index++;

			if ((index % 2) != 0)
			{
				Q_ASSERT(m_strings[index] == 0);
				str.append(ZERO_CHAR);
				index++;
			}

			int strDataSize = index - strStartIndex;

			file << addrStr(strDataSize, QString("%1 | %2").
										 arg(strLen, 14).arg(str));
		}
	}

	void VduAppSignalsInfoGenerator::printCrc64(QStringList& file) const
	{
		file << LINE;

		Q_ASSERT(m_crc64Offset == m_txtOffset);

		file << addrStr(sizeof(m_crc64), QString("CRC64 = %1").arg(hex64(m_crc64)));

		file << LINE;
	}

	void VduAppSignalsInfoGenerator::printRefInfo(QStringList& file) const
	{

		file << Separator::EMPTY_STR;
		file << Separator::EMPTY_STR;
		file << LINE;
		file << QString(" Reference information, not placed in %1 file").arg(File::VDU_APP_SIGNALS_BIN);
		file << LINE;
		file << Separator::EMPTY_STR;
		file << " Sizeof structures:";
		file << Separator::EMPTY_STR;

		file << QString(" VduAppSignalsFileHeader  %1 (%2)").
							arg(sizeof(VduAppSignalsFileHeader)).
							arg(hex16(sizeof(VduAppSignalsFileHeader)));

		file << QString(" VduAppSignal             %1 (%2)").
							arg(sizeof(VduAppSignal)).
							arg(hex16(sizeof(VduAppSignal)));

		file << QString(" VduHash32ToIndex         %1 (%2)").
							arg(sizeof(VduHash32ToIndex)).
							arg(hex16(sizeof(VduHash32ToIndex)));

		file << QString(" VduOptoPort              %1 (%2)").
							arg(sizeof(VduOptoPort)).
							arg(hex16(sizeof(VduOptoPort)));

		file << QString(" VduTxRxAppSignal         %1 (%2)").
							arg(sizeof(VduTxRxAppSignal)).
							arg(hex16(sizeof(VduTxRxAppSignal)));
		file << Separator::EMPTY_STR;
		file << LINE;

		auto it = m_context->m_vduSignals.find(m_vduOptoModule->equipmentID());

		if (it != m_context->m_vduSignals.end())
		{
			std::map<int, std::vector<Hash>> indexToHashes;

			const std::map<Hash, int>& hashToIndex = it->second;

			for(const auto& [h, indx] : hashToIndex)
			{
				auto it2 = findOrInsertKey(indexToHashes, indx);

				it2->second.emplace_back(h);
			}

			file << " SignalIndex | AppSignalID(s)";
			file << LINE;

			std::shared_ptr<SignalSet> signalSet = m_context->m_signalSet;
			QString ids;

			for(const auto& [indx, hashes] : indexToHashes)
			{
				ids.clear();

				for(Hash h : hashes)
				{
					const AppSignal* appSignal = signalSet->getSignalByHash(h);

					TEST_PTR_CONTINUE(appSignal);

					if (ids.isEmpty() == false)
					{
						ids += Separator::COMMA_SPACE;
					}

					ids += appSignal->appSignalID();
				}

				file << QString(" %1      | %2").arg(hex16(indx), ids);
			}

			file << LINE;
		}
	}

	VduSignalInOutType VduAppSignalsInfoGenerator::getVduSignalInOutType(const AppSignal* s)
	{
		TEST_PTR_RETURN_VALUE(s, VduSignalInOutType::Unknown);

		switch(s->inOutType())
		{
		case E::SignalInOutType::Input:
			return VduSignalInOutType::Input;

		case E::SignalInOutType::Output:
			return VduSignalInOutType::Output;

		case E::SignalInOutType::Internal:
			return VduSignalInOutType::Internal;

		default:
			Q_ASSERT(false);
		}

		return VduSignalInOutType::Unknown;
	}

	VduSignalType VduAppSignalsInfoGenerator::getVduSignalType(const AppSignal* s)
	{
		TEST_PTR_RETURN_VALUE(s, VduSignalType::Unknown);

		switch(s->signalType())
		{
		case E::Discrete:
			return VduSignalType::Discrete;

		case E::Analog:
			switch(s->analogSignalFormat())
			{
			case E::AnalogAppSignalFormat::Float32:
				return VduSignalType::AnalogFloat32;

			case E::AnalogAppSignalFormat::SignedInt32:
				return VduSignalType::AnalogSignedInt32;

			default:
				;
			}

		default:
			;
		}

		Q_ASSERT(false);

		return VduSignalType::Unknown;
	}

	uint32_t VduAppSignalsInfoGenerator::vduSignalLowBoundUntyped(VduSignalType type)
	{
		switch(type)
		{
		case VduSignalType::Discrete:
			return 0;

		case VduSignalType::AnalogSignedInt32:
			return std::bit_cast<uint32_t>(std::numeric_limits<qint32>::min());

		case VduSignalType::AnalogFloat32:
			return std::bit_cast<uint32_t>(std::numeric_limits<float>::lowest());

		case VduSignalType::Unknown:
			Q_ASSERT(false);
		}

		return 0;
	}

	uint32_t VduAppSignalsInfoGenerator::vduSignalHighBoundUntyped(VduSignalType type)
	{
		switch(type)
		{
		case VduSignalType::Discrete:
			return 1;

		case VduSignalType::AnalogSignedInt32:
			return std::bit_cast<uint32_t>(std::numeric_limits<qint32>::max());

		case VduSignalType::AnalogFloat32:
			return std::bit_cast<uint32_t>(std::numeric_limits<float>::max());

		case VduSignalType::Unknown:
			Q_ASSERT(false);
		}

		return 0;
	}

	uint32_t VduAppSignalsInfoGenerator::vduSignalUntypedValue(VduSignalType type, double dblValue)
	{
		switch(type)
		{
		case VduSignalType::Discrete:
			return (dblValue == 0 ? 0 : 1);

		case VduSignalType::AnalogSignedInt32:
			return std::bit_cast<uint32_t>(static_cast<int32_t>(dblValue));

		case VduSignalType::AnalogFloat32:
			return std::bit_cast<uint32_t>(static_cast<float>(dblValue));

		case VduSignalType::Unknown:
			Q_ASSERT(false);
		}

		return 0;
	}

	QString VduAppSignalsInfoGenerator::addrStr(int fieldSize, const QString& str) const
	{
		QString res = QString(" %1 | %2").arg(hex32(m_txtOffset), str);

		m_txtOffset += fieldSize;

		return res;
	}

	QString VduAppSignalsInfoGenerator::hex16(qint64 v) const
	{
		return QString("0x") + QString("%1").arg(v, 4, 16, QChar('0')).toUpper();
	}

	QString VduAppSignalsInfoGenerator::hex32(qint64 v) const
	{
		return QString("0x") + QString("%1").arg(v, 8, 16, QChar('0')).toUpper();
	}

	QString VduAppSignalsInfoGenerator::hex64(quint64 v) const
	{
		return QString("0x") + QString("%1").arg(v, 16, 16, QChar('0')).toUpper();
	}

	Hash32 VduAppSignalsInfoGenerator::utf8Hash32(const QString& str) const
	{
		return ::calcHash32(str.toUtf8());
	}
}
