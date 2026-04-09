#include <QSettings>

#include "DialogOptions.h"
#include "DialogMeasurePoint.h"
#include "DialogOptionsMvh.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PropertyPage::PropertyPage(Options* options, PropertyPageType pageType, ExtWidgets::PropertyEditor* pPropertyEditor)
	: m_options(options)
	, m_baseWidget(pPropertyEditor)
	, m_widgetType(PropertyPageWidgetType::List)
	, m_pageType(pageType)
{
	switch (pageType)
	{
		case PropertyPageType::Service_Connection:
			{
					QString categoryService = tr("Service");

					ADD_PROPERTY_GETTER_SETTER(	OT::ServerType, OT::serverConnectionParamCaption(OT::sco_Type), true, m_options->socket().type, m_options->socket().setType)
						->setCategory(categoryService)
						.setViewOrder(0);

					QString categoryConnection = tr("Server connection");

					ADD_PROPERTY_GETTER_SETTER(	OT::ServerPriority, OT::serverConnectionParamCaption(OT::sco_Priority), true, m_options->socket().priority, m_options->socket().setPriority)
						->setCategory(categoryConnection)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(	QString, OT::serverConnectionParamCaption(OT::sco_EquipmentID), true, m_options->socket().equipmentID, m_options->socket().setEquipmentID)
						->setCategory(categoryConnection)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(	QString, OT::serverConnectionParamCaption(OT::sco_ServerIP), true, m_options->socket().serverIP, m_options->socket().setServerIP)
						->setCategory(categoryConnection)
						.setViewOrder(2);
					ADD_PROPERTY_GETTER_SETTER(	int, OT::serverConnectionParamCaption(OT::sco_ServerPort), true, m_options->socket().serverPort, m_options->socket().setServerPort)
						->setCategory(categoryConnection)
						.setViewOrder(3);

					pPropertyEditor->setCategoryViewOrder (categoryService, 0);
					pPropertyEditor->setCategoryViewOrder (categoryConnection, 1);
			}
			break;

		case PropertyPageType::Module_Measure:
			{
					QString categoryIdentification = tr("Identification of module");

					ADD_PROPERTY_GETTER_SETTER(QString, OT::ModuleParamCaption(OT::mo_SuffixSN), true, m_options->module().suffixSN, m_options->module().setSuffixSN)
						->setCategory(categoryIdentification)
						.setViewOrder(0);

					QString categoryMeasure = tr("Measuring of module");

					ADD_PROPERTY_GETTER_SETTER(bool, OT::ModuleParamCaption(OT::mo_MeasureInterInsteadIn), true, m_options->module().measureInterInsteadIn, m_options->module().setMeasureInterInsteadIn)
						->setCategory(categoryMeasure)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::ModuleParamCaption(OT::mo_MeasureLinAdnCmp), true, m_options->module().measureLinAndCmp, m_options->module().setMeasureLinAndCmp)
						->setCategory(categoryMeasure)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::ModuleParamCaption(OT::mo_MeasureEntireModule), true, m_options->module().measureEntireModule, m_options->module().setMeasureEntireModule)
						->setCategory(categoryMeasure)
						.setViewOrder(2);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::ModuleParamCaption(OT::mo_ShowOnSchemas), true, m_options->module().measureShownOnSchemas, m_options->module().setMeasureShownOnSchemas)
						->setCategory(categoryMeasure)
						.setViewOrder(3);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::ModuleParamCaption(OT::mo_WarningIfMeasured), true, m_options->module().warningIfMeasured, m_options->module().setWarningIfMeasured)
						->setCategory(categoryMeasure)
						.setViewOrder(4);

					QString categoryLimits = tr("Limits");

					ADD_PROPERTY_GETTER_SETTER(int, OT::ModuleParamCaption(OT::mo_MaxInputs), true, m_options->module().maxInputCount, m_options->module().setMaxInputCount)
						->setCategory(categoryLimits)
						.setViewOrder(0);

					pPropertyEditor->setCategoryViewOrder (categoryIdentification, 0);
					pPropertyEditor->setCategoryViewOrder (categoryMeasure, 1);
					pPropertyEditor->setCategoryViewOrder (categoryLimits, 2);
			}
			break;

		case PropertyPageType::Linearity_Measure:
			{
					QString categoryErrors = tr("Metrological error");

					ADD_PROPERTY_GETTER_SETTER(double, OT::LinearityParamCaption(OT::lo_ErrorLimit), true, m_options->linearity().errorLimit, m_options->linearity().setErrorLimit)
						->setCategory(categoryErrors)
						.setViewOrder(0)
						.setPrecision(3);
					ADD_PROPERTY_GETTER_SETTER(Measure::MT::ErrorType, OT::LinearityParamCaption(OT::lo_ErrorType), true, m_options->linearity().errorType, m_options->linearity().setErrorType)
						->setCategory(categoryErrors)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(Measure::MT::CalcErrorRange, OT::LinearityParamCaption(OT::lo_CalcErrorByRange), true, m_options->linearity().calcErrorByRange, m_options->linearity().setCalcErrorByRange)
						->setCategory(categoryErrors)
						.setViewOrder(2);

					QString categoryMeasure = tr("Measurements at the single point");

					ADD_PROPERTY_GETTER_SETTER(int, OT::LinearityParamCaption(OT::lo_MeasureTime), true, m_options->linearity().measureTimeInPoint, m_options->linearity().setMeasureTimeInPoint)
						->setCategory(categoryMeasure)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(int, OT::LinearityParamCaption(OT::lo_MaxMeasuresInPoint), true, m_options->linearity().measureCountInPoint, m_options->linearity().setMeasureCountInPoint)
						->setCategory(categoryMeasure)
						.setViewOrder(1);

					QString categoryPoints = tr("Measurement points");

					ADD_PROPERTY_GETTER_SETTER(Measure::LT::LinearityDivision, OT::LinearityParamCaption(OT::lo_DivisionType), true, m_options->linearity().divisionType, m_options->linearity().setDivisionType)
						->setCategory(categoryPoints)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(int, OT::LinearityParamCaption(OT::lo_PointCount), true, m_options->linearity().measurePointsCount, m_options->linearity().setMeasurePointsCount)
						->setCategory(categoryPoints)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(double, OT::LinearityParamCaption(OT::lo_LowLimit), true, m_options->linearity().lowLimitRange, m_options->linearity().setLowLimitRange)
						->setCategory(categoryPoints)
						.setViewOrder(2)
						.setPrecision(1)
						.setReadOnly(m_options->linearity().divisionType() == Measure::LT::LinearityDivision::Manual);
					ADD_PROPERTY_GETTER_SETTER(double, OT::LinearityParamCaption(OT::lo_HighLimit), true, m_options->linearity().highLimitRange, m_options->linearity().setHighLimitRange)
						->setCategory(categoryPoints)
						.setViewOrder(3)
						.setPrecision(1)
						.setReadOnly(m_options->linearity().divisionType() == Measure::LT::LinearityDivision::Manual);
					ADD_PROPERTY_GETTER(QString, OT::LinearityParamCaption(OT::lo_ValuesOfPoints), true, m_options->linearity().measurePointsText)
						->setCategory(categoryPoints)
						.setViewOrder(4)
						.setPrecision(1)
						.setReadOnly(true);

					QString categoryViewType = tr("Type of displaying measurement list");

					ADD_PROPERTY_GETTER_SETTER(OT::LinearityViewType, OT::LinearityParamCaption(OT::lo_ViewType), true, m_options->linearity().viewType, m_options->linearity().setViewType)
						->setCategory(categoryViewType)
						.setViewOrder(0);

					pPropertyEditor->setCategoryViewOrder (categoryErrors, 0);
					pPropertyEditor->setCategoryViewOrder (categoryMeasure, 1);
					pPropertyEditor->setCategoryViewOrder (categoryPoints, 2);
					pPropertyEditor->setCategoryViewOrder (categoryViewType, 3);
			}
			break;

		case PropertyPageType::Comparator_Measure:
			{
					QString categoryErrors = tr("Metrological error");

					ADD_PROPERTY_GETTER_SETTER(double, OT::ComparatorParamCaption(OT::co_ErrorLimit), true, m_options->comparator().errorLimit, m_options->comparator().setErrorLimit)
						->setCategory(categoryErrors)
						.setViewOrder(0)
						.setPrecision(3);
					ADD_PROPERTY_GETTER_SETTER(Measure::MT::ErrorType, OT::ComparatorParamCaption(OT::co_ErrorType), true, m_options->comparator().errorType, m_options->comparator().setErrorType)
						->setCategory(categoryErrors)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(Measure::MT::CalcErrorRange, OT::ComparatorParamCaption(OT::co_CalcErrorByRange), true, m_options->comparator().calcErrorByRange, m_options->comparator().setCalcErrorByRange)
						->setCategory(categoryErrors)
						.setViewOrder(2);
					ADD_PROPERTY_GETTER_SETTER(double, OT::ComparatorParamCaption(OT::co_StartValue), true, m_options->comparator().startValueForCompare, m_options->comparator().setStartValueForCompare)
						->setCategory(categoryErrors)
						.setViewOrder(3)
						.setPrecision(3);

					QString categoryPermissions = tr("Permissions");

					ADD_PROPERTY_GETTER_SETTER(int, OT::ComparatorParamCaption(OT::co_StartFromComparator), true, m_options->comparator().startFromComparator, m_options->comparator().setStartFromComparator)
						->setCategory(categoryPermissions)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::ComparatorParamCaption(OT::co_MeasureHysteresis), true, m_options->comparator().enableMeasureHysteresis, m_options->comparator().setEnableMeasureHysteresis)
						->setCategory(categoryPermissions)
						.setViewOrder(1);

					pPropertyEditor->setCategoryViewOrder (categoryErrors, 0);
					pPropertyEditor->setCategoryViewOrder (categoryPermissions, 1);
			}
			break;

		case PropertyPageType::MeasureView_Text:
			{
					QString categoryFont = tr("Font");

					ADD_PROPERTY_GETTER_SETTER(QFont, OT::MeasureViewParamCaption(OT::mwo_Font), true, m_options->measureView().font, m_options->measureView().setFont)
						->setCategory(categoryFont)
						.setViewOrder(0);

					QString categoryColor = tr("Colors");

					ADD_PROPERTY_GETTER_SETTER(QColor, OT::MeasureViewParamCaption(OT::mwo_ColorNoError), true, m_options->measureView().colorNotError, m_options->measureView().setColorNotError)
						->setCategory(categoryColor)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::MeasureViewParamCaption(OT::mwo_ColorErrorOfLimit), true, m_options->measureView().colorErrorLimit, m_options->measureView().setColorErrorLimit)
						->setCategory(categoryColor)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::MeasureViewParamCaption(OT::mwo_ColorErrorOfControl), true, m_options->measureView().colorErrorControl, m_options->measureView().setColorErrorControl)
						->setCategory(categoryColor)
						.setViewOrder(2);

					QString categoryMeasurements = tr("Measurements");

					ADD_PROPERTY_GETTER_SETTER(bool, OT::MeasureViewParamCaption(OT::mwo_ShowNoValid), true, m_options->measureView().showNoValid, m_options->measureView().setShowNoValid)
						->setCategory(categoryMeasurements)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::MeasureViewParamCaption(OT::mwo_PrecesionByCalibrator), true, m_options->measureView().precesionByCalibrator, m_options->measureView().setPrecesionByCalibrator)
						->setCategory(categoryMeasurements)
						.setViewOrder(0);

					pPropertyEditor->setCategoryViewOrder (categoryFont, 0);
					pPropertyEditor->setCategoryViewOrder (categoryColor, 1);
					pPropertyEditor->setCategoryViewOrder (categoryMeasurements, 2);
			}
			break;

		case PropertyPageType::Panel_SignalInfo:
			{
					QString categoryFont = tr("Font");

					ADD_PROPERTY_GETTER_SETTER(QFont, OT::SignalInfoParamCaption(OT::sio_Font), true, m_options->signalInfo().font, m_options->signalInfo().setFont)
						->setCategory(categoryFont)
						.setViewOrder(0);

					QString categoryMeasure = tr("Displaying signal state");

					ADD_PROPERTY_GETTER_SETTER(bool, OT::SignalInfoParamCaption(OT::sio_ShowNoValid), true, m_options->signalInfo().showNoValid, m_options->signalInfo().setShowNoValid)
						->setCategory(categoryMeasure)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::SignalInfoParamCaption(OT::sio_ShowElectricState), true, m_options->signalInfo().showElectricState, m_options->signalInfo().setShowElectricState)
						->setCategory(categoryMeasure)
						.setViewOrder(1);

					QString categoryColor = tr("Colors");

					ADD_PROPERTY_GETTER_SETTER(QColor, OT::SignalInfoParamCaption(OT::sio_ColorFlagNoValid), true, m_options->signalInfo().colorFlagValid, m_options->signalInfo().setColorFlagValid)
						->setCategory(categoryColor)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::SignalInfoParamCaption(OT::sio_ColorFlagSim), true, m_options->signalInfo().colorFlagSim, m_options->signalInfo().setColorFlagSim)
						->setCategory(categoryColor)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::SignalInfoParamCaption(OT::sio_ColorFlagLock), true, m_options->signalInfo().colorFlagLock, m_options->signalInfo().setColorFlagLock)
						->setCategory(categoryColor)
						.setViewOrder(2);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::SignalInfoParamCaption(OT::sio_ColorFlagOverflow), true, m_options->signalInfo().colorFlagOverflow, m_options->signalInfo().setColorFlagOverflow)
						->setCategory(categoryColor)
						.setViewOrder(3);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::SignalInfoParamCaption(OT::sio_ColorFlagUnderflow), true, m_options->signalInfo().colorFlagUnderflow, m_options->signalInfo().setColorFlagUnderflow)
						->setCategory(categoryColor)
						.setViewOrder(4);

					QString categoryTime = tr("Time for updating");

					ADD_PROPERTY_GETTER_SETTER(int, OT::SignalInfoParamCaption(OT::sio_TimeForUpdate), true, m_options->signalInfo().timeForUpdate, m_options->signalInfo().setTimeForUpdate)
						->setCategory(categoryTime)
						.setViewOrder(0);

					pPropertyEditor->setCategoryViewOrder (categoryFont, 0);
					pPropertyEditor->setCategoryViewOrder (categoryMeasure, 1);
					pPropertyEditor->setCategoryViewOrder (categoryColor, 2);
					pPropertyEditor->setCategoryViewOrder (categoryTime, 3);
			}
			break;

		case PropertyPageType::Panel_ComparatorInfo:
			{
					QString categoryFont = tr("Font");

					ADD_PROPERTY_GETTER_SETTER(QFont, OT::ComparatorInfoParamCaption(OT::cio_Font), true, m_options->comparatorInfo().font, m_options->comparatorInfo().setFont)
						->setCategory(categoryFont)
						.setViewOrder(0);

					QString categoryColor = tr("Colors");

					ADD_PROPERTY_GETTER_SETTER(QColor, OT::ComparatorInfoParamCaption(OT::cio_ColorFlagSim), true, m_options->comparatorInfo().colorFlagSim, m_options->comparatorInfo().setColorFlagSim)
						->setCategory(categoryColor)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::ComparatorInfoParamCaption(OT::cio_ColorFlagLock), true, m_options->comparatorInfo().colorFlagSim, m_options->comparatorInfo().setColorFlagLock)
						->setCategory(categoryColor)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::ComparatorInfoParamCaption(OT::cio_ColorStateFalse), true, m_options->comparatorInfo().colorStateFalse, m_options->comparatorInfo().setColorStateFalse)
						->setCategory(categoryColor)
						.setViewOrder(2);
					ADD_PROPERTY_GETTER_SETTER(QColor, OT::ComparatorInfoParamCaption(OT::cio_ColorStateTrue), true, m_options->comparatorInfo().colorStateTrue, m_options->comparatorInfo().setColorStateTrue)
						->setCategory(categoryColor)
						.setViewOrder(3);

					QString categoryTime = tr("Time for updating");

					ADD_PROPERTY_GETTER_SETTER(int, OT::ComparatorInfoParamCaption(OT::cio_TimeForUpdate), true, m_options->comparatorInfo().timeForUpdate, m_options->comparatorInfo().setTimeForUpdate)
						->setCategory(categoryTime)
						.setViewOrder(0);

					pPropertyEditor->setCategoryViewOrder (categoryFont, 0);
					pPropertyEditor->setCategoryViewOrder (categoryColor, 1);
					pPropertyEditor->setCategoryViewOrder (categoryTime, 2);
			}
			break;

		case PropertyPageType::Database_Location:
			{
					QString typeDatabase = tr("Type of Database");

					ADD_PROPERTY_GETTER_SETTER(OT::DatabaseType, OT::DatabaseParamCaption(OT::dbo_Type), true, m_options->database().type, m_options->database().setType)
						->setCategory(typeDatabase)
						.setViewOrder(0);

					QString sqliteDatabase = tr("Configuration of SQLite database");

					ADD_PROPERTY_GETTER_SETTER(QString, OT::DatabaseParamCaption(OT::dbo_LocationPath), true, m_options->database().locationPath, m_options->database().setLocationPath)
						->setCategory(sqliteDatabase)
						.setViewOrder(0)
						.setSpecificEditor(E::PropertySpecificEditor::ChooseDirectoryDialog);

					QString postgresDatabase = tr("Configuration of Postgres database");

					ADD_PROPERTY_GETTER_SETTER(QString, OT::DatabaseParamCaption(OT::dbo_Ip), true, m_options->database().ip, m_options->database().setIp)
						->setCategory(postgresDatabase)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(int, OT::DatabaseParamCaption(OT::dbo_Port), true, m_options->database().port, m_options->database().setPort)
						->setCategory(postgresDatabase)
						.setViewOrder(1);
					ADD_PROPERTY_GETTER_SETTER(QString, OT::DatabaseParamCaption(OT::dbo_User), true, m_options->database().user, m_options->database().setUser)
						->setCategory(postgresDatabase)
						.setViewOrder(2);
					ADD_PROPERTY_GETTER_SETTER(QString, OT::DatabaseParamCaption(OT::dbo_Password), true, m_options->database().password, m_options->database().setPassword)
						->setCategory(postgresDatabase)
						.setPassword(true)
						.setViewOrder(3);

					pPropertyEditor->setCategoryViewOrder (typeDatabase, 0);
					pPropertyEditor->setCategoryViewOrder (sqliteDatabase, 1);
					pPropertyEditor->setCategoryViewOrder (postgresDatabase, 2);
			}
			break;

		case PropertyPageType::Database_Backup:
			{
					QString categoryEvent = tr("Events");

					ADD_PROPERTY_GETTER_SETTER(bool, OT::DatabaseParamCaption(OT::dbo_OnStart), true, m_options->database().onStart, m_options->database().setOnStart)
						->setCategory(categoryEvent)
						.setViewOrder(0);
					ADD_PROPERTY_GETTER_SETTER(bool, OT::DatabaseParamCaption(OT::dbo_OnExit), true, m_options->database().onExit, m_options->database().setOnExit)
						->setCategory(categoryEvent)
						.setViewOrder(1);

					QString categoryPath = tr("Location of reserve copy");

					ADD_PROPERTY_GETTER_SETTER(QString, OT::DatabaseParamCaption(OT::dbo_CopyPath), true, m_options->database().backupPath, m_options->database().setBackupPath)
						->setCategory(categoryPath)
						.setViewOrder(0)
						.setSpecificEditor(E::PropertySpecificEditor::ChooseDirectoryDialog);

					pPropertyEditor->setCategoryViewOrder (categoryEvent, 0);
					pPropertyEditor->setCategoryViewOrder (categoryPath, 1);
			}
			break;

		case PropertyPageType::Language_App:
			{
				QString categoryLanguage = tr("Language of application ");

				ADD_PROPERTY_GETTER_SETTER(OT::LanguageType, OT::LanguageParamCaption(OT::LanguageParam::lno_LanguageType), true, m_options->language().languageType, m_options->language().setLanguageType)
					->setCategory(categoryLanguage)
					.setViewOrder(0);
			}
			break;

		default:
			assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::PropertyPage(Options* options, PropertyPageType pageType)
	: m_options(options)
	, m_baseWidget(nullptr)
	, m_widgetType(PropertyPageWidgetType::Dialog)
	, m_pageType(pageType)
{

	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return;
	}

	switch (pageType)
	{
		case PropertyPageType::Linearity_Point:
			{
				DialogMeasurePoint* pDialog = new DialogMeasurePoint(m_options->linearity());
				if (pDialog == nullptr)
				{
					break;
				}

				connect(pDialog, &DialogMeasurePoint::dataUpdated, this, &PropertyPage::dialogDataUpdated, Qt::QueuedConnection);

				m_baseWidget = pDialog;
			}
			break;

		case PropertyPageType::MeasureView_Column:
			{
				DialogOptionsMeasureViewHeader* pDialog = new DialogOptionsMeasureViewHeader(m_options->measureView());
				if (pDialog == nullptr)
				{
					break;
				}

				connect(pDialog, &DialogOptionsMeasureViewHeader::dataUpdated, this, &PropertyPage::dialogDataUpdated, Qt::QueuedConnection);

				m_baseWidget = pDialog;
			}
			break;

		default:
			assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

PropertyPage::~PropertyPage()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

void PropertyPage::clear()
{
	if (m_widgetType == PropertyPageWidgetType::Dialog)
	{
		QDialog* pDialog = dynamic_cast<QDialog*>(m_baseWidget);
		if (pDialog != nullptr)
		{
			delete pDialog;
		}
	}

	m_baseWidget = nullptr;
	m_widgetType = PropertyPageWidgetType::NoWidgetType;

	m_pageType = PropertyPageType::NoPageType;
	m_pageTreeItem = nullptr;
}

// -------------------------------------------------------------------------------------------------------------------

void PropertyPage::dialogDataUpdated()
{
	if (ERR_PROPERTY_PAGE_TYPE(m_pageType) == true)
	{
		return;
	}

	if (m_baseWidget == nullptr)
	{
		return;
	}

	if (m_widgetType != PropertyPageWidgetType::Dialog)
	{
		return;
	}

	emit dataUpdated(m_pageType);
}

// -------------------------------------------------------------------------------------------------------------------

QString groupCaption(PropertyGroupType groupType)
{
	QString caption;

	switch (groupType)
	{
		case PropertyGroupType::Service:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Connect to server");		break;
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
		case PropertyPageType::Service_Connection:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection to service - TCP/IP");					break;
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
		case PropertyPageType::Service_Connection:		caption = QT_TRANSLATE_NOOP("DialogOptions", "Connection");					break;
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
		case PropertyPageType::Service_Connection:		group = PropertyGroupType::Service;		break;
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

PropertyPageType DialogOptions::m_currentPage = PropertyPageType::Service_Connection;

// -------------------------------------------------------------------------------------------------------------------

DialogOptions::DialogOptions(const Options& options, QWidget* parent) :
	QDialog(parent),
	m_options(options)
{
	for(int measureType = 0; measureType < Measure::TYPE_COUNT; measureType++)
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

	QRect screen = parentWidget()->screen()->availableGeometry();
	setMinimumSize(static_cast<int>(screen.width() * 0.45), static_cast<int>(screen.height() * 0.3));
	loadSettings();

	// TreeList for select pages
	//
	m_pagesTree = new QTreeWidget;
	m_pagesTree->setHeaderHidden(true);
	m_pagesTree->setFixedWidth(static_cast<int>(screen.width() * 0.1));

	// Layout for pages: show or hide pages
	//
	m_pagesLayout = new QHBoxLayout ;

	// Editor for properties
	//
	m_pPropertyEditor = new ExtWidgets::PropertyEditor(this);
	m_pPropertyEditor->setSplitterPosition(300);
	m_pPropertyEditor->setReadOnly(false);
	m_pPropertyEditor->hide();

	connect(m_pPropertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &DialogOptions::onPropertyValueChanged);

	// create property pages and append to pagesTree and pagesLayout
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
	setCurrentPage(m_currentPage);
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

		if (groupType < 0 || groupType >= static_cast<int>((groupList.size())))
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

	connect(m_pagesTree, &QTreeWidget::currentItemChanged , this, &DialogOptions::onPageChanged, Qt::QueuedConnection);

	//
	//
	m_pagesLayout->addWidget(m_pagesTree);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::removePropertyPages()
{
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
		case PropertyPageType::Service_Connection:
		case PropertyPageType::Linearity_Measure:
		case PropertyPageType::Comparator_Measure:
		case PropertyPageType::Module_Measure:
		case PropertyPageType::MeasureView_Text:
		case PropertyPageType::Panel_SignalInfo:
		case PropertyPageType::Panel_ComparatorInfo:
		case PropertyPageType::Database_Location:
		case PropertyPageType::Database_Backup:
		case PropertyPageType::Language_App:
			{
				if (m_pPropertyEditor == nullptr)
				{
					break;
				}

				pPropertyPage = std::make_shared<PropertyPage>(&m_options, pageType, m_pPropertyEditor);
			}
			break;

		case PropertyPageType::Linearity_Point:
		case PropertyPageType::MeasureView_Column:
			{
				pPropertyPage = std::make_shared<PropertyPage>(&m_options, pageType);

				if (pPropertyPage.get() != nullptr)
				{
					connect(pPropertyPage.get(), &PropertyPage::dataUpdated, this, &DialogOptions::dialogDataUpdated, Qt::QueuedConnection);
				}
			}
			break;

		default:
			assert(0);
	}

	return pPropertyPage;
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::pageTypeIsValid(PropertyPageType pageType)
{
	if (ERR_PROPERTY_PAGE_TYPE(pageType) == true)
	{
		return false;
	}

	if (pageType < 0 || pageType >= static_cast<int>((m_pagesList.size())))
	{
		return false;
	}

	std::shared_ptr<PropertyPage> page = m_pagesList.at(pageType);
	if (page.get() == nullptr)
	{
		return false;
	}

	if (ERR_PROPERTY_PAGE_TYPE(page->pageType()) == true)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::onPageChanged(QTreeWidgetItem* currentGroup, QTreeWidgetItem* previousGroup)
{
	if (currentGroup == nullptr || previousGroup == nullptr)
	{
		return;
	}

	// if group has pages, select first page of group
	//
	if (currentGroup->childCount() >= 1)
	{
		currentGroup->setExpanded(true);
		currentGroup->setSelected(false);
		currentGroup = currentGroup->child(0);
		currentGroup->setSelected(true);
	}

	// get pageType
	//
	PropertyPageType pageType = static_cast<PropertyPageType>(currentGroup->data(0, Qt::UserRole).toInt());
	if (pageTypeIsValid(pageType) == false)
	{
		return;
	}

	if (m_currentPage == pageType)
	{
		return;
	}

	// set active page
	//
	setCurrentPage(pageType);
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::setCurrentPage(PropertyPageType pageType)
{
	// hide active page
	//
	bool resultHide = hidePage(m_currentPage);
	if (resultHide == false)
	{
		return false;
	}

	// show new page
	//
	bool resultShow = showPage(pageType);
	if (resultShow == false)
	{
		return false;
	}

	// set new active page
	//
	m_currentPage = pageType;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::hidePage(PropertyPageType pageType)
{
	if (m_pagesLayout == nullptr)
	{
		return false;
	}

	if (pageTypeIsValid(pageType) == false)
	{
		return false;
	}

	std::shared_ptr<PropertyPage> page = m_pagesList.at(pageType);
	if (page.get() == nullptr)
	{
		return false;
	}

	QWidget* pWidget = page->baseWidget();
	if (pWidget == nullptr)
	{
		return false;
	}

	//
	//
	m_pagesLayout->removeWidget(pWidget);
	pWidget->hide();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::showPage(PropertyPageType pageType)
{
	if (m_pagesLayout == nullptr)
	{
		return false;
	}

	if (m_pagesTree == nullptr)
	{
		return false;
	}

	if (pageTypeIsValid(pageType) == false)
	{
		return false;
	}

	std::shared_ptr<PropertyPage> page = m_pagesList.at(pageType);
	if (page.get() == nullptr)
	{
		return false;
	}

	if (ERR_PROPERTY_PAGE_WIDGET_TYPE(page->widgetType()) == true)
	{
		return false;
	}

	QWidget* pWidget = page->baseWidget();
	if (pWidget == nullptr)
	{
		return false;
	}

	// set property object in property editor
	//
	switch(page->widgetType())
	{
		case PropertyPageWidgetType::List:
			{
				ExtWidgets::PropertyEditor* pPropertyEditor = dynamic_cast<ExtWidgets::PropertyEditor*>(pWidget);
				if (pPropertyEditor != nullptr)
				{
					QList<std::shared_ptr<PropertyObject>> propertyObjects;
					propertyObjects.push_back(page);

					pPropertyEditor->setObjects(propertyObjects);
				}
			}
			break;

		case PropertyPageWidgetType::Dialog:
			{
				switch(pageType)
				{
					case PropertyPageType::Linearity_Point:
						{
							DialogMeasurePoint* pDialog = dynamic_cast<DialogMeasurePoint*>(pWidget);
							if (pDialog == nullptr)
							{
								break;
							}

							pDialog->setLinearity(m_options.linearity());	// set options to dialog
						}
						break;

					case PropertyPageType::MeasureView_Column:
						{
							DialogOptionsMeasureViewHeader* pDialog = dynamic_cast<DialogOptionsMeasureViewHeader*>(pWidget);
							if (pDialog == nullptr)
							{
								break;
							}

							pDialog->header() = m_options.measureView();	// set options to dialog
						}
						break;

					default:
						assert(0);
				}
			}
			break;

		default:
			assert(0);
			break;
	}

	// setWindowTitle
	//
	setWindowTitle(tr("Options - %1").arg(pageCaption(pageType)));

	//
	//
	m_pagesLayout->addWidget(pWidget);
	pWidget->show();

	// select item in tree
	//
	if (page->pageTreeItem() != nullptr)
	{
		m_pagesTree->setCurrentItem(page->pageTreeItem());
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::onPropertyValueChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	if (m_pPropertyEditor == nullptr)
	{
		return;
	}

	for (const std::shared_ptr<PropertyObject>& modifiedFilter : objects)
	{
		auto properties = modifiedFilter.get();
		if (properties == nullptr)
		{
			assert(0);
			continue;
		}

		switch (m_currentPage)
		{
			case PropertyPageType::Linearity_Measure:
				{
					auto propertyDivisionType = properties->propertyByCaption(OT::LinearityParamCaption(OT::lo_DivisionType));
					if (propertyDivisionType != nullptr)
					{
						auto propertyPointCount = properties->propertyByCaption(OT::LinearityParamCaption(OT::lo_PointCount));
						if (propertyPointCount != nullptr)
						{
							propertyPointCount->setReadOnly(m_options.linearity().divisionType() == Measure::LT::LinearityDivision::Manual);
						}

						auto propertyLowLimit = properties->propertyByCaption(OT::LinearityParamCaption(OT::lo_LowLimit));
						if (propertyLowLimit != nullptr)
						{
							propertyLowLimit->setReadOnly(m_options.linearity().divisionType() == Measure::LT::LinearityDivision::Manual);
						}

						auto propertyHighLimit = properties->propertyByCaption(OT::LinearityParamCaption(OT::lo_HighLimit));
						if (propertyHighLimit != nullptr)
						{
							propertyHighLimit->setReadOnly(m_options.linearity().divisionType() == Measure::LT::LinearityDivision::Manual);
						}
					}
				}
				break;

			case PropertyPageType::MeasureView_Text:
				{
					// if any property has been changed on MeasureView_Text
					// then update measure list
					//
					for(int measureType = 0; measureType < Measure::TYPE_COUNT; measureType++)
					{
						m_options.measureView().setUpdateColumnView(static_cast<Measure::Type>(measureType), true);
					}
				}
				break;

			default:
				break;
		}
	}

	m_pPropertyEditor->updatePropertiesValues();
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::dialogDataUpdated(PropertyPageType pageType)
{
	if (pageTypeIsValid(pageType) == false)
	{
		return;
	}

	std::shared_ptr<PropertyPage> page = m_pagesList.at(pageType);
	if (page.get() == nullptr)
	{
		return;
	}

	PropertyPageWidgetType pageWidgetType = page->widgetType();
	if (ERR_PROPERTY_PAGE_WIDGET_TYPE(pageWidgetType) == true)
	{
		return;
	}

	if (pageWidgetType != PropertyPageWidgetType::Dialog)
	{
		return;
	}

	QWidget* pWidget = page->baseWidget();
	if (pWidget == nullptr)
	{
		return;
	}

	// load data from dialog to options
	//
	switch (pageType)
	{
		case PropertyPageType::Linearity_Point:
			{
				DialogMeasurePoint* dialog = dynamic_cast<DialogMeasurePoint*>(pWidget);
				if (dialog == nullptr)
				{
					break;
				}

				m_options.setLinearity(dialog->linearity());						// get options from dialog
			}
			break;

		case PropertyPageType::MeasureView_Column:
			{
				DialogOptionsMeasureViewHeader* dialog = dynamic_cast<DialogOptionsMeasureViewHeader*> (pWidget);
				if (dialog == nullptr)
				{
					break;
				}

				m_options.setMeasureView(dialog->header());

				Measure::Type measureType = dialog->measureType();
				if (ERR_MEASURE_TYPE(measureType) == true)
				{
					break;
				}

				m_options.measureView().setUpdateColumnView(measureType, true);		// get options from dialog
			}
			break;

		default:
			assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::loadSettings()
{
	QSettings s;

	QByteArray geometry = s.value(QString("%1OptionsDialog/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY)).toByteArray();
	restoreGeometry(geometry);

	m_currentPage = static_cast<PropertyPageType>(s.value(QString("%1OptionsDialog/activePage").arg(WINDOW_GEOMETRY_OPTIONS_KEY), PropertyPageType::Linearity_Measure).toInt());

	if (ERR_PROPERTY_PAGE_TYPE(m_currentPage) == true)
	{
		m_currentPage = PropertyPageType::Service_Connection;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::saveSettings()
{
	QSettings s;

	s.setValue(QString("%1OptionsDialog/Geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY), saveGeometry());
	s.setValue(QString("%1OptionsDialog/activePage").arg(WINDOW_GEOMETRY_OPTIONS_KEY), m_currentPage);
}

// -------------------------------------------------------------------------------------------------------------------

void DialogOptions::onOk()
{
	accept();
}

// -------------------------------------------------------------------------------------------------------------------

bool DialogOptions::event(QEvent*  e)
{
	switch (e->type())
	{
		case QEvent::Hide:
			{
				removePropertyPages();
				saveSettings();
			}
			break;

		case QEvent::KeyPress:
		case QEvent::KeyRelease:
			{

				QKeyEvent* keyEvent = static_cast<QKeyEvent* >(e);

				switch (keyEvent->key())
				{
					case Qt::Key_Escape:
					case Qt::Key_Enter:
					case Qt::Key_Return:
						e->ignore();
						return true;
					default:
						break;
				}
			}
			break;

		default:
			break;
	}

	return QDialog::event(e);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


