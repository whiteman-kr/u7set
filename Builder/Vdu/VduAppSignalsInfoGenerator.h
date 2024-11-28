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
	public:
		VduAppSignalsInfoGenerator();
		virtual ~VduAppSignalsInfoGenerator();

		bool writeFiles(const ModuleLogicCompiler* mlc);

	private:
		bool buildIoSignalsAddrMap();
		bool fillAppSignalsInfo();
		bool fillOptoPortsInfo();
		bool fillTxRxSignalsInfo(uint16_t portIndex,
								 const QVector<Hardware::TxRxSignalShared>& portTxRxSignals,
								 std::vector<VduTxRxAppSignal>* txRxSignals);
		bool fillHeader();

		bool appendVduSignal(const QString& appSignalID, bool isRxSignal,
							 uint16_t portIndex, const Address16& rxAddr, Hash32* h32);
		bool appendHash32AppSignalID(const QString& appSignalID);
		vdu_cstr appendString(const QString& str);
		void recalcStringsRefs(uint32_t stringsOffsetInFile);

		bool writeBinFile();
		bool writeTxtFile();

		void printHeader(QStringList& file) const;
		void printAppSignals(QStringList& file) const;
		void printHashToIndex(QStringList& file) const;
		void printOptoPortsInfo(QStringList& file) const;
		void printTxRxSignalsInfo(QStringList& file,
								  const std::vector<VduTxRxAppSignal>& txRxSignals) const;
		void printStringsTable(QStringList& file) const;
		void printCrc64(QStringList& file) const;
		void printRefInfo(QStringList& file) const;

		VduSignalInOutType getVduSignalInOutType(const AppSignal* s);
		VduSignalType getVduSignalType(const AppSignal* s);

		uint32_t vduSignalLowBoundUntyped(VduSignalType type);
		uint32_t vduSignalHighBoundUntyped(VduSignalType type);

		uint32_t vduSignalUntypedValue(VduSignalType type, double dbValue);

		QString addrStr(int fieldSize, const QString& str) const;
		QString hex16(qint64 v) const;
		QString hex32(qint64 v) const;
		QString hex64(quint64 v) const;

		Hash32 utf8Hash32(const QString& str) const;

	private:
		const ModuleLogicCompiler* m_mlc = nullptr;
		Context* m_context = nullptr;
		Hardware::OptoModuleShared m_vduOptoModule;
		SignalSetShared m_signalSet;
		BuildResultWriterShared m_resultWriter;
		IssueLogger* m_log = nullptr;

		std::map<QString, Address16> m_ioSignalsAddr;			// In/Out AppSignalID => Address16
		mutable int m_txtOffset = 0;

		VduAppSignalsFileHeader m_header;
		std::map<QString, VduAppSignal> m_vduSignals;			// AppSignalID => VduAppSignal
		std::map<Hash32, quint16> m_hash32ToSignalIndex;		// utf8Hash32(AppSignalID) => VduAppSignal.signalIndex

		std::map<Hash32, QString> m_hash32AppSignalID;			// utf8Hash32(AppSignalID) => AppSignalID,
																// to check Hash32 collisions and
																// m_context->m_vduSignals filling
		std::vector<VduOptoPort> m_optoPorts;

		std::vector<VduTxRxAppSignal> m_txAppSignals;
//		std::vector<VduTxRxAppSignal> m_rxAppSignals;

		std::vector<uint8_t> m_strings;
		std::map<Hash32, vdu_cstr> m_stringRefs;

		uint64_t m_crc64 = 0;
		uint64_t m_crc64Offset = 0;

		inline static const QString LLINE = QString().fill('-', 203);
		inline static const QString LINE = QString().fill('-', 90);

		inline static const uint16_t NOT_VALID16 = 0xFFFF;
	};
}
