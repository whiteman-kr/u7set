#pragma once

#include "FblItemRect.h"
#include "Afb.h"
#include <optional>

namespace VFrame30
{
	//
	// SchemaItemAfb
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemAfb : public FblItemRect
	{
		Q_OBJECT

	public:
		SchemaItemAfb(void);
		explicit SchemaItemAfb(SchemaUnit unit);
		SchemaItemAfb(SchemaUnit unit, const Afb::AfbElement& fblElement, QString* errorMsg);

		virtual ~SchemaItemAfb(void);

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;
		void drawAfbHelp(QPainter* painter, const QRect& drawRect) const;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Methods
		//
	public:
		virtual QString toolTipText(int dpiX, int dpiY) const final;
		virtual QString buildName() const final;

		bool setAfbParam(const QString& name, QVariant value, std::shared_ptr<VFrame30::Schema> schema, QString* errorMsg);
		Q_INVOKABLE bool setAfbParamByOpName(const QString& opName, QVariant value);

		QVariant getAfbParam(const QString& name);
		Afb::AfbParam afbParam(const QString& name);

		std::optional<bool> getAssignFlagsValue() const;

		// Set Afb element parameters
		//
		bool setAfbElementParams(Afb::AfbElement* afbElement) const;

		bool updateAfbElement(const Afb::AfbElement& sourceAfb, QString* errorMessage);

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const final;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const final;

	protected:
		void addSpecificParamProperties();
		bool executeScript(const QString& script, const Afb::AfbElement& afb, QString* errorMessage);
		Q_INVOKABLE int getParamIntValue(const QString& name);
		Q_INVOKABLE bool getParamBoolValue(const QString& name);
		Q_INVOKABLE bool setParamVisible(const QString& name, bool visible);

		Q_INVOKABLE void addInputSignal(QString caption, int type, int opIndex, int size);
		Q_INVOKABLE void addOutputSignal(QString caption, int type, int opIndex, int size);

		Q_INVOKABLE void removeInputSignals();
		Q_INVOKABLE void removeInputSignal(QString caption);

		Q_INVOKABLE void removeOutputSignals();
		Q_INVOKABLE void removeOutputSignal(QString caption);

		// Properties and Data
		//
	public:
		const QString& afbStrID() const;

		Afb::AfbElement& afbElement();
		const Afb::AfbElement& afbElement() const;

		std::vector<Afb::AfbParam>& params();
		const std::vector<Afb::AfbParam>& params() const;

		int precision() const;
		void setPrecision(int value);

	private:
		int m_precision = 2;
		Afb::AfbElement m_afbElement;
	};
}
