#include "OptionsDialog.h"

#include <QApplication>
#include <QDesktopWidget>
#include <assert.h>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>

#include "FolderPropertyManager.h"
#include "OptionsPointsDialog.h"
#include "OptionsMvhDialog.h"

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

	m_type = PROPERTY_PAGE_TYPE_UNKNOWN;
	m_page = OPTION_PAGE_UNKNOWN;

	m_pWidget = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

int OptionsDialog::m_activePage = OPTION_PAGE_LINEARITY_MEASURE;

// -------------------------------------------------------------------------------------------------------------------

OptionsDialog::OptionsDialog(QWidget *parent) :
	QDialog(parent)
{
	m_options = theOptions;

	for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
	{
		m_options.m_updateColumnView[type] = false;
	}

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
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));
	setMinimumSize(850, 400);
	move(QApplication::desktop()->availableGeometry().center() - rect().center());
	loadSettings();

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
	setActivePage(m_activePage);
}

// -------------------------------------------------------------------------------------------------------------------

QHBoxLayout* OptionsDialog::createPages()
{
	QHBoxLayout *pagesLayout = new QHBoxLayout ;

	m_pPageTree = new QTreeWidget;

	m_pPageTree->setHeaderHidden(true);
	m_pPageTree->setFixedWidth(200);

	QList<QTreeWidgetItem*> groupList;

	for(int group = 0; group < OPTION_GROUP_COUNT; group++)
	{
		QTreeWidgetItem* groupTreeItem = new QTreeWidgetItem;
		groupTreeItem->setText(0, OptionGroupTitle[group]);
		m_pPageTree->addTopLevelItem(groupTreeItem);

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
		pageTreeItem->setText(0, OptionPageShortTitle[page]);
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

	connect(m_pPageTree, &QTreeWidget::currentItemChanged , this, &OptionsDialog::onPageChanged);

	pagesLayout->addWidget(m_pPageTree);

	return pagesLayout;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::removePages()
{
	m_propertyItemList.clear();
	m_propertyValueList.clear();

	int count = m_pageList.count();
	for(int i = 0; i < count; i++)
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
	if (page < 0 || page >= OPTION_PAGE_COUNT)
	{
		return nullptr;
	}

	PropertyPage* pPropertyPage = nullptr;

	switch (page)
	{
		case OPTION_PAGE_CONFIG_SOCKET:
		case OPTION_PAGE_SIGNAL_SOCKET:
		case OPTION_PAGE_TUNING_SOCKET:
		case OPTION_PAGE_LINEARITY_MEASURE:
		case OPTION_PAGE_COMPARATOR_MEASURE:
		case OPTION_PAGE_MEASURE_VIEW_TEXT:
		case OPTION_PAGE_SIGNAL_INFO:
		case OPTION_PAGE_DATABASE:
		case OPTION_PAGE_BACKUP:				pPropertyPage = createPropertyList(page);	break;
		case OPTION_PAGE_LINEARITY_POINT:
		case OPTION_PAGE_MEASURE_VIEW_COLUMN:	pPropertyPage = createPropertyDialog(page);	break;
		default:								assert(nullptr);
	}

	return pPropertyPage;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* OptionsDialog::createPropertyList(int page)
{
	if (page < 0 || page >= OPTION_PAGE_COUNT)
	{
		return nullptr;
	}

	QtVariantProperty *item = nullptr;


	QtVariantPropertyManager *manager = new VariantManager();
	QtVariantEditorFactory *factory = new VariantFactory();

	//QtVariantPropertyManager *manager = new QtVariantPropertyManager;
	//QtVariantEditorFactory *factory = new QtVariantEditorFactory;

	QtTreePropertyBrowser *editor = new QtTreePropertyBrowser;

	switch (page)
	{
		case OPTION_PAGE_CONFIG_SOCKET:
		case OPTION_PAGE_SIGNAL_SOCKET:
		case OPTION_PAGE_TUNING_SOCKET:
			{
				int socketType = -1;

				switch (page)
				{
					case OPTION_PAGE_CONFIG_SOCKET:	socketType = SOCKET_TYPE_CONFIG;	break;
					case OPTION_PAGE_SIGNAL_SOCKET:	socketType = SOCKET_TYPE_SIGNAL;	break;
					case OPTION_PAGE_TUNING_SOCKET:	socketType = SOCKET_TYPE_TUNING;	break;
					default:						socketType = -1;					break;
				}

				if (socketType < 0 || socketType >= SOCKET_TYPE_COUNT)
				{
					break;
				}

				SocketClientOption sco = m_options.socket().client(socketType);

				QtProperty *serverGroup1 = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Server 1 (primary)"));

					item = manager->addProperty(QVariant::String, SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_EQUIPMENT_ID1]);
					item->setValue(sco.equipmentID(SOCKET_SERVER_TYPE_PRIMARY));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_EQUIPMENT_ID1);
					serverGroup1->addSubProperty(item);

					item = manager->addProperty(QVariant::String, SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_IP1]);
					item->setValue(sco.serverIP(SOCKET_SERVER_TYPE_PRIMARY));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_IP1);
					serverGroup1->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_PORT1]);
					item->setValue(sco.serverPort(SOCKET_SERVER_TYPE_PRIMARY));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 65535);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_PORT1);
					serverGroup1->addSubProperty(item);

				QtProperty *serverGroup2 = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Server 2 (reserve)"));

					item = manager->addProperty(QVariant::String, SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_EQUIPMENT_ID2]);
					item->setValue(sco.equipmentID(SOCKET_SERVER_TYPE_RESERVE));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_EQUIPMENT_ID2);
					serverGroup2->addSubProperty(item);

					if (socketType == SOCKET_TYPE_CONFIG)
					{
						item->setAttribute(QLatin1String("readOnly"), true);
					}

					item = manager->addProperty(QVariant::String, SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_IP2]);
					item->setValue(sco.serverIP(SOCKET_SERVER_TYPE_RESERVE));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_IP2);
					serverGroup2->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_PORT2]);
					item->setValue(sco.serverPort(SOCKET_SERVER_TYPE_RESERVE));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 65535);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_PORT2);
					serverGroup2->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(serverGroup1);
				editor->addProperty(serverGroup2);

			}
			break;

		case OPTION_PAGE_LINEARITY_MEASURE:
			{
				QtProperty *errorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

					item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_ERROR]);
					item->setValue(m_options.linearity().errorLimit());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, LO_PARAM_ERROR);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_ERROR_CTRL]);
					item->setValue(m_options.linearity().errorCtrl());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, LO_PARAM_ERROR_CTRL);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), LinearityParamName[LO_PARAM_ERROR_TYPE]);
					QStringList errorTypeList;
					for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
					{
						errorTypeList.append(ErrorType[e]);
					}
					item->setAttribute(QLatin1String("enumNames"), errorTypeList);
					item->setValue(m_options.linearity().errorType());
					appendProperty(item, page, LO_PARAM_ERROR_TYPE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), LinearityParamName[LO_PARAM_SHOW_INPUT_ERROR]);
					QStringList showInputErrorTypeList;
					for(int t = 0; t < LO_SHOW_INPUT_ERROR_COUNT; t++)
					{
						showInputErrorTypeList.append(ShowInputErrorStr[t]);
					}
					item->setAttribute(QLatin1String("enumNames"), showInputErrorTypeList);
					item->setValue(m_options.linearity().showInputErrorType());
					appendProperty(item, page, LO_PARAM_SHOW_INPUT_ERROR);
					errorGroup->addSubProperty(item);

				QtProperty *measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurements at the single point"));

					item = manager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_MEASURE_TIME]);
					item->setValue(m_options.linearity().measureTimeInPoint());
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 60);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, LO_PARAM_MEASURE_TIME);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_MEASURE_IN_POINT]);
					item->setValue(m_options.linearity().measureCountInPoint());
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), MAX_MEASUREMENT_IN_POINT);
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
					item->setValue(m_options.linearity().rangeType());
					appendProperty(item, page, LO_PARAM_RANGE_TYPE);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, LinearityParamName[LO_PARAM_POINT_COUNT]);
					item->setValue(m_options.linearity().points().count());
					switch(m_options.linearity().rangeType())
					{
						case LO_RANGE_TYPE_MANUAL:		item->setEnabled(false);	break;
						case LO_RANGE_TYPE_AUTOMATIC:	item->setEnabled(true);		break;
						default:						assert(0);
					}
					appendProperty(item, page, LO_PARAM_POINT_COUNT);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_LOW_RANGE]);
					item->setValue(m_options.linearity().lowLimitRange());
					item->setAttribute(QLatin1String("singleStep"), 1);
					item->setAttribute(QLatin1String("decimals"), 1);
					switch(m_options.linearity().rangeType())
					{
						case LO_RANGE_TYPE_MANUAL:		item->setEnabled(false);	break;
						case LO_RANGE_TYPE_AUTOMATIC:	item->setEnabled(true);		break;
						default:						assert(0);
					}
					appendProperty(item, page, LO_PARAM_LOW_RANGE);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, LinearityParamName[LO_PARAM_HIGH_RANGE]);
					item->setValue(m_options.linearity().highLimitRange());
					item->setAttribute(QLatin1String("singleStep"), 1);
					item->setAttribute(QLatin1String("decimals"), 1);
					switch(m_options.linearity().rangeType())
					{
						case LO_RANGE_TYPE_MANUAL:		item->setEnabled(false);	break;
						case LO_RANGE_TYPE_AUTOMATIC:	item->setEnabled(true);		break;
						default:						assert(0);
					}
					appendProperty(item, page, LO_PARAM_HIGH_RANGE);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::String, LinearityParamName[LO_PARAM_VALUE_POINTS]);
					item->setValue(m_options.linearity().points().text());
					appendProperty(item, page, LO_PARAM_VALUE_POINTS);
					pointGroup->addSubProperty(item);


				QtProperty *outputrangeGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("The list of measurements"));

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), LinearityParamName[LO_PARAM_LIST_TYPE]);
					QStringList listTypeList;
					for(int r = 0; r < LO_VIEW_TYPE_COUNT; r++)
					{
						listTypeList.append(LinearityViewTypeStr[r]);
					}
					item->setAttribute(QLatin1String("enumNames"), listTypeList);
					item->setValue(m_options.linearity().viewType());
					appendProperty(item, page, LO_PARAM_LIST_TYPE);
					outputrangeGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_SHOW_INPUT_RANGE]);
					item->setValue(m_options.linearity().showInputRangeColumn());
					appendProperty(item, page, LO_PARAM_SHOW_INPUT_RANGE);
					outputrangeGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, LinearityParamName[LO_PARAM_SHOW_OUTPUT_RANGE]);
					item->setValue(m_options.linearity().showOutputRangeColumn());
					appendProperty(item, page, LO_PARAM_SHOW_OUTPUT_RANGE);
					outputrangeGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(errorGroup);
				editor->addProperty(measureGroup);
				editor->addProperty(pointGroup);
				editor->addProperty(outputrangeGroup);
			}
			break;

		case OPTION_PAGE_COMPARATOR_MEASURE:
			{
				QtProperty *errorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

					item = manager->addProperty(QVariant::Double, ComparatorParamName[CO_PARAM_ERROR]);
					item->setValue(m_options.comparator().m_errorValue);
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, CO_PARAM_ERROR);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, ComparatorParamName[CO_PARAM_ERROR_CTRL]);
					item->setValue(m_options.comparator().m_errorCtrl);
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, CO_PARAM_ERROR_CTRL);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, ComparatorParamName[CO_PARAM_START_VALUE]);
					item->setValue(m_options.comparator().m_startValue);
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, CO_PARAM_START_VALUE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), ComparatorParamName[CO_PARAM_ERROR_TYPE]);
					QStringList errorTypeList;
					for(int e = 0; e < MEASURE_ERROR_TYPE_COUNT; e++)
					{
						errorTypeList.append(ErrorType[e]);
					}
					item->setAttribute(QLatin1String("enumNames"), errorTypeList);
					item->setValue(m_options.comparator().m_errorType);
					appendProperty(item, page, CO_PARAM_ERROR_TYPE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, ComparatorParamName[CO_PARAM_ENABLE_HYSTERESIS]);
					item->setValue(m_options.comparator().m_enableMeasureHysteresis);
					appendProperty(item, page, CO_PARAM_ENABLE_HYSTERESIS);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, ComparatorParamName[CO_PARAM_COMPARATOR_INDEX]);
					item->setValue(m_options.comparator().m_startComparatorIndex);
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 16);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, CO_PARAM_COMPARATOR_INDEX);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, ComparatorParamName[CO_PARAM_ADDITIONAL_CHECK]);
					item->setValue(m_options.comparator().m_additionalCheck);
					appendProperty(item, page, CO_PARAM_ADDITIONAL_CHECK);
					errorGroup->addSubProperty(item);


				editor->setFactoryForManager(manager, factory);

				editor->addProperty(errorGroup);
			}
			break;

		case OPTION_PAGE_MEASURE_VIEW_TEXT:
			{
				QtProperty *fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, MeasureViewParam[MWO_PARAM_FONT]);
					item->setValue(m_options.measureView().font());
					appendProperty(item, page, MWO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty *measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Displaying measurements"));

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), MeasureViewParam[MWO_PARAM_ID]);
					QStringList valueTypeIDList;
					for(int t = 0; t < SIGNAL_ID_TYPE_COUNT; t++)
					{
						valueTypeIDList.append(TypeSignalID[t]);
					}
					item->setAttribute(QLatin1String("enumNames"), valueTypeIDList);
					item->setValue(m_options.measureView().signalIdType());
					appendProperty(item, page, MWO_PARAM_ID);
					measureGroup->addSubProperty(item);

				QtProperty *colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, MeasureViewParam[MWO_PARAM_COLOR_NOT_ERROR]);
					item->setValue(m_options.measureView().colorNotError());
					appendProperty(item, page, MWO_PARAM_COLOR_NOT_ERROR);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, MeasureViewParam[MWO_PARAM_COLOR_LIMIT_ERROR]);
					item->setValue(m_options.measureView().colorErrorLimit());
					appendProperty(item, page, MWO_PARAM_COLOR_LIMIT_ERROR);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, MeasureViewParam[MWO_PARAM_COLOR_CONTROL_ERROR]);
					item->setValue(m_options.measureView().colorErrorControl());
					appendProperty(item, page, MWO_PARAM_COLOR_CONTROL_ERROR);
					colorGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(measureGroup);
				editor->addProperty(colorGroup);

				expandProperty(editor, OPTION_PAGE_MEASURE_VIEW_TEXT, MWO_PARAM_FONT, false);
				expandProperty(editor, OPTION_PAGE_MEASURE_VIEW_TEXT, MWO_PARAM_COLOR_NOT_ERROR, false);
				expandProperty(editor, OPTION_PAGE_MEASURE_VIEW_TEXT, MWO_PARAM_COLOR_LIMIT_ERROR, false);
				expandProperty(editor, OPTION_PAGE_MEASURE_VIEW_TEXT, MWO_PARAM_COLOR_CONTROL_ERROR, false);
			}
			break;

		case OPTION_PAGE_SIGNAL_INFO:
			{
				QtProperty *fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, SignalInfoParam[SIO_PARAM_FONT]);
					item->setValue(m_options.signalInfo().font());
					appendProperty(item, page, SIO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty *measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Displaying signal state"));

					item = manager->addProperty(QVariant::Bool, SignalInfoParam[SIO_PARAM_ID]);
					item->setValue(m_options.signalInfo().showCustomID());
					appendProperty(item, page, SIO_PARAM_ID);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, SignalInfoParam[SIO_PARAM_ELECTRIC_STATE]);
					item->setValue(m_options.signalInfo().showElectricState());
					appendProperty(item, page, SIO_PARAM_ELECTRIC_STATE);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, SignalInfoParam[SIO_PARAM_ADC_STATE]);
					item->setValue(m_options.signalInfo().showAdcState());
					appendProperty(item, page, SIO_PARAM_ADC_STATE);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, SignalInfoParam[SIO_PARAM_ADC_HEX_STATE]);
					item->setValue(m_options.signalInfo().showAdcHexState());
					appendProperty(item, page, SIO_PARAM_ADC_HEX_STATE);
					measureGroup->addSubProperty(item);

				QtProperty *colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, SignalInfoParam[SIO_PARAM_COLOR_FLAG_VALID]);
					item->setValue(m_options.signalInfo().colorFlagValid());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_VALID);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, SignalInfoParam[SIO_PARAM_COLOR_FLAG_OVERFLOW]);
					item->setValue(m_options.signalInfo().colorFlagOverflow());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_OVERFLOW);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, SignalInfoParam[SIO_PARAM_COLOR_FLAG_UNDERFLOW]);
					item->setValue(m_options.signalInfo().colorFlagUnderflow());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_UNDERFLOW);
					colorGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(measureGroup);
				editor->addProperty(colorGroup);

				expandProperty(editor, OPTION_PAGE_SIGNAL_INFO, SIO_PARAM_FONT, false);
				expandProperty(editor, OPTION_PAGE_SIGNAL_INFO, SIO_PARAM_COLOR_FLAG_VALID, false);
				expandProperty(editor, OPTION_PAGE_SIGNAL_INFO, SIO_PARAM_COLOR_FLAG_OVERFLOW, false);
				expandProperty(editor, OPTION_PAGE_SIGNAL_INFO, SIO_PARAM_COLOR_FLAG_UNDERFLOW, false);
			}

			break;

		case OPTION_PAGE_DATABASE:
			{
				QtProperty *databaseGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Location of Database"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), DatabaseParam[DBO_PARAM_PATH]);
					item->setValue(m_options.database().path());
					appendProperty(item, page, DBO_PARAM_PATH);
					databaseGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), DatabaseParam[DBO_PARAM_TYPE]);
					QStringList valueTypeList;
					for(int t = 0; t < DATABASE_TYPE_COUNT; t++)
					{
						valueTypeList.append(DatabaseType[t]);
					}
					item->setAttribute(QLatin1String("enumNames"), valueTypeList);
					item->setValue(m_options.database().type());
					appendProperty(item, page, DBO_PARAM_TYPE);
					databaseGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(databaseGroup);
			}
			break;

		case OPTION_PAGE_BACKUP:
			{
				QtProperty *eventGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Events"));

					item = manager->addProperty(QVariant::Bool, BackupParam[BUO_PARAM_ON_START]);
					item->setValue(m_options.backup().onStart());
					appendProperty(item, page, BUO_PARAM_ON_START);
					eventGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, BackupParam[BUO_PARAM_ON_EXIT]);
					item->setValue(m_options.backup().onExit());
					appendProperty(item, page, BUO_PARAM_ON_EXIT);
					eventGroup->addSubProperty(item);

				QtProperty *pathGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Location of reserve copy"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), BackupParam[BUO_PARAM_PATH]);
					item->setValue(m_options.backup().path());
					appendProperty(item, page, BUO_PARAM_PATH);
					pathGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(eventGroup);
				editor->addProperty(pathGroup);
			}
			break;

		default:
			assert(nullptr);
			break;
	}

	editor->setPropertiesWithoutValueMarked(true);
	editor->setRootIsDecorated(false);

	connect(manager, &QtVariantPropertyManager::valueChanged, this, &OptionsDialog::onPropertyValueChanged);
	connect(editor, &QtTreePropertyBrowser::currentItemChanged, this, &OptionsDialog::onBrowserItem);

	return (new PropertyPage(manager, factory, editor));
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* OptionsDialog::createPropertyDialog(int page)
{
	if (page < 0 || page >= OPTION_PAGE_COUNT)
	{
		return nullptr;
	}

	QDialog* pDialogPage = nullptr;

	switch (page)
	{
		case OPTION_PAGE_LINEARITY_POINT:
			{
				OptionsPointsDialog* dialog = new OptionsPointsDialog(m_options.linearity());
				connect(dialog, &OptionsPointsDialog::updateLinearityPage, this, &OptionsDialog::updateLinearityPage);

				pDialogPage = dialog;
			}
			break;

		case OPTION_PAGE_MEASURE_VIEW_COLUMN:
			{
				OptionsMeasureViewHeaderDialog* dialog = new OptionsMeasureViewHeaderDialog(m_options.measureView());
				connect(dialog, &OptionsMeasureViewHeaderDialog::updateMeasureViewPage, this, &OptionsDialog::updateMeasureViewPage);

				pDialogPage = dialog;
			}
			break;

		default:
			assert(nullptr);
			break;
	}

	if (pDialogPage != nullptr)
	{
		pDialogPage->setWindowTitle(OptionPageTitle[page]);
	}

	return (new PropertyPage(pDialogPage));
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

	m_propertyItemList[property] = (page << 8) | param;
	m_propertyValueList[property] = ((QtVariantProperty*) property)->value();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::expandProperty(QtTreePropertyBrowser* pEditor, int page, int param, bool expanded)
{
	if (pEditor == nullptr)
	{
		return;
	}

	if (page < 0 || page >= OPTION_PAGE_COUNT)
	{
		return;
	}

	QtProperty* pProperty = m_propertyItemList.key((page << 8) | param);
	if (pProperty == nullptr)
	{
		return;
	}

	pEditor->setExpanded(pEditor->items(pProperty)[0], expanded);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (current == nullptr || previous == nullptr)
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
	if (m_activePage >= 0 && m_activePage < m_pageList.count())
	{
		PropertyPage* pCurrentPage = m_pageList.at(m_activePage);
		if (pCurrentPage != nullptr)
		{
			QWidget* pWidget = pCurrentPage->getWidget();
			if (pWidget != nullptr)
			{
				m_pagesLayout->removeWidget(pWidget);
				pWidget->hide();
			}
		}
	}

	// show new page
	//
	PropertyPage* pActivePage = m_pageList.at(page);
	if (pActivePage != nullptr)
	{
		QWidget* pWidget = pActivePage->getWidget();
		if (pWidget != nullptr)
		{
			setWindowTitle(tr("Options - %1").arg(OptionPageTitle[page]));

			m_pagesLayout->addWidget(pWidget);
			pWidget->show();
		}
	}

	m_activePage = page;

	// select tree item
	//
	if (m_pPageTree != nullptr && pActivePage->m_pTreeWidgetItem != nullptr)
	{
		m_pPageTree->setCurrentItem(pActivePage->m_pTreeWidgetItem);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onPropertyValueChanged(QtProperty *property, const QVariant &value)
{
	if (property == nullptr)
	{
		return;
	}

	if (m_propertyItemList.contains(property) == false)
	{
		return;
	}

	m_currentPropertyItem = property;
	m_currentPropertyValue = value;

	int type = ((QtVariantProperty*)property)->propertyType();

	if (type == QVariant::Bool ||									// check
		type == QtVariantPropertyManager::enumTypeId() ||			// list of values
		type == QVariant::String ||									// string
		type == QVariant::Font ||									// font
		type == QVariant::Color ||									// color
		type == VariantManager::folerPathTypeId())					// folder
	{
		applyProperty();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::onBrowserItem(QtBrowserItem*)
{
	restoreProperty();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::restoreProperty()
{
	QtProperty *property = m_currentPropertyItem;
	if (property == nullptr)
	{
		return;
	}

	if (m_propertyValueList.contains(property) == false)
	{
		return;
	}

	QVariant value = m_propertyValueList[property];

	((QtVariantProperty*) property)->setValue(value);
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::applyProperty()
{
	QtProperty *property = m_currentPropertyItem;
	if (property == nullptr)
	{
		return;
	}

	if (m_propertyItemList.contains(property) == false)
	{
		return;
	}

	m_propertyValueList[property] = m_currentPropertyValue;


	int paramId = m_propertyItemList[property];

	int page = (paramId & 0xFF00) >> 8;
	if (page < 0 || page >= OPTION_PAGE_COUNT)
	{
		return;
	}

	int param = paramId & 0x00FF;

	QVariant value = m_currentPropertyValue;

	switch (page)
	{
		case OPTION_PAGE_CONFIG_SOCKET:
		case OPTION_PAGE_SIGNAL_SOCKET:
		case OPTION_PAGE_TUNING_SOCKET:
			{
				int socketType = -1;

				switch (page)
				{
					case OPTION_PAGE_CONFIG_SOCKET: socketType = SOCKET_TYPE_CONFIG;	break;
					case OPTION_PAGE_SIGNAL_SOCKET: socketType = SOCKET_TYPE_SIGNAL;	break;
					case OPTION_PAGE_TUNING_SOCKET: socketType = SOCKET_TYPE_TUNING;	break;
					default:						socketType = -1;					break;
				}

				if (socketType < 0 || socketType >= SOCKET_TYPE_COUNT)
				{
					break;
				}

				SocketClientOption sco = m_options.socket().client(socketType);

				switch(param)
				{
					case SOCKET_CLIENT_PARAM_EQUIPMENT_ID1:	sco.setEquipmentID(SOCKET_SERVER_TYPE_PRIMARY, value.toString());	break;
					case SOCKET_CLIENT_PARAM_SERVER_IP1:	sco.setServerIP(SOCKET_SERVER_TYPE_PRIMARY, value.toString());		break;
					case SOCKET_CLIENT_PARAM_SERVER_PORT1:	sco.setServerPort(SOCKET_SERVER_TYPE_PRIMARY, value.toInt());		break;
					case SOCKET_CLIENT_PARAM_EQUIPMENT_ID2:	sco.setEquipmentID(SOCKET_SERVER_TYPE_RESERVE, value.toString());	break;
					case SOCKET_CLIENT_PARAM_SERVER_IP2:	sco.setServerIP(SOCKET_SERVER_TYPE_RESERVE, value.toString());		break;
					case SOCKET_CLIENT_PARAM_SERVER_PORT2:	sco.setServerPort(SOCKET_SERVER_TYPE_RESERVE, value.toInt());		break;
					default:								assert(0);
				}

				m_options.socket().setClient(socketType, sco);
				updateServerPage();

			}
			break;

		case OPTION_PAGE_LINEARITY_MEASURE:
			{
				switch(param)
				{
					case LO_PARAM_ERROR:					m_options.linearity().setErrorLimit(value.toDouble());			break;
					case LO_PARAM_ERROR_CTRL:				m_options.linearity().setErrorCtrl(value.toDouble());			break;
					case LO_PARAM_ERROR_TYPE:				m_options.linearity().setErrorType(value.toInt());
															m_options.m_updateColumnView[MEASURE_TYPE_LINEARITY] = true;	break;
					case LO_PARAM_SHOW_INPUT_ERROR:			m_options.linearity().setShowInputErrorType(value.toInt());
															m_options.m_updateColumnView[MEASURE_TYPE_LINEARITY] = true;	break;
					case LO_PARAM_MEASURE_TIME:				m_options.linearity().setMeasureTimeInPoint(value.toInt());		break;
					case LO_PARAM_MEASURE_IN_POINT:			m_options.linearity().setMeasureCountInPoint(value.toInt());	break;
					case LO_PARAM_RANGE_TYPE:				m_options.linearity().setRangeType(value.toInt());
															m_options.linearity().recalcPoints();
															updateLinearityPage(false);										break;
					case LO_PARAM_POINT_COUNT:				m_options.linearity().recalcPoints(value.toInt());
															updateLinearityPage(false);										break;
					case LO_PARAM_LOW_RANGE:				m_options.linearity().setLowLimitRange(value.toDouble());
															m_options.linearity().recalcPoints();
															updateLinearityPage(false);										break;
					case LO_PARAM_HIGH_RANGE:				m_options.linearity().setHighLimitRange(value.toDouble());
															m_options.linearity().recalcPoints();
															updateLinearityPage(false);										break;
					case LO_PARAM_VALUE_POINTS:				setActivePage(OPTION_PAGE_LINEARITY_POINT);						break;
					case LO_PARAM_LIST_TYPE:				m_options.linearity().setViewType(value.toInt());
															m_options.m_updateColumnView[MEASURE_TYPE_LINEARITY] = true;	break;
					case LO_PARAM_SHOW_INPUT_RANGE:			m_options.linearity().setShowInputRangeColumn(value.toBool());
															m_options.m_updateColumnView[MEASURE_TYPE_LINEARITY] = true;	break;

					case LO_PARAM_SHOW_OUTPUT_RANGE:		m_options.linearity().setShowOutputRangeColumn(value.toBool());
															m_options.m_updateColumnView[MEASURE_TYPE_LINEARITY] = true;	break;
					default:								assert(0);
				}
			}
			break;

		case OPTION_PAGE_COMPARATOR_MEASURE:
			{
				switch(param)
				{
					case CO_PARAM_ERROR:				m_options.comparator().m_errorValue = value.toDouble();				break;
					case CO_PARAM_ERROR_CTRL:			m_options.comparator().m_errorCtrl = value.toDouble();				break;
					case CO_PARAM_START_VALUE:			m_options.comparator().m_startValue = value.toDouble();				break;
					case CO_PARAM_ERROR_TYPE:			m_options.comparator().m_errorType = value.toInt();					break;
					case CO_PARAM_ENABLE_HYSTERESIS:	m_options.comparator().m_enableMeasureHysteresis = value.toBool();	break;
					case CO_PARAM_COMPARATOR_INDEX:		m_options.comparator().m_startComparatorIndex = value.toInt();		break;
					case CO_PARAM_ADDITIONAL_CHECK:		m_options.comparator().m_additionalCheck = value.toBool();			break;
					default:							assert(0);
				}
			}
			break;

		case OPTION_PAGE_MEASURE_VIEW_TEXT:
			{
				switch(param)
				{
					case MWO_PARAM_FONT:				m_options.measureView().font().fromString(value.toString());			break;
					case MWO_PARAM_ID:					m_options.measureView().setSignalIdType(value.toInt());					break;
					case MWO_PARAM_COLOR_NOT_ERROR:		m_options.measureView().setColorNotError(QColor(value.toString()));		break;
					case MWO_PARAM_COLOR_LIMIT_ERROR:	m_options.measureView().setColorErrorLimit(QColor(value.toString()));	break;
					case MWO_PARAM_COLOR_CONTROL_ERROR:	m_options.measureView().setColorErrorControl(QColor(value.toString()));	break;
					default:							assert(0);
				}

				for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
				{
					m_options.m_updateColumnView[type] = true;
				}
			}
			break;

		case OPTION_PAGE_MEASURE_VIEW_COLUMN:

			break;

		case OPTION_PAGE_SIGNAL_INFO:

			switch(param)
			{
				case SIO_PARAM_FONT:					m_options.signalInfo().font().fromString(value.toString());				break;
				case SIO_PARAM_ID:						m_options.signalInfo().setShowCustomID(value.toBool());					break;
				case SIO_PARAM_ELECTRIC_STATE:			m_options.signalInfo().setShowElectricState(value.toBool());			break;
				case SIO_PARAM_ADC_STATE:				m_options.signalInfo().setShowAdcState(value.toBool());					break;
				case SIO_PARAM_ADC_HEX_STATE:			m_options.signalInfo().setShowAdcHexState(value.toBool());				break;
				case SIO_PARAM_COLOR_FLAG_VALID:		m_options.signalInfo().setColorFlagValid(QColor(value.toString()));		break;
				case SIO_PARAM_COLOR_FLAG_OVERFLOW:		m_options.signalInfo().setColorFlagOverflow(QColor(value.toString()));	break;
				case SIO_PARAM_COLOR_FLAG_UNDERFLOW:	m_options.signalInfo().setColorFlagUnderflow(QColor(value.toString()));	break;
				default:								assert(0);
			}

			break;

		case OPTION_PAGE_DATABASE:
			{
				switch(param)
				{
					case DBO_PARAM_PATH:				m_options.database().setPath(value.toString());		break;
					case DBO_PARAM_TYPE:				m_options.database().setType(value.toBool());		break;
					default:							assert(0);
				}
			}
			break;

		case OPTION_PAGE_BACKUP:
			{
				switch(param)
				{
					case BUO_PARAM_ON_START:			m_options.backup().setOnStart(value.toBool());		break;
					case BUO_PARAM_ON_EXIT:				m_options.backup().setOnExit(value.toBool());		break;
					case BUO_PARAM_PATH:				m_options.backup().setPath(value.toString());		break;
					default:							assert(0);
				}
			}
			break;

		default:
			assert(nullptr);
			break;
	}

	m_currentPropertyItem = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::updateServerPage()
{
	QtVariantProperty *property = nullptr;

	property = (QtVariantProperty*) m_propertyItemList.key((OPTION_PAGE_CONFIG_SOCKET << 8) | SOCKET_CLIENT_PARAM_EQUIPMENT_ID2);
	if (property != nullptr)
	{
		property->setValue(m_options.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::updateLinearityPage(bool isDialog)
{
	PropertyPage* page = m_pageList[OPTION_PAGE_LINEARITY_POINT];
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
		m_options.setLinearity(dialog->m_linearity);
	}
	else
	{
		// set options to dialog
		//
		dialog->m_linearity = m_options.linearity();
	}

	QtVariantProperty *property = nullptr;

	property = (QtVariantProperty*) m_propertyItemList.key((OPTION_PAGE_LINEARITY_MEASURE << 8) | LO_PARAM_RANGE_TYPE);
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().rangeType());
	}

	property = (QtVariantProperty*) m_propertyItemList.key((OPTION_PAGE_LINEARITY_MEASURE << 8) | LO_PARAM_POINT_COUNT);
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().points().count());

		switch(m_options.linearity().rangeType())
		{
			case LO_RANGE_TYPE_MANUAL:		property->setEnabled(false);	break;
			case LO_RANGE_TYPE_AUTOMATIC:	property->setEnabled(true);		break;
			default:						assert(0);
		}
	}

	property = (QtVariantProperty*) m_propertyItemList.key((OPTION_PAGE_LINEARITY_MEASURE << 8) | LO_PARAM_LOW_RANGE);
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().lowLimitRange());

		switch(m_options.linearity().rangeType())
		{
			case LO_RANGE_TYPE_MANUAL:		property->setEnabled(false);	break;
			case LO_RANGE_TYPE_AUTOMATIC:	property->setEnabled(true);		break;
			default:						assert(0);
		}
	}

	property = (QtVariantProperty*) m_propertyItemList.key((OPTION_PAGE_LINEARITY_MEASURE << 8) | LO_PARAM_HIGH_RANGE);
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().highLimitRange());

		switch(m_options.linearity().rangeType())
		{
			case LO_RANGE_TYPE_MANUAL:		property->setEnabled(false);	break;
			case LO_RANGE_TYPE_AUTOMATIC:	property->setEnabled(true);		break;
			default:						assert(0);
		}

	}

	property = (QtVariantProperty*) m_propertyItemList.key((OPTION_PAGE_LINEARITY_MEASURE << 8) | LO_PARAM_VALUE_POINTS);
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().points().text());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::updateMeasureViewPage(bool isDialog)
{
	PropertyPage* page = m_pageList[OPTION_PAGE_MEASURE_VIEW_COLUMN];
	if (page == nullptr)
	{
		return;
	}

	OptionsMeasureViewHeaderDialog* dialog = (OptionsMeasureViewHeaderDialog*) page->getWidget();
	if (dialog == nullptr)
	{
		return;
	}

	if (isDialog == true)
	{
		// get options from dialog
		//
		m_options.setMeasureView(dialog->m_header);

		int measureType = dialog->measureType();
		if (measureType >= 0 || measureType < MEASURE_TYPE_COUNT)
		{
			m_options.m_updateColumnView[measureType] = true;
		}
	}
	else
	{
		// set options to dialog
		//
		dialog->m_header = m_options.measureView();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::loadSettings()
{
	QSettings s;

	QByteArray geometry = s.value(QString("%1OptionsDialog/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();
	restoreGeometry(geometry);

	m_activePage = s.value(QString("%1OptionsDialog/activePage").arg(WINDOW_GEOMETRY_OPTIONS_KEY), OPTION_PAGE_LINEARITY_MEASURE).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void OptionsDialog::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1OptionsDialog/Geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveGeometry());
	s.setValue(QString("%1OptionsDialog/activePage").arg(WINDOW_GEOMETRY_OPTIONS_KEY), m_activePage);
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
	if (e->type() == QEvent::Hide)
	{
		saveSettings();
	}

	if (e->type() == QEvent::KeyRelease)
	{
		if (m_activePage >= 0 && m_activePage < m_pageList.count())
		{
			PropertyPage* pActivePage = m_pageList.at(m_activePage);
			if (pActivePage != nullptr)
			{
				if (pActivePage->type() == PROPERTY_PAGE_TYPE_LIST)
				{
					QKeyEvent* keyEvent = static_cast<QKeyEvent *>(e);

					if (keyEvent->key() == Qt::Key_Return)
					{
						applyProperty();
					}

					if (keyEvent->key() == Qt::Key_Escape)
					{
						restoreProperty();
					}
				}
			}
		}

	}

	return QDialog::event(e);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


