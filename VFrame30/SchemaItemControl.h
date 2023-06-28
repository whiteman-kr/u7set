#pragma once

#include "PosRectImpl.h"

namespace VFrame30
{
	/*! \class SchemaItemControl
	*/
	class SchemaItemControl : public PosRectImpl
	{
		Q_OBJECT

		/// \brief Get widget linked to SchemaItem. User must check the returned value for null. 
		Q_PROPERTY(QWidget* widget MEMBER m_widget)

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

		QWidget* createWidget(QWidget* parent, bool editMode, double zoom);
		virtual void updateWidgetProperties(QWidget* widget) const;

		void updateWdgetPosAndSize(QWidget* widget, double zoom);

	protected:
		virtual QWidget* createWidgetImpl(QWidget* parent, bool editMode, double zoom);
		virtual void afterCreateImpl(QWidget* control);

	private:
		void associateWidget(QWidget* widget);
		void afterCreate(QWidget* control);

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

		QWidget* m_widget = nullptr;
	};
}
