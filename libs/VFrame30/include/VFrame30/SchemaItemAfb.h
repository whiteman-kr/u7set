#pragma once

#include <VFrame30/FblItemRect.h>
#include <VFrame30/IMatsSchemaItemAssociations.h>

#include <HardwareLib/Afb.h>


namespace VFrame30
{
	//
	// SchemaItemAfb
	//
	class SchemaItemAfb final : public FblItemRect,
								public IMatsSchemaItemAssociations
	{
		Q_OBJECT

	public:
		SchemaItemAfb(void);
		explicit SchemaItemAfb(SchemaUnit unit);
		SchemaItemAfb(SchemaUnit unit, const Afb::AfbElement& fblElement, QString* errorMsg);

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;
		void drawAfbHelp(QPainter* painter, const QRect& drawRect) const;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:
		virtual QString toolTipText(double dpiX, double dpiY, double devicePixelRatio) const override;
		virtual QString buildName() const override;

		bool setAfbParam(const QString& name, QVariant value, QString* errorMsg);
		bool setAfbParamByOpName(const QString& opName, const Afb::AfbParamValue& value);

		QVariant getAfbParam(const QString& name);
		Afb::AfbParam afbParam(const QString& name);

		std::optional<bool> getAssignFlagsValue() const;

		// Set Afb element parameters
		//
		bool setAfbElementParams(Afb::AfbElement* afbElement) const;

		bool updateAfbElement(const Afb::AfbElement& sourceAfb, QString* errorMessage);

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

	protected:
		void addSpecificParamProperties();
		bool executeScript(const QString& script, const Afb::AfbElement& afb, QString* errorMessage);

		Q_INVOKABLE double getParamDoubleValue(const QString& name);
		Q_INVOKABLE bool setParamDoubleValue(const QString& name, double value);

		Q_INVOKABLE int getParamIntValue(const QString& name);
		Q_INVOKABLE bool setParamIntValue(const QString& name, int value);

		Q_INVOKABLE bool getParamBoolValue(const QString& name);
		Q_INVOKABLE bool setParamBoolValue(const QString& name, bool value);

		Q_INVOKABLE bool setParamVisible(const QString& name, bool visible);

		Q_INVOKABLE void addInputSignal(QString caption, int type, int opIndex, int size);
		Q_INVOKABLE void addOutputSignal(QString caption, int type, int opIndex, int size);

		Q_INVOKABLE void removeInputSignals();
		Q_INVOKABLE void removeInputSignal(QString caption);

		Q_INVOKABLE void removeOutputSignals();
		Q_INVOKABLE void removeOutputSignal(QString caption);

	private:
		QString getAfbParamValueText(const Afb::AfbParam& param) const;

		// IMatsSchemaItemAssociations implementation.
		//
	public:
		virtual QStringList associatedDiagObjectIds() const override { return {}; };
		virtual QStringList associatedAppSignalIds() const override;
		virtual QStringList associatedImpactAppSignalIds() const override;
		virtual QStringList associatedConnectionIds() const override;
		virtual QStringList associatedLoopbackIds() const override;
		virtual QStringList associatedSchemaItemLabels() const override;

		// Properties and Data
		//
	public:
		[[nodiscard]] const QString& afbStrID() const;

		[[nodiscard]] Afb::AfbElement& afbElement();
		const Afb::AfbElement& afbElement() const;

		[[nodiscard]] std::vector<Afb::AfbParam>& params();
		const std::vector<Afb::AfbParam>& params() const;

		[[nodiscard]] int precision() const;
		void setPrecision(int value);

		[[nodiscard]] bool isPackedLogic() const;

		[[nodiscard]] const QString& packedLogicId() const;
		void setPackedLogicId(const QString& value);

		[[nodiscard]] Afb::AfbElement::PackedLogicData packedLogic() const;

		[[nodiscard]] QStringList packedLogicInputSignalIds() const;
		void setPackedLogicInputSignalIds(const QStringList& value);

	private:
		int m_precision = 2;
		Afb::AfbElement m_afbElement;

		// Mats specific properties.
		//
		QStringList m_packedLogicInputSignalIds; // List of input signal ids for packed logic.
	};
}
