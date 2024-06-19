#include "include/TuningLib/TuningUiConstants.h"
#include "include/TuningLib/TuningUiItem.h"

namespace TuningLib
{
	struct TuningUiTags
	{
		static inline const QString tag_True = "true";
		static inline const QString tag_False = "false";

		static inline const QString tag_Generic = "Generic";

		static inline const QString tag_Tab = "Tab";
		static inline const QString tag_Button = "Button";
		static inline const QString tag_Counter = "Counter";
		static inline const QString tag_SchemasTab = "SchemasTab";

		static inline const QString tag_All = "All";
		static inline const QString tag_Analog = "Analog";
		static inline const QString tag_Discrete = "Discrete";

		static inline const QString tag_FiltersSwitch = "FiltersSwitch";

		static inline const QString tag_StatusBar = "StatusBar";
		static inline const QString tag_FilterTree = "FilterTree";

		static inline const char* prop_ValueColumn1AppSignalSuffixes = "Column%1AppSignalSuffixes";

		// Property names

		static inline const QLatin1String prop_Uuid = QLatin1String("Uuid");
		static inline const QLatin1String prop_Caption = QLatin1String("Caption");
		static inline const QLatin1String prop_ID = QLatin1String("ID");
		static inline const QLatin1String prop_Tags = QLatin1String("Tags");
		static inline const QLatin1String prop_Filters = QLatin1String("Filters");
		static inline const QLatin1String prop_InterfaceType = QLatin1String("InterfaceType");

		static inline const QLatin1String prop_UseColors = QLatin1String("UseColors");

		static inline const QLatin1String prop_BackColor = QLatin1String("BackColor");
		static inline const QLatin1String prop_TextColor = QLatin1String("TextColor");
		static inline const QLatin1String prop_BackSelectedColor = QLatin1String("BackSelectedColor");
		static inline const QLatin1String prop_TextSelectedColor = QLatin1String("TextSelectedColor");
		static inline const QLatin1String prop_BackAlertedColor = QLatin1String("BackAlertedColor");
		static inline const QLatin1String prop_TextAlertedColor = QLatin1String("TextAlertedColor");
		static inline const QLatin1String prop_HasDiscreteCounter = QLatin1String("HasDiscreteCounter");
		static inline const QLatin1String prop_ValueColumnsCount = QLatin1String("ValueColumnsCount");
		static inline const QLatin1String prop_TabType = QLatin1String("TabType");
		static inline const QLatin1String prop_CounterType = QLatin1String("CounterType");
		static inline const QLatin1String prop_StartSchemaId = QLatin1String("StartSchemaId");

		static inline const QLatin1String prop_ColumnCustomAppId = QLatin1String("ColumnCustomAppId");
		static inline const QLatin1String prop_ColumnAppId = QLatin1String("ColumnAppId");
		static inline const QLatin1String prop_ColumnEquipmentId = QLatin1String("ColumnEquipmentId");
		static inline const QLatin1String prop_ColumnCaption = QLatin1String("ColumnCaption");
		static inline const QLatin1String prop_ColumnUnits = QLatin1String("ColumnUnits");
		static inline const QLatin1String prop_ColumnType = QLatin1String("ColumnType");
		static inline const QLatin1String prop_ColumnLimits = QLatin1String("ColumnLimits");
		static inline const QLatin1String prop_ColumnDefault = QLatin1String("ColumnDefault");
		static inline const QLatin1String prop_ColumnValid = QLatin1String("ColumnValid");
		static inline const QLatin1String prop_ColumnOutOfRange = QLatin1String("ColumnOutOfRange");

		static inline const QLatin1String category_Columns = QLatin1String("Columns");
		static inline const QLatin1String category_ValueColumns = QLatin1String("ValueColumns");
	};

	//
	// TuningUiItem
	//

