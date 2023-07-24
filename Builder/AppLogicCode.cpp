#include "AppLogicCode.h"
#include "ModuleLogicCompiler.h"

#include "../HardwareLib/Afb.h"
#include "../lib/ConstStrings.h"

namespace Builder
{
	// -------------------------------------------------------------------------------------------b----
	//
	// CodeSnippet class implementation
	//
	// -----------------------------------------------------------------------------------------------

	CodeSnippet::CodeSnippet()
	{
	}

	void CodeSnippet::append(const CodeItem& codeItem)
	{
		m_code.emplace_back(codeItem);
	}

	void CodeSnippet::append(const CodeSnippet& codeSnippet)
	{
		m_code.insert(m_code.end(), codeSnippet.m_code.begin(), codeSnippet.m_code.end());
	}

	CodeSnippet& CodeSnippet::operator << (const CodeItem& ci)
	{
		append(ci);
		return *this;
	}

	CodeSnippet& CodeSnippet::operator << (const CodeSnippet& codeShippet)
	{
		append(codeShippet);
		return *this;
	}

	CodeSnippet& CodeSnippet::operator << (const QString& commentStr)
	{
		comment(commentStr);
		return *this;
	}

	void CodeSnippet::comment(const QString& cmt)
	{
		CodeItem commentItem;

		commentItem.setComment(cmt);

		append(commentItem);
	}

	void CodeSnippet::newLine()
	{
		comment(QString());
	}

	void CodeSnippet::comment_nl(const QString& cmt)
	{
		comment(cmt);
		newLine();
	}

	void CodeSnippet::finalizeByNewLine()
	{
		if (m_code.back().isNewLine() == false)
		{
			newLine();
		}
	}

	void CodeSnippet::clear()
	{
		m_code.clear();
	}

	void CodeSnippet::reserve(int size)
	{
		m_code.reserve(size);
	}

	void CodeSnippet::swap(CodeSnippet& code)
	{
		m_code.swap(code.m_code);
	}

	bool CodeSnippet::isEmpty() const
	{
		return m_code.empty();
	}

	int CodeSnippet::itemsCount() const
	{
		return static_cast<int>(m_code.size());
	}

	int CodeSnippet::codeSizeW(LmDescriptionConstShared lmDesc) const
	{
		TEST_PTR_RETURN_VALUE(lmDesc, 0);

		int sizeW = 0;

		for(const auto& ci : m_code)
		{
			if (ci.isCommand() == false)
			{
				continue;
			}

			const LmCommand* lmCmd = lmDesc->commandPtr(ci.lmCommandCode());

			TEST_PTR_CONTINUE(lmCmd);

			sizeW += lmCmd->codeSize;
		}

		return sizeW;
	}

	int CodeSnippet::codeSizeW(LmDescriptionConstShared lmDesc,
								CodeSnippetConstIterator start,
								CodeSnippetConstIterator end) const
	{
		TEST_PTR_RETURN_VALUE(lmDesc, 0);

		int sizeW = 0;

		CodeSnippetConstIterator it = start;

		while(it != m_code.end())
		{
			if (it->isCommand() == true)
			{
				const LmCommand* lmCmd = lmDesc->commandPtr(it->lmCommandCode());

				if (lmCmd == nullptr)
				{
					Q_ASSERT(false);
				}
				else
				{
					sizeW += lmCmd->codeSize;
				}
			}

			if (it == end)
			{
				break;
			}

			it++;
		}

		return sizeW;
	}

	void CodeSnippet::getAsmCode(LmDescriptionConstShared lmDesc, QStringList* asmCode) const
	{
		TEST_PTR_RETURN(asmCode);

		asmCode->clear();

		for(const CodeItem& codeItem : m_code)
		{
			QString str = codeItem.getAsmCode(lmDesc, true, false);

			asmCode->append(str);
		}
	}

