#pragma once

#include "PosRectImpl.h"
#include <QJSEngine>

namespace VFrame30
{
	class VFRAME30LIBSHARED_EXPORT SchemaItemControl : public PosRectImpl
	{
		Q_OBJECT

	public:
		SchemaItemControl(void);
		explicit SchemaItemControl(SchemaUnit unit);
		virtual ~SchemaItemControl(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		virtual QWidget* createWidget(QWidget* parent, bool editMode);
		virtual void updateWidgetProperties(QWidget* widget) const;

		void updateWdgetPosAndSize(QWidget* widget, double zoom);

	protected:
		QJSValue evaluateScript(QWidget* controlWidget, QString script);

		// Properties and Data
		//
	public:
		const QString& styleSheet() const;
		virtual void setStyleSheet(QString value);

		const QString& toolTip() const;
		virtual void setToolTip(QString value);

	private:
		QString m_styleSheet;
		QString m_toolTip;
	};
}