	TuningUiItem::TuningUiItem()
	{
		ADD_PROPERTY_GETTER(QString, TuningUiTags::prop_Uuid, true, TuningUiItem::uuidString);

		ADD_PROPERTY_GETTER_SETTER(QString, TuningUiTags::prop_Caption, true, TuningUiItem::caption, TuningUiItem::setCaption);

		//ADD_PROPERTY_GETTER_SETTER(QString, TuningUiTags::prop_ID, true, TuningUiItem::ID, TuningUiItem::setID);

		ADD_PROPERTY_GETTER(InterfaceType, TuningUiTags::prop_InterfaceType, true, TuningUiItem::interfaceType);

		ADD_PROPERTY_GETTER_SETTER(QString, TuningUiTags::prop_Filters, true, TuningUiItem::filters, TuningUiItem::setFilters);

		ADD_PROPERTY_GETTER_SETTER(QString, TuningUiTags::prop_Tags, true, TuningUiItem::tags, TuningUiItem::setTags);

		auto propUseColors =
			ADD_PROPERTY_GETTER_SETTER(bool, TuningUiTags::prop_UseColors, true, TuningUiItem::useColors, TuningUiItem::setUseColors);
		propUseColors->setCategory("Appearance");

		auto propBackColor =
			ADD_PROPERTY_GETTER_SETTER(QColor, TuningUiTags::prop_BackColor, true, TuningUiItem::backColor, TuningUiItem::setBackColor);
		propBackColor->setCategory("Appearance");

		auto propTextColor =
			ADD_PROPERTY_GETTER_SETTER(QColor, TuningUiTags::prop_TextColor, true, TuningUiItem::textColor, TuningUiItem::setTextColor);
		propTextColor->setCategory("Appearance");

		auto propBackSelectedColor = ADD_PROPERTY_GETTER_SETTER(QColor,
																TuningUiTags::prop_BackSelectedColor,
																true,
																TuningUiItem::backSelectedColor,
																TuningUiItem::setBackSelectedColor);
		propBackSelectedColor->setCategory("Appearance");

		auto propTextSelectedColor = ADD_PROPERTY_GETTER_SETTER(QColor,
																TuningUiTags::prop_TextSelectedColor,
																true,
																TuningUiItem::textSelectedColor,
																TuningUiItem::setTextSelectedColor);
		propTextSelectedColor->setCategory("Appearance");

		auto propBackAlertedColor = ADD_PROPERTY_GETTER_SETTER(QColor,
															   TuningUiTags::prop_BackAlertedColor,
															   true,
															   TuningUiItem::backAlertedColor,
															   TuningUiItem::setBackAlertedColor);
		propBackAlertedColor->setCategory("Appearance");

		auto propTextAlertedColor = ADD_PROPERTY_GETTER_SETTER(QColor,
															   TuningUiTags::prop_TextAlertedColor,
															   true,
															   TuningUiItem::textAlertedColor,
															   TuningUiItem::setTextAlertedColor);
		propTextAlertedColor->setCategory("Appearance");

		auto propHasCounter = ADD_PROPERTY_GETTER_SETTER(bool,
														 TuningUiTags::prop_HasDiscreteCounter,
														 true,
														 TuningUiItem::hasDiscreteCounter,
														 TuningUiItem::setHasDiscreteCounter);
		propHasCounter->setCategory("Functions");

		auto propTabValuesCount = ADD_PROPERTY_GETTER_SETTER(int,
															 TuningUiTags::prop_ValueColumnsCount,
															 true,
															 TuningUiItem::valuesColumnCount,
															 TuningUiItem::setValuesColumnCount);
		propTabValuesCount->setCategory(TuningUiTags::category_ValueColumns);

		auto propTabType =
			ADD_PROPERTY_GETTER_SETTER(TabType, TuningUiTags::prop_TabType, true, TuningUiItem::tabType, TuningUiItem::setTabType);
		propTabType->setCategory("Appearance");

		auto propCounterType = ADD_PROPERTY_GETTER_SETTER(CounterType,
														  TuningUiTags::prop_CounterType,
														  true,
														  TuningUiItem::counterType,
														  TuningUiItem::setCounterType);
		propCounterType->setCategory("Appearance");

		auto propSchemaId = ADD_PROPERTY_GETTER_SETTER(QString,
													   TuningUiTags::prop_StartSchemaId,
													   true,
													   TuningUiItem::startSchemaId,
													   TuningUiItem::setStartSchemaId);
		propSchemaId->setCategory("Schemas");

		// Columns

		int order = 100;

		auto propColumn = ADD_PROPERTY_GETTER_SETTER(bool,
													 TuningUiTags::prop_ColumnCustomAppId,
													 true,
													 TuningUiItem::columnCustomAppId,
													 TuningUiItem::setColumnCustomAppId);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn =
			ADD_PROPERTY_GETTER_SETTER(bool, TuningUiTags::prop_ColumnAppId, true, TuningUiItem::columnAppId, TuningUiItem::setColumnAppId);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn = ADD_PROPERTY_GETTER_SETTER(bool,
												TuningUiTags::prop_ColumnEquipmentId,
												true,
												TuningUiItem::columnEquipmentId,
												TuningUiItem::setColumnEquipmentId);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn = ADD_PROPERTY_GETTER_SETTER(bool,
												TuningUiTags::prop_ColumnCaption,
												true,
												TuningUiItem::columnCaption,
												TuningUiItem::setColumnCaption);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn =
			ADD_PROPERTY_GETTER_SETTER(bool, TuningUiTags::prop_ColumnUnits, true, TuningUiItem::columnUnits, TuningUiItem::setColumnUnits);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn =
			ADD_PROPERTY_GETTER_SETTER(bool, TuningUiTags::prop_ColumnType, true, TuningUiItem::columnType, TuningUiItem::setColumnType);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn = ADD_PROPERTY_GETTER_SETTER(bool,
												TuningUiTags::prop_ColumnLimits,
												true,
												TuningUiItem::columnLimits,
												TuningUiItem::setColumnLimits);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn = ADD_PROPERTY_GETTER_SETTER(bool,
												TuningUiTags::prop_ColumnDefault,
												true,
												TuningUiItem::columnDefault,
												TuningUiItem::setColumnDefault);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn =
			ADD_PROPERTY_GETTER_SETTER(bool, TuningUiTags::prop_ColumnValid, true, TuningUiItem::columnValid, TuningUiItem::setColumnValid);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);

