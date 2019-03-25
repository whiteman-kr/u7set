#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"

class QPen;
class QBrush;

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemImage : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemImage(void);
		explicit SchemaItemImage(SchemaUnit unit);
		virtual ~SchemaItemImage(void);

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
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Properties and Data
		//
	public:
		bool allowScale() const;
		void setAllowScale(bool value);

		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

		QImage image() const;
		void setImage(QImage image);

		QByteArray svgData() const;
		void setSvgData(QByteArray data);

	private:
		bool m_allowScale = true;
		bool m_keepAspectRatio = true;

		QImage m_image;
		QByteArray m_svgData;

		// Drawing resources
		//
	};
}