	void CodeSnippet::getBinCode(QByteArray* binCode) const
	{
		TEST_PTR_RETURN(binCode);

		binCode->clear();

		for(const CodeItem& codeItem : m_code)
		{
			QByteArray cmdBinCode;

			codeItem.generateBinCode(&cmdBinCode);

			binCode->append(cmdBinCode);
		}
	}

/*	void CodeSnippet::getMifCode(QStringList* mifCode) const
	{
		TEST_PTR_RETURN(mifCode);

		mifCode->clear();

		if (m_code.size() < 1)
		{
			return;
		}

		int width = 16;
		int depth = 0;

		// find last command for compute address depth
		//
		qsizetype codeItemsCount = m_code.size();

		for(qsizetype i = codeItemsCount - 1; i >= 0; i--)
		{
			if (m_code[i].isComment() == true)
			{
				continue;
			}

			depth = m_code[i].address() + m_code[i].sizeW() - 1;
			break;
		}

		mifCode->append(QString("WIDTH = %1;").arg(width));
		mifCode->append(QString("DEPTH = %1;").arg(depth + 1));

		mifCode->append("");

		mifCode->append("ADDRESS_RADIX = HEX;");
		mifCode->append("DATA_RADIX = HEX;");

		mifCode->append("");

		mifCode->append("CONTENT");
		mifCode->append("BEGIN");

		QString codeStr;
		QString str;

		for(const CodeItem& codeItem : m_code)
		{
			if (codeItem.isComment() == true)
			{
				if (codeItem.comment().isEmpty() == true)
				{
					str.clear();
				}
				else
				{
					str = QString("\t-- %1").arg(codeItem.comment());
				}

				mifCode->append(str);

				continue;
			}

			QByteArray binCode;

			codeItem.generateBinCode(&binCode);

			Q_ASSERT((binCode.count() % 2) == 0);

			qsizetype bytesCount = binCode.count();

			for(qsizetype i = 0; i < bytesCount; i++)
			{
				if (i == 0)
				{
					str = QString("\t%1 : ").arg(codeItem.address(), 4, 16, Latin1Char::ZERO);
					codeStr = str;
				}

				unsigned int b = binCode[i];

				b &= 0xFF;

				if ((i % 2) == 1)
				{
					if (i == bytesCount-1)
					{
						str = QString("%1;").arg(b, 2, 16, Latin1Char::ZERO);
					}
					else
					{
						str = QString("%1 ").arg(b, 2, 16, Latin1Char::ZERO);
					}
				}
				else
				{
					str = QString("%1").arg(b, 2, 16, Latin1Char::ZERO);
				}

				codeStr += str;
			}

			qsizetype tabLen = 40 - (codeStr.length() - 1 + 8);
			qsizetype tabCount = tabLen / 8 + ((tabLen % 8) ? 1 : 0);

			for(qsizetype i = 0; i < tabCount; i++)
			{
				codeStr += "\t";
			}

			str = QString("-- %1").arg(codeItem.mnemoCode());

			codeStr += str;

			mifCode->append(codeStr);
		}

		mifCode->append("END;");
	}  */

	void CodeSnippet::getAsmMetadataFields(QStringList* metadataFields, int* metadataVersion) const
	{
		TEST_PTR_RETURN(metadataFields);
		TEST_PTR_RETURN(metadataVersion);

		const int ASM_METADATA_VERSION = 1;
		*metadataVersion = ASM_METADATA_VERSION;

		metadataFields->clear();

		metadataFields->append("IsCommand");
		metadataFields->append("Address");
		metadataFields->append("BinCode");
		metadataFields->append("MnemoCode");
		metadataFields->append("Comment");
	}

	void CodeSnippet::getAsmMetadata(LmDescriptionConstShared lmDesc, std::vector<QVariantList>* metadata) const
	{
		TEST_PTR_RETURN(metadata);

		metadata->clear();

		for(const CodeItem& codeItem : m_code)
		{
			QVariantList data;

			bool isCommand = false;
			QString address;
			QString binCode;
			QString mnemoCode;
			QString comment;

			if (codeItem.isCommand() == true)
			{
				isCommand = true;

				address = QString("%1").arg(codeItem.address(), 4, 16, Latin1Char::ZERO);

				QByteArray cmdBinCode;

				codeItem.generateBinCode(&cmdBinCode);

				binCode = QString(cmdBinCode.toHex()).toUpper();
				mnemoCode = codeItem.mnemoCode(lmDesc);
				comment = codeItem.comment();
			}
			else
			{
				isCommand = false;

				comment = codeItem.comment();

				if (comment.isEmpty() == true)
				{
					continue;			// skip empty strings
				}
			}

			data.append(QVariant(isCommand));
			data.append(QVariant(address));
			data.append(QVariant(binCode));
			data.append(QVariant(mnemoCode));
			data.append(QVariant(comment));

			metadata->push_back(data);
		}
	}

	const std::vector<CodeItem>& CodeSnippet::code() const
	{
		return m_code;
	}

	CodeSnippetIterator CodeSnippet::begin()
	{
		return m_code.begin();
	}

	CodeSnippetConstIterator CodeSnippet::begin() const
	{
		return m_code.begin();
	}

