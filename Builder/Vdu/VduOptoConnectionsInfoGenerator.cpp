#include "WUtils.h"
#include "Context.h"
#include "VduOptoConnectionsInfoGenerator.h"

namespace Builder
{
	VduOptoConnectionsInfoGenerator::VduOptoConnectionsInfoGenerator()
	{
		m_strings.reserve(32768);
		appendString(QString());
	}

	VduOptoConnectionsInfoGenerator::~VduOptoConnectionsInfoGenerator()
	{
	}

	bool VduOptoConnectionsInfoGenerator::writeFiles(Hardware::OptoModuleShared vduModule,
													Context* context)
	{
		TEST_PTR_RETURN_FALSE(vduModule);

		m_vduModule = vduModule;

		TEST_PTR_RETURN_FALSE(context);

		m_context = context;

		m_signalSet = context->m_signalSet;

		TEST_PTR_RETURN_FALSE(m_signalSet);

		m_resultWriter = context->m_buildResultWriter;

		TEST_PTR_RETURN_FALSE(m_resultWriter);

		m_log = m_resultWriter->log();

		TEST_PTR_RETURN_FALSE(m_log);

		if (vduModule->isVdu() == false)
		{
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		bool result = true;

		result &= fillPortsInfo();
		result &= fillAppSignalsInfo();

		result &= fillHeader();

		result &= writeVciFile();
		result &= writeTxtFile();

		return result;
	}

	bool VduOptoConnectionsInfoGenerator::fillPortsInfo()
	{
		m_optoPortsInfo.clear();

		int index = 0;

		for(const auto& [equipmentID, port] : m_vduModule->ports())
		{
			TEST_PTR_CONTINUE(port);

			VduOptoPortInfo pi;

			pi.optoPortIndex = index;
			index++;

			pi.linkID = port->linkID();

			pi.rxDataSizeW = port->rxDataSizeW();
			pi.rxDataUID = port->rxDataID();

			pi.txDataSizeW = port->txDataSizeW();
			pi.txDataUID = port->txDataID();

			m_optoPortsInfo.emplace_back(pi);
		}

		return true;
	}

	bool VduOptoConnectionsInfoGenerator::fillAppSignalsInfo()
	{
		m_rxAppSignals.clear();
		m_txAppSignals.clear();

		bool result = true;

		m_vduSignalIndex = 0;

		int portIndex = 0;

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

			result &= fillSignalsInfo(portIndex, port->rxSignals(), m_rxAppSignals);

			portIndex++;
		}


		return result;
	}

	bool VduOptoConnectionsInfoGenerator::fillHeader()
	{
		const uint32_t HEADER_SIZE = static_cast<uint32_t>(sizeof(m_header));
		const uint32_t OPTO_PORTS_INFO_SIZE = static_cast<uint32_t>(m_optoPortsInfo.size() * sizeof(VduOptoPortInfo));
		const uint32_t RX_APP_SIGNALS_INFO_SIZE = static_cast<uint32_t>(m_rxAppSignals.size() * sizeof(VduAppSignalInfo));
		const uint32_t TX_APP_SIGNALS_INFO_SIZE = static_cast<uint32_t>(m_txAppSignals.size() * sizeof(VduAppSignalInfo));

		// "VCI\0"
		//
		m_header.magic[0] = 'V';
		m_header.magic[1] = 'C';
		m_header.magic[2] = 'I';
		m_header.magic[3] = '\0';

		m_header.fileVersion = 1;

		m_header.optoPortsCount = static_cast<uint16_t>(m_optoPortsInfo.size());

		Q_ASSERT(m_header.optoPortsCount == VDU_OPTO_PORTS_COUNT);

		m_header.refOptoPortsInfo = HEADER_SIZE;

		m_header.refRxAppSignalsInfo = HEADER_SIZE +
										  OPTO_PORTS_INFO_SIZE;

		m_header.refTxAppSignalsInfo = HEADER_SIZE +
										  OPTO_PORTS_INFO_SIZE +
										  RX_APP_SIGNALS_INFO_SIZE;

		m_header.refStrings = HEADER_SIZE +
								 OPTO_PORTS_INFO_SIZE +
								 RX_APP_SIGNALS_INFO_SIZE +
								 TX_APP_SIGNALS_INFO_SIZE;

		recalcStringsRefs(m_header.refStrings);

		//

		m_header.reserve1 = 0;
		m_header.reserve2 = MARKER32;

		return true;
	}

