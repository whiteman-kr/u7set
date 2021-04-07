#pragma once
#include "FblItemRect.h"

namespace VFrame30
{
	//
	//		SchemaItemLoopback
	//
	class SchemaItemLoopback : public FblItemRect
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemLoopback>;

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

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
	class SchemaItemLoopbackSource : public SchemaItemLoopback
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemLoopbackSource>;

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

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
	class SchemaItemLoopbackTarget : public SchemaItemLoopback
	{
		Q_OBJECT

		friend ::Factory<SchemaItem>::DerivedType<SchemaItemLoopbackTarget>;

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
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const final;

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
