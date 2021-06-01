#pragma once

#include "FblItemRect.h"
#include "../AppSignalLib/AppSignalParam.h"

namespace VFrame30
{
	/*! \class SchemaItemSignal
		\ingroup appLogicSchemaItems
		\brief This is functional item used for connection signal to AFB inputs/outputs, other signals etc

		Project developer can implement own PreDrawScript implementation to customize text and colors for an item.

		The next example shows how to customize an element. It assumes that item has three signal identifiers in SignalIDs property.
		It determines the layout of an item, then calculates number of signals and columns. After that it loops throug all rows
		and columns and sets text and different colors for every cell as shown on the picture after the code.

		<b>Example</b>

		\code
		(function(schemaItem)
		{
			// Column Type Constants
			//
			let ColumnAppSignalID = 0;
			let ColumnCustomSignalID = 1;
			let ColumnCaption = 2;
			let ColumnState = 3;
			let ColumnImpactAppSignalID = 32;
			let ColumnImpactCustomSignalID = 33;
			let ColumnImpactCaption = 34;
			let ColumnImpactState = 35;
			let ColumnCustomText = 64;

			// Color Constants
			//
			let backColors = ["#c00000", "#00c000", "#0000c0"];
			let textColors = ["#ffffff", "#000000", "#ffffff"];

			//	Signal identifiers
			//
			var appSignalIDs = schemaItem.appSignalIDs;
			if (appSignalIDs.length < 1 || appSignalIDs.length > 3)
			{
				return;
			}

			if (schemaItem.multiLine == false)
			{
				// Single-line mode: all signal values are displayed in corresponding column
				//
				let cellColumnCount = schemaItem.cellColumnCount;

				let stateIndex = 0;	// Index of the signal state (channel)

				for (let c = 0; c < cellColumnCount; c++)
				{
					let cellType = schemaItem.cellData(0, c);

					// Output the information depending on column data type
					//
					switch (cellType)
					{
						case ColumnCaption:
						{
							schemaItem.setCellText(0, c, "Caption is here");
							break;
						}
						case ColumnState:
						{
							schemaItem.setCellText(0, c, appSignalIDs[stateIndex] + " #" + stateIndex);
							schemaItem.setCellFillColor(0, c, backColors[stateIndex]);
							schemaItem.setCellTextColor(0, c, textColors[stateIndex]);

							stateIndex++;	// Increment state index to proceed to the next channel

							break;
						}
						default:
							// Other columns
							break;
					}
				}
			}
			else
			{
				// Multi-line mode: all signal values are displayed in separate line
				//
				let rowCount = appSignalIDs.length;
				let columnCount = schemaItem.columnCount;

				for (let r = 0; r < rowCount; r++)
				{
					for (let c = 0; c < columnCount; c++)
					{
						let columnType = schemaItem.columnData(c);

						// Output the information depending on column data type
						//
						switch (columnType)
						{
							case ColumnCaption:
							{
								schemaItem.setCellText(r, c, "Row #" + r + ", Caption is here");
								break;
							}
							case ColumnState:
							{
								schemaItem.setCellText(r, c, appSignalIDs[r]);
								schemaItem.setCellFillColor(r, c, "#c0c000");
								schemaItem.setCellTextColor(r, c, "#ffffff");
								break;
							}
							default:
								// Other columns
								break;
						}
					}
				}
			}
		})
		\endcode

		<img src="SchemaItemSignal.bmp" align="left"/>

	*/
	class SchemaItemSignal : public FblItemRect
	{
		Q_OBJECT

		/// \brief Application signal identifiers array. Use <b>appSignalIDs.length</b> to get number of identifiers.
		Q_PROPERTY(QStringList signalIDs READ appSignalIdList)
		Q_PROPERTY(QStringList SignalIDs READ appSignalIdList)

		/// \brief Application signal identifiers array. Use <b>appSignalIDs.length</b> to get number of identifiers.
		Q_PROPERTY(QStringList appSignalIDs READ appSignalIdList)
		Q_PROPERTY(QStringList AppSignalIDs READ appSignalIdList)

