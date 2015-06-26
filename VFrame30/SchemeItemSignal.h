#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	// CSchemeItemSignal
	//
	class VFRAME30LIBSHARED_EXPORT SchemeItemSignal : public FblItemRect
	{
		Q_OBJECT

		Q_PROPERTY(QString StrIDs READ signalStrIds WRITE setSignalStrIds)

	protected:
		SchemeItemSignal(void);
		SchemeItemSignal(SchemeUnit unit);
		virtual ~SchemeItemSignal(void);
	
	public:

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Scheme* scheme, const SchemeLayer* layer) const override;

		// Properties
		//
	public:
		QString signalStrIds() const;
		void setSignalStrIds(const QString& s);
		QStringList* mutable_signalStrIds();

		// Data
		//
	private:
		QStringList m_signalStrIds;
	};


	//
	// CSchemeItemInput
	//
	class VFRAME30LIBSHARED_EXPORT SchemeItemInput : public SchemeItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemeItem>::DerivedType<SchemeItemInput>;
#endif

	private:
		SchemeItemInput(void);
	public:
		explicit SchemeItemInput(SchemeUnit unit);
		virtual ~SchemeItemInput(void);

		virtual QString buildName() const override;

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
	// CSchemeItemInput
	//
	class VFRAME30LIBSHARED_EXPORT SchemeItemOutput : public SchemeItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemeItem>::DerivedType<SchemeItemOutput>;
#endif

	private:
		SchemeItemOutput(void);
	public:
		explicit SchemeItemOutput(SchemeUnit unit);
		virtual ~SchemeItemOutput(void);

		virtual QString buildName() const override;

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
