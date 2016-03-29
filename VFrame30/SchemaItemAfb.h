#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	//
	// SchemeItemAfb
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemAfb : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemAfb>;
#endif

	private:
		SchemaItemAfb(void);
	public:
		explicit SchemaItemAfb(SchemaUnit unit);
		SchemaItemAfb(SchemaUnit unit, const Afb::AfbElement& fblElement);

		virtual ~SchemaItemAfb(void);

		// Draw Functions
		//
	public:
		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* drawParam, const Schema* scheme, const SchemaLayer* pLayer) const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:

		virtual QString buildName() const override;

		bool setAfbParam(const QString& name, QVariant value, std::shared_ptr<VFrame30::Schema> scheme);
		bool setAfbParamByOpName(const QString& opName, QVariant value);

		// Set Afb element parameters
		//
		bool setAfbElementParams(Afb::AfbElement* afbElement) const;

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

	protected:
		void addSpecificParamProperties();
		bool executeScript(const QString& script, const Afb::AfbElement& afb);
		Q_INVOKABLE int getParamIntValue(const QString& name);

		Q_INVOKABLE void addInputSignal(QString caption, int type, int opIndex, int size);
		Q_INVOKABLE void addOutputSignal(QString caption, int type, int opIndex, int size);

		Q_INVOKABLE void removeInputSignals();
		Q_INVOKABLE void removeOutputSignals();
		// Properties and Data
		//
	public:
		const QString& afbStrID() const;

		const std::vector<Afb::AfbParam>& params() const;

		int precision() const;
		void setPrecision(int value);

	private:
		QString m_afbStrID;
		std::vector<Afb::AfbParam> m_params;
		int m_precision = 2;
	};

}
