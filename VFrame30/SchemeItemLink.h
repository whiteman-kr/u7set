#pragma once

#include "PosConnectionImpl.h"
#include "FblItemLine.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemeItemLink : public FblItemLine
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemeItemLink>;
#endif

	private:
		SchemeItemLink(void);
	public:
		explicit SchemeItemLink(SchemaUnit unit);
		virtual ~SchemeItemLink(void);

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
		virtual void Draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* pLayer) const override;

		// ��������� ���������� �����
		//
		virtual void SetConnectionsPos(double gridSize, int pinGridStep) override;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemaPoint* pResult, double gridSize, int pinGridStep) const override;

		// Properties and Data
	public:
	private:
	};
}
