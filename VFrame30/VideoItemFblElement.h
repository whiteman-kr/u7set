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
		friend ::Factory<VideoItem>::DerivedType<VideoItemFblElement>;
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
		bool setAfbParam(const QString& name, QVariant value);

		// Set Afb element parameters
		//
		bool setAfbElementParams(Afbl::AfbElement* afbElement) const;

	protected:
		void addQtDynamicParamProperties();

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
