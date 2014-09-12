#pragma once

#include "PosConnectionImpl.h"
#include "FblItemLine.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT CVideoItemLink : public CFblItemLine
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<CVideoItem>::DerivedType<CVideoItemLink>;
#endif

	private:
		CVideoItemLink(void);
	public:
		explicit CVideoItemLink(SchemeUnit unit);
		virtual ~CVideoItemLink(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* drawParam, const CVideoFrame* pFrame, const CVideoLayer* pLayer) const override;

		// ��������� ���������� �����
		//
		virtual void SetConnectionsPos() override;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, VideoItemPoint* pResult) const override;

		// Properties and Data
	public:
	private:
	};
}
