#pragma once

#include "AppLogicCode.h"

namespace Builder
{
	class ModuleLogicCompiler;

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
		CodeChecker(ModuleLogicCompiler& compiler);
		~CodeChecker();

		bool check(const AppLogicCode& appLogicCode);

	private:
		bool init();

		bool initReadableAreas();
		bool initOtherModulesReadableAreas();
		bool initActuatorReadableAreas();

		bool initWritableAreas();
		bool initOtherModulesWritableAreas();
		bool initActuatorWritableAreas();

		void joiningSequentialAreas(std::map<quint32, MemArea>* areas);

		bool initPartialWrittenAddresses();
		bool initLoopbackDiscretes();

		void initToRead(const MemArea& ma);

		void logError(const CodeItem& cmd, const QString& err) const;

		bool check(const CodeItem& cmd);

		//

		bool check_nothing(const CodeItem& cmd);
		bool check_startafb(const CodeItem& cmd);
		bool check_mov(const CodeItem& cmd);
		bool check_mov_addr_acc(const CodeItem& cmd);
		bool check_mov_acc_addr(const CodeItem& cmd);
		bool check_movmem(const CodeItem& cmd);
		bool check_movc(const CodeItem& cmd);
		bool check_movbc(const CodeItem& cmd);
		bool check_wrfb(const CodeItem& cmd);
		bool check_rdfb(const CodeItem& cmd);
		bool check_wrfbc(const CodeItem& cmd);
		bool check_wrfbb(const CodeItem& cmd);
		bool check_rdfbb(const CodeItem& cmd);
		bool check_rdfbcmp(const CodeItem& cmd);
		bool check_setmem(const CodeItem& cmd);
		bool check_movb(const CodeItem& cmd);
		bool check_movb_acc_addr(const CodeItem& cmd);
		bool check_movb_addr_acc(const CodeItem& cmd);
		bool check_nstart(const CodeItem& cmd);
		bool check_mov32(const CodeItem& cmd);
		bool check_movc32(const CodeItem& cmd);
		bool check_wrfb32(const CodeItem& cmd);
		bool check_rdfb32(const CodeItem& cmd);
		bool check_wrfbc32(const CodeItem& cmd);
		bool check_rdfbcmp32(const CodeItem& cmd);
		bool check_movcmpf(const CodeItem& cmd);
		bool check_pmov(const CodeItem& cmd);
		bool check_pmov32(const CodeItem& cmd);
		bool check_fillb(const CodeItem& cmd);

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

		bool addrInBitMemArea(quint32 addr, quint32 sizeW) const;
		bool addrNotInBitMemArea(quint32 addr, quint32 sizeW) const;

	private:
		ModuleLogicCompiler& m_compiler;
		LmDescriptionConstShared m_lmDesc;
		const std::map<int, LmCommand>* m_lmCommands = nullptr;
		mutable IssueLogger* m_log = nullptr;

		using CheckFuncPtr = bool (CodeChecker::*)(const CodeItem& cmd);

		static inline const std::map<QString, CheckFuncPtr> m_checkFuncMap =		// checkFuncName => checkFuncPtr
		{
			{ QStringLiteral("check_nothing"), &CodeChecker::check_nothing },
			{ QStringLiteral("check_startafb"), &CodeChecker::check_startafb },
			{ QStringLiteral("check_mov"), &CodeChecker::check_mov },
			{ QStringLiteral("check_mov_addr_acc"), &CodeChecker::check_mov_addr_acc },
			{ QStringLiteral("check_mov_acc_addr"), &CodeChecker::check_mov_acc_addr },
			{ QStringLiteral("check_movmem"), &CodeChecker::check_movmem },
			{ QStringLiteral("check_movc"), &CodeChecker::check_movc },
			{ QStringLiteral("check_movbc"), &CodeChecker::check_movbc },
			{ QStringLiteral("check_wrfb"), &CodeChecker::check_wrfb },
			{ QStringLiteral("check_rdfb"), &CodeChecker::check_rdfb },
			{ QStringLiteral("check_wrfbc"), &CodeChecker::check_wrfbc },
			{ QStringLiteral("check_wrfbb"), &CodeChecker::check_wrfbb },
			{ QStringLiteral("check_rdfbb"), &CodeChecker::check_rdfbb },
			{ QStringLiteral("check_rdfbcmp"), &CodeChecker::check_rdfbcmp },
			{ QStringLiteral("check_setmem"), &CodeChecker::check_setmem },
			{ QStringLiteral("check_movb"), &CodeChecker::check_movb },
			{ QStringLiteral("check_movb_acc_addr"), &CodeChecker::check_movb_acc_addr },
			{ QStringLiteral("check_movb_addr_acc"), &CodeChecker::check_movb_addr_acc },
			{ QStringLiteral("check_nstart"), &CodeChecker::check_nstart },
			{ QStringLiteral("check_mov32"), &CodeChecker::check_mov32 },
			{ QStringLiteral("check_movc32"), &CodeChecker::check_movc32 },
			{ QStringLiteral("check_wrfb32"), &CodeChecker::check_wrfb32 },
			{ QStringLiteral("check_rdfb32"), &CodeChecker::check_rdfb32 },
			{ QStringLiteral("check_wrfbc32"), &CodeChecker::check_wrfbc32 },
			{ QStringLiteral("check_rdfbcmp32"), &CodeChecker::check_rdfbcmp32 },
			{ QStringLiteral("check_movcmpf"), &CodeChecker::check_movcmpf },
			{ QStringLiteral("check_pmov"), &CodeChecker::check_pmov },
			{ QStringLiteral("check_pmov32"), &CodeChecker::check_pmov32 },
			{ QStringLiteral("check_fillb"), &CodeChecker::check_fillb }
		};

		quint16* m_mem = nullptr;
		quint32 m_memSizeW = 0;

		std::map<quint32, MemArea> m_readAreas;			// startAddr => MemArea	(startAddr == MemArea.startAddr)
		std::map<quint32, MemArea> m_writeAreas;		// startAddr => MemArea	(startAddr == MemArea.startAddr)

		std::set<quint32> m_addrCanBeParialWritten;

		MemArea m_bitMemArea;
		MemArea m_appWordMemArea;

		inline static const MemArea m_notValidArea;
	};
}
