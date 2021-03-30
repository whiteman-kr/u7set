#pragma once

#include "FblItemRect.h"
#include "../lib/AppSignal.h"

namespace VFrame30
{
	/*! \class SchemaItemSignal
		\ingroup appLogicSchemaItems
		\brief This is functional item used for connection signal to AFB inputs/outputs, other signals etc
	*/
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

		struct Cell
		{
			union
			{
				struct
				{
					qint16 row;
					qint16 column;
					Qt::ItemDataRole role;
				};

				quint64 id;
			};

			Cell(qint32 r, qint32 c, Qt::ItemDataRole rr) : row(static_cast<qint16>(r)), column(static_cast<qint16>(c)), role(rr)
			{
			}

			bool operator < (const Cell& that) const
			{
				return this->id < that.id;
			}
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

		void setAppSignalIds(QString s);
		QStringList* mutable_appSignalIds();

		// ImpactAppSignalIds
		//
		QString impactAppSignalIds() const;
		QStringList impactAppSignalIdList() const;

		void setImpactAppSignalIds(QString s);
		QStringList* mutable_impactAppSignalIds();

		// --
		//
		bool multiLine() const;
		void setMultiLine(bool value);

		bool multiChannel() const;

		int precision() const;
		void setPrecision(int value);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

		QString customText() const;
		void setCustomText(QString value);

		int columnCount() const;
		void setColumnCount(int value);

		double columnWidth(int columnIndex) const;
		void setColumnWidth(double value, int columnIndex);

		E::ColumnData columnData(int columnIndex) const;
		void setColumnData(E::ColumnData value, int columnIndex);

		E::HorzAlign columnHorzAlign(int columnIndex) const;
		void setColumnHorzAlign(E::HorzAlign value, int columnIndex);

		bool hasImpactColumn() const;

	public slots:
		/// \brief Returns overriden (set with setCellText) cell text
		QString cellText(int row, int column) const;

		/// \brief Overrides cell text, to reset value to default pass undefines as argumnet
		void setCellText(int row, int column, QString text);

		/// \brief Returns background color for specified cell
		QColor cellFillColor(int row, int column) const;

		/// \brief Sets background color for specified cell
		void setCellFillColor(int row, int column, QColor color);

		/// \brief Returns text color for specified cell, to reset value to default pass "" as argumnet
		QColor cellTextColor(int row, int column) const;

		/// \brief Sets text color for specified cell
		void setCellTextColor(int row, int column, QColor color);

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

		std::map<Cell, QVariant> m_runtimeCellMod;		// Cells, can be assigned by script in runtime only
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
