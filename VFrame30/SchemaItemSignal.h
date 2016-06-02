#pragma once

#include "FblItemRect.h"

namespace VFrame30
{
	// SchemaItemSignal
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
		virtual void Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		// Properties
		//
	public:
		QString appSignalIds() const;
		const QStringList& appSignalIdList() const;

		void setAppSignalIds(const QString& s);
		QStringList* mutable_appSignalIds();

		bool multiChannel() const;

		// Data
		//
	private:
		QStringList m_appSignalIds;

		// Monitor mode settings
		//
		// Columns: width, data (StrID, Value, Validity, Imitation, Simultaion, FlagCombination?)
		// Anaolog: format (0.00, 15E-12, ...), precision,
		// Discrete: 0/1, No/Yes

		// Width, %		Format
		// 80;

		enum class ColumnData
		{
			AppSignalID,
			CustomerSignalID,
			Caption,
			StateDiscrete,
			StateAnalog_e,		// format as [-]9.9e[+|-]999
			StateAnalog_E,		// format as [-]9.9E[+|-]999
			StateAnalog_f,		// format as [-]9.9
			StateAnalog_g,		// use e or f format, whichever is the most concise
			StateAnalog_G,		// use E or f format, whichever is the most concise
		};
		Q_ENUM(ColumnData)

		struct SchemaItemSignalColumn
		{
			double width;
			ColumnData data;
		};
	};


	//
	// SchemaItemInput
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
	// SchemaItemInput
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