		/// \brief Impact application signal identifiers array. Impact signal is usually related to AppSignalID in some or other way. Use <b>impactSignalIDs.length</b> to get number of identifiers.
		Q_PROPERTY(QStringList impactSignalIDs READ impactAppSignalIdList)
		Q_PROPERTY(QStringList ImpactSignalIDs READ impactAppSignalIdList)

		/// \brief Impact application signal identifiers array. Impact signal is usually related to AppSignalID in some or other way. Use <b>impactAppSignalIDs.length</b> to get number of identifiers.
		Q_PROPERTY(QStringList impactAppSignalIDs READ impactAppSignalIdList)
		Q_PROPERTY(QStringList ImpactAppSignalIDs READ impactAppSignalIdList)

		/// \brief Text to print if column data type is set to \ref E::ColumnData::CustomText "CustomText"
		Q_PROPERTY(QString customText READ customText WRITE setCustomText)

		/// \brief If <b>true</b> each signal has own row, if <b>false</b> only one row is drawn and state is displayed as multiple cells.
		Q_PROPERTY(bool multiLine READ multiLine)

		/// \brief Precision for floating point numbers.
		Q_PROPERTY(int precision READ precision WRITE setPrecision)

		/// \brief Way to display numbers.
		Q_PROPERTY(E::AnalogFormat analogFormat READ analogFormat WRITE setAnalogFormat)

		/// \brief Item column count. To get column data type use function \ref columnData.
		///
		Q_PROPERTY(int columnCount READ columnCount)
		Q_PROPERTY(int ColumnCount READ columnCount)

		/** \brief Cell column count. May differ from <b>columnCount</b> as column can have several cells.
		To get cell data type use function \ref cellData, returns \ref E::ColumnData "ColumnData".
		To get AppSignaID associated with cell use function \ref cellAppSignalId.
		*/
		Q_PROPERTY(int cellColumnCount READ cellColumnCount )

		/** \brief Cell row count. Always the same as \ref columnCount.
		To get cell data type use function \ref cellData, returns \ref E::ColumnData "ColumnData".
		To get AppSignaID associated with cell use function \ref cellAppSignalID.
		*/
		Q_PROPERTY(int cellRowCount READ cellColumnCount )

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

	public slots:
		/// \brief Returns column data type for column specified by <b>columnIndex</b>, returns \ref E::ColumnData "ColumnData"
		E::ColumnData columnData(int columnIndex) const;

	public:
		void setColumnData(E::ColumnData value, int columnIndex);

		E::HorzAlign columnHorzAlign(int columnIndex) const;
		void setColumnHorzAlign(E::HorzAlign value, int columnIndex);

		bool hasImpactColumn() const;

		int cellRowCount() const;
		int cellColumnCount() const;

	public slots:
		/// \brief Resets all previously set atrributes to cell.
		void resetCell(int row, int column);

		/// \brief Resets previously set text to cell.
		void resetCellText(int row, int column);

		/// \brief Resets previously set fill color to cell.
		void resetCellFillColor(int row, int column);

		/// \brief Resets previously set text color to cell.
		void resetCellTextColor(int row, int column);

		/// \brief Returns data type associated with the cell.
		E::ColumnData cellData(int row, int column) const;

		/// \brief Returns AppSignalID associated with the cell. <b>Note:</b> the result can be value one of \ref appSignalIDs or \ref impactSignalIDs
		QString cellAppSignalID(int row, int column) const;

		/// \brief Returns overriden (set with setCellText) cell text.
		QString cellText(int row, int column) const;

		/// \brief Overrides cell text, to reset value to default pass undefines as argumnet.
		void setCellText(int row, int column, QString text);

		/// \brief Returns background color for specified cell.
		QColor cellFillColor(int row, int column) const;

		/// \brief Sets background color for specified cell.
		void setCellFillColor(int row, int column, QColor color);

		/// \brief Returns text color for specified cell, to reset value to default pass "" as argumnet.
		QColor cellTextColor(int row, int column) const;

		/// \brief Sets text color for specified cell.
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
	class SchemaItemInput : public SchemaItemSignal
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
	class SchemaItemOutput : public SchemaItemSignal
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
	class SchemaItemInOut : public SchemaItemSignal
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
