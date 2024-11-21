#include "WUtils.h"
#include "Context.h"
#include "VduAppSignalsInfoGenerator.h"

#include "ModuleLogicCompiler.h"

#include "Crc.h"

namespace Builder
{
	VduAppSignalsInfoGenerator::VduAppSignalsInfoGenerator()
	{
		m_strings.reserve(32768);
		appendString(QString());
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

/*		m_inOutSignals = inOutSignals;			// copy Ok!



		result &= fillPortsInfo();
		result &= fillAppSignalsInfo();

		result &= fillHeader();

		result &= writeVciFile();

		if (m_context->generateExtraDebugInfo() == true)
		{
			result &= writeTxtFile();
		}*/

		return result;
	}

	bool VduAppSignalsInfoGenerator::fillPortsInfo()
	{
		m_optoPorts.clear();

		int index = 0;

		for(const auto& [equipmentID, port] : m_vduOptoModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			VduOptoPort pi;

			pi.optoPortIndex = index;
			index++;

			pi.linkID = port->linkID();

			pi.rxDataSizeW = port->rxDataSizeW();
			pi.txDataSizeW = port->txDataSizeW();

			pi.rxDataUID = port->rxDataID();
			pi.txDataUID = port->txDataID();

			m_optoPorts.emplace_back(pi);
		}

		return true;
	}

	bool VduAppSignalsInfoGenerator::fillAppSignalsInfo()
	{
		bool result = true;

		//

/*		for(const auto& [equipID, port] : m_vduOptoModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			for(const Hardware::TxRxSignalShared& rx : port->rxSignals())
			{
				for(const QString& id : rx->appSignalIDs())
				{
					m_rxSignalsHash32.insert(calcHash32(id));
				}
			}
		} */

		// append VDU module input/output/internal signals
		//
		const std::map<Hash, AppSignal*>& moduleSignals = m_mlc->moduleSignals();

		for(const auto& [h, s] : moduleSignals)
		{
			Hash32 h32 = 0;

			result &= appendVduSignal(s->appSignalID(), false, &h32);

			Q_ASSERT(h32 != 0);
		}

		// apped signals received by opto connections
		//
		for(const auto& [equipID, port] : m_vduOptoModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			for(const Hardware::TxRxSignalShared& rx : port->rxSignals())
			{
				const QStringList& ids = rx->appSignalIDs();

				if (ids.size() == 0)
				{
					Q_ASSERT(false);
					continue;
				}

				Hash32 signalHash = 0;

				result &= appendVduSignal(ids[0], true, &signalHash);

				Q_ASSERT(signalHash != 0);

				if (ids.size() > 1)
				{
					for(qsizetype i = 1; i < ids.size(); i++)
					{
						const QString& synonym = ids[i];

						Hash32 synHash = calcHash32(synonym);

						Q_ASSERT(m_rxSignalsSynonims.contains(synHash) == false);

						m_rxSignalsSynonims.emplace(synHash, signalHash);
					}
				}
			}
		}

		// set signalIndexes

		uint16_t signalIndex = 0;

		for(auto& [id, vduSignal] : m_vduSignals)
		{
			vduSignal.signalIndex = signalIndex++;
			m_hashToSignalIndex.emplace(vduSignal.hash, vduSignal.signalIndex);
		}

		for(const auto& [synonymHash, signalHash] : m_rxSignalsSynonims)
		{
			auto it = m_hashToSignalIndex.find(signalHash);

			if (it != m_hashToSignalIndex.end())
			{
				m_hashToSignalIndex.emplace(synonymHash, it->second);
			}
			else
			{
				Q_ASSERT(false);
			}
		}


//		m_mlc->m_moduleSignals

/*		for(const auto& [equipID, port] : m_inOutSignals)

		for(const auto& [equipID, port] : m_vduModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			result &= fillSignalsInfo(portIndex, port->rxSignals(), m_rxAppSignals);
		}*/

/*		int portIndex = 0;

		for(const auto& [equipID, port] : m_vduModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			result &= fillSignalsInfo(portIndex, port->rxSignals(), m_rxAppSignals);

			portIndex++;
		}

		portIndex = 0;

		for(const auto& [equipID, port] : m_vduModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			result &= fillSignalsInfo(portIndex, port->txSignals(), m_txAppSignals);

			portIndex++;
		}

		// append info of ioSignals not transmitted via opto ports
		//
		for(const auto& [appSignalID, ioTypeAddr] : m_inOutSignals)
		{
			static const quint16 NOT_VALID_PORT_INDEX = 0xFFFF;
			static const Address16 NOT_VALID_PORT_ADDR;

			result &= appendVduSignal(NOT_VALID_PORT_INDEX, appSignalID, NOT_VALID_PORT_ADDR, true, false, m_txAppSignals);
		}*/

		return result;
	}

