#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	// CVideoItemSignal
	//
	class VFRAME30LIBSHARED_EXPORT CVideoItemSignal : public CFblItemRect
	{
		Q_OBJECT

	protected:
		CVideoItemSignal(void);
		CVideoItemSignal(SchemeUnit unit);
		virtual ~CVideoItemSignal(void);
	
	public:

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// Properties and Data
	public:
	private:
	};


	//
	// CVideoItemInputSignal
	//
	class VFRAME30LIBSHARED_EXPORT CVideoItemInputSignal : public CVideoItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<CVideoItem>::DerivedType<CVideoItemInputSignal>;
#endif

	private:
		CVideoItemInputSignal(void);
	public:
		explicit CVideoItemInputSignal(SchemeUnit unit);
		virtual ~CVideoItemInputSignal(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Properties and Data
	public:
	private:
	};


	//
	// CVideoItemInputSignal
	//
	class VFRAME30LIBSHARED_EXPORT CVideoItemOutputSignal : public CVideoItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<CVideoItem>::DerivedType<CVideoItemOutputSignal>;
#endif

	private:
		CVideoItemOutputSignal(void);
	public:
		explicit CVideoItemOutputSignal(SchemeUnit unit);
		virtual ~CVideoItemOutputSignal(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Properties and Data
	public:
	private:
	};

}