		propColumn = ADD_PROPERTY_GETTER_SETTER(bool,
												TuningUiTags::prop_ColumnOutOfRange,
												true,
												TuningUiItem::columnOutOfRange,
												TuningUiItem::setColumnOutOfRange);
		propColumn->setCategory(TuningUiTags::category_Columns);
		propColumn->setViewOrder(order++);
	}

	bool TuningUiItem::load(QXmlStreamReader& reader)
	{
		if (isRoot() == false)
		{
			if (reader.attributes().hasAttribute(TuningUiTags::prop_Uuid))
			{
				setUuid(QUuid::fromString(reader.attributes().value(TuningUiTags::prop_Uuid).toString()));
			}
			else 
			{
				Q_ASSERT(false);
				setUuid(QUuid::createUuid());
			}

			/*
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ID))
			{
				setID(reader.attributes().value(TuningUiTags::prop_ID).toString());
			}*/

			if (reader.attributes().hasAttribute(TuningUiTags::prop_Caption))
			{
				setCaption(reader.attributes().value(TuningUiTags::prop_Caption).toString());
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_UseColors))
			{
				setUseColors(reader.attributes().value(TuningUiTags::prop_UseColors).toString() == TuningUiTags::tag_True);
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_BackColor))
			{
				setBackColor(QColor(reader.attributes().value(TuningUiTags::prop_BackColor).toString()));
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_TextColor))
			{
				setTextColor(QColor(reader.attributes().value(TuningUiTags::prop_TextColor).toString()));
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_BackSelectedColor))
			{
				setBackSelectedColor(QColor(reader.attributes().value(TuningUiTags::prop_BackSelectedColor).toString()));
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_TextSelectedColor))
			{
				setTextSelectedColor(QColor(reader.attributes().value(TuningUiTags::prop_TextSelectedColor).toString()));
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_BackAlertedColor))
			{
				setBackAlertedColor(QColor(reader.attributes().value(TuningUiTags::prop_BackAlertedColor).toString()));
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_TextAlertedColor))
			{
				setTextAlertedColor(QColor(reader.attributes().value(TuningUiTags::prop_TextAlertedColor).toString()));
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_HasDiscreteCounter))
			{
				setHasDiscreteCounter(reader.attributes().value(TuningUiTags::prop_HasDiscreteCounter).toString() ==
									  TuningUiTags::tag_True);
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_Tags))
			{
				setTags(reader.attributes().value(TuningUiTags::prop_Tags).toString());
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_Filters))
			{
				setFilters(reader.attributes().value(TuningUiTags::prop_Filters).toString());
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_StartSchemaId))
			{
				setStartSchemaId(reader.attributes().value(TuningUiTags::prop_StartSchemaId).toString());
			}

			// ValueColumns

			if (reader.attributes().hasAttribute(TuningUiTags::prop_ValueColumnsCount))
			{
				m_valueColumnsCount = reader.attributes().value(TuningUiTags::prop_ValueColumnsCount).toInt();

				if (m_valueColumnsCount < 0)
				{
					m_valueColumnsCount = 0;
				}
				if (m_valueColumnsCount > MaxValuesColumnCount)
				{
					m_valueColumnsCount = MaxValuesColumnCount;
				}

				m_valueColumnsAppSignalIdSuffixes.resize(m_valueColumnsCount);

				for (int i = 0; i < m_valueColumnsCount; i++)
				{
					QString propName = tr(TuningUiTags::prop_ValueColumn1AppSignalSuffixes).arg(i);

					if (reader.attributes().hasAttribute(propName) == true)
					{
						QString masks = reader.attributes().value(propName).toString();

						m_valueColumnsAppSignalIdSuffixes[i] = masks;
					}
				}
			}
			else
			{
				m_valueColumnsCount = 0;
				m_valueColumnsAppSignalIdSuffixes.clear();
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_TabType))
			{
				QString v = reader.attributes().value(TuningUiTags::prop_TabType).toString();
				if (v == TuningUiTags::tag_Generic)
				{
					setTabType(TabType::Generic);
				}
				else
				{
					if (v == TuningUiTags::tag_FiltersSwitch)
					{
						setTabType(TabType::FiltersSwitch);
					}
					else
					{
						reader.raiseError(tr("Unknown TabType value: %1").arg(v));
						return false;
					}
				}
			}

			if (reader.attributes().hasAttribute(TuningUiTags::prop_CounterType))
			{
				QString v = reader.attributes().value(TuningUiTags::prop_CounterType).toString();
				if (v == TuningUiTags::tag_StatusBar)
				{
					setCounterType(CounterType::StatusBar);
				}
				else
				{
					if (v == TuningUiTags::tag_FilterTree)
					{
						setCounterType(CounterType::FilterTree);
					}
					else
					{
						reader.raiseError(tr("Unknown CounterType value: %1").arg(v));
						return false;
					}
				}
			}

			// Columns

			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnCustomAppId))
			{
				setColumnCustomAppId(reader.attributes().value(TuningUiTags::prop_ColumnCustomAppId).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnAppId))
			{
				setColumnAppId(reader.attributes().value(TuningUiTags::prop_ColumnAppId).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnEquipmentId))
			{
				setColumnEquipmentId(reader.attributes().value(TuningUiTags::prop_ColumnEquipmentId).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnCaption))
			{
				setColumnCaption(reader.attributes().value(TuningUiTags::prop_ColumnCaption).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnUnits))
			{
				setColumnUnits(reader.attributes().value(TuningUiTags::prop_ColumnUnits).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnType))
			{
				setColumnType(reader.attributes().value(TuningUiTags::prop_ColumnType).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnLimits))
			{
				setColumnLimits(reader.attributes().value(TuningUiTags::prop_ColumnLimits).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnDefault))
			{
				setColumnDefault(reader.attributes().value(TuningUiTags::prop_ColumnDefault).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnValid))
			{
				setColumnValid(reader.attributes().value(TuningUiTags::prop_ColumnValid).toString() == TuningUiTags::tag_True);
			}
			if (reader.attributes().hasAttribute(TuningUiTags::prop_ColumnOutOfRange))
			{
				setColumnOutOfRange(reader.attributes().value(TuningUiTags::prop_ColumnOutOfRange).toString() == TuningUiTags::tag_True);
			}
		}

		int recurseLevel = 0; // recurseLevel 1 = "Values", recurseLevel 2 = "Value"

		QXmlStreamReader::TokenType t;
		do
		{
			t = reader.readNext();

			if (t == QXmlStreamReader::EndElement && recurseLevel > 0)
			{
				// This is end element of "Value" or "Values", read next element
				//
				recurseLevel--;
				t = reader.readNext();
			}

			if (t == QXmlStreamReader::StartElement)
			{
				QString tagName = reader.name().toString();

				if (tagName == TuningUiTags::tag_Generic || tagName == TuningUiTags::tag_Tab || tagName == TuningUiTags::tag_Button ||
					tagName == TuningUiTags::tag_Counter || tagName == TuningUiTags::tag_SchemasTab)
				{
					InterfaceType filterType = InterfaceType::Root;

					if (tagName == TuningUiTags::tag_Tab)
					{
						filterType = InterfaceType::Tab;
					}

					if (tagName == TuningUiTags::tag_Button)
					{
						filterType = InterfaceType::Button;
					}

					if (tagName == TuningUiTags::tag_Counter)
					{
						filterType = InterfaceType::Counter;
					}

					if (tagName == TuningUiTags::tag_SchemasTab)
					{
						filterType = InterfaceType::SchemasTab;
					}

					std::shared_ptr<TuningUiItem> of = std::make_shared<TuningUiItem>();
					of->setInterfaceType(filterType);

					if (of->load(reader) == false)
					{
						return false;
					}

					addChild(of);

					continue;
				}

				reader.raiseError(tr("Unknown tag: ") + reader.name().toString());
				return false;
			}
		} while (t != QXmlStreamReader::EndElement);

		updateOptionalProperties();

		return true;
	}

	bool TuningUiItem::save(QXmlStreamWriter& writer) const
	{
		if (isRoot() == true)
		{
			writer.writeStartElement("Root");
		}
		else
		{
			if (isTab())
			{
				writer.writeStartElement(TuningUiTags::tag_Tab);
			}
			else
			{
				if (isButton())
				{
					writer.writeStartElement(TuningUiTags::tag_Button);
				}
				else
				{
					if (isCounter())
					{
						writer.writeStartElement(TuningUiTags::tag_Counter);
					}
					else
					{
						if (isSchemasTab())
						{
							writer.writeStartElement(TuningUiTags::tag_SchemasTab);
						}
						else
						{
							Q_ASSERT(false);
							return false;
						}
					}
				}
			}
		}

		writer.writeAttribute(TuningUiTags::prop_Uuid, uuidString());
		writer.writeAttribute(TuningUiTags::prop_Caption, caption());

		writer.writeAttribute(TuningUiTags::prop_UseColors, useColors() ? TuningUiTags::tag_True : TuningUiTags::tag_False);

		writer.writeAttribute(TuningUiTags::prop_BackColor, backColor().name());
		writer.writeAttribute(TuningUiTags::prop_TextColor, textColor().name());

		writer.writeAttribute(TuningUiTags::prop_BackSelectedColor, backSelectedColor().name());
		writer.writeAttribute(TuningUiTags::prop_TextSelectedColor, textSelectedColor().name());

		writer.writeAttribute(TuningUiTags::prop_BackAlertedColor, backAlertedColor().name());
		writer.writeAttribute(TuningUiTags::prop_TextAlertedColor, textAlertedColor().name());

		writer.writeAttribute(TuningUiTags::prop_HasDiscreteCounter,
							  hasDiscreteCounter() ? TuningUiTags::tag_True : TuningUiTags::tag_False);

		writer.writeAttribute(TuningUiTags::prop_Tags, tags());
		writer.writeAttribute(TuningUiTags::prop_Filters, filters());

		writer.writeAttribute(TuningUiTags::prop_StartSchemaId, startSchemaId());

		// ValueColumns

		if (static_cast<int>(m_valueColumnsAppSignalIdSuffixes.size()) != valuesColumnCount())
		{
			Q_ASSERT(false);
			return false;
		}

		writer.writeAttribute(TuningUiTags::prop_ValueColumnsCount, QString::number(valuesColumnCount()));

		for (int i = 0; i < valuesColumnCount(); i++)
		{
			QString propName = tr(TuningUiTags::prop_ValueColumn1AppSignalSuffixes).arg(i);
			writer.writeAttribute(propName, m_valueColumnsAppSignalIdSuffixes[i]);
		}

		writer.writeAttribute(TuningUiTags::prop_TabType, E::valueToString<TabType>(static_cast<int>(tabType())));
		writer.writeAttribute(TuningUiTags::prop_CounterType, E::valueToString<CounterType>(static_cast<int>(counterType())));

		// Columns

		writer.writeAttribute(TuningUiTags::prop_ColumnCustomAppId, columnCustomAppId() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnAppId, columnAppId() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnEquipmentId, columnEquipmentId() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnCaption, columnCaption() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnUnits, columnUnits() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnType, columnType() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnLimits, columnLimits() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnDefault, columnDefault() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnValid, columnValid() ? TuningUiTags::tag_True : TuningUiTags::tag_False);
		writer.writeAttribute(TuningUiTags::prop_ColumnOutOfRange, columnOutOfRange() ? TuningUiTags::tag_True : TuningUiTags::tag_False);

		// Call write for all children
		//
		for (const auto& f : m_children)
		{
			f->save(writer);
		}

		writer.writeEndElement();

		return true;
	}

	QUuid TuningUiItem::uuid() const 
	{
		return m_uuid;
	}

	QString TuningUiItem::uuidString() const 
	{
		return m_uuid.toString();
	}

	void TuningUiItem::setUuid(const QUuid& uuid) 
	{
		m_uuid = uuid;
	}

	QString TuningUiItem::caption() const
	{
		return m_caption;
	}

	void TuningUiItem::setCaption(const QString& value)
	{
		m_caption = value;
	}

	bool TuningUiItem::isRoot() const
	{
		return interfaceType() == InterfaceType::Root;
	}

	bool TuningUiItem::isTab() const
	{
		return interfaceType() == InterfaceType::Tab;
	}

	bool TuningUiItem::isButton() const
	{
		return interfaceType() == InterfaceType::Button;
	}

	bool TuningUiItem::isCounter() const
	{
		return interfaceType() == InterfaceType::Counter;
	}

	bool TuningUiItem::isSchemasTab() const
	{
		return interfaceType() == InterfaceType::SchemasTab;
	}

	TuningUiItem::InterfaceType TuningUiItem::interfaceType() const
	{
		return m_interfaceType;
	}

	void TuningUiItem::setInterfaceType(InterfaceType value)
	{
		m_interfaceType = value;
	}

	bool TuningUiItem::useColors() const
	{
		return m_useColors;
	}

	void TuningUiItem::setUseColors(bool value)
	{
		m_useColors = value;
	}

	QColor TuningUiItem::backColor() const
	{
		return m_backColor;
	}

	void TuningUiItem::setBackColor(const QColor& value)
	{
		m_backColor = value;
	}

	QColor TuningUiItem::textColor() const
	{
		return m_textColor;
	}

	void TuningUiItem::setTextColor(const QColor& value)
	{
		m_textColor = value;
	}

	QColor TuningUiItem::backSelectedColor() const
	{
		return m_backSelectedColor;
	}

	void TuningUiItem::setBackSelectedColor(const QColor& value)
	{
		m_backSelectedColor = value;
	}

	QColor TuningUiItem::textSelectedColor() const
	{
		return m_textSelectedColor;
	}

	void TuningUiItem::setTextSelectedColor(const QColor& value)
	{
		m_textSelectedColor = value;
	}

	QColor TuningUiItem::backAlertedColor() const
	{
		return m_backAlertedColor;
	}

	void TuningUiItem::setBackAlertedColor(const QColor& value)
	{
		m_backAlertedColor = value;
	}

	QColor TuningUiItem::textAlertedColor() const
	{
		return m_textAlertedColor;
	}

	void TuningUiItem::setTextAlertedColor(const QColor& value)
	{
		m_textAlertedColor = value;
	}

	bool TuningUiItem::hasDiscreteCounter() const
	{
		return m_hasDiscreteCounter;
	}

	void TuningUiItem::setHasDiscreteCounter(bool value)
	{
		m_hasDiscreteCounter = value;
	}

	TuningUiItem::CounterType TuningUiItem::counterType() const
	{
		return m_counterType;
	}

	void TuningUiItem::setCounterType(CounterType type)
	{
		m_counterType = type;
	}

	QString TuningUiItem::tags() const
	{
		QString result;
		for (const auto& s : m_tags)
		{
			result += s + ';';
		}
		result.remove(result.length() - 1, 1);

		return result;
	}

	void TuningUiItem::setTags(const QString& value)
	{
		if (value.isEmpty() == true)
		{
			m_tags.clear();
		}
		else
		{
			m_tags = value.split(';', Qt::SkipEmptyParts);
		}
	}

	const QStringList& TuningUiItem::tagsList() const
	{
		return m_tags;
	}

	QStringList& TuningUiItem::tagsList()
	{
		return m_tags;
	}

	QString TuningUiItem::filters() const
	{
		QString result;
		for (const auto& s : m_filters)
		{
			result += s + ';';
		}
		result.remove(result.length() - 1, 1);

		return result;
	}

	void TuningUiItem::setFilters(const QString& value)
	{
		if (value.isEmpty() == true)
		{
			m_filters.clear();
		}
		else
		{
			m_filters = value.split(';', Qt::SkipEmptyParts);
		}
	}

	const QStringList& TuningUiItem::filtersList() const
	{
		return m_filters;
	}

	QStringList& TuningUiItem::filtersList()
	{
		return m_filters;
	}

	
	QString TuningUiItem::startSchemaId() const
	{
		return m_startSchemaId;
	}

	void TuningUiItem::setStartSchemaId(const QString& id)
	{
		m_startSchemaId = id;
	}

	TuningUiItem::TabType TuningUiItem::tabType() const
	{
		return m_tabType;
	}

	void TuningUiItem::setTabType(TabType type)
	{
		m_tabType = type;
	}

	int TuningUiItem::valuesColumnCount() const
	{
		return m_valueColumnsCount;
	}

	void TuningUiItem::setValuesColumnCount(int value)
	{
		if (value < 0)
		{
			value = 0;
		}
		if (value > MaxValuesColumnCount)
		{
			value = MaxValuesColumnCount;
		}

		m_valueColumnsCount = value;

		m_valueColumnsAppSignalIdSuffixes.resize(m_valueColumnsCount);
	}

	std::vector<QString> TuningUiItem::valueColumnsAppSignalIdSuffixes() const
	{
		return m_valueColumnsAppSignalIdSuffixes;
	}

	void TuningUiItem::setValueColumnsAppSignalIdSuffixes(const std::vector<QString>& suffixes) 
	{
		m_valueColumnsAppSignalIdSuffixes = suffixes;
	}

	bool TuningUiItem::columnCustomAppId() const
	{
		return m_columnCustomAppId;
	}

	void TuningUiItem::setColumnCustomAppId(bool value)
	{
		m_columnCustomAppId = value;
	}

	bool TuningUiItem::columnAppId() const
	{
		return m_columnAppId;
	}

	void TuningUiItem::setColumnAppId(bool value)
	{
		m_columnAppId = value;
	}

	bool TuningUiItem::columnEquipmentId() const
	{
		return m_columnEquipmentId;
	}

	void TuningUiItem::setColumnEquipmentId(bool value)
	{
		m_columnEquipmentId = value;
	}

	bool TuningUiItem::columnCaption() const
	{
		return m_columnCaption;
	}

	void TuningUiItem::setColumnCaption(bool value)
	{
		m_columnCaption = value;
	}

	bool TuningUiItem::columnUnits() const
	{
		return m_columnUnits;
	}

	void TuningUiItem::setColumnUnits(bool value)
	{
		m_columnUnits = value;
	}

	bool TuningUiItem::columnType() const
	{
		return m_columnType;
	}

	void TuningUiItem::setColumnType(bool value)
	{
		m_columnType = value;
	}

	bool TuningUiItem::columnLimits() const
	{
		return m_columnLimits;
	}

	void TuningUiItem::setColumnLimits(bool value)
	{
		m_columnLimits = value;
	}

	bool TuningUiItem::columnDefault() const
	{
		return m_columnDefault;
	}

	void TuningUiItem::setColumnDefault(bool value)
	{
		m_columnDefault = value;
	}

	bool TuningUiItem::columnValid() const
	{
		return m_columnValid;
	}

	void TuningUiItem::setColumnValid(bool value)
	{
		m_columnValid = value;
	}

	bool TuningUiItem::columnOutOfRange() const
	{
		return m_columnOutOfRange;
	}

	void TuningUiItem::setColumnOutOfRange(bool value)
	{
		m_columnOutOfRange = value;
	}


	TuningUiItem* TuningUiItem::parentItem() const
	{
		return m_parentItem;
	}

	void TuningUiItem::addChild(const std::shared_ptr<TuningUiItem>& child)
	{
		child->m_parentItem = this;
		m_children.push_back(child);
	}

	void TuningUiItem::insertChild(int index, const std::shared_ptr<TuningUiItem>& child)
	{
		if (index < 0 || index > childCount())
		{
			Q_ASSERT(false);
			return;
		}

		child->m_parentItem = this;
		m_children.insert(m_children.begin() + index, child);
	}

	bool TuningUiItem::removeChild(const QUuid& uuid)
	{
		bool found = false;

		for (auto it = m_children.begin(); it != m_children.end(); it++)
		{
			if (it->get()->uuid() == uuid)
			{
				m_children.erase(it);
				found = true;
				break;
			}
		}

		return found;
	}

	bool TuningUiItem::removeChild(int index)
	{
		if (index < 0 || index >= static_cast<int>(m_children.size()))
		{
			Q_ASSERT(false);
			return false;
		}

		m_children.erase(m_children.begin() + index);

		return true;
	}

	void TuningUiItem::removeAllChildren()
	{
		m_children.clear();
	}

	int TuningUiItem::childCount() const
	{
		return static_cast<int>(m_children.size());
	}

	std::shared_ptr<TuningUiItem> TuningUiItem::child(int index) const
	{
		if (index < 0 || index >= m_children.size())
		{
			Q_ASSERT(false);
			return nullptr;
		}

		return m_children[index];
	}

	std::shared_ptr<TuningUiItem> TuningUiItem::find(const QUuid& uuid) const 
	{
		for (auto& child : m_children)
		{
			if (child->uuid() == uuid)
			{
				return child;
			}

			auto result = child->find(uuid);
			if (result != nullptr)
			{
				return result;
			}
		}

		return nullptr;
	}

	void TuningUiItem::updateOptionalProperties()
	{
		// Colors
		setPropertyVisible(TuningUiTags::prop_UseColors,
						   interfaceType() == InterfaceType::Tab || interfaceType() == InterfaceType::Button ||
							   interfaceType() == InterfaceType::Counter);

		setPropertyVisible(TuningUiTags::prop_BackColor, interfaceType() == InterfaceType::Tab || interfaceType() == InterfaceType::Button);
		setPropertyVisible(TuningUiTags::prop_TextColor, interfaceType() == InterfaceType::Tab || interfaceType() == InterfaceType::Button);

		setPropertyVisible(TuningUiTags::prop_BackSelectedColor, interfaceType() == InterfaceType::Button);
		setPropertyVisible(TuningUiTags::prop_TextSelectedColor, interfaceType() == InterfaceType::Button);

		setPropertyVisible(TuningUiTags::prop_BackAlertedColor,
						   interfaceType() == InterfaceType::Button || interfaceType() == InterfaceType::Counter);
		setPropertyVisible(TuningUiTags::prop_TextAlertedColor,
						   interfaceType() == InterfaceType::Tab || interfaceType() == InterfaceType::Counter);

		setPropertyVisible(TuningUiTags::prop_ValueColumnsCount, interfaceType() == InterfaceType::Tab);

		setPropertyVisible(TuningUiTags::prop_ColumnCustomAppId, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnAppId, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnEquipmentId, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnCaption, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnUnits, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnType, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnLimits, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnDefault, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnValid, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_ColumnOutOfRange, interfaceType() == InterfaceType::Tab);

		setPropertyVisible(TuningUiTags::prop_TabType, interfaceType() == InterfaceType::Tab);
		setPropertyVisible(TuningUiTags::prop_CounterType, interfaceType() == InterfaceType::Counter);

		setPropertyVisible(TuningUiTags::prop_HasDiscreteCounter,
						   interfaceType() == InterfaceType::Root || interfaceType() == InterfaceType::Tab ||
							   interfaceType() == InterfaceType::Button);

		setPropertyVisible(TuningUiTags::prop_StartSchemaId, interfaceType() == InterfaceType::SchemasTab);

		if (interfaceType() == InterfaceType::Tab)
		{
			if (static_cast<int>(m_valueColumnsAppSignalIdSuffixes.size()) != valuesColumnCount())
			{
				m_valueColumnsAppSignalIdSuffixes.resize(valuesColumnCount());
			}

			for (int i = 0; i < MaxValuesColumnCount; i++)
			{
				QString propName = tr(TuningUiTags::prop_ValueColumn1AppSignalSuffixes).arg(i);

				if (i < valuesColumnCount())
				{
					if (propertyExists(propName) == false)
					{
						addProperty(propName, TuningUiTags::category_ValueColumns, true, m_valueColumnsAppSignalIdSuffixes[i]);
					}
					else
					{
						m_valueColumnsAppSignalIdSuffixes[i] = propertyValue(propName).toString();
					}
				}
				else
				{
					if (propertyExists(propName) == true)
					{
						removeProperty(propName);
					}
				}
			}
		}

		//
	}

	std::vector<TuningUiItem*> TuningUiItem::childernToVector() const
	{
		std::vector<TuningUiItem*> result;

		for (const auto& child : m_children) 
		{
			result.push_back(child.get());

			auto childItems = child->childernToVector();
			for (const auto& ci : childItems) 
			{
				result.push_back(ci);
			}
		}

		return result;
	}

	void TuningUiItem::setPropertyVisible(const QLatin1String& name, bool visible)
	{
		if (propertyExists(name) == false)
		{
			Q_ASSERT(false);
			return;
		}
		std::shared_ptr<Property> prop = propertyByCaption(name);
		prop->setVisible(visible);
	}

	//
	// TuningUiStorage
	//

	TuningUiStorage::TuningUiStorage():
		m_root(std::make_unique<TuningUiItem>())
	{
		m_root->setCaption(QObject::tr("Root"));
		m_root->setInterfaceType(TuningUiItem::InterfaceType::Root);
	}

	const TuningUiItem* TuningUiStorage::root() const 
	{
		return m_root.get();
	}

	TuningUiItem* TuningUiStorage::root()
	{
		return m_root.get();
	}

	TuningUiItem* TuningUiStorage::get(const QUuid& uuid) 
	{
		auto all = m_root->childernToVector();
		for (auto& item:all) 
		{
			if (item->uuid() == uuid) 
			{
				return item;
			}
		}
		return nullptr;
	}

	bool TuningUiStorage::load(const QByteArray& data, QString* errorCode)
	{
		if (errorCode == nullptr)
		{
			Q_ASSERT(errorCode);
			return false;
		}

		QXmlStreamReader reader(data);

		if (reader.readNextStartElement() == false)
		{
			reader.raiseError(QObject::tr("Failed to load root element."));
			*errorCode = reader.errorString();
			return !reader.hasError();
		}

		if (reader.name() != QLatin1String("TuningUiStorage"))
		{
			reader.raiseError(QObject::tr("The file is not an TuningUiStorage file."));
			*errorCode = reader.errorString();
			return !reader.hasError();
		}

		// Read signals
		//
		while (!reader.atEnd())
		{
			QXmlStreamReader::TokenType t = reader.readNext();

			if (t == QXmlStreamReader::TokenType::Characters)
			{
				continue;
			}

			if (t != QXmlStreamReader::TokenType::StartElement)
			{
				continue;
			}

			QString tagName = reader.name().toString();

			if (tagName == "Root")
			{
				if (m_root->load(reader) == false)
				{
					*errorCode = reader.errorString();
					return false;
				}

				m_root->setCaption(QObject::tr("All Signals"));

				continue;
			}

			reader.raiseError(QObject::tr("Unknown tag: ") + reader.name().toString());
			*errorCode = reader.errorString();
			return !reader.hasError();
		}

		return !reader.hasError();
	}

	bool TuningUiStorage::save(QByteArray& data) const
	{
		QXmlStreamWriter writer(&data);

		writer.setAutoFormatting(true);

		writer.writeStartDocument();

		writer.writeStartElement("TuningUiStorage");

		m_root->save(writer);

		writer.writeEndElement();

		writer.writeEndElement(); // TuningUiStorage

		writer.writeEndDocument();

		return true;
	}

	void TuningUiStorage::add(std::shared_ptr<TuningUiItem> filter, bool moveToTop)
	{
		if (moveToTop == true)
		{
			m_root->insertChild(0, filter);
		}
		else
		{
			m_root->addChild(filter);
		}
	}

	std::vector<std::pair<QString, QString>> TuningUiStorage::checkFilters(const QStringList& appSignalLists) 
	{
		std::vector<std::pair<QString, QString>> result;

		auto f = [appSignalLists, &result](const TuningUiItem* uiItem, auto&& f)
		{
			for (int c = 0; c < uiItem->childCount(); c++)
			{
				TuningUiItem* childUiItem = uiItem->child(c).get();
				if (childUiItem == nullptr)
				{
					Q_ASSERT(false);
					return;
				}

				const QStringList& filters = childUiItem->filtersList();
				for (const QString& filter: filters) 
				{
					if (appSignalLists.contains(filter) == false) 
					{
						result.push_back({filter, childUiItem->caption()});
					}
				}
				f(childUiItem, f);
			}
		};

		f(root(), f);

		return result;
	}

} // namespace TuningUi