#include "DialogOptions.h"

#include <QApplication>
#include <QScreen>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <assert.h>

#include "FolderPropertyManager.h"
#include "DialogMeasurePoint.h"
#include "DialogOptionsMvh.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PropertyPage::PropertyPage(PropertyPageType pageType, QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor)
	: m_baseWidget(editor)
	, m_widgetType(PropertyPageWidgetType::List)
	, m_pageType(pageType)
{
	//
	//
	m_pManager = manager;
	m_pFactory = factory;
	m_pEditor = editor;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::PropertyPage(PropertyPageType pageType, QDialog* dialog)
	: m_baseWidget(dialog)
	, m_widgetType(PropertyPageWidgetType::Dialog)
	, m_pageType(pageType)
{
	//
	//
	m_pDialog = dialog;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::~PropertyPage()
{
	switch(m_widgetType)
	{
		case PropertyPageWidgetType::List:

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

		case PropertyPageWidgetType::Dialog:

			if (m_pDialog != nullptr)
			{
				delete m_pDialog;
				m_pDialog = nullptr;
			}

			break;

		default:
			assert(0);
	}

	m_widgetType = PropertyPageWidgetType::NoWidgetType;
	m_pageType = PropertyPageType::NoPageType;

	m_baseWidget = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

QString groupCaption(PropertyGroupType groupType)
{
	QString caption;

	switch (groupType)
	{
		case PropertyGroupType::Server:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Connect to server");		break;
		case PropertyGroupType::Module:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Module");					break;
		case PropertyGroupType::Linearity:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Linearity");				break;
		case PropertyGroupType::Comparator:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Comparators");			break;
		case PropertyGroupType::MeasureView:	caption = QT_TRANSLATE_NOOP("DialogOptions", "List of measurements");	break;
		case PropertyGroupType::PanelInfo:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Panels information");		break;
		case PropertyGroupType::Database:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Database");				break;
		case PropertyGroupType::Language:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Language");				break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogOptions", "Unknown");
	}

	return qApp->translate("DialogOptions", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

QString pageCaption(PropertyPageType pageType)
{
	QString caption;

	switch (pageType)
	{
		case PropertyPageType::Socket_CfgSrv:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection to Config Server - TCP/IP");			break;
		case PropertyPageType::Socket_AppDataSrv:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection to Application Data Server - TCP/IP");	break;
		case PropertyPageType::Socket_TuningSrv:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection to Tuning Server - TCP/IP");			break;
		case PropertyPageType::Module_Measure:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Measuring of module");							break;
		case PropertyPageType::Linearity_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measurements of linearity");						break;
		case PropertyPageType::Linearity_Point:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Point of linearity");								break;
		case PropertyPageType::Comparator_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measure comparators");							break;
		case PropertyPageType::MeasureView_Text:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying data in the list of measurements");	break;
		case PropertyPageType::MeasureView_Column:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying columns in the list of measurements");	break;
		case PropertyPageType::Panel_SignalInfo:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying information of signals");				break;
		case PropertyPageType::Panel_ComparatorInfo:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying information of сomparators");			break;
		case PropertyPageType::Database_Location:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Database location");								break;
		case PropertyPageType::Database_Backup:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Database backup");								break;
		case PropertyPageType::Language_App:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Language of application");						break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogOptions", "Unknown");
	}

	return qApp->translate("DialogOptions", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

QString pageShortCaption(PropertyPageType pageType)
{
	QString caption;

	switch (pageType)
	{
		case PropertyPageType::Socket_CfgSrv:			caption = QT_TRANSLATE_NOOP("DialogOptions", "ConfigurationService");		break;
		case PropertyPageType::Socket_AppDataSrv:		caption = QT_TRANSLATE_NOOP("DialogOptions", "AppDataService");				break;
		case PropertyPageType::Socket_TuningSrv:		caption = QT_TRANSLATE_NOOP("DialogOptions", "TuningService");				break;
		case PropertyPageType::Module_Measure:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Measuring");					break;
		case PropertyPageType::Linearity_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measurements");				break;
		case PropertyPageType::Linearity_Point:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Points");						break;
		case PropertyPageType::Comparator_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measurements");				break;
		case PropertyPageType::MeasureView_Text:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying");					break;
		case PropertyPageType::MeasureView_Column:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Columns");					break;
		case PropertyPageType::Panel_SignalInfo:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Signal information");			break;
		case PropertyPageType::Panel_ComparatorInfo:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Comparator information");		break;
		case PropertyPageType::Database_Location:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Location");					break;
		case PropertyPageType::Database_Backup:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Backup");						break;
		case PropertyPageType::Language_App:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Language of application ");	break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogOptions", "Unknown");
	}

	return qApp->translate("DialogOptions", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

PropertyGroupType groupByPage(PropertyPageType pageType)
{
	PropertyGroupType group = PropertyGroupType::NoGroupType;

	switch (pageType)
	{
		case PropertyPageType::Socket_CfgSrv:			group = PropertyGroupType::Server;		break;
		case PropertyPageType::Socket_AppDataSrv:		group = PropertyGroupType::Server;		break;
		case PropertyPageType::Socket_TuningSrv:		group = PropertyGroupType::Server;		break;
		case PropertyPageType::Module_Measure:			group = PropertyGroupType::Module;		break;
		case PropertyPageType::Linearity_Measure:		group = PropertyGroupType::Linearity;	break;
		case PropertyPageType::Linearity_Point:			group = PropertyGroupType::Linearity;	break;
		case PropertyPageType::Comparator_Measure:		group = PropertyGroupType::Comparator;	break;
		case PropertyPageType::MeasureView_Text:		group = PropertyGroupType::MeasureView;	break;
		case PropertyPageType::MeasureView_Column:		group = PropertyGroupType::MeasureView;	break;
		case PropertyPageType::Panel_SignalInfo:		group = PropertyGroupType::PanelInfo;	break;
		case PropertyPageType::Panel_ComparatorInfo:	group = PropertyGroupType::PanelInfo;	break;
		case PropertyPageType::Database_Location:		group = PropertyGroupType::Database;	break;
		case PropertyPageType::Database_Backup:			group = PropertyGroupType::Database;	break;
		case PropertyPageType::Language_App:			group = PropertyGroupType::Language;	break;

		default:
			assert(0);
			group = PropertyGroupType::NoGroupType;
	}

	return group;
};


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PropertyPageType DialogOptions::m_activePage = PropertyPageType::Linearity_Measure;

// -------------------------------------------------------------------------------------------------------------------

DialogOptions::DialogOptions(const Options& options, QWidget* parent) :
	QDialog(parent),
	m_options(options)
{
	for(int measureType = 0; measureType < Measure::TypeCount; measureType++)
	{
		m_options.measureView().setUpdateColumnView(static_cast<Measure::Type>(measureType), false);
	}

	createInterface();
}

// -------------------------------------------------------------------------------------------------------------------

DialogOptions::~DialogOptions()
{
	removePropertyPages();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	setMinimumSize(static_cast<int>(screen.width() * 0.45), static_cast<int>(screen.height() * 0.3));
	loadSettings();

	// create property pages
	//
	for(int pageIndex = 0; pageIndex < PropertyPageTypeCount ; pageIndex++)
	{
		PropertyPageType pageType = static_cast<PropertyPageType>(pageIndex);
		if (ERR_PROPERTY_PAGE_TYPE(pageType))
		{
			continue;
		}

		std::shared_ptr<PropertyPage> pPropertyPage = createPropertyPage(pageType);
		if (pPropertyPage.get() == nullptr)
		{
			assert(0);
		}

		m_pagesList.push_back(pPropertyPage);
	}

	// create interface
	//
	m_pagesTree = new QTreeWidget;
	m_pagesTree->setHeaderHidden(true);
	m_pagesTree->setFixedWidth(static_cast<int>(screen.width() * 0.1));

	m_pagesLayout = new QHBoxLayout ;

	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	m_pPropertyEditor->setSplitterPosition(300);
	m_pPropertyEditor->setReadOnly(true);

	connect(m_pPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogOptions::onPropertyValueChanged_1);

	//
	createPropertyPages();

	// buttonsLayout
	//
	QHBoxLayout* buttonsLayout = new QHBoxLayout ;

	QPushButton* okButton = new QPushButton(tr("Ok"));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	connect(okButton, &QPushButton::clicked, this, &DialogOptions::onOk);
	connect(cancelButton, &QPushButton::clicked, this, &DialogOptions::reject);

	// mainLayout
	//
	QVBoxLayout* mainLayout = new QVBoxLayout;

	mainLayout->addLayout(m_pagesLayout);
	mainLayout->addLayout(buttonsLayout);

	setLayout(mainLayout);

	// set active page
	//
	setActivePage(m_activePage);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::createPropertyPages()
{
	if (m_pagesTree == nullptr)
	{
		return;
	}

	if (m_pagesLayout == nullptr)
	{
		return;
	}

	// create tree of groups
	//
	std::vector<QTreeWidgetItem*> groupList;

	for(int group = 0; group < PropertyGroupTypeCount; group++)
	{
		QTreeWidgetItem* groupTreeItem = new QTreeWidgetItem;
		groupTreeItem->setText(0, groupCaption(static_cast<PropertyGroupType>(group)));
		m_pagesTree->addTopLevelItem(groupTreeItem);

		groupList.push_back(groupTreeItem);
	}

	// create tree of pages
	//
	for(std::shared_ptr<PropertyPage> pPropertyPage : m_pagesList)
	{
		// get page
		//
		if (pPropertyPage.get() == nullptr)
		{
			assert(0);
			continue;
		}

		PropertyPageType pageType = pPropertyPage->pageType();
		if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
		{
			assert(0);
			continue;
		}

		// get group
		//
		PropertyGroupType groupType = groupByPage(pageType);
		if (ERR_PROPERTY_GROUP_TYPE(groupType))
		{
			assert(0);
			continue;
		}

		if (groupType >= TO_INT(groupList.size()))
		{
			assert(0);
			continue;
		}

		QTreeWidgetItem* groupTreeItem = groupList.at(static_cast<quint64>(groupType));

		QTreeWidgetItem* pageTreeItem = new QTreeWidgetItem;
		pageTreeItem->setText(0, pageShortCaption(pageType));
		pageTreeItem->setData(0, Qt::UserRole, pageType);

		groupTreeItem->addChild(pageTreeItem);

		pPropertyPage->setPageTreeItem(pageTreeItem);
	}

	connect(m_pagesTree, &QTreeWidget::currentItemChanged , this, &DialogOptions::onPageChanged);

	m_pagesLayout->addWidget(m_pagesTree);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::removePropertyPages()
{
	m_propertyItemList.clear();
	m_propertyValueList.clear();
	m_pagesList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

std::shared_ptr<PropertyPage> DialogOptions::createPropertyPage(PropertyPageType pageType)
{
	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return nullptr;
	}

	std::shared_ptr<PropertyPage> pPropertyPage;

	switch (pageType)
	{
		case PropertyPageType::Socket_CfgSrv:
		case PropertyPageType::Socket_AppDataSrv:
		case PropertyPageType::Socket_TuningSrv:
		case PropertyPageType::Linearity_Measure:
		case PropertyPageType::Comparator_Measure:
		case PropertyPageType::Module_Measure:
		case PropertyPageType::MeasureView_Text:
		case PropertyPageType::Panel_SignalInfo:
		case PropertyPageType::Panel_ComparatorInfo:
		case PropertyPageType::Database_Location:
		case PropertyPageType::Database_Backup:
		case PropertyPageType::Language_App:			pPropertyPage = createPropertyPageList(pageType);	break;
		case PropertyPageType::Linearity_Point:
		case PropertyPageType::MeasureView_Column:		pPropertyPage = createPropertyPageDialog(pageType);	break;

		default:
			assert(nullptr);
	}

	return pPropertyPage;
}

// -------------------------------------------------------------------------------------------------------------------

std::shared_ptr<PropertyPage> DialogOptions::createPropertyPageList(PropertyPageType pageType)
{
	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return nullptr;
	}

	QtVariantProperty* item = nullptr;


	QtVariantPropertyManager* manager = new VariantManager();
	QtVariantEditorFactory* factory = new VariantFactory();
	QtTreePropertyBrowser* editor = new QtTreePropertyBrowser;

	switch (pageType)
	{
		case PropertyPageType::Socket_CfgSrv:
		case PropertyPageType::Socket_AppDataSrv:
		case PropertyPageType::Socket_TuningSrv:
			{
				SocketType socketType = SocketType::NoSocketType;

				switch (pageType)
				{
					case PropertyPageType::Socket_CfgSrv:		socketType = SocketType::CfgSrv;	break;
					case PropertyPageType::Socket_AppDataSrv:	socketType = SocketType::AppDataSrv;	break;
					case PropertyPageType::Socket_TuningSrv:	socketType = SocketType::TuningSrv;	break;

					default:
						assert(0);
						socketType = SocketType::NoSocketType;
				}

				if (ERR_SOCKET_TYPE(socketType) == true)
				{
					break;
				}

				SocketClientOption sco = m_options.socket().client(socketType);

				QtProperty* serverGroup1 = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Server (primary)"));

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_EQUIPMENT_ID1]));
					item->setValue(sco.equipmentID(ServerType::Primary));
					appendProperty(item, pageType, SOCKET_CLIENT_PARAM_EQUIPMENT_ID1);
					serverGroup1->addSubProperty(item);

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_IP1]));
					item->setValue(sco.serverIP(ServerType::Primary));
					appendProperty(item, pageType, SOCKET_CLIENT_PARAM_SERVER_IP1);
					serverGroup1->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_PORT1]));
					item->setValue(sco.serverPort(ServerType::Primary));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 65535);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, pageType, SOCKET_CLIENT_PARAM_SERVER_PORT1);
					serverGroup1->addSubProperty(item);

				QtProperty* serverGroup2 = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Server (reserve)"));

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_EQUIPMENT_ID2]));
					item->setValue(sco.equipmentID(ServerType::Reserve));
					appendProperty(item, pageType, SOCKET_CLIENT_PARAM_EQUIPMENT_ID2);
					serverGroup2->addSubProperty(item);

					if (socketType == SocketType::CfgSrv)
					{
						item->setAttribute(QLatin1String("readOnly"), true);
					}

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_IP2]));
					item->setValue(sco.serverIP(ServerType::Reserve));
					appendProperty(item, pageType, SOCKET_CLIENT_PARAM_SERVER_IP2);
					serverGroup2->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_PORT2]));
					item->setValue(sco.serverPort(ServerType::Reserve));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 65535);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, pageType, SOCKET_CLIENT_PARAM_SERVER_PORT2);
					serverGroup2->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(serverGroup1);

				if (socketType == SocketType::AppDataSrv)
				{
					editor->addProperty(serverGroup2);
				}

			}
			break;

		case PropertyPageType::Module_Measure:
			{
				QtProperty* identificationGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Identification of module"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), qApp->translate("Options.h", ModuleParamName[MO_PARAM_SUFFIX_SN]));
					item->setValue(m_options.module().suffixSN());
					appendProperty(item, pageType, MO_PARAM_SUFFIX_SN);
					identificationGroup->addSubProperty(item);

				QtProperty* measuremoduleGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measuring of module"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_INT_INSTEAD_IN]));
					item->setValue(m_options.module().measureInterInsteadIn());
					appendProperty(item, pageType, MO_PARAM_MEASURE_INT_INSTEAD_IN);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_LIN_AND_CMP]));
					item->setValue(m_options.module().measureLinAndCmp());
					appendProperty(item, pageType, MO_PARAM_MEASURE_LIN_AND_CMP);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_ENTIRE_MODULE]));
					item->setValue(m_options.module().measureEntireModule());
					appendProperty(item, pageType, MO_PARAM_MEASURE_ENTIRE_MODULE);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_SHOWN_ON_SCHEMAS]));
					item->setValue(m_options.module().measureShownOnSchemas());
					appendProperty(item, pageType, MO_PARAM_MEASURE_SHOWN_ON_SCHEMAS);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_WARN_IF_MEASURED]));
					item->setValue(m_options.module().warningIfMeasured());
					appendProperty(item, pageType, MO_PARAM_WARN_IF_MEASURED);
					measuremoduleGroup->addSubProperty(item);

				QtProperty* limitsGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Limits"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MAX_IMPUT_COUNT]));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setValue(m_options.module().maxInputCount());
					appendProperty(item, pageType, MO_PARAM_MAX_IMPUT_COUNT);
					limitsGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(identificationGroup);
				editor->addProperty(measuremoduleGroup);
				editor->addProperty(limitsGroup);
			}
			break;

		case PropertyPageType::Linearity_Measure:
			{
				QtProperty* errorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", LinearityParamName[LO_PARAM_ERROR_LIMIT]));
					item->setValue(m_options.linearity().errorLimit());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, pageType, LO_PARAM_ERROR_LIMIT);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", LinearityParamName[LO_PARAM_ERROR_TYPE]));
					QStringList errorTypeList;
					for(int e = 0; e < Measure::ErrorTypeCount; e++)
					{
						errorTypeList.append(qApp->translate("MeasureBase", Measure::ErrorTypeCaption(static_cast<Measure::ErrorType>(e)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), errorTypeList);
					item->setValue(m_options.linearity().errorType());
					appendProperty(item, pageType, LO_PARAM_ERROR_TYPE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", LinearityParamName[LO_PARAM_CALC_ERROR_BY_RANGE]));
					QStringList showErrorFromLimitList;
					for(int t = 0; t < Measure::CalcErrorRangeCount; t++)
					{
						showErrorFromLimitList.append(qApp->translate("MeasureBase", Measure::CalcErrorRangeCaption(static_cast<Measure::CalcErrorRange>(t)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), showErrorFromLimitList);
					item->setValue(m_options.linearity().calcErrorByRange());
					appendProperty(item, pageType, LO_PARAM_CALC_ERROR_BY_RANGE);
					errorGroup->addSubProperty(item);

				QtProperty* measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurements at the single point"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", LinearityParamName[LO_PARAM_MEASURE_TIME]));
					item->setValue(m_options.linearity().measureTimeInPoint());
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 60);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, pageType, LO_PARAM_MEASURE_TIME);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", LinearityParamName[LO_PARAM_MEASURE_IN_POINT]));
					item->setValue(m_options.linearity().measureCountInPoint());
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), Measure::MaxMeasurementInPoint);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, pageType, LO_PARAM_MEASURE_IN_POINT);
					measureGroup->addSubProperty(item);

				QtProperty* pointGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurement points"));

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", LinearityParamName[LO_PARAM_RANGE_TYPE]));
					QStringList rangeTypeList;
					for(int r = 0; r < Measure::LinearityDivisionCount; r++)
					{
						rangeTypeList.append(qApp->translate("MeasurePointBase", Measure::LinearityDivisionCaption(r).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), rangeTypeList);
					item->setValue(m_options.linearity().divisionType());
					appendProperty(item, pageType, LO_PARAM_RANGE_TYPE);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", LinearityParamName[LO_PARAM_POINT_COUNT]));
					item->setValue(m_options.linearity().points().count());
					switch(m_options.linearity().divisionType())
					{
						case Measure::LinearityDivision::Manual:	item->setEnabled(false);	break;
						case Measure::LinearityDivision::Automatic:	item->setEnabled(true);		break;

						default:
							assert(0);
					}
					appendProperty(item, pageType, LO_PARAM_POINT_COUNT);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", LinearityParamName[LO_PARAM_LOW_RANGE]));
					item->setValue(m_options.linearity().lowLimitRange());
					item->setAttribute(QLatin1String("singleStep"), 1);
					item->setAttribute(QLatin1String("decimals"), 1);
					switch(m_options.linearity().divisionType())
					{
						case Measure::LinearityDivision::Manual:	item->setEnabled(false);	break;
						case Measure::LinearityDivision::Automatic:	item->setEnabled(true);		break;

						default:
							assert(0);
					}
					appendProperty(item, pageType, LO_PARAM_LOW_RANGE);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", LinearityParamName[LO_PARAM_HIGH_RANGE]));
					item->setValue(m_options.linearity().highLimitRange());
					item->setAttribute(QLatin1String("singleStep"), 1);
					item->setAttribute(QLatin1String("decimals"), 1);
					switch(m_options.linearity().divisionType())
					{
						case Measure::LinearityDivision::Manual:	item->setEnabled(false);	break;
						case Measure::LinearityDivision::Automatic:	item->setEnabled(true);		break;

						default:
							assert(0);
					}
					appendProperty(item, pageType, LO_PARAM_HIGH_RANGE);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", LinearityParamName[LO_PARAM_VALUE_POINTS]));
					item->setValue(qApp->translate("Options.cpp", m_options.linearity().points().text().toUtf8()));
					appendProperty(item, pageType, LO_PARAM_VALUE_POINTS);
					pointGroup->addSubProperty(item);


				QtProperty* showcolumnGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Type of displaying measurement list"));

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", LinearityParamName[LO_PARAM_LIST_TYPE]));
					QStringList listTypeList;
					for(int r = 0; r < LinearityViewTypeCount; r++)
					{
						listTypeList.append(qApp->translate("Options", LinearityViewTypeCaption(r).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), listTypeList);
					item->setValue(m_options.linearity().viewType());
					appendProperty(item, pageType, LO_PARAM_LIST_TYPE);
					showcolumnGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(errorGroup);
				editor->addProperty(measureGroup);
				editor->addProperty(pointGroup);
				editor->addProperty(showcolumnGroup);
			}
			break;

		case PropertyPageType::Comparator_Measure:
			{
				QtProperty* errorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_ERROR_LIMIT]));
					item->setValue(m_options.comparator().errorLimit());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, pageType, CO_PARAM_ERROR_LIMIT);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", ComparatorParamName[CO_PARAM_ERROR_TYPE]));
					QStringList errorTypeList;
					for(int e = 0; e < Measure::ErrorTypeCount; e++)
					{
						errorTypeList.append(qApp->translate("MeasureBase", Measure::ErrorTypeCaption(static_cast<Measure::ErrorType>(e)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), errorTypeList);
					item->setValue(m_options.comparator().errorType());
					appendProperty(item, pageType, CO_PARAM_ERROR_TYPE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", ComparatorParamName[CO_PARAM_CALC_ERROR_BY_RANGE]));
					QStringList showErrorFromLimitList;
					for(int t = 0; t < Measure::CalcErrorRangeCount; t++)
					{
						showErrorFromLimitList.append(qApp->translate("MeasureBase", Measure::CalcErrorRangeCaption(static_cast<Measure::CalcErrorRange>(t)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), showErrorFromLimitList);
					item->setValue(m_options.comparator().calcErrorByRange());
					appendProperty(item, pageType, CO_PARAM_CALC_ERROR_BY_RANGE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_START_VALUE]));
					item->setValue(m_options.comparator().startValueForCompare());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, pageType, CO_PARAM_START_VALUE);
					errorGroup->addSubProperty(item);

				QtProperty* permissionsGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Permissions"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_COMPARATOR_INDEX]));
					item->setValue(m_options.comparator().startComparatorIndex() + 1);
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, pageType, CO_PARAM_COMPARATOR_INDEX);
					permissionsGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_ENABLE_HYSTERESIS]));
					item->setValue(m_options.comparator().enableMeasureHysteresis());
					appendProperty(item, pageType, CO_PARAM_ENABLE_HYSTERESIS);
					permissionsGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(errorGroup);
				editor->addProperty(permissionsGroup);
			}
			break;

		case PropertyPageType::MeasureView_Text:
			{
				QtProperty* fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_FONT]));
					item->setValue(m_options.measureView().font().toString());
					appendProperty(item, pageType, MWO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty* colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_COLOR_NOT_ERROR]));
					item->setValue(m_options.measureView().colorNotError());
					appendProperty(item, pageType, MWO_PARAM_COLOR_NOT_ERROR);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_COLOR_LIMIT_ERROR]));
					item->setValue(m_options.measureView().colorErrorLimit());
					appendProperty(item, pageType, MWO_PARAM_COLOR_LIMIT_ERROR);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_COLOR_CONTROL_ERROR]));
					item->setValue(m_options.measureView().colorErrorControl());
					appendProperty(item, pageType, MWO_PARAM_COLOR_CONTROL_ERROR);
					colorGroup->addSubProperty(item);

				QtProperty* measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurements"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_SHOW_NO_VALID]));
					item->setValue(m_options.measureView().showNoValid());
					appendProperty(item, pageType, MWO_PARAM_SHOW_NO_VALID);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_PRECESION_BY_CALIBRATOR]));
					item->setValue(m_options.measureView().precesionByCalibrator());
					appendProperty(item, pageType, MWO_PARAM_PRECESION_BY_CALIBRATOR);
					measureGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(colorGroup);
				editor->addProperty(measureGroup);

				expandProperty(editor, PropertyPageType::MeasureView_Text, MWO_PARAM_FONT, false);
				expandProperty(editor, PropertyPageType::MeasureView_Text, MWO_PARAM_COLOR_NOT_ERROR, false);
				expandProperty(editor, PropertyPageType::MeasureView_Text, MWO_PARAM_COLOR_LIMIT_ERROR, false);
				expandProperty(editor, PropertyPageType::MeasureView_Text, MWO_PARAM_COLOR_CONTROL_ERROR, false);
			}
			break;

		case PropertyPageType::Panel_SignalInfo:
			{
				QtProperty* fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_FONT]));
					item->setValue(m_options.signalInfo().font().toString());
					appendProperty(item, pageType, SIO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty* measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Displaying signal state"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_SHOW_NO_VALID]));
					item->setValue(m_options.signalInfo().showNoValid());
					appendProperty(item, pageType, SIO_PARAM_SHOW_NO_VALID);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_ELECTRIC_STATE]));
					item->setValue(m_options.signalInfo().showElectricState());
					appendProperty(item, pageType, SIO_PARAM_ELECTRIC_STATE);
					measureGroup->addSubProperty(item);

				QtProperty* colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_VALID]));
					item->setValue(m_options.signalInfo().colorFlagValid());
					appendProperty(item, pageType, SIO_PARAM_COLOR_FLAG_VALID);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_SIM]));
					item->setValue(m_options.signalInfo().colorFlagSim());
					appendProperty(item, pageType, SIO_PARAM_COLOR_FLAG_SIM);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_LOCK]));
					item->setValue(m_options.signalInfo().colorFlagLock());
					appendProperty(item, pageType, SIO_PARAM_COLOR_FLAG_LOCK);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_OVERFLOW]));
					item->setValue(m_options.signalInfo().colorFlagOverflow());
					appendProperty(item, pageType, SIO_PARAM_COLOR_FLAG_OVERFLOW);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_UNDERFLOW]));
					item->setValue(m_options.signalInfo().colorFlagUnderflow());
					appendProperty(item, pageType, SIO_PARAM_COLOR_FLAG_UNDERFLOW);
					colorGroup->addSubProperty(item);

				QtProperty* timeGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Time for updating"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_TIME_FOR_UPDATE]));
					item->setValue(m_options.signalInfo().timeForUpdate());
					appendProperty(item, pageType, SIO_PARAM_TIME_FOR_UPDATE);
					timeGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(measureGroup);
				editor->addProperty(colorGroup);
				editor->addProperty(timeGroup);

				expandProperty(editor, PropertyPageType::Panel_SignalInfo, SIO_PARAM_FONT, false);
				expandProperty(editor, PropertyPageType::Panel_SignalInfo, SIO_PARAM_COLOR_FLAG_VALID, false);
				expandProperty(editor, PropertyPageType::Panel_SignalInfo, SIO_PARAM_COLOR_FLAG_OVERFLOW, false);
				expandProperty(editor, PropertyPageType::Panel_SignalInfo, SIO_PARAM_COLOR_FLAG_UNDERFLOW, false);
			}

			break;

		case PropertyPageType::Panel_ComparatorInfo:
			{
				QtProperty* fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_FONT]));
					item->setValue(m_options.comparatorInfo().font().toString());
					appendProperty(item, pageType, CIO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty* colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_FLAG_SIM]));
					item->setValue(m_options.comparatorInfo().colorFlagSim());
					appendProperty(item, pageType, CIO_PARAM_COLOR_FLAG_SIM);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_FLAG_LOCK]));
					item->setValue(m_options.comparatorInfo().colorFlagLock());
					appendProperty(item, pageType, CIO_PARAM_COLOR_FLAG_LOCK);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_STATE_FALSE]));
					item->setValue(m_options.comparatorInfo().colorStateFalse());
					appendProperty(item, pageType, CIO_PARAM_COLOR_STATE_FALSE);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_STATE_TRUE]));
					item->setValue(m_options.comparatorInfo().colorStateTrue());
					appendProperty(item, pageType, CIO_PARAM_COLOR_STATE_TRUE);
					colorGroup->addSubProperty(item);

				QtProperty* timeGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Time for updating"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_TIME_FOR_UPDATE]));
					item->setValue(m_options.comparatorInfo().timeForUpdate());
					appendProperty(item, pageType, CIO_PARAM_TIME_FOR_UPDATE);
					timeGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(colorGroup);
				editor->addProperty(timeGroup);

				expandProperty(editor, PropertyPageType::Panel_ComparatorInfo, CIO_PARAM_FONT, false);
				expandProperty(editor, PropertyPageType::Panel_ComparatorInfo, CIO_PARAM_COLOR_FLAG_SIM, false);
				expandProperty(editor, PropertyPageType::Panel_ComparatorInfo, CIO_PARAM_COLOR_FLAG_LOCK, false);
				expandProperty(editor, PropertyPageType::Panel_ComparatorInfo, CIO_PARAM_COLOR_STATE_FALSE, false);
				expandProperty(editor, PropertyPageType::Panel_ComparatorInfo, CIO_PARAM_COLOR_STATE_TRUE, false);
			}

			break;

		case PropertyPageType::Database_Location:
			{
				QtProperty* databaseGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Location of Database"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), qApp->translate("Options.h", DatabaseParam[DBO_PARAM_LOCATION_PATH]));
					item->setValue(m_options.database().locationPath());
					appendProperty(item, pageType, DBO_PARAM_LOCATION_PATH);
					databaseGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", DatabaseParam[DBO_PARAM_TYPE]));
					QStringList valueTypeList;
					for(int t = 0; t < DATABASE_TYPE_COUNT; t++)
					{
						valueTypeList.append(DatabaseType[t]);
					}
					item->setAttribute(QLatin1String("enumNames"), valueTypeList);
					item->setValue(m_options.database().type());
					appendProperty(item, pageType, DBO_PARAM_TYPE);
					databaseGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(databaseGroup);
			}
			break;

		case PropertyPageType::Database_Backup:
			{
				QtProperty* eventGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Events"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", DatabaseParam[DBO_PARAM_ON_START]));
					item->setValue(m_options.database().onStart());
					appendProperty(item, pageType, DBO_PARAM_ON_START);
					eventGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", DatabaseParam[DBO_PARAM_ON_EXIT]));
					item->setValue(m_options.database().onExit());
					appendProperty(item, pageType, DBO_PARAM_ON_EXIT);
					eventGroup->addSubProperty(item);

				QtProperty* pathGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Location of reserve copy"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), qApp->translate("Options.h", DatabaseParam[DBO_PARAM_COPY_PATH]));
					item->setValue(m_options.database().backupPath());
					appendProperty(item, pageType, DBO_PARAM_COPY_PATH);
					pathGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(eventGroup);
				editor->addProperty(pathGroup);
			}
			break;

		case PropertyPageType::Language_App:
			{
				QtProperty* languageGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Language of application "));

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", LanguageParam[LNO_PARAM_LANGUAGE_TYPE]));
					QStringList valueTypeList;
					for(int t = 0; t < LanguageTypeCount; t++)
					{
						valueTypeList.append(qApp->translate("Options", LanguageTypeCaption(static_cast<LanguageType>(t)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), valueTypeList);
					item->setValue(m_options.language().languageType());
					appendProperty(item, pageType, LNO_PARAM_LANGUAGE_TYPE);
					languageGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(languageGroup);
			}
			break;

		default:
			assert(nullptr);
	}

	editor->setPropertiesWithoutValueMarked(true);
	editor->setRootIsDecorated(false);

	connect(manager, &QtVariantPropertyManager::valueChanged, this, &DialogOptions::onPropertyValueChanged);
	connect(editor, &QtTreePropertyBrowser::currentItemChanged, this, &DialogOptions::onBrowserItem);

	//	create PropertyPage - PropertyPageWidgetType::List
	//
	return std::make_shared<PropertyPage>(pageType, manager, factory, editor);
}

