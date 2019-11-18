#pragma once

#include "PosConnectionImpl.h"
#include "FblItemLine.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemLink : public FblItemLine
	{
		Q_OBJECT

	public:
		SchemaItemLink(void);
		explicit SchemaItemLink(SchemaUnit unit);
		virtual ~SchemaItemLink(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:

		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void draw(CDrawParam* drawParam, const Schema* pFrame, const SchemaLayer* pLayer) const final;

		// ��������� ���������� �����
		//
		virtual void SetConnectionsPos(double gridSize, int pinGridStep) final;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemaPoint* pResult, double gridSize, int pinGridStep) const final;

		// Properties and Data
		//
	public:
	private:
	};
}
