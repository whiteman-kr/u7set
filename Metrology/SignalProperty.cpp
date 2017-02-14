#include "SignalProperty.h"

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

bool SignalPropertyDialog::m_showGroupHeader[SIGNAL_PROPERTY_GROUP_COUNT] =
{
    true,      //    SIGNAL_PROPERTY_GROUP_ID
    false,     //    SIGNAL_PROPERTY_GROUP_POSITION
    false,     //    SIGNAL_PROPERTY_GROUP_IN_PH_RANGE
    false,     //    SIGNAL_PROPERTY_GROUP_IN_EL_RANGE
    false,     //    SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE
    false,     //    SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE
};

// -------------------------------------------------------------------------------------------------------------------

SignalPropertyDialog::SignalPropertyDialog(const SignalParam& param, QWidget *parent) :
    QDialog(parent)
{
    if (param.isValid() == false)
    {
        assert(false);
        return;
    }

    m_param = param;

    createPropertyList();
}

// -------------------------------------------------------------------------------------------------------------------

SignalPropertyDialog::~SignalPropertyDialog()
{
    if (m_pManager != nullptr)
    {
        delete m_pManager;
        m_pManager = nullptr;
    }

    if (m_pFactory != nullptr)
    {
        delete m_pFactory;
        m_pFactory = nullptr;
    }

    if (m_pEditor != nullptr)
    {
        delete m_pEditor;
        m_pEditor = nullptr;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::createPropertyList()
{
    setWindowFlags(Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowIcon(QIcon(":/icons/Property.png"));
    setWindowTitle(tr("Properties"));
    setMinimumSize(600, 300);
    move(QApplication::desktop()->availableGeometry().center() - rect().center());

    if (m_param.isValid() == false)
    {
        assert(m_param.isValid() == false);
        return;
    }

    setWindowTitle(tr("Properties - %1").arg(m_param.appSignalID()));

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // create property list
    //

    QtVariantProperty *item = nullptr;

    m_pManager = new QtVariantPropertyManager;
    m_pFactory = new QtVariantEditorFactory;
    m_pEditor = new QtTreePropertyBrowser;

    // create property groups
    //

        // id group

        QtProperty *signalIdGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Signal ID"));

            item = m_pManager->addProperty(QVariant::String, tr("AppSignalID"));
            item->setValue( m_param.appSignalID() );
            item->setAttribute(QLatin1String("readOnly"), true);
            signalIdGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("CustomAppSignalID"));
            item->setValue( m_param.customAppSignalID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_CUSTOM_ID);
            signalIdGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("Caption"));
            item->setValue( m_param.caption() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_CAPTION);
            signalIdGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // position group

        QtProperty *positionGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Position"));

            item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
            item->setValue( m_param.position().equipmentID() );
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("Case"));
            item->setValue( m_param.position().caseStr() );
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Subblock"));
            item->setValue(m_param.position().subblock() + 1);
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Block"));
            item->setValue(m_param.position().block() + 1);
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Entry"));
            item->setValue(m_param.position().entry() + 1);
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // input physical range group

        QtProperty *inputPhysicalRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Input physical ranges: ") + m_param.inputPhysicalRangeStr());

            item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
            item->setValue( m_param.inputPhysicalLowLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.inputPhysicalPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_LOW);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
            item->setValue( m_param.inputPhysicalHighLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.inputPhysicalPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_HIGH);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
            QStringList inputPhysicalUnitList;
            int inputPhysicalUnitCount = theUnitBase.unitCount();
            for(int u = 0; u < inputPhysicalUnitCount; u++)
            {
                inputPhysicalUnitList.append( theUnitBase.unit( u ) );
            }
            item->setAttribute(QLatin1String("enumNames"), inputPhysicalUnitList);
            item->setValue( m_param.inputPhysicalUnitID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_UNIT);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( m_param.inputPhysicalPrecision() );
            item->setAttribute(QLatin1String("minimum"), 0);
            item->setAttribute(QLatin1String("maximum"), 10);
            item->setAttribute(QLatin1String("singleStep"), 1);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_PRECISION);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("ADC Limits"));
            item->setValue( m_param.adcRangeStr(false) + " (" + m_param.adcRangeStr(true) + ")" );
            item->setAttribute(QLatin1String("readOnly"), true);
            inputPhysicalRangeGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // input electric range group

        QtProperty *inputElectricRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Input electric ranges: ") + m_param.inputElectricRangeStr());

            item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
            item->setValue( m_param.inputElectricLowLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.inputElectricPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_LOW);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
            item->setValue( m_param.inputElectricHighLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.inputElectricPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_HIGH);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
            QStringList inputElectricUnitList;
            QMetaEnum ieume = QMetaEnum::fromType<E::InputUnit>();
            int inputElectricUnitCount = ieume.keyCount();
            int currentInputElectricUnit = NO_UNIT_ID;
            for(int u = 0; u < inputElectricUnitCount; u++)
            {
                inputElectricUnitList.append( theUnitBase.unit( ieume.value(u) ) );

                if (m_param.inputElectricUnitID() == ieume.value(u))
                {
                    currentInputElectricUnit = u;
                }

            }
            item->setAttribute(QLatin1String("enumNames"), inputElectricUnitList);
            item->setValue( currentInputElectricUnit );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_UNIT);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Sensor type"));
            QStringList sensorList;
            for(int s = 0; s < SENSOR_TYPE_COUNT; s++)
            {
                sensorList.append( SensorTypeStr[ s ] );
            }
            item->setAttribute(QLatin1String("enumNames"), sensorList);
            item->setValue( m_param.inputElectricSensorType() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_SENSOR);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( m_param.inputElectricPrecision() );
            item->setAttribute(QLatin1String("minimum"), 0);
            item->setAttribute(QLatin1String("maximum"), 10);
            item->setAttribute(QLatin1String("singleStep"), 1);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_PRECISION);
            inputElectricRangeGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // output physical ranges group

        QtProperty *outputPhysicalRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Output physical range: ") + m_param.outputPhysicalRangeStr());

            item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
            item->setValue( m_param.outputPhysicalLowLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.outputPhysicalPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_LOW);
            outputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
            item->setValue( m_param.outputPhysicalHighLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.outputPhysicalPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_HIGH);
            outputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
            QStringList outputPhysicalUnitList;
            int outputPhysicalUnitCount = theUnitBase.unitCount();
            for(int u = 0; u < outputPhysicalUnitCount; u++)
            {
                outputPhysicalUnitList.append( theUnitBase.unit( u ) );
            }
            item->setAttribute(QLatin1String("enumNames"), outputPhysicalUnitList);
            item->setValue( m_param.outputPhysicalUnitID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_UNIT);
            outputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( m_param.outputPhysicalPrecision() );
            item->setAttribute(QLatin1String("minimum"), 0);
            item->setAttribute(QLatin1String("maximum"), 10);
            item->setAttribute(QLatin1String("singleStep"), 1);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_PRECISION);
            outputPhysicalRangeGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // output electric ranges group

        QtProperty *outputElectricRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Output electric range: ") + m_param.outputElectricRangeStr());

            item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
            item->setValue( m_param.outputElectricLowLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.outputElectricPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_LOW);
            outputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
            item->setValue( m_param.outputElectricHighLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_param.outputElectricPrecision());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_HIGH);
            outputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
            QStringList outputElectricUnitList;
            QMetaEnum oeume = QMetaEnum::fromType<E::InputUnit>();
            int outputElectricUnitCount = oeume.keyCount();
            int currentOutputElectricUnit = NO_UNIT_ID;
            for(int u = 0; u < outputElectricUnitCount; u++)
            {
                outputElectricUnitList.append( theUnitBase.unit( oeume.value(u) ) );

                if (m_param.outputElectricUnitID() == oeume.value(u) )
                {
                    currentOutputElectricUnit = u;
                }
            }
            item->setAttribute(QLatin1String("enumNames"), outputElectricUnitList);
            item->setValue( currentOutputElectricUnit );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_UNIT);
            outputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Sensor type"));
            QStringList outputSensorList;
            for(int s = 0; s < SENSOR_TYPE_COUNT; s++)
            {
                outputSensorList.append( SensorTypeStr[ s ] );
            }
            item->setAttribute(QLatin1String("enumNames"), outputSensorList);
            item->setValue( m_param.outputElectricSensorType() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_SENSOR);
            outputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( m_param.outputElectricPrecision() );
            item->setAttribute(QLatin1String("minimum"), 0);
            item->setAttribute(QLatin1String("maximum"), 10);
            item->setAttribute(QLatin1String("singleStep"), 1);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_PRECISION);
            outputElectricRangeGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

    // show or hide property groups
    //

    m_browserItemList[SIGNAL_PROPERTY_GROUP_ID] = m_pEditor->addProperty(signalIdGroup);
    m_browserItemList[SIGNAL_PROPERTY_GROUP_POSITION] = m_pEditor->addProperty(positionGroup);

    if (m_param.isAnalog() == true)
    {
        m_browserItemList[SIGNAL_PROPERTY_GROUP_IN_PH_RANGE] = m_pEditor->addProperty(inputPhysicalRangeGroup);
        m_browserItemList[SIGNAL_PROPERTY_GROUP_IN_EL_RANGE] = m_pEditor->addProperty(inputElectricRangeGroup);
    }

    if (m_param.isOutput() == true)
    {
        m_browserItemList[SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE] = m_pEditor->addProperty(outputPhysicalRangeGroup);
        m_browserItemList[SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE] = m_pEditor->addProperty(outputElectricRangeGroup);
    }

    for(int g = 0; g < SIGNAL_PROPERTY_GROUP_COUNT; g++)
    {
        if (m_browserItemList[g] == nullptr)
        {
            continue;
        }

        m_pEditor->setExpanded(m_browserItemList[g], m_showGroupHeader[g]);
    }

    //
    //
    m_pEditor->setPropertiesWithoutValueMarked(true);
    m_pEditor->setRootIsDecorated(false);

    connect(m_pManager, &QtVariantPropertyManager::valueChanged, this, &SignalPropertyDialog::onPropertyValueChanged);
    connect(m_pEditor, &QtTreePropertyBrowser::expanded, this, &SignalPropertyDialog::onPropertyExpanded);

    // create buttons ok and cancel
    //
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SignalPropertyDialog::onOk);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SignalPropertyDialog::reject);

    // add layouts
    //
    mainLayout->addWidget(m_pEditor);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::onPropertyValueChanged(QtProperty *property, const QVariant &value)
{
    if (property == nullptr)
    {
        return;
    }

    if (m_propertyMap.contains(property) == false)
    {
        return;
    }

    int index = m_propertyMap[property];
    if (index < 0 || index >= SIGNAL_PROPERTY_ITEM_COUNT)
    {
        return;
    }

    QMetaEnum me = QMetaEnum::fromType<E::InputUnit>();

    int groupIndex = -1;

    switch(index)
    {
        case SIGNAL_PROPERTY_ITEM_CUSTOM_ID:                m_param.setCustomAppSignalID(value.toString());                                         groupIndex = SIGNAL_PROPERTY_GROUP_ID;              break;
        case SIGNAL_PROPERTY_ITEM_CAPTION:                  m_param.setCaption(value.toString());                                                   groupIndex = SIGNAL_PROPERTY_GROUP_ID;              break;

        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_LOW:          m_param.setInputPhysicalLowLimit(value.toDouble());                                     groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_HIGH:         m_param.setInputPhysicalHighLimit(value.toDouble());                                    groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_UNIT:         m_param.setInputPhysicalUnitID(value.toInt());                                          groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_PRECISION:    m_param.setInputPhysicalPrecision( value.toInt());                                      groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;

        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_LOW:          m_param.setInputElectricLowLimit(value.toDouble());                                     groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_HIGH:         m_param.setInputElectricHighLimit(value.toDouble());                                    groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_UNIT:         m_param.setInputElectricUnitID(static_cast<E::InputUnit>(me.value(value.toInt())));     groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_SENSOR:       m_param.setInputElectricSensorType(static_cast<E::SensorType>(value.toInt()));          groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_PRECISION:    m_param.setInputElectricPrecision( value.toInt());                                      groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;

        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_LOW:         m_param.setOutputPhysicalLowLimit(value.toDouble());                                    groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_HIGH:        m_param.setOutputPhysicalHighLimit(value.toDouble());                                   groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_UNIT:        m_param.setOutputPhysicalUnitID(value.toInt());                                         groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_PRECISION:   m_param.setOutputPhysicalPrecision( value.toInt());                                     groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;

        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_LOW:         m_param.setOutputElectricLowLimit(value.toDouble());                                    groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_HIGH:        m_param.setOutputElectricHighLimit(value.toDouble());                                   groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_UNIT:        m_param.setOutputElectricUnitID(static_cast<E::InputUnit>(me.value(value.toInt())));    groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_SENSOR:      m_param.setOutputElectricSensorType(static_cast<E::SensorType>(value.toInt()));         groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_PRECISION:   m_param.setOutputElectricPrecision( value.toInt());                                     groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
    }

    if (groupIndex < 0 || groupIndex >= SIGNAL_PROPERTY_GROUP_COUNT)
    {
        return;
    }

    updateGroupHeader(groupIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::updateGroupHeader(int index)
{
    if (index < 0 || index >= SIGNAL_PROPERTY_GROUP_COUNT)
    {
        return;
    }

    QtBrowserItem* browserItem = m_browserItemList[index];
    if (browserItem == nullptr)
    {
        return;
    }

    QString header;

    switch(index)
    {
        case SIGNAL_PROPERTY_GROUP_IN_PH_RANGE:     header = SignalPropertyGroup[index] + m_param.inputPhysicalRangeStr(); break;
        case SIGNAL_PROPERTY_GROUP_IN_EL_RANGE:     header = SignalPropertyGroup[index] + m_param.inputElectricRangeStr(); break;
        case SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE:    header = SignalPropertyGroup[index] + m_param.outputPhysicalRangeStr(); break;
        case SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE:    header = SignalPropertyGroup[index] + m_param.outputElectricRangeStr(); break;
    }

    browserItem->property()->setPropertyName(header);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::onPropertyExpanded(QtBrowserItem *item)
{
    if (item == nullptr)
    {
        return;
    }

    if (m_pEditor == nullptr)
    {
        return;
    }

    for(int g = 0; g < SIGNAL_PROPERTY_GROUP_COUNT; g++)
    {
        if (m_browserItemList[g] == item)
        {
            m_showGroupHeader[g] = m_pEditor->isExpanded(item);
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::onOk()
{
    if (m_param.isValid() == false)
    {
        assert(false);
        return;
    }

    theSignalBase.setSignalParam(m_param.hash(), m_param);

    accept();
}

// -------------------------------------------------------------------------------------------------------------------

