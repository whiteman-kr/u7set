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

PropertyPage::PropertyPage(QtVariantPropertyManager* manager, QtVariantEditorFactory* factory, QtTreePropertyBrowser* editor)
{
	m_type = PropertyPageType::List;

	m_pManager = manager;
	m_pFactory = factory;
	m_pEditor = editor;

	m_pWidget = editor;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::PropertyPage(QDialog* dialog)
{
	m_type = PropertyPageType::Dialog;

	m_pDialog = dialog;

	m_pWidget = dialog;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::~PropertyPage()
{
	switch(m_type)
	{
		case PropertyPageType::List:

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

		case PropertyPageType::Dialog:

			if (m_pDialog != nullptr)
			{
				delete m_pDialog;
				m_pDialog = nullptr;
			}

			break;

		default:
			assert(0);
	}

	m_type = PropertyPageType::NoPageType;
	m_page = OptionPage::NoOptionPage;

	m_pWidget = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

QString groupCaption(int group)
{
	QString caption;

	switch (group)
	{
		case OptionGroup::Server:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Connect to server");		break;
		case OptionGroup::Module:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Module");					break;
		case OptionGroup::Linearity:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Linearity");				break;
		case OptionGroup::Comparator:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Comparators");			break;
		case OptionGroup::MeasureView:	caption = QT_TRANSLATE_NOOP("DialogOptions", "List of measurements");	break;
		case OptionGroup::PanelInfo:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Panels information");		break;
		case OptionGroup::Database:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Database");				break;
		case OptionGroup::Language:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Language");				break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogOptions", "Unknown");
	}

	return qApp->translate("DialogOptions", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

QString pageCaption(OptionPage page)
{
	QString caption;

	switch (page)
	{
		case OptionPage::Socket_Cfg:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection to Config Server - TCP/IP");			break;
		case OptionPage::Socket_AppDataSrv:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection to Application Data Server - TCP/IP");	break;
		case OptionPage::Socket_Tuning:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection to Tuning Server - TCP/IP");			break;
		case OptionPage::Module_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measuring of module");							break;
		case OptionPage::Linearity_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measurements of linearity");						break;
		case OptionPage::Linearity_Point:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Point of linearity");								break;
		case OptionPage::Comparator_Measure:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Measure comparators");							break;
		case OptionPage::MeasureView_Text:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying data in the list of measurements");	break;
		case OptionPage::MeasureView_Column:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying columns in the list of measurements");	break;
		case OptionPage::Panel_SignalInfo:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying information of signals");				break;
		case OptionPage::Panel_ComparatorInfo:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying information of сomparators");			break;
		case OptionPage::Database_Location:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Database location");								break;
		case OptionPage::Database_Backup:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Database backup");								break;
		case OptionPage::Language_App:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Language of application");						break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogOptions", "Unknown");
	}

	return qApp->translate("DialogOptions", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

QString pageShortCaption(OptionPage page)
{
	QString caption;

	switch (page)
	{
		case OptionPage::Socket_Cfg:			caption = QT_TRANSLATE_NOOP("DialogOptions", "ConfigurationService");		break;
		case OptionPage::Socket_AppDataSrv:		caption = QT_TRANSLATE_NOOP("DialogOptions", "AppDataService");				break;
		case OptionPage::Socket_Tuning:			caption = QT_TRANSLATE_NOOP("DialogOptions", "TuningService");				break;
		case OptionPage::Module_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measuring");					break;
		case OptionPage::Linearity_Measure:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Measurements");				break;
		case OptionPage::Linearity_Point:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Points");						break;
		case OptionPage::Comparator_Measure:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Measurements");				break;
		case OptionPage::MeasureView_Text:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Displaying");					break;
		case OptionPage::MeasureView_Column:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Columns");					break;
		case OptionPage::Panel_SignalInfo:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Signal information");			break;
		case OptionPage::Panel_ComparatorInfo:	caption = QT_TRANSLATE_NOOP("DialogOptions", "Comparator information");		break;
		case OptionPage::Database_Location:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Location");					break;
		case OptionPage::Database_Backup:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Backup");						break;
		case OptionPage::Language_App:			caption = QT_TRANSLATE_NOOP("DialogOptions", "Language of application ");	break;

		default:
			assert(0);
			caption = QT_TRANSLATE_NOOP("DialogOptions", "Unknown");
	}

	return qApp->translate("DialogOptions", caption.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

OptionGroup groupByPage(OptionPage page)
{
	OptionGroup group = OptionGroup::NoOptionGroup;

	switch (page)
	{
		case OptionPage::Socket_Cfg:			group = OptionGroup::Server;		break;
		case OptionPage::Socket_AppDataSrv:		group = OptionGroup::Server;		break;
		case OptionPage::Socket_Tuning:			group = OptionGroup::Server;		break;
		case OptionPage::Module_Measure:		group = OptionGroup::Module;		break;
		case OptionPage::Linearity_Measure:		group = OptionGroup::Linearity;		break;
		case OptionPage::Linearity_Point:		group = OptionGroup::Linearity;		break;
		case OptionPage::Comparator_Measure:	group = OptionGroup::Comparator;	break;
		case OptionPage::MeasureView_Text:		group = OptionGroup::MeasureView;	break;
		case OptionPage::MeasureView_Column:	group = OptionGroup::MeasureView;	break;
		case OptionPage::Panel_SignalInfo:		group = OptionGroup::PanelInfo;		break;
		case OptionPage::Panel_ComparatorInfo:	group = OptionGroup::PanelInfo;		break;
		case OptionPage::Database_Location:		group = OptionGroup::Database;		break;
		case OptionPage::Database_Backup:		group = OptionGroup::Database;		break;
		case OptionPage::Language_App:			group = OptionGroup::Language;		break;

		default:
			assert(0);
			group = OptionGroup::NoOptionGroup;
	}

	return group;
};


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OptionPage DialogOptions::m_activePage = OptionPage::Linearity_Measure;

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
	removePages();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::createInterface()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	setWindowIcon(QIcon(":/icons/Options.png"));

	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	setMinimumSize(static_cast<int>(screen.width() * 0.45), static_cast<int>(screen.height() * 0.3));
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

QHBoxLayout* DialogOptions::createPages()
{
	QHBoxLayout* pagesLayout = new QHBoxLayout ;

	m_pPageTree = new QTreeWidget;

	m_pPageTree->setHeaderHidden(true);
	m_pPageTree->setFixedWidth(200);

	std::vector<QTreeWidgetItem*> groupList;

	for(int group = 0; group < OptionGroupCount; group++)
	{
		QTreeWidgetItem* groupTreeItem = new QTreeWidgetItem;
		groupTreeItem->setText(0, groupCaption(group));
		m_pPageTree->addTopLevelItem(groupTreeItem);

		groupList.push_back(groupTreeItem);
	}

	for(int pageIndex = 0; pageIndex < OptionPageCount; pageIndex++)
	{
		OptionPage page = static_cast<OptionPage>(pageIndex);
		if (ERR_OPTION_PAGE(page))
		{
			continue;
		}

		OptionGroup group = groupByPage(page);
		if (ERR_OPTION_GROUP(group))
		{
			continue;
		}

		if (group < 0 || group >= TO_INT(groupList.size()))
		{
			continue;
		}

		QTreeWidgetItem* groupTreeItem = groupList.at(static_cast<quint64>(group));

		QTreeWidgetItem* pageTreeItem = new QTreeWidgetItem;
		pageTreeItem->setText(0, pageShortCaption(page));
		pageTreeItem->setData(0, Qt::UserRole, page);

		groupTreeItem->addChild(pageTreeItem);

		PropertyPage* pPropertyPage = createPage(page);
		if (pPropertyPage != nullptr)
		{
			pPropertyPage->m_page = page;
			pPropertyPage->m_pTreeWidgetItem = pageTreeItem;
		}

		m_pageList.push_back(pPropertyPage);
	}

	connect(m_pPageTree, &QTreeWidget::currentItemChanged , this, &DialogOptions::onPageChanged);

	pagesLayout->addWidget(m_pPageTree);

	return pagesLayout;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::removePages()
{
	m_propertyItemList.clear();
	m_propertyValueList.clear();

	for(PropertyPage* pPropertyPage : m_pageList)
	{
		if (pPropertyPage == nullptr)
		{
			continue;
		}

		delete pPropertyPage;
	}
}

// -------------------------------------------------------------------------------------------------------------------

QHBoxLayout* DialogOptions::createButtons()
{
	QHBoxLayout* buttonsLayout = new QHBoxLayout ;

	QPushButton* okButton = new QPushButton(tr("Ok"));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"));

	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	connect(okButton, &QPushButton::clicked, this, &DialogOptions::onOk);
	connect(cancelButton, &QPushButton::clicked, this, &DialogOptions::reject);

	return buttonsLayout;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* DialogOptions::createPage(OptionPage page)
{
	if (ERR_OPTION_PAGE(page) == true)
	{
		return nullptr;
	}

	PropertyPage* pPropertyPage = nullptr;

	switch (page)
	{
		case OptionPage::Socket_Cfg:
		case OptionPage::Socket_AppDataSrv:
		case OptionPage::Socket_Tuning:
		case OptionPage::Linearity_Measure:
		case OptionPage::Comparator_Measure:
		case OptionPage::Module_Measure:
		case OptionPage::MeasureView_Text:
		case OptionPage::Panel_SignalInfo:
		case OptionPage::Panel_ComparatorInfo:
		case OptionPage::Database_Location:
		case OptionPage::Database_Backup:
		case OptionPage::Language_App:			pPropertyPage = createPropertyList(page);	break;
		case OptionPage::Linearity_Point:
		case OptionPage::MeasureView_Column:	pPropertyPage = createPropertyDialog(page);	break;

		default:
			assert(nullptr);
	}

	return pPropertyPage;
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* DialogOptions::createPropertyList(OptionPage page)
{
	if (ERR_OPTION_PAGE(page) == true)
	{
		return nullptr;
	}

	QtVariantProperty* item = nullptr;


	QtVariantPropertyManager* manager = new VariantManager();
	QtVariantEditorFactory* factory = new VariantFactory();

	//QtVariantPropertyManager* manager = new QtVariantPropertyManager;
	//QtVariantEditorFactory* factory = new QtVariantEditorFactory;

	QtTreePropertyBrowser* editor = new QtTreePropertyBrowser;

	switch (page)
	{
		case OptionPage::Socket_Cfg:
		case OptionPage::Socket_AppDataSrv:
		case OptionPage::Socket_Tuning:
			{
				int socketType = -1;

				switch (page)
				{
					case OptionPage::Socket_Cfg:		socketType = SOCKET_TYPE_CONFIG;	break;
					case OptionPage::Socket_AppDataSrv:	socketType = SOCKET_TYPE_SIGNAL;	break;
					case OptionPage::Socket_Tuning:		socketType = SOCKET_TYPE_TUNING;	break;

					default:
						assert(0);
						socketType = SOCKET_TYPE_UNDEFINED;
				}

				if (socketType < 0 || socketType >= SOCKET_TYPE_COUNT)
				{
					break;
				}

				SocketClientOption sco = m_options.socket().client(socketType);

				QtProperty* serverGroup1 = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Server (primary)"));

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_EQUIPMENT_ID1]));
					item->setValue(sco.equipmentID(SOCKET_SERVER_TYPE_PRIMARY));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_EQUIPMENT_ID1);
					serverGroup1->addSubProperty(item);

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_IP1]));
					item->setValue(sco.serverIP(SOCKET_SERVER_TYPE_PRIMARY));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_IP1);
					serverGroup1->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_PORT1]));
					item->setValue(sco.serverPort(SOCKET_SERVER_TYPE_PRIMARY));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 65535);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_PORT1);
					serverGroup1->addSubProperty(item);

				QtProperty* serverGroup2 = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Server (reserve)"));

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_EQUIPMENT_ID2]));
					item->setValue(sco.equipmentID(SOCKET_SERVER_TYPE_RESERVE));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_EQUIPMENT_ID2);
					serverGroup2->addSubProperty(item);

					if (socketType == SOCKET_TYPE_CONFIG)
					{
						item->setAttribute(QLatin1String("readOnly"), true);
					}

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_IP2]));
					item->setValue(sco.serverIP(SOCKET_SERVER_TYPE_RESERVE));
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_IP2);
					serverGroup2->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", SocketClientParamName[socketType][SOCKET_CLIENT_PARAM_SERVER_PORT2]));
					item->setValue(sco.serverPort(SOCKET_SERVER_TYPE_RESERVE));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 65535);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, SOCKET_CLIENT_PARAM_SERVER_PORT2);
					serverGroup2->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(serverGroup1);

				if (socketType == SOCKET_TYPE_SIGNAL)
				{
					editor->addProperty(serverGroup2);
				}

			}
			break;

		case OptionPage::Module_Measure:
			{
				QtProperty* identificationGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Identification of module"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), qApp->translate("Options.h", ModuleParamName[MO_PARAM_SUFFIX_SN]));
					item->setValue(m_options.module().suffixSN());
					appendProperty(item, page, MO_PARAM_SUFFIX_SN);
					identificationGroup->addSubProperty(item);

				QtProperty* measuremoduleGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measuring of module"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_INT_INSTEAD_IN]));
					item->setValue(m_options.module().measureInterInsteadIn());
					appendProperty(item, page, MO_PARAM_MEASURE_INT_INSTEAD_IN);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_LIN_AND_CMP]));
					item->setValue(m_options.module().measureLinAndCmp());
					appendProperty(item, page, MO_PARAM_MEASURE_LIN_AND_CMP);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_ENTIRE_MODULE]));
					item->setValue(m_options.module().measureEntireModule());
					appendProperty(item, page, MO_PARAM_MEASURE_ENTIRE_MODULE);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MEASURE_SHOWN_ON_SCHEMAS]));
					item->setValue(m_options.module().measureShownOnSchemas());
					appendProperty(item, page, MO_PARAM_MEASURE_SHOWN_ON_SCHEMAS);
					measuremoduleGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ModuleParamName[MO_PARAM_WARN_IF_MEASURED]));
					item->setValue(m_options.module().warningIfMeasured());
					appendProperty(item, page, MO_PARAM_WARN_IF_MEASURED);
					measuremoduleGroup->addSubProperty(item);

				QtProperty* limitsGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Limits"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", ModuleParamName[MO_PARAM_MAX_IMPUT_COUNT]));
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setValue(m_options.module().maxInputCount());
					appendProperty(item, page, MO_PARAM_MAX_IMPUT_COUNT);
					limitsGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(identificationGroup);
				editor->addProperty(measuremoduleGroup);
				editor->addProperty(limitsGroup);
			}
			break;

		case OptionPage::Linearity_Measure:
			{
				QtProperty* errorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", LinearityParamName[LO_PARAM_ERROR_LIMIT]));
					item->setValue(m_options.linearity().errorLimit());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, LO_PARAM_ERROR_LIMIT);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", LinearityParamName[LO_PARAM_ERROR_TYPE]));
					QStringList errorTypeList;
					for(int e = 0; e < Measure::ErrorTypeCount; e++)
					{
						errorTypeList.append(qApp->translate("MeasureBase", Measure::ErrorTypeCaption(static_cast<Measure::ErrorType>(e)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), errorTypeList);
					item->setValue(m_options.linearity().errorType());
					appendProperty(item, page, LO_PARAM_ERROR_TYPE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", LinearityParamName[LO_PARAM_CALC_ERROR_BY_RANGE]));
					QStringList showErrorFromLimitList;
					for(int t = 0; t < Measure::CalcErrorRangeCount; t++)
					{
						showErrorFromLimitList.append(qApp->translate("MeasureBase", Measure::CalcErrorRangeCaption(static_cast<Measure::CalcErrorRange>(t)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), showErrorFromLimitList);
					item->setValue(m_options.linearity().calcErrorByRange());
					appendProperty(item, page, LO_PARAM_CALC_ERROR_BY_RANGE);
					errorGroup->addSubProperty(item);

				QtProperty* measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurements at the single point"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", LinearityParamName[LO_PARAM_MEASURE_TIME]));
					item->setValue(m_options.linearity().measureTimeInPoint());
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), 60);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, LO_PARAM_MEASURE_TIME);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", LinearityParamName[LO_PARAM_MEASURE_IN_POINT]));
					item->setValue(m_options.linearity().measureCountInPoint());
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("maximum"), Measure::MaxMeasurementInPoint);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, LO_PARAM_MEASURE_IN_POINT);
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
					appendProperty(item, page, LO_PARAM_RANGE_TYPE);
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
					appendProperty(item, page, LO_PARAM_POINT_COUNT);
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
					appendProperty(item, page, LO_PARAM_LOW_RANGE);
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
					appendProperty(item, page, LO_PARAM_HIGH_RANGE);
					pointGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::String, qApp->translate("Options.h", LinearityParamName[LO_PARAM_VALUE_POINTS]));
					item->setValue(qApp->translate("Options.cpp", m_options.linearity().points().text().toUtf8()));
					appendProperty(item, page, LO_PARAM_VALUE_POINTS);
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
					appendProperty(item, page, LO_PARAM_LIST_TYPE);
					showcolumnGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(errorGroup);
				editor->addProperty(measureGroup);
				editor->addProperty(pointGroup);
				editor->addProperty(showcolumnGroup);
			}
			break;

		case OptionPage::Comparator_Measure:
			{
				QtProperty* errorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Metrological error"));

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_ERROR_LIMIT]));
					item->setValue(m_options.comparator().errorLimit());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, CO_PARAM_ERROR_LIMIT);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", ComparatorParamName[CO_PARAM_ERROR_TYPE]));
					QStringList errorTypeList;
					for(int e = 0; e < Measure::ErrorTypeCount; e++)
					{
						errorTypeList.append(qApp->translate("MeasureBase", Measure::ErrorTypeCaption(static_cast<Measure::ErrorType>(e)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), errorTypeList);
					item->setValue(m_options.comparator().errorType());
					appendProperty(item, page, CO_PARAM_ERROR_TYPE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", ComparatorParamName[CO_PARAM_CALC_ERROR_BY_RANGE]));
					QStringList showErrorFromLimitList;
					for(int t = 0; t < Measure::CalcErrorRangeCount; t++)
					{
						showErrorFromLimitList.append(qApp->translate("MeasureBase", Measure::CalcErrorRangeCaption(static_cast<Measure::CalcErrorRange>(t)).toUtf8()));
					}
					item->setAttribute(QLatin1String("enumNames"), showErrorFromLimitList);
					item->setValue(m_options.comparator().calcErrorByRange());
					appendProperty(item, page, CO_PARAM_CALC_ERROR_BY_RANGE);
					errorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Double, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_START_VALUE]));
					item->setValue(m_options.comparator().startValueForCompare());
					item->setAttribute(QLatin1String("singleStep"), 0.1);
					item->setAttribute(QLatin1String("decimals"), 3);
					appendProperty(item, page, CO_PARAM_START_VALUE);
					errorGroup->addSubProperty(item);

				QtProperty* permissionsGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Permissions"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_COMPARATOR_INDEX]));
					item->setValue(m_options.comparator().startComparatorIndex() + 1);
					item->setAttribute(QLatin1String("minimum"), 1);
					item->setAttribute(QLatin1String("singleStep"), 1);
					appendProperty(item, page, CO_PARAM_COMPARATOR_INDEX);
					permissionsGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", ComparatorParamName[CO_PARAM_ENABLE_HYSTERESIS]));
					item->setValue(m_options.comparator().enableMeasureHysteresis());
					appendProperty(item, page, CO_PARAM_ENABLE_HYSTERESIS);
					permissionsGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(errorGroup);
				editor->addProperty(permissionsGroup);
			}
			break;

		case OptionPage::MeasureView_Text:
			{
				QtProperty* fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_FONT]));
					item->setValue(m_options.measureView().font().toString());
					appendProperty(item, page, MWO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty* colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_COLOR_NOT_ERROR]));
					item->setValue(m_options.measureView().colorNotError());
					appendProperty(item, page, MWO_PARAM_COLOR_NOT_ERROR);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_COLOR_LIMIT_ERROR]));
					item->setValue(m_options.measureView().colorErrorLimit());
					appendProperty(item, page, MWO_PARAM_COLOR_LIMIT_ERROR);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_COLOR_CONTROL_ERROR]));
					item->setValue(m_options.measureView().colorErrorControl());
					appendProperty(item, page, MWO_PARAM_COLOR_CONTROL_ERROR);
					colorGroup->addSubProperty(item);

				QtProperty* measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Measurements"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_SHOW_NO_VALID]));
					item->setValue(m_options.measureView().showNoValid());
					appendProperty(item, page, MWO_PARAM_SHOW_NO_VALID);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", MeasureViewParam[MWO_PARAM_PRECESION_BY_CALIBRATOR]));
					item->setValue(m_options.measureView().precesionByCalibrator());
					appendProperty(item, page, MWO_PARAM_PRECESION_BY_CALIBRATOR);
					measureGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(colorGroup);
				editor->addProperty(measureGroup);

				expandProperty(editor, OptionPage::MeasureView_Text, MWO_PARAM_FONT, false);
				expandProperty(editor, OptionPage::MeasureView_Text, MWO_PARAM_COLOR_NOT_ERROR, false);
				expandProperty(editor, OptionPage::MeasureView_Text, MWO_PARAM_COLOR_LIMIT_ERROR, false);
				expandProperty(editor, OptionPage::MeasureView_Text, MWO_PARAM_COLOR_CONTROL_ERROR, false);
			}
			break;

		case OptionPage::Panel_SignalInfo:
			{
				QtProperty* fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_FONT]));
					item->setValue(m_options.signalInfo().font().toString());
					appendProperty(item, page, SIO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty* measureGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Displaying signal state"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_SHOW_NO_VALID]));
					item->setValue(m_options.signalInfo().showNoValid());
					appendProperty(item, page, SIO_PARAM_SHOW_NO_VALID);
					measureGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_ELECTRIC_STATE]));
					item->setValue(m_options.signalInfo().showElectricState());
					appendProperty(item, page, SIO_PARAM_ELECTRIC_STATE);
					measureGroup->addSubProperty(item);

				QtProperty* colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_VALID]));
					item->setValue(m_options.signalInfo().colorFlagValid());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_VALID);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_SIM]));
					item->setValue(m_options.signalInfo().colorFlagSim());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_SIM);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_LOCK]));
					item->setValue(m_options.signalInfo().colorFlagLock());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_LOCK);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_OVERFLOW]));
					item->setValue(m_options.signalInfo().colorFlagOverflow());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_OVERFLOW);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_COLOR_FLAG_UNDERFLOW]));
					item->setValue(m_options.signalInfo().colorFlagUnderflow());
					appendProperty(item, page, SIO_PARAM_COLOR_FLAG_UNDERFLOW);
					colorGroup->addSubProperty(item);

				QtProperty* timeGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Time for updating"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", SignalInfoParam[SIO_PARAM_TIME_FOR_UPDATE]));
					item->setValue(m_options.signalInfo().timeForUpdate());
					appendProperty(item, page, SIO_PARAM_TIME_FOR_UPDATE);
					timeGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(measureGroup);
				editor->addProperty(colorGroup);
				editor->addProperty(timeGroup);

				expandProperty(editor, OptionPage::Panel_SignalInfo, SIO_PARAM_FONT, false);
				expandProperty(editor, OptionPage::Panel_SignalInfo, SIO_PARAM_COLOR_FLAG_VALID, false);
				expandProperty(editor, OptionPage::Panel_SignalInfo, SIO_PARAM_COLOR_FLAG_OVERFLOW, false);
				expandProperty(editor, OptionPage::Panel_SignalInfo, SIO_PARAM_COLOR_FLAG_UNDERFLOW, false);
			}

			break;

		case OptionPage::Panel_ComparatorInfo:
			{
				QtProperty* fontGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Font"));

					item = manager->addProperty(QVariant::Font, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_FONT]));
					item->setValue(m_options.comparatorInfo().font().toString());
					appendProperty(item, page, CIO_PARAM_FONT);
					fontGroup->addSubProperty(item);

				QtProperty* colorGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Colors"));

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_FLAG_SIM]));
					item->setValue(m_options.comparatorInfo().colorFlagSim());
					appendProperty(item, page, CIO_PARAM_COLOR_FLAG_SIM);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_FLAG_LOCK]));
					item->setValue(m_options.comparatorInfo().colorFlagLock());
					appendProperty(item, page, CIO_PARAM_COLOR_FLAG_LOCK);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_STATE_FALSE]));
					item->setValue(m_options.comparatorInfo().colorStateFalse());
					appendProperty(item, page, CIO_PARAM_COLOR_STATE_FALSE);
					colorGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Color, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_COLOR_STATE_TRUE]));
					item->setValue(m_options.comparatorInfo().colorStateTrue());
					appendProperty(item, page, CIO_PARAM_COLOR_STATE_TRUE);
					colorGroup->addSubProperty(item);

				QtProperty* timeGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Time for updating"));

					item = manager->addProperty(QVariant::Int, qApp->translate("Options.h", ComparatorInfoParam[CIO_PARAM_TIME_FOR_UPDATE]));
					item->setValue(m_options.comparatorInfo().timeForUpdate());
					appendProperty(item, page, CIO_PARAM_TIME_FOR_UPDATE);
					timeGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(fontGroup);
				editor->addProperty(colorGroup);
				editor->addProperty(timeGroup);

				expandProperty(editor, OptionPage::Panel_ComparatorInfo, CIO_PARAM_FONT, false);
				expandProperty(editor, OptionPage::Panel_ComparatorInfo, CIO_PARAM_COLOR_FLAG_SIM, false);
				expandProperty(editor, OptionPage::Panel_ComparatorInfo, CIO_PARAM_COLOR_FLAG_LOCK, false);
				expandProperty(editor, OptionPage::Panel_ComparatorInfo, CIO_PARAM_COLOR_STATE_FALSE, false);
				expandProperty(editor, OptionPage::Panel_ComparatorInfo, CIO_PARAM_COLOR_STATE_TRUE, false);
			}

			break;

		case OptionPage::Database_Location:
			{
				QtProperty* databaseGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Location of Database"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), qApp->translate("Options.h", DatabaseParam[DBO_PARAM_LOCATION_PATH]));
					item->setValue(m_options.database().locationPath());
					appendProperty(item, page, DBO_PARAM_LOCATION_PATH);
					databaseGroup->addSubProperty(item);

					item = manager->addProperty(QtVariantPropertyManager::enumTypeId(), qApp->translate("Options.h", DatabaseParam[DBO_PARAM_TYPE]));
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

		case OptionPage::Database_Backup:
			{
				QtProperty* eventGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Events"));

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", DatabaseParam[DBO_PARAM_ON_START]));
					item->setValue(m_options.database().onStart());
					appendProperty(item, page, DBO_PARAM_ON_START);
					eventGroup->addSubProperty(item);

					item = manager->addProperty(QVariant::Bool, qApp->translate("Options.h", DatabaseParam[DBO_PARAM_ON_EXIT]));
					item->setValue(m_options.database().onExit());
					appendProperty(item, page, DBO_PARAM_ON_EXIT);
					eventGroup->addSubProperty(item);

				QtProperty* pathGroup = manager->addProperty(QtVariantPropertyManager::groupTypeId(), tr("Location of reserve copy"));

					item = manager->addProperty(VariantManager::folerPathTypeId(), qApp->translate("Options.h", DatabaseParam[DBO_PARAM_COPY_PATH]));
					item->setValue(m_options.database().backupPath());
					appendProperty(item, page, DBO_PARAM_COPY_PATH);
					pathGroup->addSubProperty(item);

				editor->setFactoryForManager(manager, factory);

				editor->addProperty(eventGroup);
				editor->addProperty(pathGroup);
			}
			break;

		case OptionPage::Language_App:
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
					appendProperty(item, page, LNO_PARAM_LANGUAGE_TYPE);
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

	return (new PropertyPage(manager, factory, editor));
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage* DialogOptions::createPropertyDialog(OptionPage page)
{
	if (ERR_OPTION_PAGE(page) == true)
	{
		return nullptr;
	}

	QDialog* pDialogPage = nullptr;

	switch (page)
	{
		case OptionPage::Linearity_Point:
			{
				DialogMeasurePoint* dialog = new DialogMeasurePoint(m_options.linearity());
				connect(dialog, &DialogMeasurePoint::updateLinearityPage, this, &DialogOptions::updateLinearityPage);

				pDialogPage = dialog;
			}
			break;

		case OptionPage::MeasureView_Column:
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
		pDialogPage->setWindowTitle(pageCaption(page).toUtf8());
	}

	return (new PropertyPage(pDialogPage));
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::appendProperty(QtProperty* property, OptionPage page, int param)
{
	if (property == nullptr)
	{
		return;
	}

	if (ERR_OPTION_PAGE(page) == true)
	{
		return;
	}

	m_propertyItemList[property] = (page << 8) | param;
	m_propertyValueList[property] = dynamic_cast<QtVariantProperty*>(property)->value();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::expandProperty(QtTreePropertyBrowser* pEditor, OptionPage page, int param, bool expanded)
{
	if (pEditor == nullptr)
	{
		return;
	}

	if (ERR_OPTION_PAGE(page) == true)
	{
		return;
	}

	QtProperty* pProperty = m_propertyItemList.key((page << 8) | param);
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
	if (page < 0 || page >= TO_INT(m_pageList.size()))
	{
		return;
	}

	if (ERR_OPTION_PAGE(page) == true)
	{
		return;
	}

	setActivePage(static_cast<OptionPage>(page));
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::setActivePage(OptionPage page)
{
	if (ERR_OPTION_PAGE(page) == true)
	{
		return false;
	}

	if (m_pagesLayout == nullptr)
	{
		return false;
	}

	if (page < 0 || page >= TO_INT(m_pageList.size()))
	{
		return false;
	}

	// hide current page
	//
	if (m_activePage >= 0 && m_activePage < TO_INT(m_pageList.size()))
	{
		PropertyPage* pCurrentPage = m_pageList.at(static_cast<quint64>(m_activePage));
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
	PropertyPage* pActivePage = m_pageList.at(static_cast<quint64>(page));
	if (pActivePage != nullptr)
	{
		QWidget* pWidget = pActivePage->getWidget();
		if (pWidget != nullptr)
		{
			setWindowTitle(tr("Options - %1").arg(pageCaption(page)));

			m_pagesLayout->addWidget(pWidget);
			pWidget->show();
		}
	}

	m_activePage = page;

	// select tree item
	//
	if (m_pPageTree != nullptr && pActivePage != nullptr)
	{
		if (pActivePage->m_pTreeWidgetItem != nullptr)
		{
			m_pPageTree->setCurrentItem(pActivePage->m_pTreeWidgetItem);
		}
	}

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

	int page = (paramId & 0xFF00) >> 8;
	if (ERR_OPTION_PAGE(page) == true)
	{
		return;
	}

	int param = paramId & 0x00FF;

	QVariant value = m_currentPropertyValue;

	switch (page)
	{
		case OptionPage::Socket_Cfg:
		case OptionPage::Socket_AppDataSrv:
		case OptionPage::Socket_Tuning:
			{
				int socketType = -1;

				switch (page)
				{
					case OptionPage::Socket_Cfg:		socketType = SOCKET_TYPE_CONFIG;	break;
					case OptionPage::Socket_AppDataSrv: socketType = SOCKET_TYPE_SIGNAL;	break;
					case OptionPage::Socket_Tuning:		socketType = SOCKET_TYPE_TUNING;	break;

					default:
						assert(0);
						socketType = SOCKET_TYPE_UNDEFINED;
				}

				if (socketType < 0 || socketType >= SOCKET_TYPE_COUNT)
				{
					break;
				}

				SocketClientOption sco = m_options.socket().client(socketType);

				switch(param)
				{
					case SOCKET_CLIENT_PARAM_EQUIPMENT_ID1:	sco.setEquipmentID(SOCKET_SERVER_TYPE_PRIMARY, value.toString());			break;
					case SOCKET_CLIENT_PARAM_SERVER_IP1:	sco.setServerIP(SOCKET_SERVER_TYPE_PRIMARY, value.toString());				break;
					case SOCKET_CLIENT_PARAM_SERVER_PORT1:	sco.setServerPort(SOCKET_SERVER_TYPE_PRIMARY, value.toInt());				break;
					case SOCKET_CLIENT_PARAM_EQUIPMENT_ID2:	sco.setEquipmentID(SOCKET_SERVER_TYPE_RESERVE, value.toString());			break;
					case SOCKET_CLIENT_PARAM_SERVER_IP2:	sco.setServerIP(SOCKET_SERVER_TYPE_RESERVE, value.toString());				break;
					case SOCKET_CLIENT_PARAM_SERVER_PORT2:	sco.setServerPort(SOCKET_SERVER_TYPE_RESERVE, value.toInt());				break;

					default:
						assert(0);
				}

				m_options.socket().setClient(socketType, sco);
				updateServerPage();

			}
			break;

		case OptionPage::Module_Measure:
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

		case OptionPage::Linearity_Measure:
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
					case LO_PARAM_VALUE_POINTS:				setActivePage(OptionPage::Linearity_Point);									break;
					case LO_PARAM_LIST_TYPE:				m_options.linearity().setViewType(value.toInt());
															m_options.measureView().setUpdateColumnView(Measure::Type::Linearity,true);	break;
					default:
						assert(0);
				}
			}
			break;

		case OptionPage::Comparator_Measure:
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

		case OptionPage::MeasureView_Text:
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

		case OptionPage::MeasureView_Column:

			break;

		case OptionPage::Panel_SignalInfo:

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

		case OptionPage::Panel_ComparatorInfo:

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

		case OptionPage::Database_Location:
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

		case OptionPage::Database_Backup:
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

		case OptionPage::Language_App:
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

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((OptionPage::Socket_Cfg << 8) | SOCKET_CLIENT_PARAM_EQUIPMENT_ID2));
	if (property != nullptr)
	{
		property->setValue(m_options.socket().client(SOCKET_TYPE_CONFIG).equipmentID(SOCKET_SERVER_TYPE_PRIMARY));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::updateLinearityPage(bool isDialog)
{
	PropertyPage* page = m_pageList[OptionPage::Linearity_Point];
	if (page == nullptr)
	{
		return;
	}

	DialogMeasurePoint* dialog = dynamic_cast<DialogMeasurePoint*>(page->getWidget());
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

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((OptionPage::Linearity_Measure << 8) | LO_PARAM_RANGE_TYPE));
	if (property != nullptr)
	{
		property->setValue(m_options.linearity().divisionType());
	}

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((OptionPage::Linearity_Measure << 8) | LO_PARAM_POINT_COUNT));
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

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((OptionPage::Linearity_Measure << 8) | LO_PARAM_LOW_RANGE));
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

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((OptionPage::Linearity_Measure << 8) | LO_PARAM_HIGH_RANGE));
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

	property = dynamic_cast<QtVariantProperty*>(m_propertyItemList.key((OptionPage::Linearity_Measure << 8) | LO_PARAM_VALUE_POINTS));
	if (property != nullptr)
	{
		property->setValue(qApp->translate("Options.cpp", m_options.linearity().points().text().toUtf8()));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::updateMeasureViewPage(bool isDialog)
{
	PropertyPage* page = m_pageList[OptionPage::MeasureView_Column];
	if (page == nullptr)
	{
		return;
	}

	DialogOptionsMeasureViewHeader* dialog = dynamic_cast<DialogOptionsMeasureViewHeader*> (page->getWidget());
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

	m_activePage = static_cast<OptionPage>(s.value(QString("%1OptionsDialog/activePage").arg(WINDOW_GEOMETRY_OPTIONS_KEY), OptionPage::Linearity_Measure).toInt());
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
		saveSettings();
	}

	if (e->type() == QEvent::KeyRelease)
	{
		if (m_activePage >= 0 && m_activePage < TO_INT(m_pageList.size()))
		{
			PropertyPage* pActivePage = m_pageList.at(m_activePage);
			if (pActivePage != nullptr)
			{
				if (pActivePage->type() == PropertyPageType::List)
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