// -------------------------------------------------------------------------------------------------------------------

std::shared_ptr<PropertyPage> DialogOptions::createPropertyPageDialog(PropertyPageType pageType)
{
	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return nullptr;
	}

	QDialog* pDialogPage = nullptr;

	switch (pageType)
	{
		case PropertyPageType::Linearity_Point:
			{
				DialogMeasurePoint* dialog = new DialogMeasurePoint(m_options.linearity());
				connect(dialog, &DialogMeasurePoint::updateLinearityPage, this, &DialogOptions::updateLinearityPage);

				pDialogPage = dialog;
			}
			break;

		case PropertyPageType::MeasureView_Column:
			{
				DialogOptionsMeasureViewHeader* dialog = new DialogOptionsMeasureViewHeader(m_options.measureView());
				connect(dialog, &DialogOptionsMeasureViewHeader::updateMeasureViewPage, this, &DialogOptions::updateMeasureViewPage);

				pDialogPage = dialog;
			}
			break;

		default:
			assert(nullptr);
	}

	if (pDialogPage != nullptr)
	{
		pDialogPage->setWindowTitle(pageCaption(pageType));
	}

	//	create PropertyPage - PropertyPageWidgetType::Dialog
	//
	return std::make_shared<PropertyPage>(pageType, pDialogPage);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::appendProperty(QtProperty* property, PropertyPageType pageType, int param)
{
	if (property == nullptr)
	{
		return;
	}

	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return;
	}

	m_propertyItemList[property] = (pageType << 8) | param;
	m_propertyValueList[property] = dynamic_cast<QtVariantProperty*>(property)->value();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::expandProperty(QtTreePropertyBrowser* pEditor, PropertyPageType pageType, int param, bool expanded)
{
	if (pEditor == nullptr)
	{
		return;
	}

	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return;
	}

	QtProperty* pProperty = m_propertyItemList.key((pageType << 8) | param);
	if (pProperty == nullptr)
	{
		return;
	}

	QtBrowserItem* pItem = pEditor->items(pProperty).at(0);
	if (pItem == nullptr)
	{
		return;
	}

	pEditor->setExpanded(pItem, expanded);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::onPageChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
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
	if (page < 0 || page >= TO_INT(m_pagesList.size()))
	{
		return;
	}

	if (ERR_PROPERTY_PAGE_TYPE(page) == true)
	{
		return;
	}

	setActivePage(static_cast<PropertyPageType>(page));
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::setActivePage(PropertyPageType pageType)
{
	if (m_pagesLayout == nullptr)
	{
		return false;
	}

	if (m_pPropertyEditor == nullptr)
	{
		return false;
	}

	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return false;
	}

	if (pageType < 0 || pageType >= TO_INT(m_pagesList.size()))
	{
		return false;
	}

	// hide current page
	//
	if (m_activePage >= 0 && m_activePage < TO_INT(m_pagesList.size()))
	{
		std::shared_ptr<PropertyPage> pCurrentPage = m_pagesList.at(static_cast<quint64>(m_activePage));
		if (pCurrentPage.get() != nullptr)
		{
			QWidget* pWidget = pCurrentPage->baseWidget();
			if (pWidget != nullptr)
			{
				m_pagesLayout->removeWidget(pWidget);
				pWidget->hide();
			}
		}
	}

	// show new page
	//
	std::shared_ptr<PropertyPage> pActivePage = m_pagesList.at(static_cast<quint64>(pageType));
	if (pActivePage.get() != nullptr)
	{
		QWidget* pWidget = pActivePage->baseWidget();
		if (pWidget != nullptr)
		{
			setWindowTitle(tr("Options - %1").arg(pageCaption(pageType)));

			m_pagesLayout->addWidget(pWidget);
			pWidget->show();
		}
	}

	// select tree item
	//
	if (m_pagesTree != nullptr && pActivePage.get() != nullptr)
	{
		if (pActivePage->pageTreeItem() != nullptr)
		{
			m_pagesTree->setCurrentItem(pActivePage->pageTreeItem());
		}
	}

	//
	//
	m_activePage = pageType;

	//
	//
