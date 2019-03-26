#pragma once

#include "PosRectImpl.h"
#include "FontParam.h"
#include <QXmlStreamReader>
#include <QSvgRenderer>
#include <optional>

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

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

	private:
		void drawImage(CDrawParam* drawParam, const QRectF& rect) const;
		void drawSvg(CDrawParam* drawParam, const QRectF& rect) const;
		void drawError(CDrawParam* drawParam, const QRectF& rect, QString errorText) const;

	protected:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Properties and Data
		//
	public:
		bool keepAspectRatio() const;
		void setKeepAspectRatio(bool value);

		QImage image() const;
		void setImage(QImage image);

		QString svgData() const;
		void setSvgData(QString data);

	private:
		bool m_keepAspectRatio = true;

		QImage m_image;
		QString m_svgData;

		// Drawing resources
		//
		mutable std::optional<QSvgRenderer> m_svgRenderer;
	};
}