	bool VduOptoConnectionsInfoGenerator::writeVciFile()
	{
		QByteArray data;

		data.append(reinterpret_cast<const char*>(&m_header),
					sizeof(m_header));

		data.append(reinterpret_cast<const char*>(m_optoPortsInfo.data()),
					m_optoPortsInfo.size() * sizeof(VduOptoPortInfo));

		data.append(reinterpret_cast<const char*>(m_rxAppSignals.data()),
					m_rxAppSignals.size() * sizeof(VduAppSignalInfo));

		data.append(reinterpret_cast<const char*>(m_txAppSignals.data()),
					m_txAppSignals.size() * sizeof(VduAppSignalInfo));

		data.append(reinterpret_cast<const char*>(m_strings.data()),
					m_strings.size() * sizeof(char16_t));

		return m_resultWriter->addFile(Directory::VDUs + Separator::DIR + m_vduModule->equipmentID(),
									   File::OPTO_CONNECTIONS_INFO_VCI, data, false);
	}

	bool VduOptoConnectionsInfoGenerator::writeTxtFile()
	{
		QStringList file;


		file << QString(" VDU EquipmentID: %1\n").arg(m_vduModule->equipmentID());

		file << QString("               -----------------------------------------------------------------------");
		file << QString("  Address       Header field         | Value");
		file << QString("               -----------------------------------------------------------------------\n");

		file << addrStr(sizeof(m_header.magic),
						  QString(" Signature            | '%1\\0'").arg(m_header.magic));

		file << addrStr(sizeof(m_header.fileVersion),
						  QString(" FileVersion          | %1").arg(m_header.fileVersion)) ;

		file << addrStr(sizeof(m_header.optoPortsCount),
						  QString(" OptoPortsCount       | %1").arg(m_header.optoPortsCount)) ;

		file << addrStr(sizeof(m_header.refOptoPortsInfo),
						  QString(" RefOptoPortsInfo     | %1").arg(hex32(m_header.refOptoPortsInfo)));

		file << addrStr(sizeof(m_header.refRxAppSignalsInfo),
						  QString(" RefRxAppSignalsInfo  | %1").arg(hex32(m_header.refRxAppSignalsInfo)));

		file << addrStr(sizeof(m_header.refTxAppSignalsInfo),
						  QString(" RefTxAppSignalsInfo  | %1").arg(hex32(m_header.refTxAppSignalsInfo)));

		file << addrStr(sizeof(m_header.refStrings),
						  QString(" RefStrings           | %1").arg(hex32(m_header.refStrings)));

		file << addrStr(sizeof(m_header.reserve1), QString(" Reserve1             | %1").arg(hex32(m_header.reserve1)));
		file << addrStr(sizeof(m_header.reserve2), QString(" Reserve2             | %1").arg(hex32(m_header.reserve2)));

		//

		return m_resultWriter->addFile(Directory::VDUs + Separator::DIR + m_vduModule->equipmentID(),
										File::OPTO_CONNECTIONS_INFO_TXT, file, false);
	}

