#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	//
	// CVideoItemInputSignal
	//
	class VFRAME30LIBSHARED_EXPORT VideoItemFblElement : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemeItem>::DerivedType<VideoItemFblElement>;
#endif

	private:
		VideoItemFblElement(void);
	public:
		explicit VideoItemFblElement(SchemeUnit unit);
		VideoItemFblElement(SchemeUnit unit, const Afbl::AfbElement& fblElement);

		virtual ~VideoItemFblElement(void);

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

		const std::vector<Afbl::AfbElementParam>& params() const;

	private:
		QString m_afbStrID;
		std::vector<Afbl::AfbElementParam> m_params;
	};

}
