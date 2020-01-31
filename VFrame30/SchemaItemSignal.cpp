#include "SchemaItemSignal.h"
#include "LogicSchema.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "AppSignalController.h"

namespace VFrame30
{
	//
	// SchemaItemSignal
	//
	SchemaItemSignal::SchemaItemSignal(void) :
		SchemaItemSignal(SchemaUnit::Inch)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
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

		Property* prop = nullptr;

		prop = addProperty<QString, SchemaItemSignal, &SchemaItemSignal::appSignalIds, &SchemaItemSignal::setAppSignalIds>(PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true);
		prop->setValidator(PropertyNames::appSignalIDsValidator);

		prop = addProperty<QString, SchemaItemSignal, &SchemaItemSignal::impactAppSignalIds, &SchemaItemSignal::setImpactAppSignalIds>(PropertyNames::impactAppSignalIDs, PropertyNames::functionalCategory, true);
		prop->setValidator(PropertyNames::appSignalIDsValidator);

		addProperty<bool, SchemaItemSignal, &SchemaItemSignal::multiLine, &SchemaItemSignal::setMultiLine>(PropertyNames::multiLine, PropertyNames::appearanceCategory, true);
		addProperty<int, SchemaItemSignal, &SchemaItemSignal::precision, &SchemaItemSignal::setPrecision>(PropertyNames::precision, PropertyNames::monitorCategory, true);
		addProperty<E::AnalogFormat, SchemaItemSignal, &SchemaItemSignal::analogFormat, &SchemaItemSignal::setAnalogFormat>(PropertyNames::analogFormat, PropertyNames::monitorCategory, true);
		addProperty<int, SchemaItemSignal, &SchemaItemSignal::columnCount, &SchemaItemSignal::setColumnCount>(PropertyNames::columnCount, PropertyNames::monitorCategory, true);

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

		for (const QString& strId : m_impactAppSignalIds)
		{
			signal->add_impactappsignalids(strId.toStdString());
		}

		signal->set_multiline(m_multiLine);
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

		// --
		//
		m_appSignalIds.clear();
		m_appSignalIds.reserve(signal.appsignalids_size());

		for (int i = 0; i < signal.appsignalids_size(); i++)
		{
			QString s;
			Proto::Read(signal.appsignalids().Get(i), &s);
			m_appSignalIds.push_back(s);
		}

		// --
		//
		m_impactAppSignalIds.clear();
		m_impactAppSignalIds.reserve(signal.impactappsignalids_size());

		for (const auto& is : signal.impactappsignalids())
		{
			m_impactAppSignalIds.push_back(QString::fromStdString(is));
		}

		// --
		//
		m_multiLine = signal.multiline();
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


	void SchemaItemSignal::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		if (multiChannel() == true)
		{
			QString pinCaption = QString::number(m_appSignalIds.size());

			std::vector<VFrame30::AfbPin>* ins = const_cast<SchemaItemSignal*>(this)->mutableInputs();
			std::vector<VFrame30::AfbPin>* outs = const_cast<SchemaItemSignal*>(this)->mutableOutputs();

			for (VFrame30::AfbPin& pin : *ins)
			{
				pin.setCaption(pinCaption);
			}

			for (VFrame30::AfbPin& pin : *outs)
			{
				pin.setCaption(pinCaption);
			}
		}

		FblItemRect::draw(drawParam, schema, layer);

		if (drawParam->isMonitorMode() == true)
		{
			if (drawParam->appSignalController() ==  nullptr)
			{
				assert(drawParam->appSignalController() != nullptr);
				return;
			}
		}