//	QList<std::shared_ptr<PropertyObject>> propertyObjects;
//	propertyObjects.push_back(m_pagesList.at(pageType));
//	m_pPropertyEditor->setObjects(propertyObjects);

//	if (m_pagesList.at(pageType)->widgetType() == PropertyPageWidgetType::List)
//	{
//		m_pagesLayout->addWidget(m_pPropertyEditor);
//	}
//	else
//	{
//		m_pagesLayout->removeWidget(m_pPropertyEditor);
//	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::onPropertyValueChanged(QtProperty* property, const QVariant &value)
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

	int type = dynamic_cast<QtVariantProperty*>(property) ->propertyType();

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

void DialogOptions::onPropertyValueChanged_1(QList<std::shared_ptr<PropertyObject>> objects)
{
	for (const std::shared_ptr<PropertyObject>& modifiedFilter : objects)
	{
		if (modifiedFilter.get() == nullptr)
		{
			assert(0);
			continue;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::onBrowserItem(QtBrowserItem*)
{
	restoreProperty();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::restoreProperty()
{
	QtProperty* property = m_currentPropertyItem;
	if (property == nullptr)
	{
		return;
	}

	if (m_propertyValueList.contains(property) == false)
	{
		return;
	}

	QVariant value = m_propertyValueList[property];

	dynamic_cast<QtVariantProperty*>(property)->setValue(value);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::applyProperty()
{
	QtProperty* property = m_currentPropertyItem;
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

	PropertyPageType pageType = static_cast<PropertyPageType>((paramId & 0xFF00) >> 8);
	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return;
	}

	int param = paramId & 0x00FF;

	QVariant value = m_currentPropertyValue;

	switch (pageType)
	{
		case PropertyPageType::Socket_CfgSrv:
		case PropertyPageType::Socket_AppDataSrv:
		case PropertyPageType::Socket_TuningSrv:
			{
				SocketType socketType = SocketType::NoSocketType;

				switch (pageType)
				{
					case PropertyPageType::Socket_CfgSrv:		socketType = SocketType::CfgSrv;	break;
					case PropertyPageType::Socket_AppDataSrv:	socketType = SocketType::AppDataSrv;	break;
					case PropertyPageType::Socket_TuningSrv:	socketType = SocketType::TuningSrv;	break;

					default:
						assert(0);
						socketType = SocketType::NoSocketType;
				}

				if (ERR_SOCKET_TYPE(socketType) == true)
				{
					break;
				}

				SocketClientOption sco = m_options.socket().client(socketType);

				switch(param)
				{
					case SOCKET_CLIENT_PARAM_EQUIPMENT_ID1:	sco.setEquipmentID(ServerType::Primary, value.toString());			break;
					case SOCKET_CLIENT_PARAM_SERVER_IP1:	sco.setServerIP(ServerType::Primary, value.toString());				break;
					case SOCKET_CLIENT_PARAM_SERVER_PORT1:	sco.setServerPort(ServerType::Primary, value.toInt());				break;
					case SOCKET_CLIENT_PARAM_EQUIPMENT_ID2:	sco.setEquipmentID(ServerType::Reserve, value.toString());			break;
					case SOCKET_CLIENT_PARAM_SERVER_IP2:	sco.setServerIP(ServerType::Reserve, value.toString());				break;
					case SOCKET_CLIENT_PARAM_SERVER_PORT2:	sco.setServerPort(ServerType::Reserve, value.toInt());				break;

					default:
						assert(0);
				}

				m_options.socket().setClient(socketType, sco);
				updateServerPage();

			}
			break;

		case PropertyPageType::Module_Measure:
			{
				switch(param)
				{
					case MO_PARAM_SUFFIX_SN:				m_options.module().setSuffixSN(value.toString());							break;
					case MO_PARAM_MEASURE_INT_INSTEAD_IN:	m_options.module().setMeasureInterInsteadIn(value.toBool());				break;
					case MO_PARAM_MEASURE_LIN_AND_CMP:		m_options.module().setMeasureLinAndCmp(value.toBool());						break;
					case MO_PARAM_MEASURE_ENTIRE_MODULE:	m_options.module().setMeasureEntireModule(value.toBool());					break;
					case MO_PARAM_MEASURE_SHOWN_ON_SCHEMAS:	m_options.module().setMeasureShownOnSchemas(value.toBool());				break;
					case MO_PARAM_WARN_IF_MEASURED:			m_options.module().setWarningIfMeasured(value.toBool());					break;
					case MO_PARAM_MAX_IMPUT_COUNT:			m_options.module().setMaxInputCount(value.toInt());							break;

					default:
						assert(0);
				}
			}
			break;

		case PropertyPageType::Linearity_Measure:
			{
				switch(param)
				{
					case LO_PARAM_ERROR_LIMIT:				m_options.linearity().setErrorLimit(value.toDouble());						break;
					case LO_PARAM_ERROR_TYPE:				m_options.linearity().setErrorType(value.toInt());
															m_options.measureView().setUpdateColumnView(Measure::Type::Linearity, true);break;
					case LO_PARAM_CALC_ERROR_BY_RANGE:		m_options.linearity().setCalcErrorByRange(value.toInt());
															m_options.measureView().setUpdateColumnView(Measure::Type::Linearity, true);break;
					case LO_PARAM_MEASURE_TIME:				m_options.linearity().setMeasureTimeInPoint(value.toInt());					break;
					case LO_PARAM_MEASURE_IN_POINT:			m_options.linearity().setMeasureCountInPoint(value.toInt());				break;
					case LO_PARAM_RANGE_TYPE:				m_options.linearity().setDivisionType(value.toInt());
															m_options.linearity().recalcPoints();
															updateLinearityPage(false);													break;
					case LO_PARAM_POINT_COUNT:				m_options.linearity().recalcPoints(value.toInt());
															updateLinearityPage(false);													break;
					case LO_PARAM_LOW_RANGE:				m_options.linearity().setLowLimitRange(value.toDouble());
															m_options.linearity().recalcPoints();
															updateLinearityPage(false);													break;
					case LO_PARAM_HIGH_RANGE:				m_options.linearity().setHighLimitRange(value.toDouble());
															m_options.linearity().recalcPoints();
															updateLinearityPage(false);													break;
					case LO_PARAM_VALUE_POINTS:				setActivePage(PropertyPageType::Linearity_Point);									break;
					case LO_PARAM_LIST_TYPE:				m_options.linearity().setViewType(value.toInt());
															m_options.measureView().setUpdateColumnView(Measure::Type::Linearity,true);	break;
					default:
						assert(0);
				}
			}
			break;

		case PropertyPageType::Comparator_Measure:
			{
				switch(param)
				{
					case CO_PARAM_ERROR_LIMIT:				m_options.comparator().setErrorLimit(value.toDouble());							break;
					case CO_PARAM_ERROR_TYPE:				m_options.comparator().setErrorType(value.toInt());
															m_options.measureView().setUpdateColumnView(Measure::Type::Comparators, true);	break;
					case CO_PARAM_CALC_ERROR_BY_RANGE:		m_options.comparator().setCalcErrorByRange(value.toInt());
															m_options.measureView().setUpdateColumnView(Measure::Type::Comparators, true);	break;
					case CO_PARAM_START_VALUE:				m_options.comparator().setStartValueForCompare(value.toDouble());				break;
					case CO_PARAM_COMPARATOR_INDEX:			m_options.comparator().setStartComparatorIndex(value.toInt() - 1);				break;
					case CO_PARAM_ENABLE_HYSTERESIS:		m_options.comparator().setEnableMeasureHysteresis(value.toBool());				break;

					default:
						assert(0);
				}
			}
			break;

		case PropertyPageType::MeasureView_Text:
			{
				switch(param)
				{
					case MWO_PARAM_FONT:					m_options.measureView().setFont(value.toString());							break;
					case MWO_PARAM_COLOR_NOT_ERROR:			m_options.measureView().setColorNotError(QColor(value.toString()));			break;
					case MWO_PARAM_COLOR_LIMIT_ERROR:		m_options.measureView().setColorErrorLimit(QColor(value.toString()));		break;
					case MWO_PARAM_COLOR_CONTROL_ERROR:		m_options.measureView().setColorErrorControl(QColor(value.toString()));		break;
					case MWO_PARAM_SHOW_NO_VALID:			m_options.measureView().setShowNoValid(value.toBool());						break;
					case MWO_PARAM_PRECESION_BY_CALIBRATOR:	m_options.measureView().setPrecesionByCalibrator(value.toBool());			break;

					default:
						assert(0);
				}

				for(int measureType = 0; measureType < Measure::TypeCount; measureType++)
				{
					m_options.measureView().setUpdateColumnView(static_cast<Measure::Type>(measureType), true);
				}
			}
			break;

		case PropertyPageType::MeasureView_Column:

			break;

		case PropertyPageType::Panel_SignalInfo:

			switch(param)
			{
				case SIO_PARAM_FONT:					m_options.signalInfo().setFont(value.toString());								break;
				case SIO_PARAM_SHOW_NO_VALID:			m_options.signalInfo().setShowNoValid(value.toBool());							break;
				case SIO_PARAM_ELECTRIC_STATE:			m_options.signalInfo().setShowElectricState(value.toBool());					break;
				case SIO_PARAM_COLOR_FLAG_VALID:		m_options.signalInfo().setColorFlagValid(QColor(value.toString()));				break;
				case SIO_PARAM_COLOR_FLAG_SIM:			m_options.signalInfo().setColorFlagSim(QColor(value.toString()));				break;
				case SIO_PARAM_COLOR_FLAG_LOCK:			m_options.signalInfo().setColorFlagLock(QColor(value.toString()));				break;
				case SIO_PARAM_COLOR_FLAG_OVERFLOW:		m_options.signalInfo().setColorFlagOverflow(QColor(value.toString()));			break;
				case SIO_PARAM_COLOR_FLAG_UNDERFLOW:	m_options.signalInfo().setColorFlagUnderflow(QColor(value.toString()));			break;
				case SIO_PARAM_TIME_FOR_UPDATE:			m_options.signalInfo().setTimeForUpdate(value.toInt());							break;

				default:
					assert(0);
			}

			break;

		case PropertyPageType::Panel_ComparatorInfo:

			switch(param)
			{
				case CIO_PARAM_FONT:					m_options.comparatorInfo().setFont(value.toString());							break;
				case CIO_PARAM_COLOR_FLAG_SIM:			m_options.comparatorInfo().setColorFlagSim(QColor(value.toString()));			break;
				case CIO_PARAM_COLOR_FLAG_LOCK:			m_options.comparatorInfo().setColorFlagLock(QColor(value.toString()));			break;
				case CIO_PARAM_COLOR_STATE_FALSE:		m_options.comparatorInfo().setColorStateFalse(QColor(value.toString()));		break;
				case CIO_PARAM_COLOR_STATE_TRUE:		m_options.comparatorInfo().setColorStateTrue(QColor(value.toString()));			break;
				case CIO_PARAM_TIME_FOR_UPDATE:			m_options.comparatorInfo().setTimeForUpdate(value.toInt());						break;

				default:
					assert(0);
			}

			break;

		case PropertyPageType::Database_Location:
			{
				switch(param)
				{
					case DBO_PARAM_LOCATION_PATH:		m_options.database().setLocationPath(value.toString());							break;
					case DBO_PARAM_TYPE:				m_options.database().setType(value.toBool());									break;

					default:
						assert(0);
				}
			}
			break;

		case PropertyPageType::Database_Backup:
			{
				switch(param)
				{
					case DBO_PARAM_ON_START:			m_options.database().setOnStart(value.toBool());								break;
					case DBO_PARAM_ON_EXIT:				m_options.database().setOnExit(value.toBool());									break;
					case DBO_PARAM_COPY_PATH:			m_options.database().setBackupPath(value.toString());							break;

					default:
						assert(0);
				}
			}
			break;

		case PropertyPageType::Language_App:
			{
				switch(param)
				{
					case LNO_PARAM_LANGUAGE_TYPE:		m_options.language().setLanguageType(value.toInt());							break;

					default:
						assert(0);
				}
			}
			break;

		default:
			assert(nullptr);
	}

	m_currentPropertyItem = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::updateServerPage()
{
	QtVariantProperty* property = nullptr;

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((PropertyPageType::Socket_CfgSrv << 8) | SOCKET_CLIENT_PARAM_EQUIPMENT_ID2));
	if (property != nullptr)
	{
		property->setValue(m_options.socket().client(SocketType::CfgSrv).equipmentID(ServerType::Primary));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::updateLinearityPage(bool isDialog)
{
	std::shared_ptr<PropertyPage> page = m_pagesList[PropertyPageType::Linearity_Point];
	if (page.get() == nullptr)
	{
		return;
	}

	DialogMeasurePoint* dialog = dynamic_cast<DialogMeasurePoint*>(page->baseWidget());
	if (dialog == nullptr)
	{
		return;
	}

	if (isDialog == true)
	{
		// get options from dialog
		//
		m_options.setLinearity(dialog->linearity());
	}
	else
	{
		// set options to dialog
		//
		dialog->setLinearity(m_options.linearity());
	}

	QtVariantProperty* property = nullptr;

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((PropertyPageType::Linearity_Measure << 8) | LO_PARAM_RANGE_TYPE));
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().divisionType());
	}

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((PropertyPageType::Linearity_Measure << 8) | LO_PARAM_POINT_COUNT));
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().points().count());

		switch(m_options.linearity().divisionType())
		{
			case Measure::LinearityDivision::Manual:	property->setEnabled(false);	break;
			case Measure::LinearityDivision::Automatic:	property->setEnabled(true);		break;

			default:
				assert(0);
		}
	}

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((PropertyPageType::Linearity_Measure << 8) | LO_PARAM_LOW_RANGE));
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().lowLimitRange());

		switch(m_options.linearity().divisionType())
		{
			case Measure::LinearityDivision::Manual:	property->setEnabled(false);	break;
			case Measure::LinearityDivision::Automatic:	property->setEnabled(true);		break;

			default:
				assert(0);
		}
	}

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((PropertyPageType::Linearity_Measure << 8) | LO_PARAM_HIGH_RANGE));
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().highLimitRange());

		switch(m_options.linearity().divisionType())
		{
			case Measure::LinearityDivision::Manual:	property->setEnabled(false);	break;
			case Measure::LinearityDivision::Automatic:	property->setEnabled(true);		break;

			default:
				assert(0);
		}

	}

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((PropertyPageType::Linearity_Measure << 8) | LO_PARAM_VALUE_POINTS));
	if (property != nullptr)
	{
		property->setValue(qApp->translate("Options.cpp", m_options.linearity().points().text().toUtf8()));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::updateMeasureViewPage(bool isDialog)
{
	std::shared_ptr<PropertyPage> page = m_pagesList[PropertyPageType::MeasureView_Column];
	if (page.get() == nullptr)
	{
		return;
	}

	DialogOptionsMeasureViewHeader* dialog = dynamic_cast<DialogOptionsMeasureViewHeader*> (page->baseWidget());
	if (dialog == nullptr)
	{
		return;
	}

	if (isDialog == true)
	{
		// get options from dialog
		//
		m_options.setMeasureView(dialog->header());

		Measure::Type measureType = dialog->measureType();
		if (ERR_MEASURE_TYPE(measureType) == false)
		{
			m_options.measureView().setUpdateColumnView(measureType, true);
		}
	}
	else
	{
		// set options to dialog
		//
		dialog->header() = m_options.measureView();
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::loadSettings()
{
	QSettings s;

	QByteArray geometry = s.value(QString("%1OptionsDialog/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();
	restoreGeometry(geometry);

	m_activePage = static_cast<PropertyPageType>(s.value(QString("%1OptionsDialog/activePage").arg(WINDOW_GEOMETRY_OPTIONS_KEY), PropertyPageType::Linearity_Measure).toInt());
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1OptionsDialog/Geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveGeometry());
	s.setValue(QString("%1OptionsDialog/activePage").arg(WINDOW_GEOMETRY_OPTIONS_KEY), m_activePage);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::onOk()
{
	accept();
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::event(QEvent*  e)
{
	if (e->type() == QEvent::Hide)
	{
		removePropertyPages();
		saveSettings();
	}

	if (e->type() == QEvent::KeyRelease)
	{
		if (m_activePage >= 0 && m_activePage < TO_INT(m_pagesList.size()))
		{
			std::shared_ptr<PropertyPage> pActivePage = m_pagesList.at(m_activePage);
			if (pActivePage.get() != nullptr)
			{
				if (pActivePage->widgetType() == PropertyPageWidgetType::List)
				{
					QKeyEvent* keyEvent = static_cast<QKeyEvent* >(e);

					if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
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


