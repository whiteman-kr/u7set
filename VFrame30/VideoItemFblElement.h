#pragma once
#include "FblItemRect.h"

namespace VFrame30
{
	//
	// CVideoItemInputSignal
	//
	class VFRAME30LIBSHARED_EXPORT CVideoItemFblElement : public CFblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<CVideoItem>::DerivedType<CVideoItemFblElement>;
#endif

	private:
		CVideoItemFblElement(void);
	public:
		explicit CVideoItemFblElement(SchemeUnit unit);
		CVideoItemFblElement(SchemeUnit unit, const Fbl::FblElement& fblElement);

		virtual ~CVideoItemFblElement(void);

		// Draw Functions
		//
	public:
		// ��������� ��������, ����������� � 100% ��������.
		// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
		//
		virtual void Draw(CDrawParam* drawParam, const CVideoFrame* pFrame, const CVideoLayer* pLayer) const override;

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
