#pragma once

#include "ApplicationLogicCode.h"

namespace Builder
{
	class ModuleLogicCompiler;

	class CodeChecker;

	typedef bool (CodeChecker::*CheckFuncPtr)(const CodeItem& cmd);

	class CodeChecker
	{
	public:

		class MemArea
		{
		public:
			MemArea();
			MemArea(quint32 startAddr, quint32 sizeW);

			bool isValid() const;

			void setStartAddr(quint32 startAddr);
			quint32 startAddr() const;

			void setSizeW(quint32 sizeW);
			quint32 sizeW() const;
			void addSizeW(quint32 sizeW);

			bool addressInArea(quint32 addr, quint32 sizeW) const;

		private:
			inline static const quint32 NOT_INIT = 0xFFFFFFFF;

			quint32 m_startAddr = NOT_INIT;
			quint32 m_sizeW = NOT_INIT;
		};


	public:
		CodeChecker(const ModuleLogicCompiler& compiler);
		~CodeChecker();

		bool check(const AppLogicCode& appLogicCode);

	private:
		bool init();

		bool initReadableAreas();
		bool initWritableAreas();
		void joiningSequentialAreas(std::map<quint32, MemArea>* areas);

		bool initPartialWrittenAddresses();
		bool initLoopbackDiscretes();

		void initToRead(const MemArea& ma);

		void logError(const CodeItem& cmd, const QString& err) const;

		bool check(const CodeItem& cmd);

		//

		bool checkNoCommand(const CodeItem& cmd);
		bool checkNop(const CodeItem& cmd);
		bool checkStart(const CodeItem& cmd);
		bool checkStop(const CodeItem& cmd);
		bool checkMov(const CodeItem& cmd);
		bool checkMovMem(const CodeItem& cmd);
		bool checkMovConst(const CodeItem& cmd);
		bool checkMovBitConst(const CodeItem& cmd);
		bool checkWriteFuncBlock(const CodeItem& cmd);
		bool checkReadFuncBlock(const CodeItem& cmd);
		bool checkWriteFuncBlockConst(const CodeItem& cmd);
		bool checkWriteFuncBlockBit(const CodeItem& cmd);
		bool checkReadFuncBlockBit(const CodeItem& cmd);
		bool checkReadFuncBlockTest(const CodeItem& cmd);
		bool checkSetMem(const CodeItem& cmd);
		bool checkMovBit(const CodeItem& cmd);
		bool checkNstart(const CodeItem& cmd);
		bool checkAppStart(const CodeItem& cmd);
		bool checkMov32(const CodeItem& cmd);
		bool checkMovConst32(const CodeItem& cmd);
		bool checkWriteFuncBlock32(const CodeItem& cmd);
		bool checkReadFuncBlock32(const CodeItem& cmd);
		bool checkWriteFuncBlockConst32(const CodeItem& cmd);
		bool checkReadFuncBlockTest32(const CodeItem& cmd);
		bool checkMovCompareFlag(const CodeItem& cmd);
		bool checkPrevMov(const CodeItem& cmd);
		bool checkPrevMov32(const CodeItem& cmd);
		bool checkFill(const CodeItem& cmd);

		//

		bool checkFbTypeAndInstance(const CodeItem& cmd);

		bool checkCanRead16(const CodeItem& cmd, quint32 readAddr) const;
		bool checkCanRead32(const CodeItem& cmd, quint32 readAddr) const;
		bool checkCanRead(const CodeItem& cmd, quint32 readAddr, quint32 sizeW, bool enableReadUnwritten = false) const;
		bool checkCanReadBit(const CodeItem& cmd, quint32 readAddr, quint32 bitNo) const;
		bool addrCanBePartialWritten(quint32 readAddr) const;

		bool checkCanWrite16(const CodeItem& cmd, quint32 writeAddr) const;
		bool checkCanWrite32(const CodeItem& cmd, quint32 writeAddr) const;
		bool checkCanWrite(const CodeItem& cmd, quint32 writeAddr, quint32 sizeW) const;
		bool checkCanWriteBit(const CodeItem& cmd, quint32 writeAddr, quint32 bitNo) const;
		void writeBit(quint32 writeAddr, quint32 bitNo) const;

		const MemArea& findMemAreaToWrite(quint32 writeAddr, quint32 sizeW) const;
		const MemArea& findMemAreaToRead(quint32 readAddr, quint32 sizeW) const;
		const MemArea& findMemArea(const std::map<quint32, MemArea>& areas,
								   quint32 addr, quint32 sizeW) const;

	private:
		const ModuleLogicCompiler& m_compiler;
		const LmDescription* m_lmDesc = nullptr;
		mutable IssueLogger* m_log = nullptr;

		static CheckFuncPtr m_checkFunc[LM_COMMANDS_COUNT];

		quint16* m_mem = nullptr;
		quint32 m_memSizeW = 0;

		std::map<quint32, MemArea> m_readAreas;
		std::map<quint32, MemArea> m_writeAreas;

		std::set<quint32> m_addrCanBeParialWritten;

		inline static const MemArea m_notValidArea;
	};
}
