#pragma once

#include "FblItemRect.h"
#include "Afb.h"

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
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* pLayer) const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual QString buildName() const override;

		bool setAfbParam(const QString& name, QVariant value, std::shared_ptr<VFrame30::Schema> schema, QString* errorMsg);
		Q_INVOKABLE bool setAfbParamByOpName(const QString& opName, QVariant value);

		QVariant getAfbParam(const QString& name);
		Afb::AfbParam afbParam(const QString& name);

		// Set Afb element parameters
		//
		bool setAfbElementParams(Afb::AfbElement* afbElement) const;

		bool updateAfbElement(const Afb::AfbElement& sourceAfb, QString* errorMessage);

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

	protected:
		void addSpecificParamProperties();
		bool executeScript(const QString& script, const Afb::AfbElement& afb, QString* errorMessage);
		Q_INVOKABLE int getParamIntValue(const QString& name);
		Q_INVOKABLE bool getParamBoolValue(const QString& name);

		Q_INVOKABLE void addInputSignal(QString caption, int type, int opIndex, int size);
		Q_INVOKABLE void addOutputSignal(QString caption, int type, int opIndex, int size);

		Q_INVOKABLE void removeInputSignals();
		Q_INVOKABLE void removeOutputSignals();

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
