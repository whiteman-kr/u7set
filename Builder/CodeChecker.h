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
		CodeChecker(const ModuleLogicCompiler& compiler);
		~CodeChecker();

		bool check(const AppLogicCode& appLogicCode);

	private:

		inline static const quint32 NOT_VALUE = 0xFFFFFFFF;

		struct MemArea
		{
			quint32 startAddr = NOT_VALUE;
			quint32 sizeW = NOT_VALUE;

			bool isValid() const { return (startAddr != NOT_VALUE && sizeW != NOT_VALUE); }
		};

	private:
		bool init();

		bool initReadableAreas();
		void initToRead(const MemArea& ma);

		bool initWritableAreas();

		void logError(const QString& err, const CodeItem& cmd);

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
		bool checkMovConstIfFlag(const CodeItem& cmd);
		bool checkPrevMov(const CodeItem& cmd);
		bool checkPrevMov32(const CodeItem& cmd);
		bool checkFill(const CodeItem& cmd);

		//

		bool checkFbTypeAndInstance(const CodeItem& cmd);
		bool checkCanRead(const CodeItem& cmd, quint32 readAddr, quint32 sizeW);
		bool checkCanWrite(const CodeItem& cmd, quint32 writeAddr, quint32 sizeW);


	private:
		const ModuleLogicCompiler& m_compiler;
		const LmDescription* m_lmDesc = nullptr;
		IssueLogger* m_log = nullptr;

		static CheckFuncPtr m_checkFunc[LM_COMMANDS_COUNT];

		quint16* m_mem = nullptr;
		quint32 m_memSizeW = 0;

		std::map<quint32, MemArea> m_readAreas;
		std::map<quint32, MemArea> m_writeAreas;
	};
}
