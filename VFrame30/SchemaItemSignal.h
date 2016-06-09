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

	protected:
		void createColumnProperties();

		// Properties
		//
	public:
		QString appSignalIds() const;
		const QStringList& appSignalIdList() const;

		void setAppSignalIds(const QString& s);
		QStringList* mutable_appSignalIds();

		bool multiChannel() const;

		int precision() const;
		void setPrecision(int value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

		int columnCount() const;
		void setColumnCount(int value);

		double columnWidth(int columnIndex) const;
		void setColumnWidth(double value, int columnIndex);

		E::ColumnData columnData(int columnIndex) const;
		void setColumnData(E::ColumnData value, int columnIndex);

		E::HorzAlign columnHorzAlign(int columnIndex) const;
		void setColumnHorzAlign(E::HorzAlign value, int columnIndex);

		//	Data Structures
		//
	public:
		struct Column
		{
			double width = 20.0;
			E::ColumnData data = E::ColumnData::AppSignalID;
			E::HorzAlign horzAlign = E::HorzAlign::AlignLeft;
		};

		// Data
		//
	protected:
		QStringList m_appSignalIds;

		int m_precision = 2;
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;

		// Monitor mode settings
		//
		// Columns: width, data (StrID, Value, Validity, Imitation, Simultaion, FlagCombination?)
		// Anaolog: format (0.00, 15E-12, ...), precision,
		// Discrete: 0/1, No/Yes

		// Width, %		Format
		// 80;
		std::vector<Column> m_columns;
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
	// SchemaItemOutput
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

	//
	// SchemaItemInOut
	//
	class VFRAME30LIBSHARED_EXPORT SchemaItemInOut : public SchemaItemSignal
	{
		Q_OBJECT

#ifdef VFRAME30LIB_LIBRARY
		friend ::Factory<SchemaItem>::DerivedType<SchemaItemInOut>;
#endif

	private:
		SchemaItemInOut(void);
	public:
		explicit SchemaItemInOut(SchemaUnit unit);
		virtual ~SchemaItemInOut(void);

		virtual QString buildName() const override;

		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

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
