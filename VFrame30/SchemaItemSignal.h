#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	// CSchemeItemSignal
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemSignal : public FblItemRect
	{
		Q_OBJECT

	protected:
		SchemaItemSignal(void);
		SchemaItemSignal(SchemaUnit unit);
		virtual ~SchemaItemSignal(void);
	
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
	class VFRAME30LIBSHARED_EXPORT SchemaItemInput : public SchemaItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemInput>;
#endif

	private:
		SchemaItemInput(void);
	public:
		explicit SchemaItemInput(SchemaUnit unit);
		virtual ~SchemaItemInput(void);

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
	class VFRAME30LIBSHARED_EXPORT SchemaItemOutput : public SchemaItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemOutput>;
#endif

	private:
		SchemaItemOutput(void);
	public:
		explicit SchemaItemOutput(SchemaUnit unit);
		virtual ~SchemaItemOutput(void);

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
