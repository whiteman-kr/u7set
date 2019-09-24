#pragma once
#include "FblItemRect.h"

namespace VFrame30
{
	//
	//		SchemaItemLoopback
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemLoopback : public FblItemRect
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemLoopback>;
#endif

	public:
		SchemaItemLoopback();
		SchemaItemLoopback(SchemaUnit unit);
		virtual ~SchemaItemLoopback();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Methods
		//
	public:
		// Properties
		//
	public:
		QString loopbackId() const;
		void setLoopbackId(QString value);

		// Data
		//
	private:
		QString m_loobackId = {"LOOPBACKID"};
	};


	//
	//
	//		SchemaItemLoopbackSource
	//
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemLoopbackSource : public SchemaItemLoopback
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemLoopbackSource>;
#endif

	public:
		SchemaItemLoopbackSource();
		SchemaItemLoopbackSource(SchemaUnit unit);
		virtual ~SchemaItemLoopbackSource();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

		// Public Methods
		//
	public:
		virtual QString buildName() const final;
		virtual QString toolTipText(int dpiX, int dpiY) const final;

		// Properties
		//
	public:

		// Data
		//
	private:
	};


	//
	//
	//		SchemaItemLoopbackTarget
	//
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemLoopbackTarget : public SchemaItemLoopback
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemLoopbackTarget>;
#endif

	public:
		SchemaItemLoopbackTarget();
		SchemaItemLoopbackTarget(SchemaUnit unit);
		virtual ~SchemaItemLoopbackTarget();

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Draw Functions
		//
	public:
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

		// Public Methods
		//
	public:
		virtual QString buildName() const final;
		virtual QString toolTipText(int dpiX, int dpiY) const final;

		// Properties
		//
	public:

		// Data
		//
	private:
	};

}