		//--
		//
		QPen linePen(lineColor());
		linePen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);		// Don't use getter!

		// Draw slash lines
		//
		if (multiChannel() == true)
		{
			drawMultichannelSlashLines(drawParam, linePen);
		}

		//
		// Draw columns text and devider
		//

		// If there is no column just draw identififers
		//
		if (columnCount() == 0)
		{
			drawFullLineIds(drawParam);
		}
		else
		{
			if (multiChannel() == true)
			{
				drawMultichannelValues(drawParam, linePen);
			}
			else
			{
				drawSinglechannelValues(drawParam, linePen);
			}
		}

		// Draw highlights for m_appSignalIds, m_impactAppSignalIds
		//
		QStringList allIds = m_appSignalIds + m_impactAppSignalIds;

		for (const QString& appSignalId : allIds)
		{
			if (drawParam->hightlightIds().contains(appSignalId) == true)
			{
				QRectF highlightRect = itemRectPinIndent(drawParam);
				drawHighlightRect(drawParam, highlightRect);
				break;
			}
		}

		return;
	}

	QString SchemaItemSignal::getCoulumnText(CDrawParam* drawParam,
											 const E::ColumnData& data,
											 const AppSignalParam& signal,
											 const AppSignalState& signalState,
											 const AppSignalParam& impactSignal,
											 const AppSignalState& impactSignalState,
											 E::AnalogFormat analogFormat,
											 int precision)
	{
		QString text;

		switch (data)
		{
		case E::ColumnData::AppSignalID:
			text = signal.appSignalId();
			break;

		case E::ColumnData::ImpactAppSignalID:
			text = impactSignal.appSignalId();
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

		case E::ColumnData::ImpactCustomSignalID:
			if (drawParam->isMonitorMode() == true)
			{
				text = impactSignal.customSignalId();
				if (text.isEmpty() == true)
				{
					text = QLatin1String("?");
				}
			}
			else
			{
				text = impactSignal.appSignalId();
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
				text = signal.appSignalId();			// Good to see AppSignalID while editing
			}
			break;

		case E::ColumnData::ImpactCaption:
			if (drawParam->isMonitorMode() == true)
			{
				text = impactSignal.caption();
				if (text.isEmpty() == true)
				{
					text = QLatin1String("?");
				}
			}
			else
			{
				text = impactSignal.appSignalId();			// Good to see AppSignalID while editing
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

		case E::ColumnData::ImpactState:
			{
				if (drawParam->isMonitorMode() == true)
				{
					if (impactSignalState.m_flags.valid == false)
					{
						const static QString nonValidStr = "?";
						text = nonValidStr;
					}
					else
					{
						if (impactSignal.isAnalog())
						{
							text = QString::number(impactSignalState.m_value, static_cast<char>(analogFormat), precision);
						}
						else
						{
							text = QString::number(impactSignalState.m_value);
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
			Q_ASSERT(false);
		}

		return text;
	}

	void SchemaItemSignal::drawFullLineIds(CDrawParam* drawParam) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* painter = drawParam->painter();

		QRectF rect = itemRectPinIndent(drawParam);

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
				AppSignalState signalState = drawParam->appSignalController()->signalState(text, &stateOk);

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
		QRectF rect = itemRectPinIndent(drawParam);

		// Get signakls and siognal states
		//
		//const VFrame30::LogicSchema* logicSchema = dynamic_cast<const VFrame30::LogicSchema*>(drawParam->schema());

		// Get AppSignals
		//
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

			if (isMonitorMode == true)
			{
				// Get signal description/state
				//

				if (drawParam->schema()->isUfbSchema() == true)
				{
					appSignals[signalIndex] = AppSignalParam();
					appSignalStates[signalIndex] = AppSignalState();

					appSignals[signalIndex].setAppSignalId(id);		// If signal tot found it allows to show AppSignalID at least
				}
				else
				{
					appSignals[signalIndex] = drawParam->appSignalController()->signalParam(id, nullptr);
					appSignalStates[signalIndex] = drawParam->appSignalController()->signalState(id, nullptr);
				}
			}

			signalIndex ++;
		}

		// Get ImpactAppSignals
		//
		const QStringList& impactSignalIds = impactAppSignalIdList();
		if (signalIds.size() == 0)
		{
			return;
		}

		std::vector<AppSignalParam> impactAppSignals;
		impactAppSignals.resize(std::max(impactSignalIds.size(), signalIds.size()));

		std::vector<AppSignalState> impactAppSignalStates;
		impactAppSignalStates.resize(std::max(impactSignalIds.size(), signalIds.size()));

		signalIndex = 0;
		for (const QString& id : impactSignalIds)
		{
			impactAppSignals[signalIndex].setAppSignalId(id);
			impactAppSignalStates[signalIndex].m_flags.valid = false;

			signalIndex ++;
		}

		signalIndex = 0;
		for (const QString& id : impactSignalIds)
		{
			if (isMonitorMode == true)
			{
				// Get signal description/state
				//
				impactAppSignals[signalIndex] = drawParam->appSignalController()->signalParam(id, nullptr);
				impactAppSignalStates[signalIndex] = drawParam->appSignalController()->signalState(id, nullptr);
			}

			signalIndex ++;
		}

		Q_ASSERT(impactAppSignals.size() >= appSignals.size());
		Q_ASSERT(impactAppSignalStates.size() >= appSignalStates.size());

		// Draw column text
		//
		double columntHeight = 0;
		double minLineHeight = qMax(drawParam->gridSize() * drawParam->pinGridStep(), m_font.drawSize());

		if (multiLine() == true)
		{
			columntHeight = rect.height() / signalIds.size();
			if (columntHeight < minLineHeight)
			{
				columntHeight = minLineHeight;
			}
		}
		else
		{
			columntHeight = minLineHeight;
		}

		int lineCount = signalIds.size();
		if (multiLine() == false && lineCount != 0)
		{
			lineCount = 1;
		}

		for (int i = 0; i < lineCount; i++)
		{
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

				if (multiLine() == false &&
					(column.data == E::ColumnData::State || column.data == E::ColumnData::ImpactState))
				{
					// Divide column state on signal count and draw all them
					//
					if (column.data == E::ColumnData::State)
					{
						double subColumnWidth = columnWidth / signalIds.size();

						for (int f = 0; f < signalIds.size(); f++)
						{
							QString text = getCoulumnText(drawParam,
														  column.data,
														  appSignals[f],
														  appSignalStates[f],
														  AppSignalParam{},
														  AppSignalState{},
														  m_analogFormat,
														  m_precision);

							QRectF textRect(left + subColumnWidth * f, top, subColumnWidth, columntHeight);
							textRect.setLeft(textRect.left() + m_font.drawSize() / 8.0);
							textRect.setRight(textRect.right() - m_font.drawSize() / 8.0);

							if (textRect.width() > m_font.drawSize() / 8.0)
							{
								painter->setPen(textColor());
								QRectF boundingRect = rect.intersected(textRect);
								DrawHelper::drawText(painter, m_font, itemUnit(), text, boundingRect, column.horzAlign | Qt::AlignVCenter);
							}
						}
					}
					else
					{
						Q_ASSERT(column.data == E::ColumnData::ImpactState);

						double subColumnWidth = columnWidth / impactSignalIds.size();

						for (int f = 0; f < impactSignalIds.size(); f++)
						{
							QString text = getCoulumnText(drawParam,
														  column.data,
														  AppSignalParam{},
														  AppSignalState{},
														  impactAppSignals[f],
														  impactAppSignalStates[f],
														  m_analogFormat,
														  m_precision);

							QRectF textRect(left + subColumnWidth * f, top, subColumnWidth, columntHeight);
							textRect.setLeft(textRect.left() + m_font.drawSize() / 8.0);
							textRect.setRight(textRect.right() - m_font.drawSize() / 8.0);

							if (textRect.width() > m_font.drawSize() / 8.0)
							{
								painter->setPen(textColor());
								QRectF boundingRect = rect.intersected(textRect);
								DrawHelper::drawText(painter, m_font, itemUnit(), text, boundingRect, column.horzAlign | Qt::AlignVCenter);
							}
						}
					}
				}
				else
				{
					QString text = getCoulumnText(drawParam,
												  column.data,
												  appSignals[i],
												  appSignalStates[i],
												  impactAppSignals[i],
												  impactAppSignalStates[i],
												  m_analogFormat,
												  m_precision);

					QRectF textRect(left, top, columnWidth, columntHeight);
					textRect.setLeft(textRect.left() + m_font.drawSize() / 4.0);
					textRect.setRight(textRect.right() - m_font.drawSize() / 4.0);

					if (textRect.width() > m_font.drawSize() / 4.0)
					{
						painter->setPen(textColor());
						QRectF boundingRect = rect.intersected(textRect);
						DrawHelper::drawText(painter, m_font, itemUnit(), text, boundingRect, column.horzAlign | Qt::AlignVCenter);
					}
				}

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

		if (multiLine() == true)
		{
			for (int i = 0; i < lineCount; i++)
			{
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
		}

		//  Draw vertical deviders
		//
		std::vector<double> xpos;
		xpos.reserve(32);

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

			if (multiLine() == false && c.data == E::ColumnData::State)
			{
				double subColumnWidth = width / signalIds.size();

				for (int f = 0; f < signalIds.size(); f++)
				{
					double x = rect.left() + startOffset + subColumnWidth * (f + 1);
					xpos.push_back(x);
				}

				startOffset += width;
				continue;
			}

			if (multiLine() == false && c.data == E::ColumnData::ImpactState)
			{
				double subColumnWidth = width / impactSignalIds.size();

				for (int f = 0; f < impactSignalIds.size(); f++)
				{
					double x = rect.left() + startOffset + subColumnWidth * (f + 1);
					xpos.push_back(x);
				}

				startOffset += width;
				continue;
			}

			startOffset += width;

			if (i < m_columns.size() - 1)	// don't draw last vert line
			{
				xpos.push_back(rect.left() + startOffset);
			}
		}

		for (double x : xpos)
		{
			QPointF pt1 = drawParam->gridToDpi(x, rect.top());
			QPointF pt2 = drawParam->gridToDpi(x, rect.bottom());

			if (pt1.x() >= rect.right())
			{
				break;
			}

			painter->drawLine(pt1, pt2);
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

		QRectF rect = itemRectPinIndent(drawParam);

		double startOffset = 0;

		// Get AppSignal
		//
		QString appSignalId = appSignalIds();

		AppSignalParam signal;
		signal.setAppSignalId(appSignalId);

		AppSignalState signalState;
		signalState.m_flags.valid = false;

		if (drawParam->isMonitorMode() == true && isCommented() == false)
		{
			if (drawParam->schema()->isUfbSchema() == true)
			{
				//signal = AppSignalParam();
				//signalState = AppSignalState();

				signal.setAppSignalId(appSignalId);		// If signal tot found it allows to show AppSignalID at least
			}
			else
			{
				signal = drawParam->appSignalController()->signalParam(appSignalId, nullptr);
				signalState = drawParam->appSignalController()->signalState(appSignalId, nullptr);
			}
		}

		// Get ImpactSignal
		//
		QString impactAppSignalId = impactAppSignalIds();

		AppSignalParam impactSignal;
		impactSignal.setAppSignalId(impactAppSignalId);

		AppSignalState impactSignalState;
		impactSignalState.m_flags.valid = false;

		if (drawParam->isMonitorMode() == true && isCommented() == false)
		{
			impactSignal = drawParam->appSignalController()->signalParam(impactAppSignalId, nullptr);
			impactSignalState = drawParam->appSignalController()->signalState(impactAppSignalId, nullptr);
		}

		// --
		//
		for (size_t i = 0; i < m_columns.size(); i++)
		{
			const Column& c = m_columns[i];

			double width = rect.width() * (c.width / 100.0);
			if (i == m_columns.size() - 1)
			{
				width = rect.width() - startOffset; // if this is the last column, give all rest width to it
			}

			// Draw data
			//
			QString text = getCoulumnText(drawParam,
										  c.data,
										  signal,
										  signalState,
										  impactSignal,
										  impactSignalState,
										  m_analogFormat,
										  m_precision);

			QRectF textRect(rect.left() + startOffset, rect.top(), width, rect.height());
			textRect.setLeft(textRect.left() + m_font.drawSize() / 4.0);
			textRect.setRight(textRect.right() - m_font.drawSize() / 4.0);

			if (textRect.width() > 0)
			{
				painter->setPen(textColor());
				DrawHelper::drawText(painter, m_font, itemUnit(), text, textRect, c.horzAlign | Qt::AlignTop);
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

		// Block signals is required here, as removeProperty, addProperty emit propertyListChanged signal,
		// but PropertyEditor tries to update values and we have assers
		//
		blockSignals(true);

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
								[this, i](){return columnWidth(static_cast<int>(i));},
								[this, i](auto value){return setColumnWidth(value, static_cast<int>(i));});

			addProperty<E::ColumnData>(column_data_caption[i],
									   monitorColumnsCategory,
									   true,
									   [this, i](){return columnData(static_cast<int>(i));},
									   [this, i](auto value){return setColumnData(value, static_cast<int>(i));});

			addProperty<E::HorzAlign>(column_horzAlign_caption[i],
									  monitorColumnsCategory,
									  true,
									  [this, i](){return columnHorzAlign(static_cast<int>(i));},
									  [this, i](auto value){return setColumnHorzAlign(value, static_cast<int>(i));});
		}

		// Allow signals and notify PropertyEditor that it can update property list now
		//
		blockSignals(false);

		emit propertyListChanged();

		return;
	}

	double SchemaItemSignal::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		if (m_multiLine == true)
		{
			int linesCount = qBound(1, m_appSignalIds.size(), 64);
			return linesCount * gridSize * pinGridStep;
		}
		else
		{
			int linesCount = 1;
			return linesCount * gridSize * pinGridStep;
		}
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

	QString SchemaItemSignal::toolTipText(int dpiX, int dpiY) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);

		QString str = {"Signal(s): "};

		for (QString signalId : m_appSignalIds)
		{
			str.append(tr("\n\t%1").arg(signalId));
		}

		str.append(tr("\n\nHint: Press F2 to edit AppSignalID(s)"));

		return str;
	}

	SchemaItemPtr SchemaItemSignal::transformIntoInput()
	{
		return transformIntoType<VFrame30::SchemaItemInput>();
	}

	SchemaItemPtr SchemaItemSignal::transformIntoInOut()
	{
		return transformIntoType<VFrame30::SchemaItemInOut>();
	}

	SchemaItemPtr SchemaItemSignal::transformIntoOutput()
	{
		return transformIntoType<VFrame30::SchemaItemOutput>();
	}

	template <typename TYPE>
	SchemaItemPtr SchemaItemSignal::transformIntoType()
	{
		Proto::Envelope message;
		SaveData(&message);

		std::shared_ptr<TYPE> item = std::make_shared<TYPE>(this->itemUnit());
		item->loadData(message, false);

		// item has restored pins which we need to remove and create new ones depending on TYPE
		//
		item->removeAllInputs();
		item->removeAllOutputs();

		QString className(item->metaObject()->className());
		if (className == "VFrame30::SchemaItemInput")
		{
			item->addOutput();
		}

		if (className == "VFrame30::SchemaItemInOut")
		{
			item->addInput();
			item->addOutput();
		}

		if (className == "VFrame30::SchemaItemOutput")
		{
			item->addInput();
		}

		return item;
	}

	// AppSignalIDs
	//
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

	QStringList SchemaItemSignal::appSignalIdList() const
	{
		QStringList result;
		result.reserve(m_appSignalIds.size());

		for (const QString& s : m_appSignalIds)
		{
			result.push_back(s.trimmed());
		}

		return result;
	}

	void SchemaItemSignal::setAppSignalIds(const QString& s)
	{
		if (s.contains(';') == true)
		{
			QString sLineFeed(s);

			sLineFeed.replace(';', QChar::LineFeed);

			m_appSignalIds = sLineFeed.split(QChar::LineFeed, QString::SkipEmptyParts);
		}
		else
		{
			m_appSignalIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
		}

		for (QString& sref : m_appSignalIds)
		{
			sref = sref.trimmed();
		}

		adjustHeight();
		return;
	}

	QStringList* SchemaItemSignal::mutable_appSignalIds()
	{
		return &m_appSignalIds;
	}

	// ImpactAppSignalIds
	//
	QString SchemaItemSignal::impactAppSignalIds() const
	{
		QString result;

		for (QString s : m_impactAppSignalIds)
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

	QStringList SchemaItemSignal::impactAppSignalIdList() const
	{
		QStringList result;
		result.reserve(m_impactAppSignalIds.size());

		for (const QString& s : m_impactAppSignalIds)
		{
			result.push_back(s.trimmed());
		}

		return result;
	}

	void SchemaItemSignal::setImpactAppSignalIds(const QString& s)
	{
		if (s.contains(';') == true)
		{
			QString sLineFeed(s);
			sLineFeed.replace(';', QChar::LineFeed);

			m_impactAppSignalIds = sLineFeed.split(QChar::LineFeed, QString::SkipEmptyParts);
		}
		else
		{
			m_impactAppSignalIds = s.split(QChar::LineFeed, QString::SkipEmptyParts);
		}

		for (QString& sref : m_impactAppSignalIds)
		{
			sref = sref.trimmed();
		}

		adjustHeight();
		return;
	}

	QStringList* SchemaItemSignal::mutable_impactAppSignalIds()
	{
		return &m_impactAppSignalIds;
	}

	// Multiline
	//
	bool SchemaItemSignal::multiLine() const
	{
		return m_multiLine;
	}

	void SchemaItemSignal::setMultiLine(const bool& value)
	{
		m_multiLine = value;
	}

	bool SchemaItemSignal::multiChannel() const
	{
		return m_appSignalIds.size() > 1;
	}

	int SchemaItemSignal::precision() const
	{
		return m_precision;
	}

	void SchemaItemSignal::setPrecision(const int& value)
	{
		m_precision = qBound(0, value, 12);
	}

	E::AnalogFormat SchemaItemSignal::analogFormat() const
	{
		return m_analogFormat;
	}

	void SchemaItemSignal::setAnalogFormat(const E::AnalogFormat& value)
	{
		m_analogFormat = value;
	}

	int SchemaItemSignal::columnCount() const
	{
		return static_cast<int>(m_columns.size());
	}

	void SchemaItemSignal::setColumnCount(const int& value)
	{
		int c = qBound(1, value, 8);

		if (m_columns.size() != c)
		{
			m_columns.resize(c);
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

	bool SchemaItemSignal::hasImpactColumn() const
	{
		for (const Column& c : m_columns)
		{
			if (c.data == E::ColumnData::ImpactAppSignalID ||
				c.data == E::ColumnData::ImpactCaption ||
				c.data == E::ColumnData::ImpactCustomSignalID ||
				c.data == E::ColumnData::ImpactState)
			{
				return true;
			}
		}

		return false;
	}

	//
	// CSchemaItemInputSignal
	//
	SchemaItemInput::SchemaItemInput(void)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
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
		bool ok = loadData(message, true);
		return ok;
	}

	bool SchemaItemInput::loadData(const Proto::Envelope& message, bool loadOwnData)
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
		if (loadOwnData == false)
		{
			return true;
		}

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
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
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
		bool ok = loadData(message, true);
		return ok;
	}

	bool SchemaItemOutput::loadData(const Proto::Envelope& message, bool loadOwnData)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}


		bool result = SchemaItemSignal::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (loadOwnData == false)
		{
			return true;
		}

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
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
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
		bool ok = loadData(message, true);
		return ok;
	}

	bool SchemaItemInOut::loadData(const Proto::Envelope& message, bool loadOwnData)
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
		if (loadOwnData == false)
		{
			return true;
		}

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

