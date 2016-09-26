#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	class UfbSchema;
}

namespace VFrame30
{
	//
	// SchemaItemUfb
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemUfb : public FblItemRect
	{
		Q_OBJECT

	public:
		SchemaItemUfb(void);
		explicit SchemaItemUfb(SchemaUnit unit);
		SchemaItemUfb(SchemaUnit unit, const UfbSchema* ufbSchema, QString* errorMsg);

		virtual ~SchemaItemUfb(void);

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

		// Set Afb element parameters
		//
		bool setAfbElementParams(Afb::AfbElement* afbElement) const;

		bool updateElement(const UfbSchema* ufbSchema, QString* errorMessage);

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
		const Afb::AfbElement& afbElement() const;

		std::vector<Afb::AfbParam>& params();
		const std::vector<Afb::AfbParam>& params() const;

		QString ufbSchemaId() const;
		void setUfbSchemaId(const QString& value);

		int ufbSchemaVersion() const;
		void setUfbSchemaVersion(int value);

		QString label() const;
		void setLabel(const QString& value);

	private:
		QString m_ufbSchemaId;
		int m_version = -1;
		QString m_label;
	};
}
