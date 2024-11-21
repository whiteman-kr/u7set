#pragma once

#include "VduAppSignals.h"

#include "OptoModule.h"
#include "BuildResultWriter.h"
#include "SignalSet.h"

namespace Builder
{
	class ModuleLogicCompiler;
	class Context;

	class VduAppSignalsInfoGenerator
	{
		inline static const uint32_t MARKER16 = 0x2323;			//	'##'
		inline static const uint32_t MARKER32 = 0x23232323;		//	'####'

	public:
		VduAppSignalsInfoGenerator();
		virtual ~VduAppSignalsInfoGenerator();

		bool writeFiles(const ModuleLogicCompiler* mlc);
	private:
		bool fillPortsInfo();
		bool fillAppSignalsInfo();
		bool fillHeader();

		bool writeVciFile();
		bool writeTxtFile();

		void printSignalsInfo(const std::vector<VduAppSignal>& appSignals,
							  const QString& line,
							  QStringList& file);

		bool fillSignalsInfo(int portIndex,
							 const QVector<Hardware::TxRxSignalShared>& portSignals,
							 std::vector<VduAppSignal>& vduSignals);

		bool appendVduSignal(const QString& appSignalID, bool isRxSignal, Hash32* h32);

		VduSignalInOutType getVduSignalInOutType(const AppSignal* s);
		VduSignalType getVduSignalType(const AppSignal* s);

		uint32_t vduSignalLowBoundUntyped(VduSignalType type);
		uint32_t vduSignalHighBoundUntyped(VduSignalType type);

		// returns offset in m_strings NOT in file!
		//
		vdu_cstr appendString(const QString& str);

		void recalcStringsRefs(uint32_t stringsOffsetInFile);

		QString addrStr(int fieldSize, const QString& str);
		QString hex16(qint64 v);
		QString hex32(qint64 v);
		QString hex64(quint64 v);

		Hash32 calcHash32(const QString& str) const;

	private:
		const ModuleLogicCompiler* m_mlc = nullptr;
		Context* m_context = nullptr;
		Hardware::OptoModuleShared m_vduOptoModule;
		SignalSetShared m_signalSet;
		BuildResultWriterShared m_resultWriter;
		IssueLogger* m_log = nullptr;

		int m_txtOffset = 0;

		VduAppSignalsFileHeader m_header;
		std::map<QString, VduAppSignal> m_vduSignals;			// AppSignalID => VduAppSignal
		std::map<Hash32, quint16> m_hashToSignalIndex;
		std::map<Hash32, Hash32> m_rxSignalsSynonims;			// calcHash32(rxSignal appSignalID synonim) => calcHash32(rxSignal->appSignalIDs[0])
		std::vector<VduOptoPort> m_optoPorts;
		std::vector<VduTxRxAppSignal> m_rxAppSignals;
		std::vector<VduTxRxAppSignal> m_txAppSignals;

		std::vector<uint8_t> m_strings;
		std::map<Hash32, vdu_cstr> m_stringRefs;

		uint64_t m_crc64 = 0;
		uint64_t m_crc64Offset = 0;
	};
}
