#include "OptionsDialog.h"

#include <assert.h>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>

#include "MainWindow.h"
#include "OptionsPointsDialog.h"

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent)
{
    createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(const Options& options, QWidget *parent) :
    QDialog(parent),
    m_options(options)
{
    createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::~OptionsDialog()
{
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::createInterface()
{
    setMinimumSize(680, 300);

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
    setActivePage(OPTION_PAGE_LINEARETY_MEASURE);
}

// -------------------------------------------------------------------------------------------------------------------

QHBoxLayout* OptionsDialog::createPages()
{
    QHBoxLayout *pagesLayout = new QHBoxLayout ;

    QTreeWidget* pTree = new QTreeWidget;

    pTree->setHeaderHidden(true);
    pTree->setFixedWidth(200);

    QList<QTreeWidgetItem*> groupList;

    for(int g = 0; g < OPTION_GROUP_COUNT; g++)
    {
        QTreeWidgetItem* groupItem = new QTreeWidgetItem;
        groupItem->setText(0, OptionGroup[g]);
        pTree->addTopLevelItem(groupItem);

        groupList.append(groupItem);
    }

    for(int p = 0; p < OPTION_PAGE_COUNT; p++)
    {
        int groupIndex = OptionGroupPage[p];
        if (groupIndex < 0 || groupIndex >= groupList.count())
        {
            continue;
        }

        QTreeWidgetItem* groupItem = groupList.at(groupIndex);

        QTreeWidgetItem* pageItem = new QTreeWidgetItem;
        pageItem->setText(0, OptionPageShort[p]);
        pageItem->setData(0, Qt::UserRole, p);

        groupItem->addChild(pageItem);

        m_pageList.append(createPage(p));
    }

    connect(pTree, &QTreeWidget::currentItemChanged , this, &OptionsDialog::onPageChanged );

    pagesLayout->addWidget(pTree);

    return pagesLayout;
}

// -------------------------------------------------------------------------------------------------------------------

QWidget* OptionsDialog::createPage(int page)
{
    QWidget* pPropertyPage = nullptr;

    switch (page)
    {
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

QWidget* OptionsDialog::createPropertyList(int page)
{
    QtVariantProperty *item = nullptr;

    QtVariantPropertyManager *variantManager = new QtVariantPropertyManager();
    QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();
    QtTreePropertyBrowser *variantEditor = new QtTreePropertyBrowser();

    switch (page)
    {
        case OPTION_PAGE_LINEARETY_MEASURE:
            {
                QtProperty *errorGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

                item = variantManager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_ERROR]);
                item->setValue( m_options.getLinearity().m_errorValue );
                item->setAttribute(QLatin1String("singleStep"), 0.1);
                item->setAttribute(QLatin1String("decimals"), 3);
                appendProperty(item, page, LO_PARAM_ERROR);
                errorGroup->addSubProperty(item);

                item = variantManager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_ERROR_CTRL]);
                item->setValue( m_options.getLinearity().m_errorCtrl );
                item->setAttribute(QLatin1String("singleStep"), 0.1);
                item->setAttribute(QLatin1String("decimals"), 3);
                appendProperty(item, page, LO_PARAM_ERROR_CTRL);
                errorGroup->addSubProperty(item);

                item = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), LinearityParamName[LO_PARAM_ERROR_TYPE]);
                QStringList errorTypeList;
                for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
                {
                    errorTypeList.append(MeasureErrorTypeStr[e]);
                }
                item->setAttribute(QLatin1String("enumNames"), errorTypeList);
                item->setValue(m_options.getLinearity().m_errorType);
                appendProperty(item, page, LO_PARAM_ERROR_TYPE);
                errorGroup->addSubProperty(item);

                item = variantManager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_ERROR_BY_SCO]);
                item->setValue( m_options.getLinearity().m_errorCalcBySCO );
                appendProperty(item, page, LO_PARAM_ERROR_BY_SCO);
                errorGroup->addSubProperty(item);


                QtProperty *measureGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurements at the single point"));

                item = variantManager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_MEASURE_TIME]);
                item->setValue(m_options.getLinearity().m_measureTimeInPoint);
                item->setAttribute(QLatin1String("minimum"), 1);
                item->setAttribute(QLatin1String("maximum"), 60);
                item->setAttribute(QLatin1String("singleStep"), 1);
                appendProperty(item, page, LO_PARAM_MEASURE_TIME);
                measureGroup->addSubProperty(item);


                item = variantManager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_MEASURE_IN_POINT]);
                item->setValue(m_options.getLinearity().m_measureCountInPoint);
                item->setAttribute(QLatin1String("minimum"), 1);
                item->setAttribute(QLatin1String("maximum"), 20);
                item->setAttribute(QLatin1String("singleStep"), 1);
                appendProperty(item, page, LO_PARAM_MEASURE_IN_POINT);
                measureGroup->addSubProperty(item);


                QtProperty *pointGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurement points"));

                item = variantManager->addProperty(QtVariantPropertyManager::enumTypeId(), LinearityParamName[LO_PARAM_RANGE_TYPE]);
                QStringList rangeTypeList;
                for(int r = 0; r < LO_RANGE_TYPE_COUNT; r++)
                {
                    rangeTypeList.append(LinearityRangeTypeStr[r]);
                }
                item->setAttribute(QLatin1String("enumNames"), rangeTypeList);
                item->setValue(m_options.getLinearity().m_rangeType);
                appendProperty(item, page, LO_PARAM_RANGE_TYPE);
                pointGroup->addSubProperty(item);

                item = variantManager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_POINT_COUNT]);
                item->setValue(m_options.getLinearity().m_pointBase.count());
                //item->setAttribute(QLatin1String("readOnly"), true);
                appendProperty(item, page, LO_PARAM_POINT_COUNT);
                pointGroup->addSubProperty(item);

                item = variantManager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_LOW_RANGE]);
                item->setValue( m_options.getLinearity().m_lowLimitRange );
                item->setAttribute(QLatin1String("singleStep"), 1);
                item->setAttribute(QLatin1String("decimals"), 1);
                appendProperty(item, page, LO_PARAM_LOW_RANGE);
                pointGroup->addSubProperty(item);

                item = variantManager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_HIGH_RANGE]);
                item->setValue( m_options.getLinearity().m_highLimitRange );
                item->setAttribute(QLatin1String("singleStep"), 1);
                item->setAttribute(QLatin1String("decimals"), 1);
                appendProperty(item, page, LO_PARAM_HIGH_RANGE);
                pointGroup->addSubProperty(item);

                item = variantManager->addProperty(QVariant::String, LinearityParamName[LO_PARAM_VALUE_POINTS]);
                item->setValue(m_options.getLinearity().m_pointBase.text());
                item->setAttribute(QLatin1String("readOnly"), true);
                appendProperty(item, page, LO_PARAM_VALUE_POINTS);
                pointGroup->addSubProperty(item);


                QtProperty *outputrangeGroup = variantManager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("The output values"));

                item = variantManager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_OUTPUT_RANGE]);
                item->setValue( m_options.getLinearity().m_showOutputRangeColumn );
                appendProperty(item, page, LO_PARAM_OUTPUT_RANGE);
                outputrangeGroup->addSubProperty(item);

                item = variantManager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_CORRECT_OUTPUT]);
                item->setValue( m_options.getLinearity().m_considerCorrectOutput );
                appendProperty(item, page, LO_PARAM_CORRECT_OUTPUT);
                outputrangeGroup->addSubProperty(item);

                variantEditor->setFactoryForManager(variantManager, variantFactory);

                variantEditor->addProperty(errorGroup);
                variantEditor->addProperty(measureGroup);
                variantEditor->addProperty(pointGroup);
                variantEditor->addProperty(outputrangeGroup);
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

    variantEditor->setPropertiesWithoutValueMarked(true);
    variantEditor->setRootIsDecorated(false);

    connect(variantManager, &QtVariantPropertyManager::valueChanged, this, &OptionsDialog::onPropertyChanged );

    return variantEditor;
}