	bool VduOptoConnectionsInfoGenerator::fillSignalsInfo(int portIndex,
														const QVector<Hardware::TxRxSignalShared>& portSignals,
														std::vector<VduAppSignalInfo>& vduSignals)
	{
		auto it = m_context->m_vduSignals.find(m_vduModule->equipmentID());

		if (it == m_context->m_vduSignals.end())
		{
			auto [newIt, b] = m_context->m_vduSignals.emplace(m_vduModule->equipmentID(), std::map<Hash, int>{});
			it = newIt;
		}

		std::map<Hash, int>& indexMap = it->second;

		bool result = true;

		for(const Hardware::TxRxSignalShared& portSignal : portSignals)
		{
			const AppSignal* appSignal = m_signalSet->getSignal(portSignal->appSignalID());

			if (appSignal == nullptr)
			{
				LOG_INTERNAL_ERROR_MSG(m_log, QString("AppSignal %1 not found").arg(portSignal->appSignalID()));
				result = false;
				continue;
			}

			VduAppSignalInfo si;

			si.portIndex = portIndex;
			si.signalIndex = m_vduSignalIndex;

			indexMap.emplace(calcHash(portSignal->appSignalID()), m_vduSignalIndex);

			m_vduSignalIndex++;

			si.vduSignalType = static_cast<uint16_t>(getVduSignalType(portSignal));

			Address16 addrInBuf = portSignal->addrInBuf();

			si.valueOffsetW = addrInBuf.offset();
			si.valueBitNo = addrInBuf.bit();

			si.reserv1 = MARKER16;

			// here this is offsets in m_string table, NOT in file!
			//
			si.refAppSignalID = appendString(appSignal->appSignalID());
			si.refCustomAppSignalID = appendString(appSignal->customAppSignalID());
			si.refCaption = appendString(appSignal->caption());
			si.refUnit = appendString(appSignal->unit());

			si.reserv2 = MARKER32;

			vduSignals.emplace_back(si);
		}

		return result;
	}

	VduSignalType VduOptoConnectionsInfoGenerator::getVduSignalType(Hardware::TxRxSignalShared s)
	{
		TEST_PTR_RETURN_VALUE(s, VduSignalType::Unknown);

		switch(s->signalType())
		{
		case E::Discrete:
			return VduSignalType::Discrete;

		case E::Analog:
			switch(s->analogFormat())
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

	vdu_string_ref VduOptoConnectionsInfoGenerator::appendString(const QString& str)
	{
		if (str.isEmpty() == true && m_strings.size() > 0)
		{
			// first string always "empty string"!
			return 0;
		}

		vdu_string_ref ref = static_cast<vdu_string_ref>(m_strings.size() * sizeof(char16_t));

		Q_ASSERT(str.length() < std::numeric_limits<char16_t>::max());

		m_strings.push_back(static_cast<char16_t>(str.length()));

		const QChar* strChar = str.data();

		TEST_PTR_RETURN_VALUE(strChar, 0);

		while(strChar->isNull() == false)
		{
			m_strings.push_back(strChar->unicode());
			strChar++;
		}

		m_strings.push_back(QChar::Null);

		if ((m_strings.size() % 2) != 0)
		{
			m_strings.push_back(QChar::Null);
		}

		return ref;
	}

	void VduOptoConnectionsInfoGenerator::recalcStringsRefs(uint32_t stringsOffsetInFile)
	{
		for(VduAppSignalInfo& s : m_rxAppSignals)
		{
			s.refAppSignalID += stringsOffsetInFile;
			s.refCustomAppSignalID += stringsOffsetInFile;
			s.refCaption += stringsOffsetInFile;
			s.refUnit += stringsOffsetInFile;
		}

		for(VduAppSignalInfo& s : m_txAppSignals)
		{
			s.refAppSignalID += stringsOffsetInFile;
			s.refCustomAppSignalID += stringsOffsetInFile;
			s.refCaption += stringsOffsetInFile;
			s.refUnit += stringsOffsetInFile;
		}
	}

	QString VduOptoConnectionsInfoGenerator::addrStr(int fieldSize, const QString& str)
	{
		static int offset = 0;

		QString res = QString(" %1    %2").arg(hex32(offset)).arg(str);

		offset += fieldSize;

		return res;
	}

	QString VduOptoConnectionsInfoGenerator::hex32(qint64 v)
	{
		return QString("0x") + QString("%1").arg(v, 8, 16, QChar('0')).toUpper();
	}

	QString VduOptoConnectionsInfoGenerator::hex16(qint64 v)
	{
		return QString("0x") + QString("%1").arg(v, 4, 16, QChar('0')).toUpper();
	}
}
