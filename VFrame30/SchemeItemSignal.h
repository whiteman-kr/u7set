#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	// CSchemeItemSignal
	//
	class VFRAME30LIBSHARED_EXPORT SchemeItemSignal : public FblItemRect
	{
		Q_OBJECT

	protected:
		SchemeItemSignal(void);
		SchemeItemSignal(SchemaUnit unit);
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
		virtual void Draw(CDrawParam* drawParam, const Schema* scheme, const SchemaLayer* layer) const override;

		// Properties
		//
	public:
		QString signalStrIds() const;
		const QStringList& signalStrIdList() const;

		void setSignalStrIds(const QString& s);
		QStringList* mutable_signalStrIds();

		bool multiChannel() const;

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
		explicit SchemeItemInput(SchemaUnit unit);
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
		explicit SchemeItemOutput(SchemaUnit unit);
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