	CodeSnippetIterator CodeSnippet::end()
	{
		return m_code.end();
	}

	CodeSnippetConstIterator CodeSnippet::end() const
	{
		return m_code.end();
	}

	// ----------------------------------------------------------------------------------
	//
	// AppLogicCode class implementation
	//
	// ----------------------------------------------------------------------------------

	AppLogicCode::AppLogicCode(Type type, bool optimized) :
		m_codeType(type),
		m_optimized(optimized)
	{
	}

	void AppLogicCode::setAppStartAddr(int addr)
	{
		Q_ASSERT(m_codeType == Type::IDR_Code);

		for(CodeItem& codeItem : m_code)
		{
			if (codeItem.lmCommandCode() == LmCommand::APPSTART)
			{
				codeItem.appStart(addr);
				return;
			}
		}

		Q_ASSERT(false);
	}

	bool AppLogicCode::finalize(LmDescriptionConstShared lmDesc)
	{
		TEST_PTR_RETURN_FALSE(lmDesc);

		m_codeSizeW = 0;
		m_clockCount = 0;
		m_commandsCount = 0;

		m_lmCodeMemUsage = 0;
		m_execTimeMcs = 0;
		m_lmCycleTimeUsage = 0;

		if (m_code.empty() == true)
		{
			return true;
		}

		bool result = true;

		// read commands and calculate code runtime
		//
		int prevCmdExecTime = 0;
		int waitTime = 0;
		int execTime = 0;
		int fbExecTime = 0;
		int waitFbTime = 0;
		int phaseClockCount = 0;
		bool firstAlpCommand = false;

		for(CodeItem& codeItem : m_code)
		{
			if (codeItem.isCommand() == false)
			{
				continue;
			}

			m_commandsCount++;

			codeItem.setBinParams(lmDesc, m_codeSizeW);

			m_codeSizeW += codeItem.sizeW();

			waitFbTime = 0;

			if (codeItem.isWaitingForFbExecution() == true)
			{
				waitFbTime = getFbRemainingExecTime(codeItem.getFbType());
			}

			codeItem.calcRunTime(lmDesc, prevCmdExecTime, waitFbTime,
								 &waitTime, &execTime, &fbExecTime, firstAlpCommand);

			firstAlpCommand = false;

			m_clockCount += (waitTime + execTime);
			phaseClockCount += (waitTime + execTime);

			codeItem.setClockCount(phaseClockCount);

			decFbExecTime(waitTime + execTime);

			prevCmdExecTime = execTime;

			if (fbExecTime != 0)
			{
				Q_ASSERT(codeItem.lmCommandCode() == LmCommand::STARTAFB ||
						 codeItem.lmCommandCode() == LmCommand::NSTART);

				startFbExec(codeItem.getFbType(), fbExecTime);
			}

			if (codeItem.lmCommandCode() == LmCommand::STOP)
			{
				int addTime = getMaxFbRemainingExecTimeAndClear() - prevCmdExecTime;

				if (addTime > 0)
				{
					codeItem.addExecTime(addTime);
					m_clockCount += addTime;
				}

				prevCmdExecTime = 0;
				phaseClockCount = 0;

				firstAlpCommand = true;
			}
		}

		RETURN_IF_FALSE(result);

		if (lmDesc->memory().m_codeMemorySize != 0)
		{
			m_lmCodeMemUsage = static_cast<double>(m_codeSizeW * 100) /
								static_cast<double>(lmDesc->memory().m_codeMemorySize);
		}

		m_execTimeMcs = m_clockCount * lmDesc->logicUnit().clockTimeSecs() * 1000000.0;

		double totalTimeMcs = 0;

		switch(m_codeType)
		{
		case Type::IDR_Code:
			totalTimeMcs = lmDesc->logicUnit().m_idrPhaseTime;
			break;

		case Type::ALP_Code:
			totalTimeMcs = lmDesc->logicUnit().m_alpPhaseTime;
			break;

		case Type::AllCode:
			totalTimeMcs =	lmDesc->logicUnit().m_idrPhaseTime +
							lmDesc->logicUnit().m_alpPhaseTime;
			break;

		default:
			Q_ASSERT(false);
			return false;
		}

		if (totalTimeMcs !=  0)
		{
			m_lmCycleTimeUsage = static_cast<double>(m_execTimeMcs * 100) / totalTimeMcs;
		}

		return result;
	}

	void AppLogicCode::clear()
	{
		CodeSnippet::clear();

		m_runningAfbs.clear();

		m_codeSizeW = -1;
		m_clockCount = -1;
		m_commandsCount = -1;

		m_lmCodeMemUsage = 0;
		m_execTimeMcs = 0;
		m_lmCycleTimeUsage = 0;
	}

