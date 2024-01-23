#pragma once

#include "CodeItem.h"

namespace Builder
{
	using CodeSnippetIterator = std::vector<CodeItem>::iterator;
	using CodeSnippetConstIterator = std::vector<CodeItem>::const_iterator;

	class CodeSnippet
	{
	public:
		CodeSnippet();

		// code snippet modification methods
		//
		void append(const CodeItem& codeItem);
		void append(const CodeSnippet& codeShippet);

		CodeSnippet& operator << (const CodeItem& ci);
		CodeSnippet& operator << (const CodeSnippet& codeShippet);
		CodeSnippet& operator << (const QString& commentStr);

		void comment(const QString& cmt);
		void newLine();
		void comment_nl(const QString& cmt);
		void finalizeByNewLine();
		void clear();
		void reserve(int size);

		void swap(CodeSnippet& code);

		//

		bool isEmpty() const;
		int itemsCount() const;
		int codeSizeW(LmDescriptionConstShared lmDesc) const;
		int codeSizeW(LmDescriptionConstShared lmDesc,
					  CodeSnippetConstIterator start,
					  CodeSnippetConstIterator end) const;

		//

		void getAsmCode(LmDescriptionConstShared lmDesc, QStringList* asmCode) const;
		void getBinCode(QByteArray* binCode) const;
//		void getMifCode(QStringList* mifCode) const;

		void getAsmMetadataFields(QStringList* metadataFields, int* metadataVersion) const;
		void getAsmMetadata(LmDescriptionConstShared lmDesc, std::vector<QVariantList>* metadata) const;

		const std::vector<CodeItem>& code() const;

		CodeSnippetIterator begin();
		CodeSnippetConstIterator begin() const;

		CodeSnippetIterator end();
		CodeSnippetConstIterator end() const;

	protected:
		std::vector<CodeItem> m_code;
	};

	class AppLogicCode : public CodeSnippet
	{
	public:
		enum class Type
		{
			Unknown,
			IDR_Code,
			ALP_Code,
			AllCode
		};

	public:
		AppLogicCode(Type type, bool optimized);

		void setAppStartAddr(int addr);

		[[nodiscard]] bool finalize(LmDescriptionConstShared lmDesc);

		void clear();

		Type codeType() const;

		bool optimized() const;
		void setOptimized( bool optimized);

		int codeSizeW() const;
		int clockCount() const;
		int commandsCount() const;

		double lmCodeMemoryUsage() const;
		double execTimeMcs() const;
		double lmCycleTimeUsage() const;

		bool getCommandsStatistics(LmDescriptionConstShared lmDesc,
								   std::vector<CommandStatistics>* stat) const;

		void removeStopCommand();

	private:
		void startFbExec(int fbOpCode, int fbRuntime);
		void decFbExecTime(int time);
		int getFbRemainingExecTime(int fbOpCode);
		int getMaxFbRemainingExecTimeAndClear();

	private:
		Type m_codeType = Type::Unknown;
		bool m_optimized = false;

		std::map<int, int> m_runningAfbs;		// AFB opCode -> AFB runtime

		int m_codeSizeW = -1;
		int m_commandsCount = -1;
		int m_clockCount = -1;

		double m_lmCodeMemUsage = 0;
		double m_execTimeMcs = 0;
		double m_lmCycleTimeUsage = 0;
	};
}