	bool VduAppSignalsInfoGenerator::fillHeader()
	{
		const uint32_t HEADER_SIZE = static_cast<uint32_t>(sizeof(m_header));
		const uint32_t OPTO_PORTS_SIZE = static_cast<uint32_t>(m_optoPorts.size() * sizeof(VduOptoPort));
		const uint32_t RX_APP_SIGNALS_SIZE = static_cast<uint32_t>(m_rxAppSignals.size() * sizeof(VduAppSignal));
		const uint32_t TX_APP_SIGNALS_INFO_SIZE = static_cast<uint32_t>(m_txAppSignals.size() * sizeof(VduAppSignal));

		// "VAS\0"
		//
		m_header.magic[0] = 'V';
		m_header.magic[1] = 'A';
		m_header.magic[2] = 'S';
		m_header.magic[3] = '\0';

		m_header.fileVersion = VAS_FILE_VERSION;

		m_header.optoPortsCount = static_cast<uint16_t>(m_optoPorts.size());
		m_header.rxAppSignalsCount = static_cast<uint16_t>(m_rxAppSignals.size());
		m_header.txAppSignalsCount = static_cast<uint16_t>(m_txAppSignals.size());

		Q_ASSERT(m_header.optoPortsCount == VDU_OPTO_PORTS_COUNT);

		m_header.refOptoPorts = HEADER_SIZE;

		m_header.refRxAppSignals = HEADER_SIZE + OPTO_PORTS_SIZE;

		m_header.refTxAppSignals = HEADER_SIZE + OPTO_PORTS_SIZE + RX_APP_SIGNALS_SIZE;

		m_header.refStrings = HEADER_SIZE + OPTO_PORTS_SIZE + RX_APP_SIGNALS_SIZE + TX_APP_SIGNALS_INFO_SIZE;

		recalcStringsRefs(m_header.refStrings);

		//

//		m_header.reserve1 = MARKER32;

		return true;
	}

	bool VduAppSignalsInfoGenerator::writeVciFile()
	{
		QByteArray data;

		data.append(reinterpret_cast<const char*>(&m_header),
					sizeof(m_header));

		data.append(reinterpret_cast<const char*>(m_optoPorts.data()),
					m_optoPorts.size() * sizeof(VduOptoPort));

		data.append(reinterpret_cast<const char*>(m_rxAppSignals.data()),
					m_rxAppSignals.size() * sizeof(VduAppSignal));

		data.append(reinterpret_cast<const char*>(m_txAppSignals.data()),
					m_txAppSignals.size() * sizeof(VduAppSignal));

		data.append(reinterpret_cast<const char*>(m_strings.data()),
					m_strings.size() * sizeof(char16_t));

		m_crc64Offset = data.size();
		m_crc64 = Crc64().add(data);

		data.append(reinterpret_cast<const char*>(&m_crc64), sizeof(m_crc64));

		return m_resultWriter->addFile(Directory::VDUs + Separator::DIR + m_vduOptoModule->equipmentID(),
									   File::OPTO_CONNECTIONS_INFO_VCI, data, false);
	}

