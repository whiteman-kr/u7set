#pragma once

#include "FblItemRect.h"
#include "../lib/AppSignal.h"

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

		//	Data Structures
		//
	public:
		struct Column
		{
			double width = 20.0;
			E::ColumnData data = E::ColumnData::AppSignalID;
			E::HorzAlign horzAlign = E::HorzAlign::AlignLeft;
		};

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const override;

		static QString getCoulumnText(CDrawParam* drawParam,
									  const SchemaItem* schemaItem,
									  const E::ColumnData& data,
									  const AppSignalParam& signal,
									  const AppSignalState& signalState,
									  const AppSignalParam& impactSignal,
									  const AppSignalState& impactSignalState,
									  E::AnalogFormat analogFormat,
									  int precision);

	protected:
		void drawFullLineIds(CDrawParam* drawParam) const;

		void drawMultichannelValues(CDrawParam* drawParam, QPen& linePen) const;
		void drawSinglechannelValues(CDrawParam* drawParam, QPen& linePen) const;

		void createColumnProperties();

		virtual double minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const override;
		virtual double minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const override;

		// Other
		//
	public:
		virtual QString toolTipText(int dpiX, int dpiY) const override;

		SchemaItemPtr transformIntoInput();
		SchemaItemPtr transformIntoInOut();
		SchemaItemPtr transformIntoOutput();

		template <typename TYPE>
		SchemaItemPtr transformIntoType();

		// Properties
		//
	public:
		// AppSignalIDs
		//
		QString appSignalIds() const;
		QStringList appSignalIdList() const;

		void setAppSignalIds(const QString& s);
		QStringList* mutable_appSignalIds();

		// ImpactAppSignalIds
		//
		QString impactAppSignalIds() const;
		QStringList impactAppSignalIdList() const;

		void setImpactAppSignalIds(const QString& s);
		QStringList* mutable_impactAppSignalIds();

		// --
		//
		bool multiLine() const;
		void setMultiLine(const bool& value);

		bool multiChannel() const;

		int precision() const;
		void setPrecision(const int& value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(const E::AnalogFormat& value);

		QString customText() const;
		void setCustomText(const QString& value);

		int columnCount() const;
		void setColumnCount(const int& value);

		double columnWidth(int columnIndex) const;
		void setColumnWidth(double value, int columnIndex);

		E::ColumnData columnData(int columnIndex) const;
		void setColumnData(E::ColumnData value, int columnIndex);

		E::HorzAlign columnHorzAlign(int columnIndex) const;
		void setColumnHorzAlign(E::HorzAlign value, int columnIndex);

		bool hasImpactColumn() const;

		// Data
		//
	protected:
		QStringList m_appSignalIds;
		QStringList m_impactAppSignalIds;

		bool m_multiLine = true;		// Show multichannel signlas in multi/single line

		int m_precision = 2;
		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;

		QString m_customText;

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

		friend class SchemaItemSignal;

	public:
		SchemaItemInput(void);
		explicit SchemaItemInput(SchemaUnit unit);
		virtual ~SchemaItemInput(void);

		virtual QString buildName() const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;
		bool loadData(const Proto::Envelope& message, bool loadOwnData);

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

		friend class SchemaItemSignal;

	public:
		SchemaItemOutput(void);
		explicit SchemaItemOutput(SchemaUnit unit);
		virtual ~SchemaItemOutput(void);

		virtual QString buildName() const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;
		bool loadData(const Proto::Envelope& message, bool loadOwnData);

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

		friend class SchemaItemSignal;

	public:
		SchemaItemInOut(void);
		explicit SchemaItemInOut(SchemaUnit unit);
		virtual ~SchemaItemInOut(void);

		virtual QString buildName() const override;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;
		bool loadData(const Proto::Envelope& message, bool loadOwnData);

		// Properties and Data
	public:
	private:
	};

}
