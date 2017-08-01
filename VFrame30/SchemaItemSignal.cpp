#include "SchemaItemSignal.h"
#include "LogicSchema.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "../lib/AppSignalManager.h"

namespace VFrame30
{
	//
	// SchemaItemSignal
	//
	SchemaItemSignal::SchemaItemSignal(void) :
		SchemaItemSignal(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemSignal::SchemaItemSignal(SchemaUnit unit) :
		FblItemRect(unit)
	{
		m_columns.resize(2);

		Column& c0 = m_columns[0];
		c0.width = 80;
		c0.data = E::ColumnData::AppSignalID;
		c0.horzAlign = E::HorzAlign::AlignLeft;

		Column& c1 = m_columns[1];
		c1.width = 20;
		c1.data = E::ColumnData::State;
		c1.horzAlign = E::HorzAlign::AlignHCenter;

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true, SchemaItemSignal::appSignalIds, SchemaItemSignal::setAppSignalIds);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::precision, PropertyNames::monitorCategory, true, SchemaItemSignal::precision, SchemaItemSignal::setPrecision);
		ADD_PROPERTY_GET_SET_CAT(E::AnalogFormat, PropertyNames::analogFormat, PropertyNames::monitorCategory, true, SchemaItemSignal::analogFormat, SchemaItemSignal::setAnalogFormat);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::columnCount, PropertyNames::monitorCategory, true, SchemaItemSignal::columnCount, SchemaItemSignal::setColumnCount);

		createColumnProperties();

		return;
	}

	SchemaItemSignal::~SchemaItemSignal(void)
	{
	}

	bool SchemaItemSignal::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemSignal* signal = message->mutable_schemaitem()->mutable_signal();

		for (const QString& strId : m_appSignalIds)
		{
			::Proto::wstring* ps = signal->add_appsignalids();
			Proto::Write(ps, strId);
		}

		signal->set_precision(m_precision);
		signal->set_analogformat(static_cast<int>(m_analogFormat));

		for (const Column& c : m_columns)
		{
			::Proto::SchemaItemSignalColumn* protoColumn = signal->add_columns();

			protoColumn->set_width(c.width);
			protoColumn->set_data(static_cast<int>(c.data));
			protoColumn->set_horzalign(static_cast<int>(c.horzAlign));
		}

		return true;
	}

	bool SchemaItemSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_signal() == false)
		{
			assert(message.schemaitem().has_signal());
			return false;
		}

		const Proto::SchemaItemSignal& signal = message.schemaitem().signal();

		m_appSignalIds.clear();
		m_appSignalIds.reserve(signal.appsignalids_size());

		for (int i = 0; i < signal.appsignalids_size(); i++)
		{
			QString s;
			Proto::Read(signal.appsignalids().Get(i), &s);
			m_appSignalIds.push_back(s);
		}

		m_precision = signal.precision();
		m_analogFormat = static_cast<E::AnalogFormat>(signal.analogformat());

		m_columns.resize(signal.columns_size());
		for (int i = 0; i < signal.columns_size(); i++)
		{
			const ::Proto::SchemaItemSignalColumn& c = signal.columns(i);

			m_columns[i].width = c.width();
			m_columns[i].data = static_cast<E::ColumnData>(c.data());
			m_columns[i].horzAlign = static_cast<E::HorzAlign>(c.horzalign());
		}
		createColumnProperties();

		return true;
	}


	void SchemaItemSignal::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);

		if (drawParam->isMonitorMode() == true)
		{
			if (drawParam->appSignalManager() ==  nullptr)
			{
				assert(drawParam->appSignalManager() != nullptr);
				return;
			}
		}

		//--
		//
		QPainter* p = drawParam->painter();

		QPen linePen(lineColor());
		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);		// Don't use getter!

		// Draw slash lines
		//
		if (multiChannel() == true)
		{
			drawMultichannelSlashLines(p, linePen);
		}

		//
		// Draw columns text and devider
		//

		// If there is no column just draw identififers
		//
		if (columnCount() == 0)
		{
			drawFullLineIds(drawParam);
			return;
		}

		if (multiChannel() == true)
		{
			drawMultichannelValues(drawParam, linePen);
		}
		else
		{
			drawSinglechannelValues(drawParam, linePen);
		}

		return;
	}

	QString SchemaItemSignal::getCoulumnText(CDrawParam* drawParam,
											 const E::ColumnData& data,
											 const AppSignalParam& signal,
											 const AppSignalState& signalState,
											 E::AnalogFormat analogFormat,
											 int precision)
	{
		QString text;

		switch (data)
		{
		case E::ColumnData::AppSignalID:
			text = signal.appSignalId();
			break;

		case E::ColumnData::CustomSignalID:
			if (drawParam->isMonitorMode() == true)
			{
				text = signal.customSignalId();

				if (text.isEmpty() == true)
				{
					text = QLatin1String("?");
				}
			}
			else
			{
				text = signal.appSignalId();
			}
			break;

		case E::ColumnData::Caption:
			if (drawParam->isMonitorMode() == true)
			{
				text = signal.caption();

				if (text.isEmpty() == true)
				{
					text = QLatin1String("?");
				}
			}
			else
			{
				//text = QLatin1String("Caption");
				text = signal.appSignalId();			// Good to see AppSignalID while editing
			}
			break;


		case E::ColumnData::State:
			{
				if (drawParam->isMonitorMode() == true)
				{
					if (signalState.m_flags.valid == false)
					{
						const static QString nonValidStr = "?";
						text = nonValidStr;
					}
					else
					{
						if (signal.isAnalog())
						{
							text = QString::number(signalState.m_value, static_cast<char>(analogFormat), precision);
						}
						else
						{
							text = QString::number(signalState.m_value);
						}
					}
				}
				else
				{
					text = QLatin1String("0");
				}
			}
			break;

		default:
			assert(false);
		}

		return text;
	}


	void SchemaItemSignal::drawMultichannelSlashLines(QPainter* painter, QPen& linePen) const
	{
		if (painter == nullptr)
		{
			assert(painter);
			return;
		}

		double pinWidth = GetPinWidth(itemUnit(), painter->device());

		painter->setPen(linePen);

		QRectF r = itemRectWithPins();

		if (inputsCount() > 0)
		{
			const std::vector<AfbPin>& inputPins = inputs();
			assert(inputPins.empty() == false);

			painter->drawLine(QPointF(r.left() + (pinWidth / 3.0) * 2.0, inputPins.front().y() - pinWidth / 4.0),
							  QPointF(r.left() + (pinWidth / 3.0) * 1.0, inputPins.front().y() + pinWidth / 4.0));
		}

		if (outputsCount() > 0)
		{
			const std::vector<AfbPin>& pins = outputs();
			assert(pins.empty() == false);

			painter->drawLine(QPointF(r.right() - (pinWidth / 3.0) * 2.0, pins.front().y() + pinWidth / 4.0),
							  QPointF(r.right() - (pinWidth / 3.0) * 1.0, pins.front().y() - pinWidth / 4.0));
		}

		return;
	}

	void SchemaItemSignal::drawFullLineIds(CDrawParam* drawParam) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* painter = drawParam->painter();

		QRectF rect = itemRectPinIndent(drawParam->device());

		rect.setLeft(rect.left() + m_font.drawSize() / 4.0);
		rect.setRight(rect.right() - m_font.drawSize() / 4.0);

		painter->setPen(textColor());

		QString text;

		const VFrame30::LogicSchema* logicSchema = dynamic_cast<const VFrame30::LogicSchema*>(drawParam->schema());

		if (multiChannel() == true && logicSchema != nullptr && appSignalIds().size() >= 1)
		{
			text = appSignalIds();
		}
		else
		{
			text = appSignalIds();

			if (drawParam->isMonitorMode() == true)
			{
				bool stateOk;
				AppSignalState signalState = drawParam->appSignalManager()->signalState(text, &stateOk);

				if (signalState.m_flags.valid == false)
				{
					text += QString("    ?");
				}
				else
				{
					text += QString("    %1").arg(QString::number(signalState.m_value));
				}
			}
		}

		DrawHelper::drawText(painter, m_font, itemUnit(), text, rect, Qt::AlignLeft | Qt::AlignTop);
	}

	void SchemaItemSignal::drawMultichannelValues(CDrawParam* drawParam, QPen& linePen) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* painter = drawParam->painter();
		QRectF rect = itemRectPinIndent(drawParam->device());

		// Get signakls and siognal states
		//
		//const VFrame30::LogicSchema* logicSchema = dynamic_cast<const VFrame30::LogicSchema*>(drawParam->schema());

		const QStringList& signalIds = appSignalIdList();
		if (signalIds.size() == 0)
		{
			return;
		}

		std::vector<AppSignalParam> appSignals;
		appSignals.resize(signalIds.size());

		std::vector<AppSignalState> appSignalStates;
		appSignalStates.resize(signalIds.size());

		bool isMonitorMode = drawParam->isMonitorMode();

		int signalIndex = 0;
		for (const QString& id : signalIds)
		{
			appSignals[signalIndex].setAppSignalId(id);
			appSignalStates[signalIndex].m_flags.valid = false;

			bool signalFound = false;

			if (isMonitorMode == true)
			{
				// Get signal description
				//
				appSignals[signalIndex] = drawParam->appSignalManager()->signalParam(id, &signalFound);

				// Get signal state
				//
				appSignalStates[signalIndex] = drawParam->appSignalManager()->signalState(id, nullptr);
			}

			signalIndex ++;
		}

		// Draw column text
		//
		double columntHeight = rect.height() / signalIds.size();

		if (columntHeight < m_font.drawSize())
		{
			columntHeight = m_font.drawSize();
		}

		for (int i = 0; i < signalIds.size(); i++)
		{
			QString appSignalId = signalIds[i];

			double top = rect.top() + i * columntHeight;

			double startOffset = 0;

			for (size_t columnIndex = 0; columnIndex < m_columns.size(); columnIndex++)
			{
				const Column& column = m_columns[columnIndex];

				double columnWidth = rect.width() * (column.width / 100.0);
				if (columnIndex == m_columns.size() - 1)
				{
					// if this is the last column, give all rest width to it
					//
					columnWidth = rect.width() - startOffset;
				}

				double left = rect.left() + startOffset;

				// --
				//
				QString text = getCoulumnText(drawParam, column.data, appSignals[i], appSignalStates[i], m_analogFormat, m_precision);

				QRectF textRect(left, top, columnWidth, columntHeight);

				textRect.setLeft(textRect.left() + m_font.drawSize() / 4.0);
				textRect.setRight(textRect.right() - m_font.drawSize() / 4.0);

				painter->setPen(textColor());

				QRectF boundingRect = rect.intersected(textRect);

				DrawHelper::drawText(painter, m_font, itemUnit(), text, boundingRect, column.horzAlign | Qt::AlignVCenter);

				// --
				//
				startOffset += columnWidth;

				if (startOffset >= rect.width())
				{
					break;
				}
			}
		}

		// Draw horizontal dividers
		//
		painter->setPen(linePen);

		for (int i = 0; i < signalIds.size(); i++)
		{
			QString appSignalId = signalIds[i];

			double top = rect.top() + i * columntHeight;
			double bottom = top + columntHeight;

			if (bottom > rect.bottom() ||
				i == signalIds.size() - 1)	// Dont draw the last line
			{
				break;
			}

			painter->drawLine(drawParam->gridToDpi(rect.left(), bottom),
							  drawParam->gridToDpi(rect.right(), bottom));
		}

		//  Draw vertical deviders
		//
		double startOffset = 0;
		for (size_t i = 0; i < m_columns.size(); i++)
		{
			const Column& c = m_columns[i];

			double width = rect.width() * (c.width / 100.0);

			// if this is the last column, give all rest width to it
			//
			if (i == m_columns.size() - 1)
			{
				width = rect.width() - startOffset;
			}

			// --
			//
			startOffset += width;

			if (startOffset >= rect.width())
			{
				break;
			}

			// Draw vertical line devider from othe columns
			//
			if (i < m_columns.size() - 1)	// For all columns exceprt last
			{
				painter->drawLine(drawParam->gridToDpi(rect.left() + startOffset, rect.top()),
								  drawParam->gridToDpi(rect.left() + startOffset, rect.bottom()));
			}
		}

		return;
	}

	void SchemaItemSignal::drawSinglechannelValues(CDrawParam* drawParam, QPen& linePen) const
	{
		if (drawParam == nullptr ||
			multiChannel() == true)
		{
			assert(drawParam);
			assert(multiChannel() == false);
			return;
		}

		QPainter* painter = drawParam->painter();

		QRectF rect = itemRectPinIndent(drawParam->device());

		rect.setLeft(rect.left() + m_font.drawSize() / 4.0);
		rect.setRight(rect.right() - m_font.drawSize() / 4.0);

		double startOffset = 0;

		QString appSignalId = appSignalIds();

		AppSignalParam signal;
		signal.setAppSignalId(appSignalId);

		AppSignalState signalState;
		signalState.m_flags.valid = false;

		bool signalFound = false;

		if (drawParam->isMonitorMode() == true)
		{
			signal = drawParam->appSignalManager()->signalParam(appSignalId, &signalFound);
			signalState = drawParam->appSignalManager()->signalState(appSignalId, nullptr);
		}

		for (size_t i = 0; i < m_columns.size(); i++)
		{
			const Column& c = m_columns[i];

			double width = rect.width() * (c.width / 100.0);

			// if this is the last column, give all rest width to it
			//
			if (i == m_columns.size() - 1)
			{
				width = rect.width() - startOffset;
			}

			// Draw data
			//
			QString text = getCoulumnText(drawParam, c.data, signal, signalState, m_analogFormat, m_precision);

			QRectF textRect(rect.left() + startOffset, rect.top(), width, rect.height());
			textRect.setLeft(textRect.left() + m_font.drawSize() / 4.0);
			textRect.setRight(textRect.right() - m_font.drawSize() / 4.0);

			painter->setPen(textColor());

			DrawHelper::drawText(painter, m_font, itemUnit(), text, textRect, c.horzAlign | Qt::AlignTop);

			// --
			//
			startOffset += width;

			if (startOffset >= rect.width())
			{
				break;
			}

			// Draw vertical line devider from othe columns
			//
			if (i < m_columns.size() - 1)	// For all columns exceprt last
			{
				painter->setPen(linePen);

				painter->drawLine(drawParam->gridToDpi(rect.left() + startOffset, rect.top()),
								  drawParam->gridToDpi(rect.left() + startOffset, rect.bottom()));
			}
		}

		return;
	}




	void SchemaItemSignal::createColumnProperties()
	{
static const QString monitorColumnsCategory = "MonitorColumns";

static const QString column_width_caption[8] = {"Column_00_Width", "Column_01_Width", "Column_02_Width", "Column_03_Width",
												"Column_04_Width", "Column_05_Width", "Column_06_Width", "Column_07_Width"};

static const QString column_data_caption[8] = {"Column_00_Data", "Column_01_Data", "Column_02_Data", "Column_03_Data",
											   "Column_04_Data", "Column_05_Data", "Column_06_Data", "Column_07_Data"};

static const QString column_horzAlign_caption[8] = {"Column_00_HorzAlign", "Column_01_HorzAlign", "Column_02_HorzAlign", "Column_03_HorzAlign",
											   "Column_04_HorzAlign", "Column_05_HorzAlign", "Column_06_HorzAlign", "Column_07_HorzAlign"};

		if (m_columns.size() > 8)
		{
			assert(m_columns.size() <= 8);
			return;
		}

		// Delete all ColumnXX props
		//
		std::vector<std::shared_ptr<Property>> allProperties = properties();

		for (const std::shared_ptr<Property>& p : allProperties)
		{
			if (p->caption().startsWith(QLatin1String("Column_")) == true)
			{
				removeProperty(p->caption());
			}
		}

		// Create new ColumnXX props
		//
		for (size_t i = 0; i < m_columns.size(); i++)
		{
			addProperty<double>(column_width_caption[i],
								monitorColumnsCategory,
								true,
								std::bind(&SchemaItemSignal::columnWidth, this, static_cast<int>(i)),
								std::bind(&SchemaItemSignal::setColumnWidth, this, std::placeholders::_1, static_cast<int>(i)));

			addProperty<E::ColumnData>(column_data_caption[i],
									   monitorColumnsCategory,
									   true,
									   std::bind(&SchemaItemSignal::columnData, this, static_cast<int>(i)),
									   std::bind(&SchemaItemSignal::setColumnData, this, std::placeholders::_1, static_cast<int>(i)));

			addProperty<E::HorzAlign>(column_horzAlign_caption[i],
									  monitorColumnsCategory,
									  true,
									  std::bind(&SchemaItemSignal::columnHorzAlign, this, static_cast<int>(i)),
									  std::bind(&SchemaItemSignal::setColumnHorzAlign, this, std::placeholders::_1, static_cast<int>(i)));
		}

		return;
	}

	double SchemaItemSignal::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		int linesCount = m_appSignalIds.size();
		if (linesCount == 0)
		{
			linesCount = 1;
		}

		return linesCount * gridSize * pinGridStep;
	}

	double SchemaItemSignal::minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		return m_cachedGridSize * 10;
	}

	// Text search
	//
	bool SchemaItemSignal::searchText(const QString& text) const
	{
		return	FblItemRect::searchText(text) ||
				appSignalIds().contains(text, Qt::CaseInsensitive);
	}

	QString SchemaItemSignal::appSignalIds() const
	{
		QString result;

		for (QString s : m_appSignalIds)
		{
			s = s.trimmed();

			if (result.isEmpty() == false)
			{
				result.append(QChar::LineFeed);
			}

			result.append(s);
		}

		return result;
	}

	const QStringList& SchemaItemSignal::appSignalIdList() const
	{
		return m_appSignalIds;
	}

	void SchemaItemSignal::setAppSignalIds(const QString& s)
	{
		m_appSignalIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);

		for (QString& s : m_appSignalIds)
		{
			s = s.trimmed();
		}

		adjustHeight();
		return;
	}

	QStringList* SchemaItemSignal::mutable_appSignalIds()
	{
		return &m_appSignalIds;
	}

	bool SchemaItemSignal::multiChannel() const
	{
		return m_appSignalIds.size() > 1;
	}

	int SchemaItemSignal::precision() const
	{
		return m_precision;
	}

	void SchemaItemSignal::setPrecision(int value)
	{
		if (value < 0)
		{
			value = 0;
		}

		if (value > 12)
		{
			value = 12;
		}

		m_precision = value;
	}

	E::AnalogFormat SchemaItemSignal::analogFormat() const
	{
		return m_analogFormat;
	}

	void SchemaItemSignal::setAnalogFormat(E::AnalogFormat value)
	{
		m_analogFormat = value;
	}

	int SchemaItemSignal::columnCount() const
	{
		return static_cast<int>(m_columns.size());
	}

	void SchemaItemSignal::setColumnCount(int value)
	{
		if (value < 1)
		{
			value = 1;
		}

		if (value > 8)
		{
			value = 8;
		}

		if (m_columns.size() != value)
		{
			m_columns.resize(value);
			createColumnProperties();
		}

		return;
	}

	double SchemaItemSignal::columnWidth(int columnIndex) const
	{
		if (columnIndex < 0 ||
			columnIndex >= m_columns.size())
		{
			assert(columnIndex >= 0);
			assert(columnIndex < m_columns.size());
			return 0.0;
		}

		return m_columns[columnIndex].width;
	}

	void SchemaItemSignal::setColumnWidth(double value, int columnIndex)
	{
		if (columnIndex < 0 ||
			columnIndex >= m_columns.size())
		{
			assert(columnIndex >= 0);
			assert(columnIndex < m_columns.size());
			return;
		}

		if (value < 0)
		{
			value = 0;
		}

		if (value > 100)
		{
			value = 100;
		}

		m_columns[columnIndex].width = value;
		return;
	}

	E::ColumnData SchemaItemSignal::columnData(int columnIndex) const
	{
		if (columnIndex < 0 ||
			columnIndex >= m_columns.size())
		{
			assert(columnIndex >= 0);
			assert(columnIndex < m_columns.size());
			return E::ColumnData::AppSignalID;
		}

		return m_columns[columnIndex].data;
	}

	void SchemaItemSignal::setColumnData(E::ColumnData value, int columnIndex)
	{
		if (columnIndex < 0 ||
			columnIndex >= m_columns.size())
		{
			assert(columnIndex >= 0);
			assert(columnIndex < m_columns.size());
			return;
		}

		m_columns[columnIndex].data = value;
		return;
	}

	E::HorzAlign SchemaItemSignal::columnHorzAlign(int columnIndex) const
	{
		if (columnIndex < 0 ||
			columnIndex >= m_columns.size())
		{
			assert(columnIndex >= 0);
			assert(columnIndex < m_columns.size());
			return E::HorzAlign::AlignLeft;
		}

		return m_columns[columnIndex].horzAlign;
	}

	void SchemaItemSignal::setColumnHorzAlign(E::HorzAlign value, int columnIndex)
	{
		if (columnIndex < 0 ||
			columnIndex >= m_columns.size())
		{
			assert(columnIndex >= 0);
			assert(columnIndex < m_columns.size());
			return;
		}

		m_columns[columnIndex].horzAlign = value;
		return;
	}

	//
	// CSchemaItemInputSignal
	//
	SchemaItemInput::SchemaItemInput(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemInput::SchemaItemInput(SchemaUnit unit) :
		SchemaItemSignal(unit)
	{
		addOutput();
		setAppSignalIds("#IN_STRID");
	}

	SchemaItemInput::~SchemaItemInput(void)
	{
		assert(outputsCount() == 1);
	}

	QString SchemaItemInput::buildName() const
	{
		return QString("Input (%1)").arg(appSignalIds());
	}

	// Serialization
	//
	bool SchemaItemInput::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemSignal::SaveData(message);
		
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemInputSignal* inputSignal = */message->mutable_schemaitem()->mutable_inputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool SchemaItemInput::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = SchemaItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}
		
		// --
		//
		if (message.schemaitem().has_inputsignal() == false)
		{
			assert(message.schemaitem().has_inputsignal());
			return false;
		}

		/*const Proto::VideoItemInputSignal& inputSignal = */message.schemaitem().inputsignal();
		//fill = inputSignal.fill();

		return true;
	}
	
	//
	// CSchemaItemOutputSignal
	//
	SchemaItemOutput::SchemaItemOutput(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemOutput::SchemaItemOutput(SchemaUnit unit) :
		SchemaItemSignal(unit)
	{
		addInput();
		setAppSignalIds("#OUT_STRID");
	}

	SchemaItemOutput::~SchemaItemOutput(void)
	{
		assert(inputsCount() == 1);
	}

	QString SchemaItemOutput::buildName() const
	{
		return QString("Output (%1)").arg(appSignalIds());
	}

	// Serialization
	//
	bool SchemaItemOutput::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemSignal::SaveData(message);
		
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemOutputSignal* outputSignal = */message->mutable_schemaitem()->mutable_outputsignal();

		//inputSignal->set_weight(weight);

		return true;
	}

	bool SchemaItemOutput::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = SchemaItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_outputsignal() == false)
		{
			assert(message.schemaitem().has_outputsignal());
			return false;
		}

		/*const Proto::VideoItemOutputSignal& outputSignal = */message.schemaitem().outputsignal();
		//fill = inputSignal.fill();

		return true;
	}


	//
	// SchemaItemInOut
	//
	SchemaItemInOut::SchemaItemInOut(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemInOut::SchemaItemInOut(SchemaUnit unit) :
		SchemaItemSignal(unit)
	{
		addInput();
		addOutput();
		setAppSignalIds("#APPSIGNALID");

		m_columns.resize(1);

		Column& c0 = m_columns[0];
		c0.width = 100;
		c0.data = E::ColumnData::State;
		c0.horzAlign = E::HorzAlign::AlignHCenter;

		createColumnProperties();
	}

	SchemaItemInOut::~SchemaItemInOut(void)
	{
		assert(inputsCount() == 1);
		assert(outputsCount() == 1);
	}

	QString SchemaItemInOut::buildName() const
	{
		return QString("Input/Output (%1)").arg(appSignalIds());
	}

	// Serialization
	//
	bool SchemaItemInOut::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItemSignal::SaveData(message);

		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		/*Proto::VideoItemOutputSignal* inoutSignal = */message->mutable_schemaitem()->mutable_inoutsignal();

		//inoutSignal->set_weight(weight);

		return true;
	}

	bool SchemaItemInOut::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = SchemaItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_inoutsignal() == false)
		{
			assert(message.schemaitem().has_inoutsignal());
			return false;
		}

		/*const Proto::VideoItemOutputSignal& inoutSignal = */message.schemaitem().inoutsignal();
		//fill = inoutSignal.fill();

		return true;
	}

}

