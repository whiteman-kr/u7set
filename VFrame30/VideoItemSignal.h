#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	// CVideoItemSignal
	//
	class VFRAME30LIBSHARED_EXPORT VideoItemSignal : public FblItemRect
	{
		Q_OBJECT

	protected:
		VideoItemSignal(void);
		VideoItemSignal(SchemeUnit unit);
		virtual ~VideoItemSignal(void);
	
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
	class VFRAME30LIBSHARED_EXPORT VideoItemInputSignal : public VideoItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<VideoItem>::DerivedType<VideoItemInputSignal>;
#endif

	private:
		VideoItemInputSignal(void);
	public:
		explicit VideoItemInputSignal(SchemeUnit unit);
		virtual ~VideoItemInputSignal(void);

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
	class VFRAME30LIBSHARED_EXPORT VideoItemOutputSignal : public VideoItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<VideoItem>::DerivedType<VideoItemOutputSignal>;
#endif

	private:
		VideoItemOutputSignal(void);
	public:
		explicit VideoItemOutputSignal(SchemeUnit unit);
		virtual ~VideoItemOutputSignal(void);

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