	AppLogicCode::Type AppLogicCode::codeType() const
	{
		return m_codeType;
	}

	bool AppLogicCode::optimized() const
	{
		return m_optimized;
	}

	void AppLogicCode::setOptimized( bool optimized)
	{
		m_optimized = optimized;
	}

	int AppLogicCode::codeSizeW() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_codeSizeW;
	}

	int AppLogicCode::clockCount() const
	{
		Q_ASSERT(m_clockCount != -1);		// finalize() should be called first
		return m_clockCount;
	}

	int AppLogicCode::commandsCount() const
	{
		Q_ASSERT(m_commandsCount != -1);	// finalize() should be called first
		return m_commandsCount;
	}

	double AppLogicCode::lmCodeMemoryUsage() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_lmCodeMemUsage;
	}

	double AppLogicCode::execTimeMcs() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_execTimeMcs;
	}

	double AppLogicCode::lmCycleTimeUsage() const
	{
		Q_ASSERT(m_codeSizeW != -1);		// finalize() should be called first
		return m_lmCycleTimeUsage;
	}

	bool AppLogicCode::getCommandsStatistics(LmDescriptionConstShared lmDesc,
											 std::vector<CommandStatistics>* stat) const
	{
		TEST_PTR_RETURN_FALSE(lmDesc);
		TEST_PTR_RETURN_FALSE(stat);

		stat->clear();

		std::map<LmCommandCode, CommandStatistics> statMap;

		const std::map<int, LmCommand>& lmCommands = lmDesc->commands();

		for(auto const& p : lmCommands)
		{
			LmCommandCode lmcCode = static_cast<LmCommandCode>(p.first);

			const LmCommand& lmc = p.second;

			if (lmcCode == LmCommand::NO_COMMAND)
			{
				continue;
			}

			statMap.insert({lmcCode, CommandStatistics(lmc.code) });
		}

		for(const CodeItem& ci : m_code)
		{
			if (ci.isCommand() == false)
			{
				continue;
			}

			auto it = statMap.find(ci.lmCommandCode());

			if (it == statMap.end())
			{
				Q_ASSERT(false);
				continue;
			}

			CommandStatistics& cs = it->second;

			cs.usedCount++;
			cs.codeSizeW += ci.sizeW();
			cs.execTime += ci.waitTime() + ci.execTime();
		}

		stat->reserve(statMap.size());

		for(auto const& p : statMap)
		{
			stat->emplace_back(p.second);
		}

		return true;
	}

	void AppLogicCode::removeStopCommand()
	{
		auto it = std::find_if(m_code.rbegin(), m_code.rend(),
								[] (const CodeItem& ci)
								{
									return ci.lmCommandCode() == LmCommand::STOP;
								});

		if (it == m_code.rend())
		{
			Q_ASSERT(false);
			return;
		}

		m_code.erase((it + 1).base(), m_code.end());
	}

	void AppLogicCode::startFbExec(int fbOpCode, int fbRuntime)
	{
		auto it = m_runningAfbs.find(fbOpCode);

		if (it == m_runningAfbs.end())
		{
			// FB with fbOpCode is NOT running now
			//
			m_runningAfbs.insert({fbOpCode, fbRuntime});
		}
		else
		{
			Q_ASSERT(false);
		}
	}

	void AppLogicCode::decFbExecTime(int time)
	{
		if (m_runningAfbs.empty() == true)
		{
			return;
		}

		std::map<int, int> stillRunningAfbs;

		for(auto& p : m_runningAfbs)
		{
			Q_ASSERT(p.second > 0);

			p.second -= time;

			if (p.second > 0)
			{
				stillRunningAfbs.emplace(p);
			}
		}

		m_runningAfbs.swap(stillRunningAfbs);
	}

	int AppLogicCode::getFbRemainingExecTime(int fbOpCode)
	{
		int remainingTime = 0;

		auto it = m_runningAfbs.find(fbOpCode);

		if (it != m_runningAfbs.end())
		{
			remainingTime = it->second;
			m_runningAfbs.erase(it);
		}

		return remainingTime;
	}

	int AppLogicCode::getMaxFbRemainingExecTimeAndClear()
	{
		int maxTime = 0;

		for(auto& p : m_runningAfbs)
		{
			if (p.second > maxTime)
			{
				maxTime = p.second;
			}
		}

		m_runningAfbs.clear();

		return maxTime;
	}
}
