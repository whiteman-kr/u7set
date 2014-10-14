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
		VideoItemFblElement(SchemeUnit unit, const Fbl::FblElement& fblElement);

		virtual ~VideoItemFblElement(void);

		// Draw Functions
		//
	public:
		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* drawParam, const Scheme* pFrame, const SchemeLayer* pLayer) const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Properties and Data
		//
	public:
		const Fbl::FblElement& fblElement() const;

	private:
		Fbl::FblElement m_fblElement;
	};

}
