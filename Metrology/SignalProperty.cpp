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

SignalPropertyDialog::SignalPropertyDialog(const Hash& signalHash, QWidget *parent) :
    QDialog(parent)
{
    if (signalHash == 0)
    {
        reject();
        return;
    }

    m_signalHash = signalHash;

    m_signal = theSignalBase.signal(m_signalHash);
    if (m_signal.param().appSignalID().isEmpty() == true || m_signal.param().hash() == 0)
    {
        assert(false);
        return;
    }

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

    if (m_signal.param().appSignalID().isEmpty() == true || m_signal.param().hash() == 0)
    {
        assert(false);
        return;
    }

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
            item->setValue( m_signal.param().appSignalID() );
            item->setAttribute(QLatin1String("readOnly"), true);
            signalIdGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("CustomAppSignalID"));
            item->setValue( m_signal.param().customAppSignalID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_CUSTOM_ID);
            signalIdGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("Caption"));
            item->setValue( m_signal.param().caption() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_CAPTION);
            signalIdGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // position group

        QtProperty *positionGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Position"));

            item = m_pManager->addProperty(QVariant::String, tr("EquipmentID"));
            item->setValue( m_signal.param().equipmentID() );
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("Case"));
            item->setValue( m_signal.position().caseString() );
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Subblock"));
            item->setValue(m_signal.position().subblock() + 1);
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Block"));
            item->setValue(m_signal.position().block() + 1);
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Entry"));
            item->setValue(m_signal.position().entry() + 1);
            item->setAttribute(QLatin1String("readOnly"), true);
            positionGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // input physical range group

        QtProperty *inputPhysicalRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Input physical ranges: ") + m_signal.inputPhysicalRange());

            item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
            item->setValue( m_signal.param().lowEngeneeringUnits() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_signal.param().decimalPlaces());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_LOW);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
            item->setValue( m_signal.param().highEngeneeringUnits() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_signal.param().decimalPlaces());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_HIGH);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
            QStringList unitList;
            int unitCount = theSignalBase.unitCount();
            for(int u = 0; u < unitCount; u++)
            {
                unitList.append( theSignalBase.unit( u ) );
            }
            item->setAttribute(QLatin1String("enumNames"), unitList);
            item->setValue( m_signal.param().unitID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_UNIT);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( m_signal.param().decimalPlaces() );
            item->setAttribute(QLatin1String("minimum"), 0);
            item->setAttribute(QLatin1String("maximum"), 10);
            item->setAttribute(QLatin1String("singleStep"), 1);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_PRECISION);
            inputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::String, tr("ADC Limits"));
            item->setValue( m_signal.adcRange(false) + " (" + m_signal.adcRange(true) + ")" );
            item->setAttribute(QLatin1String("readOnly"), true);
            inputPhysicalRangeGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // input electric range group

        QtProperty *inputElectricRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Input electric ranges: ") + m_signal.inputElectricRange());

            item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
            item->setValue( m_signal.param().inputLowLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), 3);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_LOW);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
            item->setValue( m_signal.param().inputHighLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), 3);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_HIGH);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
            QStringList inputUnitList;
            int inputUnitCount = theSignalBase.unitCount();
            for(int u = 0; u < inputUnitCount; u++)
            {
                inputUnitList.append( theSignalBase.unit( u ) );
            }
            item->setAttribute(QLatin1String("enumNames"), inputUnitList);
            item->setValue( m_signal.param().inputUnitID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_UNIT);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Sensor type"));
            QStringList sensorList;
            for(int s = 0; s < SENSOR_TYPE_COUNT; s++)
            {
                sensorList.append( SensorTypeStr[ s ] );
            }
            item->setAttribute(QLatin1String("enumNames"), sensorList);
            item->setValue( m_signal.param().inputSensorID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_SENSOR);
            inputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( 3 );
            item->setAttribute(QLatin1String("minimum"), 0);
            item->setAttribute(QLatin1String("maximum"), 10);
            item->setAttribute(QLatin1String("singleStep"), 1);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_PRECISION);
            inputElectricRangeGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // output physical ranges group

        QtProperty *outputPhysicalRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Output physical range: ") + m_signal.outputPhysicalRange());

            item = m_pManager->addProperty(QVariant::Double, tr("Low limit"));
            item->setValue( m_signal.param().outputLowLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_signal.param().decimalPlaces());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_LOW);
            outputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Double, tr("High limit"));
            item->setValue( m_signal.param().outputHighLimit() );
            item->setAttribute(QLatin1String("singleStep"), 0.001);
            item->setAttribute(QLatin1String("decimals"), m_signal.param().decimalPlaces());
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_HIGH);
            outputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Unit"));
            QStringList outputUnitList;
            int outputUnitCount = theSignalBase.unitCount();
            for(int u = 0; u < outputUnitCount; u++)
            {
                outputUnitList.append( theSignalBase.unit( u ) );
            }
            item->setAttribute(QLatin1String("enumNames"), outputUnitList);
            item->setValue( m_signal.param().outputUnitID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_UNIT);
            outputPhysicalRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( 2 );
            item->setAttribute(QLatin1String("minimum"), 0);
            item->setAttribute(QLatin1String("maximum"), 10);
            item->setAttribute(QLatin1String("singleStep"), 1);
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_PRECISION);
            outputPhysicalRangeGroup->addSubProperty(item);

        m_pEditor->setFactoryForManager(m_pManager, m_pFactory);

        // output electric ranges group

        QtProperty *outputElectricRangeGroup = m_pManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Output electric range: ") + m_signal.outputElectricRange());

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Limits"));
            QStringList outputElectricRangeList;
            for(int olr = 0; olr < OUTPUT_MODE_COUNT; olr++)
            {
                outputElectricRangeList.append( OutputModeStr[ olr ] );
            }
            item->setAttribute(QLatin1String("enumNames"), outputElectricRangeList);
            item->setValue( m_signal.param().outputModeInt() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_MODE);
            outputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QtVariantPropertyManager::enumTypeId(), tr("Sensor type"));
            QStringList outputSensorList;
            for(int s = 0; s < SENSOR_TYPE_COUNT; s++)
            {
                outputSensorList.append( SensorTypeStr[ s ] );
            }
            item->setAttribute(QLatin1String("enumNames"), outputSensorList);
            item->setValue( m_signal.param().outputSensorID() );
            m_propertyMap.insert( item, SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_SENSOR);
            outputElectricRangeGroup->addSubProperty(item);

            item = m_pManager->addProperty(QVariant::Int, tr("Precision"));
            item->setValue( 3 );
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

    if (m_signal.param().isAnalog() == true)
    {
        m_browserItemList[SIGNAL_PROPERTY_GROUP_IN_PH_RANGE] = m_pEditor->addProperty(inputPhysicalRangeGroup);
        m_browserItemList[SIGNAL_PROPERTY_GROUP_IN_EL_RANGE] = m_pEditor->addProperty(inputElectricRangeGroup);
    }

    if (m_signal.param().isOutput() == true)
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

    Signal param = m_signal.param();

    int groupIndex = -1;


    switch(index)
    {
        case SIGNAL_PROPERTY_ITEM_CUSTOM_ID:                param.setCustomAppSignalID(value.toString());       groupIndex = SIGNAL_PROPERTY_GROUP_ID;              break;
        case SIGNAL_PROPERTY_ITEM_CAPTION:                  param.setCaption(value.toString());                 groupIndex = SIGNAL_PROPERTY_GROUP_ID;              break;

        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_LOW:          param.setLowEngeneeringUnits(value.toDouble());     groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_HIGH:         param.setHighEngeneeringUnits(value.toDouble());    groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_UNIT:         param.setUnitID(value.toInt());                     groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_PH_RANGE_PRECISION:    param.setDecimalPlaces( value.toInt());             groupIndex = SIGNAL_PROPERTY_GROUP_IN_PH_RANGE;     break;

        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_LOW:          param.setInputLowLimit(value.toDouble());           groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_HIGH:         param.setInputHighLimit(value.toDouble());          groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_UNIT:         param.setInputUnitID(value.toInt());                groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_SENSOR:       param.setInputSensorID(value.toInt());              groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;
        case SIGNAL_PROPERTY_ITEM_IN_EL_RANGE_PRECISION:                                                        groupIndex = SIGNAL_PROPERTY_GROUP_IN_EL_RANGE;     break;

        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_LOW:         param.setOutputLowLimit(value.toDouble());          groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_HIGH:        param.setOutputHighLimit(value.toDouble());         groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_UNIT:        param.setOutputUnitID(value.toInt());               groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_PH_RANGE_PRECISION:                                                       groupIndex = SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE;    break;

        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_MODE:                                                            groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_SENSOR:      param.setOutputSensorID(value.toInt());             groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
        case SIGNAL_PROPERTY_ITEM_OUT_EL_RANGE_PRECISION:                                                       groupIndex = SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE;    break;
    }

    m_signal.setParam(param);

    if (groupIndex < 0 || groupIndex >= SIGNAL_PROPERTY_GROUP_COUNT)
    {
        return;
    }

    updateGroupHeader(groupIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPropertyDialog::updateGroupHeader(const int& index)
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
        case SIGNAL_PROPERTY_GROUP_IN_PH_RANGE:     header = SignalPropertyGroup[index] + m_signal.inputPhysicalRange(); break;
        case SIGNAL_PROPERTY_GROUP_IN_EL_RANGE:     header = SignalPropertyGroup[index] + m_signal.inputElectricRange(); break;
        case SIGNAL_PROPERTY_GROUP_OUT_PH_RANGE:    header = SignalPropertyGroup[index] + m_signal.outputPhysicalRange(); break;
        case SIGNAL_PROPERTY_GROUP_OUT_EL_RANGE:    header = SignalPropertyGroup[index] + m_signal.outputElectricRange(); break;
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
    theSignalBase.setSignalParam(m_signalHash, m_signal.param());

    accept();
}

// -------------------------------------------------------------------------------------------------------------------

