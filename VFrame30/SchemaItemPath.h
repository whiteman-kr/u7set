#pragma once

#include "PosConnectionImpl.h"

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemPath : public PosConnectionImpl
	{
		Q_OBJECT

	public:
		SchemaItemPath(void);
		explicit SchemaItemPath(SchemaUnit unit);
		virtual ~SchemaItemPath(void);

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

		// Properties and Data
	public:
		double weight() const;
		void setWeight(double weight);

		QColor lineColor() const;
		void setLineColor(QColor color);

	private:
		double m_weight;					// ������� �����, �������� � ������ ��� ������ � ����������� �� UnitDocPt
		QColor m_lineColor;
	};
}