	bool VduAppSignalsInfoGenerator::writeTxtFile()
	{
		QStringList file;

		static const QString LINE(QString().fill('-', 216));

		file << QString(" VDU EquipmentID: %1\n").arg(m_vduOptoModule->equipmentID());

		file << LINE;
		file << "              Opto connections info file header";
		file << LINE;
		file << "  Address   | Header field          | Value";
		file << LINE;

		file << addrStr(sizeof(m_header.magic),
							QString("signature             | '%1\\0'").arg(m_header.magic));

		file << addrStr(sizeof(m_header.fileVersion),
							QString("fileVersion           | %1").arg(m_header.fileVersion)) ;

		file << addrStr(sizeof(m_header.optoPortsCount),
							QString("optoPortsCount        | %1").arg(m_header.optoPortsCount)) ;

		file << addrStr(sizeof(m_header.rxAppSignalsCount),
							QString("rxAppSignalsCount     | %1").arg(m_header.rxAppSignalsCount)) ;

		file << addrStr(sizeof(m_header.txAppSignalsCount),
							QString("txAppSignalsCount     | %1").arg(m_header.txAppSignalsCount)) ;

		file << addrStr(sizeof(m_header.refOptoPorts),
							QString("refOptoPorts          | %1").arg(hex32(m_header.refOptoPorts)));

		file << addrStr(sizeof(m_header.refRxAppSignals),
							QString("refRxAppSignals       | %1").arg(hex32(m_header.refRxAppSignals)));

		file << addrStr(sizeof(m_header.refTxAppSignals),
							QString("refTxAppSignals       | %1").arg(hex32(m_header.refTxAppSignals)));

		file << addrStr(sizeof(m_header.refStrings),
							QString("refStrings            | %1").arg(hex32(m_header.refStrings)));

		// file << addrStr(sizeof(m_header.reserve1),
		// 					QString("reserve1              | %1").arg(hex32(m_header.reserve1)));

		//

		file << LINE;
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

		file << LINE;

		// file << "              Received app signals info table";
		// printSignalsInfo(m_rxAppSignals, LINE, file);

		// file << LINE;

		// file << "              Transmitted app signals info table";
		// printSignalsInfo(m_txAppSignals, LINE, file);

		file << LINE;
		file << "              Strings table";
		file << LINE;

		int len = -1;
		QString str;
		size_t index = 0;
		char16_t ch = 0;
		QString printStr;

		while(index < m_strings.size())
		{
			int dataSizeW = 0;

			len = m_strings[index];
			index++;
			dataSizeW++;

			str.clear();
			str.reserve(len);
			printStr.clear();

			do
			{
				ch = m_strings[index];
				index++;
				dataSizeW++;

				if (ch != 0)
				{
					str.append(QChar(ch));
				}
				else
				{
					printStr.append(QString("%1, '%2\\0'").arg(len).arg(str));
				}
			}
			while(ch != 0 && index < m_strings.size());

			Q_ASSERT(len == str.length());

			if ((index % 2) != 0)
			{
				if (index < m_strings.size())
				{
					Q_ASSERT(m_strings[index] == 0);
					printStr.append(QStringLiteral(", 0x0000"));

					index++;
					dataSizeW++;
				}
				else
				{
					Q_ASSERT(false);
				}
			}

			file << addrStr(dataSizeW * sizeof(char16_t), printStr);
		}

		file << LINE;

		Q_ASSERT(m_crc64Offset == m_txtOffset);

		file << addrStr(sizeof(m_crc64), QString("CRC64 = %1").arg(hex64(m_crc64)));

		file << LINE;

		//

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

			file << Separator::EMPTY_STR;
			file << Separator::EMPTY_STR;
			file << "Indexes of opto signals to AppSignalIDs synonims (just for reference, not placed in *.vci file):";
			file << Separator::EMPTY_STR;

			file << LINE;
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

				file << QString(" %1      | %2").arg(hex16(indx)).arg(ids);
			}
		}

		//

		return m_resultWriter->addFile(Directory::VDUs + Separator::DIR + m_vduOptoModule->equipmentID(),
										File::OPTO_CONNECTIONS_INFO_TXT, file, false);
	}

	void VduAppSignalsInfoGenerator::printSignalsInfo(const std::vector<VduAppSignal>& appSignals,
														   const QString& line,
															QStringList& file)
	{
/*		file << line;
		file << QString("  Address   | signalIndex | portIndex | inOutType | signalType | offsetW | bitNo  | boolProps | refAppSignalID | refCustSignalID | refCaption | refUnit    | tunDefault | tunLowBound | tunHighBound | ioffset | ioBit");
		file << line;

		for(const VduAppSignal& si : appSignals)
		{
			file << addrStr(sizeof(si),
							QString("%1      | %2    | %3    | %4     | %5  | %6 | %7    | %8     | %9      | %10 | %11 | %12 | %13  | %14   | %15  | %16").
							arg(hex16(si.signalIndex)).
							arg(hex16(si.optoPortIndex)).
							arg(hex16(si.vduSignalInOutType)).
							arg(hex16(si.vduSignalType)).
							arg(hex16(si.valueOffsetW)).
							arg(hex16(si.valueBitNo)).
							arg(hex16(si.boolProps)).
							arg(hex32(si.refAppSignalID)).
							arg(hex32(si.refCustomAppSignalID)).
							arg(hex32(si.refCaption)).
							arg(hex32(si.refUnit)).
							arg(hex32(si.tuningDefaultValue)).
							arg(hex32(si.tuningLowBound)).
							arg(hex32(si.tuningHighBound)).
							arg(hex16(si.ioOffset)).
							arg(hex16(si.ioBit)));
		}*/
	}

	bool VduAppSignalsInfoGenerator::fillSignalsInfo(int portIndex,
														const QVector<Hardware::TxRxSignalShared>& portSignals,
														std::vector<VduAppSignal>& vduSignals)
	{

		auto it = m_context->m_vduSignals.find(m_vduOptoModule->equipmentID());

		if (it == m_context->m_vduSignals.end())
		{
			auto [newIt, b] = m_context->m_vduSignals.emplace(m_vduOptoModule->equipmentID(), std::map<Hash, int>{});
			it = newIt;
		}

		bool result = true;

//		std::map<Hash, int>& indexMap = it->second;

/*

		bool isTxSignals = (&vduSignals == &m_txAppSignals);

		for(const Hardware::TxRxSignalShared& portSignal : portSignals)
		{
			const QStringList& appSignalIDs = portSignal->appSignalIDs();

			if (appSignalIDs.size() == 0)
			{
				Q_ASSERT(false);
				continue;
			}

			result &= appendVduSignal(portIndex, appSignalIDs[0], portSignal->addrInBuf(), isTxSignals, true, vduSignals);
		}*/

		return result;
	}

	bool VduAppSignalsInfoGenerator::appendVduSignal(const QString& appSignalID, bool isRxSignal, Hash32* h32)
	{
		TEST_PTR_RETURN_FALSE(h32);

		qDebug() << appSignalID;

		*h32 = 0;

		auto it = m_vduSignals.find(appSignalID);

		if (it != m_vduSignals.end())
		{
			*h32 = it->second.hash;
			return true;				// already appended
		}

		const AppSignal* appSignal = m_signalSet->getSignal(appSignalID);

		if (appSignal == nullptr)
		{
			LOG_INTERNAL_ERROR_MSG(m_log, QString("AppSignal %1 not found").arg(appSignal->appSignalID()));
			return false;
		}

		Hash32 hash = calcHash32(appSignalID);

		*h32 = hash;

		auto [it2, b] = m_vduSignals.emplace(appSignalID, VduAppSignal{});

		VduAppSignal& si = it2->second;

		si.hash = hash;
		si.signalIndex = -1;		// will set after m_vduSignals full filling

		VduSignalInOutType inOutType = VduSignalInOutType::Internal;

		if (!isRxSignal)
		{
			// NOT rx signal
			//
			inOutType = getVduSignalInOutType(appSignal);
		}
		else
		{
			// rx signals always treats as Internal
		}

		si.vduSignalInOutType = static_cast<uint16_t>(inOutType);

		//

		VduSignalType vduSignalType = getVduSignalType(appSignal);

		si.vduSignalType = static_cast<uint16_t>(vduSignalType);

		si.boolProps = 0;

		if (!isRxSignal)
		{
			si.enableTuning = appSignal->enableTuning() ? 1 : 0;
		}

		//

		VduSignalInOutType ioType = VduSignalInOutType::Internal;

		si.ioOffset = 0xFFFF;
		si.ioBit = 0xFFFF;

/*		if (isTxSignals)
		{
			auto ioIt = m_inOutSignals.find(appSignal->appSignalID());

			if (ioIt != m_inOutSignals.end())
			{
				ioType = getVduSignalInOutType(appSignal);

				si.ioOffset = static_cast<quint16>(ioIt->second.second.offset());
				si.ioBit = static_cast<quint16>(ioIt->second.second.bit());

				//

				if (eraseIoSignal)
				{
					m_inOutSignals.erase(ioIt);
				}
			}
		}*/


		//

		// here this is offsets in m_string table, NOT in file!
		//
		si.refAppSignalID = appendString(appSignal->appSignalID());
		si.refCustomAppSignalID = appendString(appSignal->customAppSignalID());
		si.refCaption = appendString(appSignal->caption());
		si.refUnit = appendString(appSignal->unit());

		//

		if (appSignal->enableTuning())
		{
			si.tuningDefaultValue = appSignal->tuningDefaultValue().untypedUInt32Value();
			si.tuningLowBound = appSignal->tuningLowBound().untypedUInt32Value();
			si.tuningHighBound = appSignal->tuningHighBound().untypedUInt32Value();
		}
		else
		{
			si.tuningDefaultValue = 0;
			si.tuningLowBound = vduSignalLowBoundUntyped(vduSignalType);
			si.tuningHighBound = vduSignalHighBoundUntyped(vduSignalType);
		}

		//

		// si.ioOffset, si.ioBit - already filled

		return true;
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

	vdu_cstr VduAppSignalsInfoGenerator::appendString(const QString& str)
	{
		QByteArray utf8Str = str.toUtf8();

		Hash32 hash = ::calcHash32(utf8Str);

		auto it = m_stringRefs.find(hash);

		if (it != m_stringRefs.end())
		{
			return it->second;
		}

		vdu_cstr ref = static_cast<vdu_cstr>(m_strings.size() * sizeof(char16_t));

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

		size_t len = m_strings.size() % 4;

		if (len != 0)
		{
			// align m_strings on 4 bytes
			//
			while(len < 4)
			{
				m_strings.push_back(0);
				len++;
			}
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

	QString VduAppSignalsInfoGenerator::addrStr(int fieldSize, const QString& str)
	{
		QString res = QString(" %1 | %2").arg(hex32(m_txtOffset)).arg(str);

		m_txtOffset += fieldSize;

		return res;
	}

	QString VduAppSignalsInfoGenerator::hex16(qint64 v)
	{
		return QString("0x") + QString("%1").arg(v, 4, 16, QChar('0')).toUpper();
	}

	QString VduAppSignalsInfoGenerator::hex32(qint64 v)
	{
		return QString("0x") + QString("%1").arg(v, 8, 16, QChar('0')).toUpper();
	}

	QString VduAppSignalsInfoGenerator::hex64(quint64 v)
	{
		return QString("0x") + QString("%1").arg(v, 16, 16, QChar('0')).toUpper();
	}

	Hash32 VduAppSignalsInfoGenerator::calcHash32(const QString& str) const
	{
		return ::calcHash32(str.toUtf8());
	}

}
