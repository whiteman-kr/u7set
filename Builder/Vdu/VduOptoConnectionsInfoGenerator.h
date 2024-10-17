#pragma once

#include "BuildResultWriter.h"
#include "OptoModule.h"
#include "SignalSet.h"

#include "VduOptoConnectionsInfoFile.h"

namespace Builder
{
	class VduOptoConnectionsInfoGenerator
	{
		inline static const uint32_t MARKER16 = 0x2323;			//	'##'
		inline static const uint32_t MARKER32 = 0x23232323;		//	'####'

	public:
		VduOptoConnectionsInfoGenerator();
		virtual ~VduOptoConnectionsInfoGenerator();

		bool writeFiles(Hardware::OptoModuleShared vduModule,
						Context* context);
	private:
		bool fillPortsInfo();
		bool fillAppSignalsInfo();
		bool fillHeader();

		bool writeVciFile();
		bool writeTxtFile();

		void printSignalsInfo(const std::vector<VduAppSignalInfo>& appSignals,
							  const QString& line,
							  QStringList& file);

		bool fillSignalsInfo(int portIndex,
							 const QVector<Hardware::TxRxSignalShared>& portSignals,
							 std::vector<VduAppSignalInfo>& vduSignals);

		VduSignalType getVduSignalType(Hardware::TxRxSignalShared s);
		uint32_t vduSignalLowBoundUntyped(VduSignalType type);
		uint32_t vduSignalHighBoundUntyped(VduSignalType type);

		// returns offset in m_strings NOT in file!
		//
		vdu_string_ref appendString(const QString& str);

		void recalcStringsRefs(uint32_t stringsOffsetInFile);

		QString addrStr(int fieldSize, const QString& str);
		QString hex16(qint64 v);
		QString hex32(qint64 v);
		QString hex64(quint64 v);

	private:
		Context* m_context = nullptr;
		Hardware::OptoModuleShared m_vduModule;
		SignalSetShared m_signalSet;
		BuildResultWriterShared m_resultWriter;
		IssueLogger* m_log = nullptr;

		int m_vduSignalIndex = 0;
		int m_txtOffset = 0;

		VduOptoConnectionsInfoFileHeader m_header;
		std::vector<VduOptoPortInfo> m_optoPortsInfo;
		std::vector<VduAppSignalInfo> m_rxAppSignals;
		std::vector<VduAppSignalInfo> m_txAppSignals;

		std::vector<char16_t> m_strings;
		std::map<Hash, vdu_string_ref> m_stringRefs;

		uint64_t m_crc64 = 0;
		uint64_t m_crc64Offset = 0;
	};
}
