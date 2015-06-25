#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	//
	// SchemeItemAfb
	//
	class VFRAME30LIBSHARED_EXPORT SchemeItemAfb : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemeItem>::DerivedType<SchemeItemAfb>;
#endif

	private:
		SchemeItemAfb(void);
	public:
		explicit SchemeItemAfb(SchemeUnit unit);
		SchemeItemAfb(SchemeUnit unit, const Afbl::AfbElement& fblElement);

		virtual ~SchemeItemAfb(void);

		// Draw Functions
		//
	public:
		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* pLayer) const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Methods
		//
	public:

		virtual QString buildName() const override;

		bool setAfbParam(const QString& name, QVariant value, std::shared_ptr<VFrame30::Scheme> scheme);

		// Set Afb element parameters
		//
		bool setAfbElementParams(Afbl::AfbElement* afbElement) const;

	protected:
		void addQtDynamicParamProperties();
		bool executeScript(const QString& script, const Afbl::AfbElement& afb);
		Q_INVOKABLE int getParamIntValue(const QString& name);

		Q_INVOKABLE void addInputSignal(QString caption, int type, int opIndex, int size);
		Q_INVOKABLE void addOutputSignal(QString caption, int type, int opIndex, int size);

		Q_INVOKABLE void removeInputSignals();
		Q_INVOKABLE void removeOutputSignals();
		// Properties and Data
		//
	public:
		const QString& afbStrID() const;

		const std::vector<Afbl::AfbParam>& params() const;

	private:
		QString m_afbStrID;
		std::vector<Afbl::AfbParam> m_params;
	};

}
