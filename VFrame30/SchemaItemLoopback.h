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
		virtual void draw(CDrawParam* drawParam) const override;

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
	class SchemaItemLoopbackSource final : public SchemaItemLoopback
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
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

		// Public Methods
		//
	public:
		virtual QString buildName() const override;
		virtual QString toolTipText(double dpiX, double dpiY, double devicePixelRatio) const override;

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
	class SchemaItemLoopbackTarget final : public SchemaItemLoopback
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
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

		// Public Methods
		//
	public:
		virtual QString buildName() const override;
		virtual QString toolTipText(double dpiX, double dpiY, double devicePixelRatio) const override;

		// Properties
		//
	public:

		// Data
		//
	private:
	};

}