// -------------------------------------------------------------------------------------------------------------------

QWidget* OptionsDialog::createPropertyDialog(int page)
{
    QWidget* pPropertyPage = nullptr;

    switch (page)
    {
        case OPTION_PAGE_LINEARETY_POINT:
            {
                OptionsPointsDialog* dialog = new OptionsPointsDialog(m_options.getLinearity());
                connect(dialog, &OptionsPointsDialog::updateLinearityPage, this, &OptionsDialog::updateLinearityPage );
                pPropertyPage = dialog;
            }
            break;

        case OPTION_PAGE_MEASURE_VIEW_COLUMN:

            break;

        default:
            assert(nullptr);
            break;
    }

    return pPropertyPage;
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
    if (m_activePage >= 0 && m_activePage < m_pageList.count() )
    {
        QWidget* oldPage = m_pageList.at(m_activePage);
        if (oldPage != nullptr)
        {
            m_pagesLayout->removeWidget(oldPage);
            oldPage->hide();
        }
    }

    // show new page
    //
    QWidget* newPage = m_pageList.at(page);
    if (newPage == nullptr)
    {
        return false;
    }

    setWindowTitle(tr("Options - %1").arg(OptionPage[page]));
    m_pagesLayout->addWidget(newPage);
    newPage->show();

    m_activePage = page;

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
        case OPTION_PAGE_LINEARETY_MEASURE:
            {
                m_options.getLinearity();

                switch(param)
                {
                    case LO_PARAM_ERROR:            m_options.getLinearity().m_errorValue = value.toDouble();          break;
                    case LO_PARAM_ERROR_CTRL:       m_options.getLinearity().m_errorCtrl = value.toDouble();           break;
                    case LO_PARAM_ERROR_TYPE:       m_options.getLinearity().m_errorType = value.toInt();              break;
                    case LO_PARAM_ERROR_BY_SCO:     m_options.getLinearity().m_errorCalcBySCO = value.toBool();        break;
                    case LO_PARAM_MEASURE_TIME:     m_options.getLinearity().m_measureTimeInPoint = value.toInt();     break;
                    case LO_PARAM_MEASURE_IN_POINT: m_options.getLinearity().m_measureCountInPoint = value.toInt();    break;
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

void OptionsDialog::updateLinearityPage(bool isDialog)
{
    OptionsPointsDialog* page = (OptionsPointsDialog*) m_pageList[OPTION_PAGE_LINEARETY_POINT];
    if (page == nullptr)
    {
        return;
    }

    if (isDialog == false)
    {
        page->m_linearity = m_options.getLinearity();
    }
    else
    {
        m_options.setLinearity( page->m_linearity );
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
    MainWindow* wnd = (MainWindow*) parentWidget();
    if (wnd == nullptr)
    {
        return;
    }

    wnd->m_options = m_options;
    wnd->m_options.save();
}

// -------------------------------------------------------------------------------------------------------------------

