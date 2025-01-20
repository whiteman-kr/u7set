#pragma once

#include <VFrame30/SchemaView.h>

namespace ReportLib
{
	enum class ReportSchemaCompareAction;

	//
	// ReportSchemaView
	//
	class ReportSchemaView : public VFrame30::SchemaView
	{
	public:
		explicit ReportSchemaView(bool infoMode);
		virtual ~ReportSchemaView();

		void adjust(QPainter* painter, double startX, double startY, double zoom) const;
		void drawCompareOutlines(VFrame30::CDrawParam* drawParam,
								 const QRectF& clipRect,
								 const std::map<QUuid, ReportSchemaCompareAction>& compareActions);

		virtual VFrame30::DrawMode drawMode() const override;

		bool infoMode() const;
		void setInfoMode(bool value);

	private:
		bool m_infoMode;
	};

} // namespace ReportLib
