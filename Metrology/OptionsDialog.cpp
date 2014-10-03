#include "OptionsDialog.h"

#include <assert.h>
#include <QSettings>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>

#include "OptionsPointsDialog.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PropertyPage::PropertyPage(QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor)
{
    m_type = PROPERTY_PAGE_TYPE_LIST;

    m_pManager = manager;
    m_pFactory = factory;
    m_pEditor = editor;

    m_pWidget = editor;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::PropertyPage(QDialog* dialog)
{
    m_type = PROPERTY_PAGE_TYPE_DIALOG;

    m_pDialog = dialog;

    m_pWidget = dialog;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::~PropertyPage()
{
    switch(m_type)
    {
        case PROPERTY_PAGE_TYPE_LIST:

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

            break;

        case PROPERTY_PAGE_TYPE_DIALOG:

            if (m_pDialog != nullptr)
            {
                delete m_pDialog;
                m_pDialog = nullptr;
            }

            break;

        default:
            assert(0);
            break;

    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

static int activePage = OPTION_PAGE_LINEARETY_MEASURE;

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent)
{
    m_options = theOptions;

    createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::~OptionsDialog()
{
    removePages();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::createInterface()
{
    setMinimumSize(800, 400);
    restoreWindowPosition(this);

    // create interface
    //
    m_pagesLayout = createPages();
    m_buttonsLayout = createButtons();

    QVBoxLayout* mainLayout = new QVBoxLayout;

    mainLayout->addLayout(m_pagesLayout);
    mainLayout->addLayout(m_buttonsLayout);

    setLayout(mainLayout);

    // set active page
    //
    setActivePage(activePage);
}

// -------------------------------------------------------------------------------------------------------------------

QHBoxLayout* OptionsDialog::createPages()
{
    QHBoxLayout *pagesLayout = new QHBoxLayout ;

    QTreeWidget* pPageTree = new QTreeWidget;

    pPageTree->setHeaderHidden(true);
    pPageTree->setFixedWidth(200);

    QList<QTreeWidgetItem*> groupList;

    for(int group = 0; group < OPTION_GROUP_COUNT; group++)
    {
        QTreeWidgetItem* groupTreeItem = new QTreeWidgetItem;
        groupTreeItem->setText(0, OptionGroup[group]);
        pPageTree->addTopLevelItem(groupTreeItem);

        groupList.append(groupTreeItem);
    }

    for(int page = 0; page < OPTION_PAGE_COUNT; page++)
    {
        int groupIndex = OptionGroupPage[page];
        if (groupIndex < 0 || groupIndex >= groupList.count())
        {
            continue;
        }

        QTreeWidgetItem* groupTreeItem = groupList.at(groupIndex);

        QTreeWidgetItem* pageTreeItem = new QTreeWidgetItem;
        pageTreeItem->setText(0, OptionPageShort[page]);
        pageTreeItem->setData(0, Qt::UserRole, page);

        groupTreeItem->addChild(pageTreeItem);

        PropertyPage* pPropertyPage = createPage(page);
        if (pPropertyPage != nullptr)
        {
            pPropertyPage->m_page = page;
            pPropertyPage->m_pTreeWidgetItem = pageTreeItem;
        }

        m_pageList.append(pPropertyPage);
    }

    connect(pPageTree, &QTreeWidget::currentItemChanged , this, &OptionsDialog::onPageChanged );

    pagesLayout->addWidget(pPageTree);

    return pagesLayout;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::removePages()
{
    int count = m_pageList.count();
    for(int i = 0; i < count; i++ )
    {
        PropertyPage* pPropertyPage = m_pageList.at(i);
        if (pPropertyPage == nullptr)
        {
            continue;
        }

        delete pPropertyPage;
    }
}

// -------------------------------------------------------------------------------------------------------------------

QHBoxLayout* OptionsDialog::createButtons()
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout ;

    QPushButton* okButton = new QPushButton(tr("OK"));
    QPushButton* cancelButton = new QPushButton(tr("Cancel"));
    QPushButton* applyButton = new QPushButton(tr("Apply"));

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(applyButton);

    connect(okButton, &QPushButton::clicked, this, &OptionsDialog::onOk);
    connect(cancelButton, &QPushButton::clicked, this, &OptionsDialog::reject);
    connect(applyButton, &QPushButton::clicked, this, &OptionsDialog::onApply);

    return buttonsLayout;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* OptionsDialog::createPage(int page)
{
    PropertyPage* pPropertyPage = nullptr;

    switch (page)
    {
        case OPTION_PAGE_TCP_IP:
        case OPTION_PAGE_LINEARETY_MEASURE:
        case OPTION_PAGE_SETTING_MEASURE:
        case OPTION_PAGE_MEASURE_VIEW_TEXT:
        case OPTION_PAGE_SIGNAL_INFO:
        case OPTION_PAGE_REPORT_HEADER:
        case OPTION_PAGE_DATABASE:
        case OPTION_PAGE_BACKUP:                pPropertyPage = createPropertyList(page);     break;
        case OPTION_PAGE_LINEARETY_POINT:
        case OPTION_PAGE_MEASURE_VIEW_COLUMN:   pPropertyPage = createPropertyDialog(page);   break;
        default:                                assert(nullptr);                              break;
    }

    return pPropertyPage;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* OptionsDialog::createPropertyList(int page)
{
    QtVariantProperty *item = nullptr;

    QtVariantPropertyManager *manager = new QtVariantPropertyManager;
    QtVariantEditorFactory *factory = new QtVariantEditorFactory;
    QtTreePropertyBrowser *editor = new QtTreePropertyBrowser;

    switch (page)
    {
        case OPTION_PAGE_TCP_IP:
            {
                QtProperty *serverGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Server"));

                item = manager->addProperty(QVariant::String, TcpIpParamName[TCPIP_PARAM_SERVER_IP]);
                item->setValue( m_options.getTcpIp().m_serverIP );
                appendProperty(item, page, TCPIP_PARAM_SERVER_IP);
                serverGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::Int, TcpIpParamName[TCPIP_PARAM_SERVER_PORT]);
                item->setValue( m_options.getTcpIp().m_serverPort );
                item->setAttribute(QLatin1String("minimum"), 1);
                item->setAttribute(QLatin1String("maximum"), 65535);
                item->setAttribute(QLatin1String("singleStep"), 1);
                appendProperty(item, page, TCPIP_PARAM_SERVER_PORT);
                serverGroup->addSubProperty(item);

                editor->setFactoryForManager(manager, factory);

                editor->addProperty(serverGroup);
            }
            break;

        case OPTION_PAGE_LINEARETY_MEASURE:
            {
                QtProperty *errorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

                item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_ERROR]);
                item->setValue( m_options.getLinearity().m_errorValue );
                item->setAttribute(QLatin1String("singleStep"), 0.1);
                item->setAttribute(QLatin1String("decimals"), 3);
                appendProperty(item, page, LO_PARAM_ERROR);
                errorGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_ERROR_CTRL]);
                item->setValue( m_options.getLinearity().m_errorCtrl );
                item->setAttribute(QLatin1String("singleStep"), 0.1);
                item->setAttribute(QLatin1String("decimals"), 3);
                appendProperty(item, page, LO_PARAM_ERROR_CTRL);
                errorGroup->addSubProperty(item);

                item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), LinearityParamName[LO_PARAM_ERROR_TYPE]);
                QStringList errorTypeList;
                for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
                {
                    errorTypeList.append(MeasureErrorTypeStr[e]);
                }
                item->setAttribute(QLatin1String("enumNames"), errorTypeList);
                item->setValue(m_options.getLinearity().m_errorType);
                appendProperty(item, page, LO_PARAM_ERROR_TYPE);
                errorGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_ERROR_BY_SCO]);
                item->setValue( m_options.getLinearity().m_errorCalcBySCO );
                appendProperty(item, page, LO_PARAM_ERROR_BY_SCO);
                errorGroup->addSubProperty(item);


                QtProperty *measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurements at the single point"));

                item = manager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_MEASURE_TIME]);
                item->setValue(m_options.getLinearity().m_measureTimeInPoint);
                item->setAttribute(QLatin1String("minimum"), 1);
                item->setAttribute(QLatin1String("maximum"), 60);
                item->setAttribute(QLatin1String("singleStep"), 1);
                appendProperty(item, page, LO_PARAM_MEASURE_TIME);
                measureGroup->addSubProperty(item);


                item = manager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_MEASURE_IN_POINT]);
                item->setValue(m_options.getLinearity().m_measureCountInPoint);
                item->setAttribute(QLatin1String("minimum"), 1);
                item->setAttribute(QLatin1String("maximum"), 20);
                item->setAttribute(QLatin1String("singleStep"), 1);
                appendProperty(item, page, LO_PARAM_MEASURE_IN_POINT);
                measureGroup->addSubProperty(item);


                QtProperty *pointGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurement points"));

                item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), LinearityParamName[LO_PARAM_RANGE_TYPE]);
                QStringList rangeTypeList;
                for(int r = 0; r < LO_RANGE_TYPE_COUNT; r++)
                {
                    rangeTypeList.append(LinearityRangeTypeStr[r]);
                }
                item->setAttribute(QLatin1String("enumNames"), rangeTypeList);
                item->setValue(m_options.getLinearity().m_rangeType);
                appendProperty(item, page, LO_PARAM_RANGE_TYPE);
                pointGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_POINT_COUNT]);
                item->setValue(m_options.getLinearity().m_pointBase.count());
                //item->setAttribute(QLatin1String("readOnly"), true);
                appendProperty(item, page, LO_PARAM_POINT_COUNT);
                pointGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_LOW_RANGE]);
                item->setValue( m_options.getLinearity().m_lowLimitRange );
                item->setAttribute(QLatin1String("singleStep"), 1);
                item->setAttribute(QLatin1String("decimals"), 1);
                appendProperty(item, page, LO_PARAM_LOW_RANGE);
                pointGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_HIGH_RANGE]);
                item->setValue( m_options.getLinearity().m_highLimitRange );
                item->setAttribute(QLatin1String("singleStep"), 1);
                item->setAttribute(QLatin1String("decimals"), 1);
                appendProperty(item, page, LO_PARAM_HIGH_RANGE);
                pointGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::String, LinearityParamName[LO_PARAM_VALUE_POINTS]);
                item->setValue(m_options.getLinearity().m_pointBase.text());
                appendProperty(item, page, LO_PARAM_VALUE_POINTS);
                pointGroup->addSubProperty(item);


                QtProperty *outputrangeGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("The output values"));

                item = manager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_OUTPUT_RANGE]);
                item->setValue( m_options.getLinearity().m_showOutputRangeColumn );
                appendProperty(item, page, LO_PARAM_OUTPUT_RANGE);
                outputrangeGroup->addSubProperty(item);

                item = manager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_CORRECT_OUTPUT]);
                item->setValue( m_options.getLinearity().m_considerCorrectOutput );
                appendProperty(item, page, LO_PARAM_CORRECT_OUTPUT);
                outputrangeGroup->addSubProperty(item);

                editor->setFactoryForManager(manager, factory);

                editor->addProperty(errorGroup);
                editor->addProperty(measureGroup);
                editor->addProperty(pointGroup);
                editor->addProperty(outputrangeGroup);
            }
            break;

        case OPTION_PAGE_SETTING_MEASURE:
            break;
        case OPTION_PAGE_MEASURE_VIEW_TEXT:
            break;
        case OPTION_PAGE_SIGNAL_INFO:
            break;
        case OPTION_PAGE_REPORT_HEADER:
            break;
        case OPTION_PAGE_DATABASE:
            break;
        case OPTION_PAGE_BACKUP:
            break;
        default:
            assert(nullptr);
            break;
    }

    editor->setPropertiesWithoutValueMarked(true);
    editor->setRootIsDecorated(false);

    connect(manager, &QtVariantPropertyManager::valueChanged, this, &OptionsDialog::onPropertyChanged );

    return (new PropertyPage(manager, factory, editor));
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* OptionsDialog::createPropertyDialog(int page)
{
    QDialog* pDialogPage = nullptr;

    switch (page)
    {
        case OPTION_PAGE_LINEARETY_POINT:
            {
                OptionsPointsDialog* dialog = new OptionsPointsDialog(m_options.getLinearity());

                connect(dialog, &OptionsPointsDialog::updateLinearityPage, this, &OptionsDialog::updateLinearityPage );
                pDialogPage = dialog;
            }
            break;

        case OPTION_PAGE_MEASURE_VIEW_COLUMN:
            {
                pDialogPage = new QDialog;
                pDialogPage->setStyleSheet(".QDialog { border: 1px solid grey } ");
            }
            break;

        default:
            assert(nullptr);
            break;
    }

    return ( new PropertyPage(pDialogPage) );
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::appendProperty(QtProperty* property, int page, int param)
{
    if (property == nullptr)
    {
        return;
    }

    if (page < 0 || page >= OPTION_PAGE_COUNT)
    {
        return;
    }

    m_propertyList[property] = (page << 8) | param;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
    if (current  == nullptr || previous == nullptr)
    {
        return;
    }

    if (current->childCount() != 0)
    {
        current->setExpanded(true);
        current->setSelected(false);
        current = current->child(0);
        current->setSelected(true);
    }

    int page = current->data(0, Qt::UserRole).toInt();
    if (page < 0 || page >= m_pageList.count())
    {
        return;
    }

    setActivePage(page);
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionsDialog::setActivePage(int page)
{
    if (page < 0 || page >= m_pageList.count())
    {
        return false;
    }

    // hide current page
    //
    if (activePage >= 0 && activePage < m_pageList.count() )
    {
        PropertyPage* oldPage = m_pageList.at(activePage);
        if (oldPage != nullptr )
        {
            QWidget* pWidget = oldPage->getWidget();
            if (pWidget != nullptr )
            {
                m_pagesLayout->removeWidget(pWidget);
                pWidget->hide();
            }
        }
    }

    // show new page
    //
    PropertyPage* newPage = m_pageList.at(page);
    if (newPage != nullptr)
    {
        QWidget* pWidget = newPage->getWidget();
        if (pWidget != nullptr )
        {
            setWindowTitle(tr("Options - %1").arg(OptionPage[page]));

            m_pagesLayout->addWidget(pWidget);
            pWidget->show();
        }
    }

    activePage = page;

    // select tree item
    //
    QTreeWidgetItem* item = newPage->m_pTreeWidgetItem;
    if (item != nullptr)
    {
        QTreeWidgetItem* parent = item->parent();
        if (parent != nullptr)
        {
            parent->setExpanded(true);
            for(int c = 0; c < parent->childCount(); c++)
            {
                parent->child(c)->setSelected(false);
            }
        }

        item->setSelected(true);
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onPropertyChanged(QtProperty *property, const QVariant &value)
{
    if (property == nullptr)
    {
        return;
    }

    if (m_propertyList.contains(property) == false)
    {
        return;
    }

    int page = m_propertyList[property] & 0xFF00;
    if (page < 0 || page >= OPTION_PAGE_COUNT)
    {
        return;
    }

    int param = m_propertyList[property] & 0x00FF;

    switch (page)
    {
        case OPTION_PAGE_TCP_IP:
            {
                switch(param)
                {
                    case TCPIP_PARAM_SERVER_IP:     m_options.getTcpIp().m_serverIP = value.toString();                 break;
                    case TCPIP_PARAM_SERVER_PORT:   m_options.getTcpIp().m_serverPort = value.toInt();                  break;
                }
            }
            break;

        case OPTION_PAGE_LINEARETY_MEASURE:
            {
                switch(param)
                {
                    case LO_PARAM_ERROR:            m_options.getLinearity().m_errorValue = value.toDouble();           break;
                    case LO_PARAM_ERROR_CTRL:       m_options.getLinearity().m_errorCtrl = value.toDouble();            break;
                    case LO_PARAM_ERROR_TYPE:       m_options.getLinearity().m_errorType = value.toInt();               break;
                    case LO_PARAM_ERROR_BY_SCO:     m_options.getLinearity().m_errorCalcBySCO = value.toBool();         break;
                    case LO_PARAM_MEASURE_TIME:     m_options.getLinearity().m_measureTimeInPoint = value.toInt();      break;
                    case LO_PARAM_MEASURE_IN_POINT: m_options.getLinearity().m_measureCountInPoint = value.toInt();     break;
                    case LO_PARAM_RANGE_TYPE:       m_options.getLinearity().m_rangeType = value.toInt();
                                                    m_options.getLinearity().recalcPoints();
                                                    updateLinearityPage(false);                                         break;
                    case LO_PARAM_POINT_COUNT:      m_options.getLinearity().recalcPoints(value.toInt());
                                                    updateLinearityPage(false);                                         break;
                    case LO_PARAM_LOW_RANGE:        m_options.getLinearity().m_lowLimitRange = value.toDouble();
                                                    m_options.getLinearity().recalcPoints();
                                                    updateLinearityPage(false);                                         break;
                    case LO_PARAM_HIGH_RANGE:       m_options.getLinearity().m_highLimitRange = value.toDouble();
                                                    m_options.getLinearity().recalcPoints();
                                                    updateLinearityPage(false);                                         break;
                    case LO_PARAM_VALUE_POINTS:     setActivePage(OPTION_PAGE_LINEARETY_POINT);                         break;
                    case LO_PARAM_OUTPUT_RANGE:     m_options.getLinearity().m_showOutputRangeColumn = value.toBool();  break;
                    case LO_PARAM_CORRECT_OUTPUT:	m_options.getLinearity().m_considerCorrectOutput = value.toBool();  break;
                    default:                        assert(0);                                                          break;
                }
            }
            break;

        case OPTION_PAGE_SETTING_MEASURE:

            break;

        case OPTION_PAGE_MEASURE_VIEW_TEXT:

            break;

        case OPTION_PAGE_MEASURE_VIEW_COLUMN:

            break;

        case OPTION_PAGE_SIGNAL_INFO:

            break;

        case OPTION_PAGE_REPORT_HEADER:

            break;

        case OPTION_PAGE_DATABASE:

            break;

        case OPTION_PAGE_BACKUP:

            break;

        default:
            assert(nullptr);
            break;
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::updateLinearityPage(bool isDialog)
{
    PropertyPage* page = m_pageList[OPTION_PAGE_LINEARETY_POINT];
    if (page == nullptr)
    {
        return;
    }

    OptionsPointsDialog* dialog = (OptionsPointsDialog*) page->getWidget();
    if (dialog == nullptr)
    {
        return;
    }

    if (isDialog == true)
    {
        // get options from dialog
        //
        m_options.setLinearity( dialog->m_linearity );
    }
    else
    {
        // set options to dialog
        //
        dialog->m_linearity = m_options.getLinearity();
    }

    QtVariantProperty *property = nullptr;

    property = (QtVariantProperty*) m_propertyList.key( (OPTION_PAGE_LINEARETY_MEASURE << 8) | LO_PARAM_RANGE_TYPE );
    if (property != nullptr)
    {
        property->setValue( m_options.getLinearity().m_rangeType );
    }

    property = (QtVariantProperty*) m_propertyList.key( (OPTION_PAGE_LINEARETY_MEASURE << 8) | LO_PARAM_POINT_COUNT );
    if (property != nullptr)
    {
        property->setValue( m_options.getLinearity().m_pointBase.count() );
    }

    property = (QtVariantProperty*) m_propertyList.key( (OPTION_PAGE_LINEARETY_MEASURE << 8) | LO_PARAM_LOW_RANGE );
    if (property != nullptr)
    {
        property->setValue( m_options.getLinearity().m_lowLimitRange );
    }

    property = (QtVariantProperty*) m_propertyList.key( (OPTION_PAGE_LINEARETY_MEASURE << 8) | LO_PARAM_HIGH_RANGE );
    if (property != nullptr)
    {
        property->setValue( m_options.getLinearity().m_highLimitRange );
    }

    property = (QtVariantProperty*) m_propertyList.key( (OPTION_PAGE_LINEARETY_MEASURE << 8) | LO_PARAM_VALUE_POINTS );
    if (property != nullptr)
    {
        property->setValue( m_options.getLinearity().m_pointBase.text() );
    }
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onOk()
{
    onApply();

    accept();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onApply()
{
    theOptions = m_options;
    theOptions.save();
}

// -------------------------------------------------------------------------------------------------------------------

bool OptionsDialog::event(QEvent * e)
{
    if ( e->type() == QEvent::Hide)
    {
        saveWindowPosition(this);
    }

    return QDialog::event(e);
}

// -------------------------------------------------------------------------------------------------------------------
