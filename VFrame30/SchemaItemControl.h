#pragma once

#include "PosRectImpl.h"
#include "ClientSchemaView.h"

namespace VFrame30
{
	/*! \class SchemaItemControl
	*/
	class SchemaItemControl : public PosRectImpl
	{
		Q_OBJECT

		/// \brief Get the widget linked to SchemaItem. User must check the returned value for null. 
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
		virtual void updateWidgetProperties(QWidget* widget, bool editMode) const;

		void updateWidgetPosAndSize(QWidget* widget, double zoom);

	protected:
		virtual QWidget* createWidgetImpl(QWidget* parent, bool editMode, double zoom);
		virtual void afterCreateImpl(QWidget* control);

	private:
		void associateWidget(QWidget* widget);
		void afterCreate(QWidget* control);

	protected:
		QJSValue evaluateScript(QString scriptName, QWidget* controlWidget, QString script);

		template <typename WidgetType, typename... ScriptArgs>
		void runEventScript(QJSValue& evaluatedJs, bool allowMessageBox, QString scriptName, WidgetType* widget, ScriptArgs... scriptArgs);

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


	template <typename WidgetType, typename... ScriptArgs>
	void SchemaItemControl::runEventScript(QJSValue& evaluatedJs, bool allowMessageBox, QString scriptName, WidgetType* widget, ScriptArgs... scriptArgs)
	{
		if (evaluatedJs.isError() == true ||
			evaluatedJs.isNull() == true)
		{
			return;
		}

		// Suppose that parent of sender is ClientSchemaView, if not, then this is EditMode?
		//
		ClientSchemaView* schemaView = dynamic_cast<ClientSchemaView*>(widget->parentWidget());
		if (schemaView == nullptr)
		{
			assert(schemaView);
			return;
		}

		bool prevAllowMessageBoxState = schemaView->setScriptMessageBoxAllowed(allowMessageBox);

		QJSEngine* engine = schemaView->jsEngine();
		assert(engine);

		// Set argument list
		//
		QJSValue jsSchemaItem = engine->newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsWidget = engine->newQObject(widget);
		QQmlEngine::setObjectOwnership(widget, QQmlEngine::CppOwnership);

		QJSValueList args;

		args << jsSchemaItem;
		args << jsWidget;
		(args.push_back(scriptArgs), ...); // Push all other specific arguments.

		// Run script
		//
		QJSValue jsResult = evaluatedJs.call(args);

		schemaView->setScriptMessageBoxAllowed(prevAllowMessageBoxState);

		if (jsResult.isError() == true)
		{
			reportScriptError(scriptName, jsResult, schemaView->logFile());
			return;
		}

		engine->collectGarbage();
		return;
	}
}
