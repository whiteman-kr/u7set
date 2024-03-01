#include "SchemasReportGenerator.h"
#include "../VFrame30/Context.h"
#include "../VFrame30/LogicSchema.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaDetails.h"
#include "../VFrame30/SchemaItems/SchemaItemConnection.h"
#include "../VFrame30/SchemaItems/SchemaItemLoopback.h"
#include "../VFrame30/SchemaItems/SchemaItemSignal.h"

namespace Builder
{
	using namespace ReportLib;


	SchemasReportOptions SchemasReportOptions::optionsForSingleSchema()
	{
		SchemasReportOptions options;

		// All options are cleared

		return options;
	}

	SchemasReportOptions SchemasReportOptions::optionsForSchemasAlbum(DbController* db)
	{
		SchemasReportOptions options;

		// Default options are set

		options.setFooters(true);
		options.setTableOfContents(true);

		options.load(db);
		return options;
	}

	//
	// SchemasReportOptions
	//
	bool SchemasReportOptions::load(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		QString value;
		db->getUserProperty("SchemasReportOptions.singleFile", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			m_singleFile = (value == "true") ? true : false;
		}

		db->getUserProperty("SchemasReportOptions.footers", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			m_footers = (value == "true") ? true : false;
		}

		db->getUserProperty("SchemasReportOptions.itemsLabels", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			m_itemsLabels = (value == "true") ? true : false;
		}

		db->getUserProperty("SchemasReportOptions.folders", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			m_folders = (value == "true") ? true : false;
		}

		db->getUserProperty("SchemasReportOptions.tableOfContents", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			m_tableOfContents = (value == "true") ? true : false;
		}

		db->getUserProperty("SchemasReportOptions.signalsDetails", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			m_signalsDetails = (value == "true") ? true : false;
		}

		db->getUserProperty("SchemasReportOptions.startPageNumber", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			setStartPageNumber(value.toInt());
		}

		db->getUserProperty("SchemasReportOptions.contentsTextFontSize", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			setContentsTextFontSize(value.toInt());
		}

		db->getUserProperty("SchemasReportOptions.contentsTableFontSize", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			setContentsTableFontSize(value.toInt());
		}

		db->getUserProperty("SchemasReportOptions.textFontSize", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			setTextFontSize(value.toInt());
		}

		db->getUserProperty("SchemasReportOptions.tableFontSize", &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			setTableFontSize(value.toInt());
		}

		// Load schema tags
		//
		db->getUserProperty("SchemasReportOptions.schemaTags", &value, QString(), nullptr);
		m_schemaTags.clear();
		QStringList tagsStrings = value.split(";", Qt::SkipEmptyParts);
		for (const QString& tagsString : tagsStrings)
		{
			QStringList ts = tagsString.split("=");
			if (ts.size() != 2)
			{
				Q_ASSERT(false);
				continue;
			}
			m_schemaTags[ts[0]] = ts[1] == "1" ? true : false;
		}

		// Load user variables
		//
		{
			value.clear();
			db->getUserProperty("SchemasReportOptions.userVariablesNames", &value, nullptr);
			QStringList variablesNames = value.split("`", Qt::SkipEmptyParts);

			value.clear();
			db->getUserProperty("SchemasReportOptions.userVariablesValues", &value, nullptr);
			QStringList variablesValues = value.split("`", Qt::SkipEmptyParts);

			m_userVariables.clear();
			if (variablesNames.size() == variablesValues.size())
			{
				int size = variablesNames.size();
				for (int i = 0; i < size; i++)
				{
					m_userVariables[variablesNames[i]] = variablesValues[i];
				}
			}
		}

		// Load project variables
		//
		{
			value.clear();
			db->getProjectProperty("SchemasReportOptions.projectVariablesNames", &value, nullptr);
			QStringList variablesNames = value.split("`", Qt::SkipEmptyParts);

			value.clear();
			db->getProjectProperty("SchemasReportOptions.projectVariablesValues", &value, nullptr);
			QStringList variablesValues = value.split("`", Qt::SkipEmptyParts);

			m_projectVariables.clear();
			if (variablesNames.size() == variablesValues.size())
			{
				int size = variablesNames.size();
				for (int i = 0; i < size; i++)
				{
					m_projectVariables[variablesNames[i]] = variablesValues[i];
				}
			}
		}

		return true;
	}

	bool SchemasReportOptions::save(DbController* db)
	{
		if (db == nullptr)
		{
			Q_ASSERT(db);
			return false;
		}

		db->setUserProperty("SchemasReportOptions.singleFile", singleFile() ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.folders", folders() ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.tableOfContents", tableOfContents() ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.footers", footers() ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.itemsLabels", itemsLabels() ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.signalsDetails", signalsDetails() ? "true" : "false", nullptr);
		db->setUserProperty("SchemasReportOptions.startPageNumber", QString::number(startPageNumber()), nullptr);

		db->setUserProperty("SchemasReportOptions.contentsTextFontSize", QString::number(contentsTextFontSize()), nullptr);
		db->setUserProperty("SchemasReportOptions.contentsTableFontSize", QString::number(contentsTableFontSize()), nullptr);
		db->setUserProperty("SchemasReportOptions.textFontSize", QString::number(textFontSize()), nullptr);
		db->setUserProperty("SchemasReportOptions.tableFontSize", QString::number(tableFontSize()), nullptr);

		// Save schema tags
		//
		QString tagsString;
		for (const auto& it : m_schemaTags)
		{
			tagsString.append(it.first + "=");
			tagsString.append(it.second == true ? "1;" : "0;");
		}
		db->setUserProperty("SchemasReportOptions.schemaTags", tagsString, nullptr);

		// Save user variables
		//
		{
			QStringList variablesNames;
			QStringList variablesValues;
			for (const auto& [name, value] : m_userVariables)
			{
				variablesNames.push_back(name);
				variablesValues.push_back(value);
			}
			db->setUserProperty("SchemasReportOptions.userVariablesNames", variablesNames.join('`'), nullptr);
			db->setUserProperty("SchemasReportOptions.userVariablesValues", variablesValues.join('`'), nullptr);
		}

		// Save project variables
		//
		{
			QStringList variablesNames;
			QStringList variablesValues;
			for (const auto& [name, value] : m_projectVariables)
			{
				variablesNames.push_back(name);
				variablesValues.push_back(value);
			}
			db->setProjectProperty("SchemasReportOptions.projectVariablesNames", variablesNames.join('`'), nullptr);
			db->setProjectProperty("SchemasReportOptions.projectVariablesValues", variablesValues.join('`'), nullptr);
		}

		return true;
	}

	void SchemasReportOptions::setSignleFile(bool value)
	{
		m_singleFile = value;
	}

	bool SchemasReportOptions::singleFile() const
	{
		return m_singleFile;
	}

	void SchemasReportOptions::setFooters(bool value)
	{
		m_footers = value;
	}

	bool SchemasReportOptions::footers() const
	{
		return m_footers;
	}

	void SchemasReportOptions::setItemsLabels(bool value)
	{
		m_itemsLabels = value;
	}

	bool SchemasReportOptions::itemsLabels() const
	{
		return m_itemsLabels;
	}

	void SchemasReportOptions::setFolders(bool value)
	{
		m_folders = value;
	}

	bool SchemasReportOptions::folders() const
	{
		return m_folders;
	}

	void SchemasReportOptions::setTableOfContents(bool value)
	{
		m_tableOfContents = value;
	}

	bool SchemasReportOptions::tableOfContents() const
	{
		return m_tableOfContents;
	}

	void SchemasReportOptions::setSignalsDetails(bool value)
	{
		m_signalsDetails = value;
	}

	bool SchemasReportOptions::signalsDetails() const
	{
		return m_signalsDetails;
	}

	void SchemasReportOptions::setStartPageNumber(int value)
	{
		m_startPageNumber = value;
	}

	int SchemasReportOptions::startPageNumber() const
	{
		return m_startPageNumber;
	}

	void SchemasReportOptions::setContentsTextFontSize(int value)
	{
		 m_contentsTextFontSize = value;
	}
	
	int SchemasReportOptions::contentsTextFontSize() const
	{
		return m_contentsTextFontSize;
	}

	void SchemasReportOptions::setContentsTableFontSize(int value)
	{
		m_contentsTableFontSize = value;
	}
	
	int SchemasReportOptions::contentsTableFontSize() const
	{
		return m_contentsTableFontSize;
	}

	void SchemasReportOptions::setTextFontSize(int value)
	{
		m_textFontSize = value;	 
	}

	int SchemasReportOptions::textFontSize() const
	{
		return m_textFontSize;
	}
	
	void SchemasReportOptions::setTableFontSize(int value)
	{
		m_tableFontSize = value;
	}
	
	int SchemasReportOptions::tableFontSize() const
	{
		return m_tableFontSize;
	}

	void SchemasReportOptions::setSchemaTags(const std::set<QString>& tagsSet)
	{
		for (const QString& tag : tagsSet)
		{
			if (m_schemaTags.find(tag) == m_schemaTags.end())
			{
				m_schemaTags[tag] = true;
			}
		}
	}

	const std::map<QString, bool>& SchemasReportOptions::schemaTags() const
	{
		return m_schemaTags;
	}

	std::map<QString, bool>& SchemasReportOptions::schemaTags()
	{
		return m_schemaTags;
	}

	void SchemasReportOptions::setUserVariables(const std::map<QString, QString>& variables)
	{
		m_userVariables = variables;
	}

	const std::map<QString, QString>& SchemasReportOptions::userVariables() const
	{
		return m_userVariables;
	}

	void SchemasReportOptions::setProjectVariables(const std::map<QString, QString>& variables)
	{
		m_projectVariables = variables;
	}

	const std::map<QString, QString>& SchemasReportOptions::projectVariables() const
	{
		return m_projectVariables;
	}

	//
	// SchemasReportFileTypeParams
	//
	SchemaTypesParams::SchemaTypesParams(int fileId, const QString& caption, bool selected, const QPageLayout& initPageLayout, const QStringList& layoutNames) :
		m_fileId(fileId),
		m_caption(caption),
		m_selected(selected)
	{
		Q_ASSERT(layoutNames.size() == 1 || layoutNames.size() == 2);	// For text and schemas

		for (const QString& layoutName : layoutNames)
		{
			m_pageLayouts.push_back({layoutName, initPageLayout, false});
		}
	}

	int SchemaTypesParams::fileId() const
	{
		return m_fileId;
	}

	bool SchemaTypesParams::hasFileId() const
	{
		return m_fileId != -1;
	}
	
	const QString& SchemaTypesParams::caption() const
	{
		return m_caption;
	}

	QPageLayout SchemaTypesParams::pageLayoutWithMargins(int index) const
	{
		QPageLayout l = pageLayout(index);
		if (noMargins(index) == true)
		{
			l.setMargins(QMarginsF(0, 0, 0, 0));
		}
		return l;
	}

			
	bool SchemaTypesParams::selected() const
	{
		return m_selected;
	}

	void SchemaTypesParams::setSelected(bool value)
	{
		m_selected = value;
	}

	int SchemaTypesParams::pageLayoutCount() const
	{
		return std::ssize(m_pageLayouts);
	}

	const QString& SchemaTypesParams::pageLayoutCaption(int index) const
	{
		static QString err;
		if (index < 0 || index >= m_pageLayouts.size())
		{
			Q_ASSERT(false);
			return err;
		}
		return m_pageLayouts[index].caption;
	}

	void SchemaTypesParams::setPageLayoutCaption(int index, const QString& value)
	{
		if (index < 0 || index >= m_pageLayouts.size())
		{
			Q_ASSERT(false);
			return;
		}
		m_pageLayouts[index].caption = value;
	}

	const QPageLayout& SchemaTypesParams::pageLayout(int index) const
	{
		static QPageLayout err;
		if (index < 0 || index >= m_pageLayouts.size())
		{
			Q_ASSERT(false);
			return err;
		}
		return m_pageLayouts[index].layout;
	}

	void SchemaTypesParams::setPageLayout(int index, const QPageLayout& layout)
	{
		if (index < 0 || index >= m_pageLayouts.size())
		{
			Q_ASSERT(false);
			return;
		}
		m_pageLayouts[index].layout = layout;
	}

	bool SchemaTypesParams::noMargins(int index) const
	{
		if (index < 0 || index >= m_pageLayouts.size())
		{
			Q_ASSERT(false);
			return false;
		}
		return m_pageLayouts[index].noMargins;	
	}

	void SchemaTypesParams::setNoMargins(int index, bool value)
	{
		if (index < 0 || index >= m_pageLayouts.size())
		{
			Q_ASSERT(false);
			return;
		}
		m_pageLayouts[index].noMargins = value;
	}

	bool SchemaTypesParams::load(DbController* db)
	{
		QString value;
		db->getUserProperty(QObject::tr("SchemaTypesParams.%1.selected").arg(caption()), &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			setSelected(value == "true" ? true : false);
		}

		int count = 0;
		db->getUserProperty(QObject::tr("SchemaTypesParams.%1.count").arg(caption()), &value, QString(), nullptr);
		if (value.isEmpty() == false)
		{
			count = value.toInt();
		}

		if (count > m_pageLayouts.size())
		{
			// Maybe some saved version has been changed
			Q_ASSERT(false);
			count = std::ssize(m_pageLayouts);
		}

		for (int i = 0; i < count; i++)
		{
			db->getUserProperty(QObject::tr("SchemaTypesParams.%1.%2.noMargins").arg(i).arg(caption()), &value, QString(), nullptr);
			if (value.isEmpty() == false)
			{
				setNoMargins(i, value == "true" ? true : false);
			}

			bool layoutOk = false;
			bool orientationOk = false;
			bool marginsOk = false;
			QPageLayout l(m_pageLayouts[i].layout);

			db->getUserProperty(QObject::tr("SchemaTypesParams.%1.%2.pageLayout").arg(i).arg(caption()), &value, QString(), nullptr);
			if (value.isEmpty() == false)
			{
				bool ok = false;
				int id = value.toInt(&ok);
				if (ok == true)
				{
					l.setPageSize(QPageSize(static_cast<QPageSize::PageSizeId>(id)));
					layoutOk = true;
				}
			}

			db->getUserProperty(QObject::tr("SchemaTypesParams.%1.%2.orientation").arg(i).arg(caption()), &value, QString(), nullptr);
			if (value.isEmpty() == false)
			{
				l.setOrientation(value == "portrait" ? QPageLayout::Portrait : QPageLayout::Landscape);
				orientationOk = true;
			}

			db->getUserProperty(QObject::tr("SchemaTypesParams.%1.%2.margins").arg(i).arg(caption()), &value, QString(), nullptr);
			if (value.isEmpty() == false)
			{
				QStringList ms = value.split(";");
				if (ms.size() == 4)
				{
					l.setMargins(QMarginsF(ms[0].toDouble(), ms[1].toDouble(), ms[2].toDouble(), ms[3].toDouble()));
					marginsOk = true;
				}
			}

			if (layoutOk == true && orientationOk == true && marginsOk == true)
			{
				setPageLayout(i, l);
			}
		}

		return true;
	}

	bool SchemaTypesParams::save(DbController* db) const
	{
		db->setUserProperty(QObject::tr("SchemaTypesParams.%1.selected").arg(caption()), selected() ? "true" : "false", nullptr);
		db->setUserProperty(QObject::tr("SchemaTypesParams.%1.count").arg(caption()), QObject::tr("%1").arg(m_pageLayouts.size()), nullptr);

		for (int i = 0; i < m_pageLayouts.size(); i++)
		{
			const QPageLayout& layout = m_pageLayouts[i].layout;
			bool noMargins = m_pageLayouts[i].noMargins;

			db->setUserProperty(QObject::tr("SchemaTypesParams.%1.%2.noMargins").arg(i).arg(caption()), noMargins ? "true" : "false", nullptr);


			QPageSize::PageSizeId id = QPageSize::id(layout.pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
			if (id == QPageSize::Custom)
			{
				id = QPageSize::A4;
			}
			db->setUserProperty(QObject::tr("SchemaTypesParams.%1.%2.pageLayout").arg(i).arg(caption()), QString::number(id), nullptr);

			db->setUserProperty(QObject::tr("SchemaTypesParams.%1.%2.orientation").arg(i).arg(caption()), layout.orientation() == QPageLayout::Portrait ? "portrait" : "landscape", nullptr);

			QMarginsF margins = layout.margins();
			QString marginsStr = QObject::tr("%1;%2;%3;%4").arg(margins.left()).arg(margins.top()).arg(margins.right()).arg(margins.bottom());
			db->setUserProperty(QObject::tr("SchemaTypesParams.%1.%2.margins").arg(i).arg(caption()), marginsStr, nullptr);
		}

		return true;
	}


	//
	// SchemaInfo
	//
	SchemaInfo::SchemaInfo(const QString& fullFileName, const std::shared_ptr<VFrame30::Schema>& schema):
		m_folder(fullFileName),
		m_fileName(fullFileName),
		m_schema(schema)
	{
		int pos = m_folder.lastIndexOf('/');
		if (pos != -1)
		{
			m_folder.chop(m_folder.length() - pos);
			m_fileName.remove(0, pos + 1);
		}
	}

	const QString& SchemaInfo::folder() const
	{
		return m_folder;
	}

	const QString& SchemaInfo::fileName() const
	{
		return m_fileName;
	}

	const std::shared_ptr<VFrame30::Schema>& SchemaInfo::schema() const
	{
		return m_schema;
	}

	//
	// SchemaSignalInfo
	//

	SchemaSignalInfo::SchemaSignalInfo(const VFrame30::FblItemRect* item, const QString& appSignalId, const QStringList& otherSchemasIds, IAppSignalManager& appSignals)
	{
		input = item->isInputSignalElement();
		received = item->isReceiverElement();
		commented = item->commented();
		x = item->left();
		y = item->top();
		signalId = appSignalId;

		bool found = false;
		AppSignalParam asp = appSignals.signalParam(signalId, &found);

		if (found == true)
		{
			caption = asp.caption();
		}
		else
		{
			caption = QObject::tr("<font color=\"red\">%1</font>").arg(signalId);
		}

		if (item->isSignalElement() == true)
		{
			impact = item->toSignalElement()->impactAppSignalIdList().contains(signalId);
		}

		if (otherSchemasIds.empty() == false)
		{
			schemasList = otherSchemasIds.join("<br>");
		}
		else
		{
			if (received == true)
			{
				schemasList = QObject::tr("This Schema Only");
			}
			else
			{
				schemasList = input ? QObject::tr("Start Point") : QObject::tr("End Point");
			}
		}

		if (item->textColor() != Qt::black)
		{
			color = item->textColor().name();
		}
	}

	QStringList SchemaSignalInfo::toStringList() const
	{
		QStringList l;

		if (color.isEmpty() == false)
		{
			l.push_back(QObject::tr("<font color=\"%1\">%2</font>").arg(color).arg(signalId));
		}
		else
		{
			l.push_back(signalId);
		}

		l.push_back(caption);

		QString typeStr;
		if (received == true)
		{
			typeStr = QObject::tr("Received");
		}
		else
		{
			typeStr = input ? QObject::tr("Input") : QObject::tr("Output");
		}
		if (impact == true)
		{
			typeStr += QObject::tr(", Impact");
		}
		if (commented == true)
		{
			typeStr += QObject::tr(", Commented");
		}
		l.push_back(typeStr);

		l.push_back(schemasList);

		return l;
	}

	bool SchemaSignalInfo::less(const SchemaSignalInfo& a, const SchemaSignalInfo& b)
	{
		if (a.input != b.input)
		{
			return a.input > b.input;
		}
		if (a.x != b.x)
		{
			return a.x < b.x;
		}
		if (a.y != b.y)
		{
			return a.y < b.y;
		}
		return a.signalId < b.signalId;
	}

	//
	// SchemaLoopbackInfo
	//
	SchemaLoopbackInfo::SchemaLoopbackInfo(const VFrame30::SchemaItemLoopback* loopbackItem, const QStringList& otherSchemasIds)
	{
		source = loopbackItem->isLoopbackSourceElement();
		commented = loopbackItem->commented();
		x = loopbackItem->left();
		y = loopbackItem->top();

		loopbackId = loopbackItem->loopbackId();

		if (otherSchemasIds.empty() == false)
		{
			schemasList = otherSchemasIds.join("<br>");
		}
		else
		{
			schemasList = QObject::tr("This Schema Only");
		}

		if (loopbackItem->textColor() != Qt::black)
		{
			color = loopbackItem->textColor().name();
		}
	}

	QStringList SchemaLoopbackInfo::toStringList() const
	{
		QStringList l;

		if (color.isEmpty() == false)
		{
			l.push_back(QObject::tr("<font color=\"%1\">%2</font>").arg(color).arg(loopbackId));
		}
		else
		{
			l.push_back(loopbackId);
		}

		QString typeStr = source ? QObject::tr("Source") : QObject::tr("Target");
		if (commented == true)
		{
			typeStr += QObject::tr(", Commented");
		}
		l.push_back(typeStr);

		l.push_back(schemasList);
		return l;
	}

	bool SchemaLoopbackInfo::less(const SchemaLoopbackInfo& a, const SchemaLoopbackInfo& b)
	{
		if (a.source != b.source)
		{
			return a.source > b.source;
		}
		if (a.x != b.x)
		{
			return a.x < b.x;
		}
		if (a.y != b.y)
		{
			return a.y < b.y;
		}
		return a.loopbackId < b.loopbackId;
	}

	//
	// SchemaConnectionInfo
	//
	SchemaConnectionInfo::SchemaConnectionInfo(const VFrame30::SchemaItemConnection* connectionItem, const QString& connectionId, const QStringList& otherSchemasIds)
	{
		transmitter = connectionItem->isTransmitterElement();
		commented = connectionItem->commented();
		x = connectionItem->left();
		y = connectionItem->top();

		this->connectionId = connectionId;

		if (otherSchemasIds.empty() == false)
		{
			schemasList = otherSchemasIds.join("<br>");
		}
		else
		{
			schemasList = QObject::tr("This Schema Only");
		}

		if (connectionItem->textColor() != Qt::black)
		{
			color = connectionItem->textColor().name();
		}
	}

	QStringList SchemaConnectionInfo::toStringList() const
	{
		QStringList l;

		if (color.isEmpty() == false)
		{
			l.push_back(QObject::tr("<font color=\"%1\">%2</font>").arg(color).arg(connectionId));
		}
		else
		{
			l.push_back(connectionId);
		}

		QString typeStr = transmitter ? QObject::tr("Transmitter") : QObject::tr("Receiver");
		if (commented == true)
		{
			typeStr += QObject::tr(", Commented");
		}
		l.push_back(typeStr);

		l.push_back(schemasList);
		return l;
	}

	bool SchemaConnectionInfo::less(const SchemaConnectionInfo& a, const SchemaConnectionInfo& b)
	{
		if (a.transmitter != b.transmitter)
		{
			return a.transmitter > b.transmitter;
		}
		if (a.x != b.x)
		{
			return a.x < b.x;
		}
		if (a.y != b.y)
		{
			return a.y < b.y;
		}
		return a.connectionId < b.connectionId;
	}

	//
	// SchemasReportGenerator
	//

	SchemasReportGenerator::SchemasReportGenerator(std::shared_ptr<ReportSchemaView> schemaView,
												   const AppSignalSet* signalSet,
												   const QString& serverIp,
												   int serverPort,
												   const QString& serverUserName,
												   const QString& serverPassword,
												   const QString& projectName,
												   const QString& userName,
												   const QString& userPassword,
												   std::vector<DbFileInfo> files,
												   const QString& filePath,
												   const SchemasReportOptions& options,
												   const std::vector<SchemaTypesParams>& schemaTypesParams) :
		m_schemaView(schemaView),
		m_printer(schemaView),
		m_diagStateProvider(),
		m_diagStateController(m_diagStateProvider, nullptr),
		m_appSignalProvider(signalSet),
		m_appSignalController(m_appSignalProvider, nullptr),
		m_inputFiles(files),
		m_filePath(filePath),
		m_serverIp(serverIp),
		m_serverPort(serverPort),
		m_serverUserName(serverUserName),
		m_serverPassword(serverPassword),
		m_projectName(projectName),
		m_userName(userName),
		m_userPassword(userPassword),
		m_contentsTextFont{"Arial", options.contentsTextFontSize(), QFont::Normal},
		m_contentsTableFont{"Arial", options.contentsTableFontSize(), QFont::Normal},
		m_textFont{"Arial", options.textFontSize(), QFont::Normal},
		m_tableFont{"Arial", options.tableFontSize(), QFont::Normal},
		m_marginFont{"Arial", 8, QFont::Normal},
		m_options(options),
		m_schemaTypesParams(schemaTypesParams)
	{
		return;
	}

	SchemasReportGenerator::~SchemasReportGenerator()
	{
		qDebug() << "SchemasReportWorker deleted";
	}

	std::vector<SchemaTypesParams> SchemasReportGenerator::defaultFileTypesParams(DbController* db)
	{
		std::vector<SchemaTypesParams> result;

		if (db == nullptr || db->isProjectOpened() == false)
		{
			Q_ASSERT(false);
			return result;
		}

		QStringList layoutNames{"Schemas Page Layout", "Text Page Layout"};

		result.push_back({db->systemFileId(DbDir::AppLogicDir),
						  QObject::tr("Application Logic"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
									  QPageLayout::Orientation::Landscape,
									  QMarginsF(30, 20, 15, 20),
									  QPageLayout::Unit::Millimeter),
						  layoutNames});

		result.push_back({db->systemFileId(DbDir::DiagSchemasDir),
						  QObject::tr("Diagnostics Schemas"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
									  QPageLayout::Orientation::Landscape,
									  QMarginsF(30, 20, 15, 20),
									  QPageLayout::Unit::Millimeter),
						  layoutNames});

		result.push_back({db->systemFileId(DbDir::MonitorSchemasDir),
						  QObject::tr("Monitor Schemas"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
									  QPageLayout::Orientation::Landscape,
									  QMarginsF(30, 20, 15, 20),
									  QPageLayout::Unit::Millimeter),
						  layoutNames});

		result.push_back({db->systemFileId(DbDir::TuningSchemasDir),
						  QObject::tr("Tuning Schemas"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
									  QPageLayout::Orientation::Landscape,
									  QMarginsF(30, 20, 15, 20),
									  QPageLayout::Unit::Millimeter),
						  layoutNames});


		result.push_back({db->systemFileId(DbDir::UfblDir),
						  QObject::tr("UFBL Descriptions"),
						  true,
						  QPageLayout(QPageSize(QPageSize::A3),
									  QPageLayout::Orientation::Landscape,
									  QMarginsF(30, 20, 15, 20),
									  QPageLayout::Unit::Millimeter),
						  layoutNames});


		result.push_back({-1,
						  QObject::tr("Single-File Report"),
						  false,
						  QPageLayout(QPageSize(QPageSize::A3),
									  QPageLayout::Orientation::Landscape,
									  QMarginsF(30, 20, 15, 20),
									  QPageLayout::Unit::Millimeter),
						  layoutNames});

		return result;
	}

	void SchemasReportGenerator::exportSchemasToMultiplePdf()
	{
		if (filePath().isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		VFrame30::SchemaDetailsSet detailsSet;
		std::vector<SchemaInfo> schemas;

		try
		{
			openProject();

			loadSchemas({}, m_inputFiles, schemas, detailsSet);

			sortSchemas(schemas);

			closeProject();
		}

		catch (QString errorMessage)
		{
			closeProject();

			emit finished(errorMessage);
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Rendering;
			m_statistics.m_schemaIndex = 0;
		}

		// Save schemas to PDF
		//
		for (const auto& schemaInfo : schemas)
		{
			if (m_stop == true)
			{
				break;
			}

			const auto& schemaId = schemaInfo.schema()->schemaId();

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_schemaIndex++;
				m_statistics.m_currentSchemaId = schemaId;
			}

			std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName,
																	  filePath() + QDir::separator() + schemaId + ".pdf");

			if (m_options.footers() == true)
			{
				report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});
				report->addMarginItem({tr("%PAGE%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});
				report->addMarginItem({"%TAG%", -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});
			}

			{
				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schemaId), {}, schemaInfo.schema(), {});
				auto pageLayout = getSchemaPageLayout(schemaInfo);
				auto schemaDrawingSection = report->addSection(ReportSection::create(schemaId, pageLayout));
				schemaDrawingSection->addSchema(reportSchema);
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_currentStatus = WorkerStatus::Printing;
			}

			// Print to file
			//
			bool ok = m_printer.print(*report, report->path(), m_stop);
			if (ok == false)
			{
				emit finished(tr("Error writing report to file %1!").arg(QDir::toNativeSeparators(report->path())));
				return;
			}
		}

		emit finished(QString());

		return;
	}

	void SchemasReportGenerator::exportSchemasToSinglePdf()
	{
		if (filePath().isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		VFrame30::SchemaDetailsSet detailsSet;
		std::vector<SchemaInfo> schemas;

		try
		{
			openProject();

			loadSchemas({}, m_inputFiles, schemas, detailsSet);

			sortSchemas(schemas);

			closeProject();
		}

		catch (QString errorMessage)
		{
			closeProject();

			emit finished(errorMessage);
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Rendering;
			m_statistics.m_schemaIndex = 0;
		}

		std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName, filePath());

		// Init margins
		//
		if (m_options.footers() == true)
		{
			report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});
			report->addMarginItem({tr("%PAGE%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});
			report->addMarginItem({"%TAG%", -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});
		}

		{
			for (const auto& schemaInfo : schemas)
			{
				if (m_stop == true)
				{
					break;
				}

				const auto& schemaId = schemaInfo.schema()->schemaId();

				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.m_schemaIndex++;
					m_statistics.m_currentSchemaId = schemaId;
				}

				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schemaId), {}, schemaInfo.schema(), {});
				auto pageLayout = getSchemaPageLayout(schemaInfo);
				auto schemaDrawingSection = report->addSection(ReportSection::create(schemaId, pageLayout));
				schemaDrawingSection->setTag(tr("%1 - %2").arg(schemaId).arg(schemaInfo.schema()->caption()));
				schemaDrawingSection->addSchema(reportSchema);
			}
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Printing;
		}

		// Print to file
		//
		bool ok = m_printer.print(*report, report->path(), m_stop);
		if (ok == false)
		{
			emit finished(tr("Error writing report to file %1!").arg(QDir::toNativeSeparators(report->path())));
			return;
		}

		emit finished(QString());

		return;
	}

	void SchemasReportGenerator::exportSchemasToAlbums()
	{

		const int schemaPageLayoutIndex = 0;
		const int textPageLayoutIndex = 1;

		try
		{
			openProject();

			// This data is for single-file report
			//
			std::vector<SchemaInfo> allSchemas;
			VFrame30::SchemaDetailsSet allDetailsSet;
			DbFileTree allFoldersTree;
			if (m_options.singleFile() == true)
			{
				bool ok = db()->getFileListTree(&allFoldersTree, db()->systemFileId(DbDir::SchemasDir), true /*removeDeleted*/, nullptr);
				if (ok == false)
				{
					throw(tr("DbController::getFileListTree failed on fileId = %1").arg(db()->systemFileId(DbDir::SchemasDir)));
				}
			}


			for (auto& stp : m_schemaTypesParams)
			{
				std::vector<DbFileInfo> schemasFiles;

				if (m_stop == true)
				{
					break;
				}

				if (stp.hasFileId() == false)
				{
					continue;
				}

				if (stp.selected() == false)
				{
					continue;
				}

				// Fill schemas files
				//
				DbFileTree fileTree;

				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.m_currentSchemaType = stp.caption();
				}

				bool ok = db()->getFileListTree(&fileTree, stp.fileId(), true /*removeDeleted*/, nullptr);
				if (ok == false)
				{
					throw(tr("DbController::getFileListTree failed on fileId = %1").arg(stp.fileId()));
				}

				const std::map<int, std::shared_ptr<DbFileInfo>>& files = fileTree.files();

				for (auto it = files.begin(); it != files.end(); it++)
				{
					const std::shared_ptr<DbFileInfo>& fi = it->second;

					// Filter files by extension
					//
					if (fi->fileName().endsWith("." + QString(Db::File::AlFileExtension)) == false &&
						fi->fileName().endsWith("." + QString(Db::File::UfbFileExtension)) == false &&
						fi->fileName().endsWith("." + QString(Db::File::MvsFileExtension)) == false &&
						fi->fileName().endsWith("." + QString(Db::File::TvsFileExtension)) == false &&
						fi->fileName().endsWith("." + QString(Db::File::DvsFileExtension)) == false)
					{
						continue;
					}

					// Filter files by schema tags
					//
					VFrame30::SchemaDetails details;
					ok = details.parseDetails(fi->details());
					if (ok == true)
					{
						bool schemaTagFound = false;
						for (const auto& [tag, tagEnabled] : m_options.schemaTags())
						{
							if (tagEnabled == true && details.schemaTags().contains(tag) == true)
							{
								schemaTagFound = true;
								break;
							}
						}
						if (schemaTagFound == false)
						{
							continue;
						}
					}

					// Add file to list
					//
					schemasFiles.push_back(*fi);
				}

				// Load and parse schemas
				//
				std::vector<SchemaInfo> schemas;

				VFrame30::SchemaDetailsSet detailsSet;

				loadSchemas(m_options.singleFile() ? allFoldersTree : fileTree, schemasFiles, schemas, detailsSet);

				if (schemas.empty() == true)
				{
					continue;
				}

				// Render schemas
				//
				if (m_stop == true)
				{
					break;
				}

				if (m_options.singleFile() == true)
				{
					// Single file mode - add all loaded data to global array
					//
					allSchemas.insert(allSchemas.end(), schemas.begin(), schemas.end());
					auto details = detailsSet.schemasDetails();
					for (const auto& d : details)
					{
						allDetailsSet.add(d);
					}
				}
				else
				{
					// Multiple files - render them now
					//
					sortSchemas(schemas);
					renderSchemasToAlbums(schemas, detailsSet, stp.caption(), 
						stp.pageLayoutWithMargins(schemaPageLayoutIndex), stp.pageLayoutWithMargins(textPageLayoutIndex));
				}
			}

			// Single file - render it now
			//
			if (m_options.singleFile() == true)
			{
				QPageLayout schemasLayout;
				QPageLayout textLayout;
				for (const auto& stp : m_schemaTypesParams)
				{
					if (stp.hasFileId() == false)
					{
						// This is global setting for single-file report
						//
						schemasLayout = stp.pageLayoutWithMargins(schemaPageLayoutIndex);
						textLayout = stp.pageLayoutWithMargins(textPageLayoutIndex);
						break;
					}
				}

				sortSchemas(allSchemas);
				renderSchemasToAlbums(allSchemas, allDetailsSet, tr("Schemas Album"), schemasLayout, textLayout);
			}

			closeProject();
		}

		catch (QString errorMessage)
		{
			closeProject();

			emit finished(errorMessage);

			return;
		}

		emit finished(QString());

		return;
	}

	void SchemasReportGenerator::stop()
	{
		m_stop = true;
	}

	void SchemasReportGenerator::progressRequested()
	{
		QString progressText;

		int progress = 0;
		int progressMin = 0;
		int progressMax = 0;

		getProgress(&progress, &progressMin, &progressMax, &progressText);

		emit progressChanged(progress, 0, progressMax, progressText);

		return;
	}

	void SchemasReportGenerator::getProgress(int* progress, int* progressMin, int* progressMax, QString* progressText)
	{
		if (progress == nullptr || progressMin == nullptr || progressMax == nullptr || progressText == nullptr)
		{
			Q_ASSERT(progress);
			Q_ASSERT(progressMin);
			Q_ASSERT(progressMax);
			Q_ASSERT(progressText);
			return;
		}

		SchemasReportGenerator::Statistics stats = statistics();

		*progressMin = 0;

		switch (stats.m_currentStatus)
		{
		case SchemasReportGenerator::WorkerStatus::Idle:
			{
				*progressText = tr("Idle");
			}
			break;
		case SchemasReportGenerator::WorkerStatus::Loading:
			{
				if (stats.m_currentSchemaType.isEmpty() == false)
				{
					*progressText = tr("Loading schema: %1/%2")
										.arg(stats.m_currentSchemaType)
										.arg(stats.m_currentSchemaId);
				}
				else
				{
					*progressText = tr("Loading schema: %1").arg(stats.m_currentSchemaId);
				}
				*progress = stats.m_schemaIndex;
				*progressMax = stats.m_schemasCount;
			}
			break;
		case SchemasReportGenerator::WorkerStatus::Parsing:
			{
				if (stats.m_currentSchemaType.isEmpty() == false)
				{
					*progressText = tr("Parsing schema: %1/%2")
										.arg(stats.m_currentSchemaType)
										.arg(stats.m_currentSchemaId);
				}
				else
				{
					*progressText = tr("Parsing schema: %1")
										.arg(stats.m_currentSchemaId);
				}
				*progress = stats.m_schemaIndex;
				*progressMax = stats.m_schemasCount;
			}
			break;
		case SchemasReportGenerator::WorkerStatus::Rendering:
			{
				if (stats.m_currentSchemaType.isEmpty() == false)
				{
					*progressText = tr("Rendering schema: %1/%2")
										.arg(stats.m_currentSchemaType)
										.arg(stats.m_currentSchemaId);
				}
				else
				{
					*progressText = tr("Rendering schema: %1").arg(stats.m_currentSchemaId);
				}
				*progress = stats.m_schemaIndex;
				*progressMax = stats.m_schemasCount;
			}
			break;
		case SchemasReportGenerator::WorkerStatus::Printing:
			{
				*progressText = tr("Printing to PDF Document...");

				const ReportPrinter::Statistics ps = m_printer.statistics();

				switch (ps.status)
				{
				case ReportPrinter::Statistics::Preview:
					*progressText = tr("Generating preview, section: %1/%2")
										.arg(ps.sectionIndex)
										.arg(ps.sectionCount);
					*progress = ps.sectionIndex;
					*progressMax = ps.sectionCount;
					break;
				case ReportPrinter::Statistics::Rendering:
					*progressText = tr("Rendering report, section: %1/%2")
										.arg(ps.sectionIndex)
										.arg(ps.sectionCount);
					*progress = ps.sectionIndex;
					*progressMax = ps.sectionCount;
					break;
				case ReportPrinter::Statistics::Printing:
					*progressText = tr("Printing report, page: %1/%2")
										.arg(ps.pageIndex)
										.arg(ps.pagesCount);
					*progress = ps.pageIndex;
					*progressMax = ps.pagesCount;
					break;
				}
			}
		}
	}

	QStringList SchemasReportGenerator::outputFilesList() const
	{
		QStringList result;
		for (const auto& it : m_outputData)
		{
			result.push_back(it.first);
		}
		return result;
	}

	const QByteArray& SchemasReportGenerator::outputData(const QString& fileName)
	{
		auto it = m_outputData.find(fileName);
		if (it == m_outputData.end())
		{
			Q_ASSERT(false);
			static QByteArray e;
			return e;
		}

		return it->second;
	}

	SchemasReportGenerator::Statistics SchemasReportGenerator::statistics() const
	{
		QMutexLocker l(&m_statisticsMutex);
		return m_statistics;
	}

	DbController* SchemasReportGenerator::db()
	{
		return &m_db;
	}

	const QString& SchemasReportGenerator::filePath() const
	{
		return m_filePath;
	}

	void SchemasReportGenerator::openProject()
	{
		if (db()->isProjectOpened() == true)
		{
			Q_ASSERT(false);
			throw(tr("Failed to open project - it is open!"));
		}

		db()->disableProgress();

		db()->setHost(m_serverIp);
		db()->setPort(m_serverPort);
		db()->setServerUsername(m_serverUserName);
		db()->setServerPassword(m_serverPassword);

		bool ok = db()->openProject(m_projectName, m_userName, m_userPassword, nullptr);
		if (ok == false)
		{
			throw(tr("Failed to open project!"));
		}

		return;
	}

	void SchemasReportGenerator::closeProject()
	{
		if (db()->isProjectOpened() == false)
		{
			return;
		}

		db()->closeProject(nullptr);

		return;
	}


	void SchemasReportGenerator::loadSchemas(const DbFileTree& foldersTree,
											 const std::vector<DbFileInfo>& files,
											 std::vector<SchemaInfo>& schemas,
											 VFrame30::SchemaDetailsSet& detailsSet)
	{
		schemas.clear();

		// Load schemas from files
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Loading;

			m_statistics.m_schemasCount = static_cast<int>(files.size());
			m_statistics.m_schemaIndex = 0;
		}

		// Get files from the database

		std::vector<std::shared_ptr<DbFile>> out;

		for (const DbFileInfo& fi : files)
		{
			if (m_stop == true)
			{
				break;
			}

			std::shared_ptr<DbFile> f;

			bool ok = db()->getLatestVersion(fi, &f, nullptr);
			if (ok == false)
			{
				throw(tr("Failed to load file %1").arg(fi.fileName()));
			}

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_schemaIndex++;
				m_statistics.m_currentSchemaId = f->fileName();
			}

			out.push_back(f);
		}

		// Parse schemas

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Parsing;
			m_statistics.m_schemaIndex = 0;
		}

		// Calculate if selected files have different parent
		//
		int firstParentId = -1;

		for (const std::shared_ptr<DbFile>& dbFile : out)
		{
			if (firstParentId == -1)
			{
				firstParentId = dbFile->parentId();
				continue;
			}

			if (firstParentId != dbFile->parentId())
			{
				break;
			}
		}

		auto context = VFrame30::Context::create(&m_diagStateController, &m_appSignalController, nullptr, nullptr, nullptr);

		// Load schemas from files
		//
		for (const std::shared_ptr<DbFile>& dbFile : out)
		{
			if (m_stop == true)
			{
				break;
			}

			std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(dbFile->data());
			if (schema == nullptr)
			{
				throw(tr("Failed to load schema from '%1'!").arg(dbFile->fileName()));
			}

			schema->setContext(context);

			{
				QMutexLocker l(&m_statisticsMutex);
				m_statistics.m_schemaIndex++;
				m_statistics.m_currentSchemaId = schema->schemaId();
			}

			QString schemaFilePath = foldersTree.filePath(dbFile->fileId()) + "/" + dbFile->fileName();
			schemas.push_back(SchemaInfo{schemaFilePath, schema});

			if (m_options.signalsDetails() == true)
			{
				detailsSet.add(schema->details("."));
			}
		}

		return;
	}

	void SchemasReportGenerator::sortSchemas(std::vector<SchemaInfo>& schemas)
	{
		// Sort schemas array
		//
		if (m_options.folders() == false)
		{
			std::sort(schemas.begin(), schemas.end(), [](const SchemaInfo& a, const SchemaInfo& b)
					  {
						  return a.schema()->schemaId() < b.schema()->schemaId();
					  });
		}
		else
		{
			std::sort(schemas.begin(), schemas.end(), [](const SchemaInfo& a, const SchemaInfo& b)
					  {
						  if (a.folder() == b.folder())
						  {
							  return a.schema()->schemaId() < b.schema()->schemaId();
						  }
						  else
						  {
							  return a.folder() < b.folder();
						  }
					  });
		}
	}

	bool SchemasReportGenerator::renderSchemasToAlbums(const std::vector<SchemaInfo>& schemas,
													   const VFrame30::SchemaDetailsSet& detailsSet,
													   const QString& groupName,
													   const QPageLayout& schemaPageLayout,
													   const QPageLayout& textPageLayout)
	{
		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Rendering;
			m_statistics.m_currentSchemaType = groupName;
			m_statistics.m_schemaIndex = 0;
			m_statistics.m_schemasCount = static_cast<int>(schemas.size());
		}

		std::shared_ptr<Report> report = std::make_shared<Report>(m_projectName,
																  tr("%1/%2_%3.pdf").arg(filePath()).arg(m_projectName).arg(groupName));

		// Set report options and variables
		//
		std::map<QString, QString> variables;
		variables.insert(m_options.projectVariables().begin(), m_options.projectVariables().end());
		variables.insert(m_options.userVariables().begin(), m_options.userVariables().end());
		report->reportVariables().setVariables(variables);

		report->setStartPage(m_options.startPageNumber());

		// Init margins
		//
		if (m_options.footers() == true)
		{
			report->addMarginItem({tr("Project: %1").arg(m_projectName), -1, -1, {m_marginFont, Qt::AlignLeft | Qt::AlignTop}});
			report->addMarginItem({tr("%PAGE%"), -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignBottom}});
			report->addMarginItem({"%TAG%", -1, -1, {m_marginFont, Qt::AlignRight | Qt::AlignTop}});
		}

		// Create table of contents
		//
		if (m_options.tableOfContents() == true)
		{
			createTableOfContents(report, textPageLayout, schemas, tr("Table of Contents"));
		}

		// Render schemas
		//
		{
			auto context = VFrame30::Context::create(&m_diagStateController, &m_appSignalController, nullptr, &report->reportVariables(), nullptr);

			for (const auto& schemaInfo : schemas)
			{
				if (m_stop == true)
				{
					break;
				}

				const auto& schemaId = schemaInfo.schema()->schemaId();

				{
					QMutexLocker l(&m_statisticsMutex);
					m_statistics.m_schemaIndex++;
					m_statistics.m_currentSchemaId = schemaId;
				}

				schemaInfo.schema()->setContext(context);

				auto reportSchema = ReportSchema::create(tr("Schema: %1").arg(schemaId), {}, schemaInfo.schema(), {});

				auto schemaDrawingSection = report->addSection(ReportSection::create(schemaId, schemaPageLayout));
				schemaDrawingSection->setTag(tr("%1 - %2").arg(schemaId).arg(schemaInfo.schema()->caption()));
				schemaDrawingSection->addSchema(reportSchema);

				if (m_options.signalsDetails() == true && schemaInfo.schema()->isLogicSchema() == true)
				{
					createLogicSchemaSignalsDetails(report, textPageLayout, schemaInfo, schemas, detailsSet);
				}
			}
		}

		{
			QMutexLocker l(&m_statisticsMutex);
			m_statistics.m_currentStatus = WorkerStatus::Printing;
		}

		// Preview the report to calculate page numbers for every section
		{
			std::vector<ReportLib::RenderedSection> renderedSections;
			if (m_printer.preview(*report, renderedSections, m_stop) == false)
			{
				return true;
			}


			int page = report->startPage();
			for (const ReportLib::RenderedSection& rs : renderedSections)
			{
				rs.section()->setStartPage(page);
				page += rs.pagesCount();

				// Create variable with section name and its start page
				//
				report->reportVariables().setVariable("REPORT_PAGE_" + rs.section()->caption(), rs.section()->startPage());
			}

			// Create variable with total pages count
			//
			report->reportVariables().setVariable("REPORT_PAGE_COUNT", page - 1);
		}

		// Print report
		//
		if (report->path().isEmpty() == false)
		{
			// Print to file PDF
			//
			bool ok = m_printer.print(*report, report->path(), m_stop);
			if (ok == false)
			{
				throw(tr("Error writing report to file %1!").arg(QDir::toNativeSeparators(report->path())));
				return false;
			}
		}
		else
		{
			// Print to buffer
			//
			QBuffer buffer(&m_outputData[groupName + ".pdf"]);
			bool ok = m_printer.print(*report, buffer, m_stop);
			if (ok == false)
			{
				throw(tr("Error writing report to memory!"));
				return false;
			}
		}

		return true;
	}

	void SchemasReportGenerator::createTableOfContents(const std::shared_ptr<ReportLib::Report> report,
													   const QPageLayout& pageLayout,
													   const std::vector<SchemaInfo>& schemas,
													   const QString& caption)
	{
		auto contentsSection = report->addSection(ReportSection::create(caption, pageLayout));
		contentsSection->setTag(caption);

		contentsSection->addText(caption, {m_contentsTextFont, Qt::AlignHCenter});

		auto contentsTable = ReportTable::create({m_contentsTableFont,
												  {tr("Schema ID"), tr("Caption"), tr("Page")},
												  {30, 50, 20},
												  Qt::AlignLeft});

		contentsSection->addTable(contentsTable);

		QString currentFolder;

		for (const auto& schemaInfo : schemas)
		{
			if (m_stop == true)
			{
				break;
			}

			// If new folder is processed
			//
			if (m_options.folders() == true)
			{
				QString folder = schemaInfo.folder();

				if (currentFolder != folder)
				{
					currentFolder = folder;

					// Add folder record
					//
					QStringList l;
					l.push_back(folder);
					l.push_back(QString());
					l.push_back(QString());
					contentsTable->insertRow(l);
				}
			}

			// Add file record
			//
			const auto& schemaId = schemaInfo.schema()->schemaId();

			QStringList l;
			if (m_options.folders() == true)
			{
				l.push_back("\u2800" + schemaId);
			}
			else
			{
				l.push_back(schemaId);
			}
			l.push_back(schemaInfo.schema()->caption());
			l.push_back(tr("%1(%2)")
							.arg(ReportTagStorage::tagSectionStartPage)
							.arg(schemaId));
			contentsTable->insertRow(l);
		}
	}

	QPageLayout SchemasReportGenerator::getSchemaPageLayout(const SchemaInfo& schemaInfo) const
	{
		qreal marginSizeMM = m_options.footers() ? 15 : 0;

		const auto& schema = schemaInfo.schema();

		// Initialize PDF page size
		//
		QPageLayout::Orientation orientation = (schema->docWidth() < schema->docHeight()) ?
												   QPageLayout::Portrait :
												   QPageLayout::Landscape;

		switch (schema->unit())
		{
		case SchemaUnit::Inch:
			return QPageLayout(QPageSize(QSizeF(schema->docWidth(), schema->docHeight()), QPageSize::Inch),
							   QPageLayout::Portrait,
							   QMarginsF(marginSizeMM / 25.4, marginSizeMM / 25.4, marginSizeMM / 25.4, marginSizeMM / 25.4),
							   QPageLayout::Inch);

		case SchemaUnit::Millimeter:
			return QPageLayout(QPageSize(QSizeF(schema->docWidth(), schema->docHeight()), QPageSize::Millimeter),
							   QPageLayout::Portrait,
							   QMarginsF(marginSizeMM, marginSizeMM, marginSizeMM, marginSizeMM),
							   QPageLayout::Millimeter);

		default:
			// If schema size specified in pixels, use A3 format
			//
			Q_ASSERT(schema->unit() == SchemaUnit::Display);
			return QPageLayout(QPageSize(QPageSize::A3), orientation, QMarginsF(marginSizeMM, marginSizeMM, marginSizeMM, marginSizeMM), QPageLayout::Millimeter);
		}
	}

	void SchemasReportGenerator::createLogicSchemaSignalsDetails(const std::shared_ptr<Report> report,
																 const QPageLayout& pageLayout,
																 const SchemaInfo& schemaInfo,
																 const std::vector<SchemaInfo>& allSchemas,
																 const VFrame30::SchemaDetailsSet& detailsSet)
	{
		if (schemaInfo.schema()->isLogicSchema() == false)
		{
			Q_ASSERT(false);
			return;
		}
		VFrame30::LogicSchema* logicSchema = schemaInfo.schema()->toLogicSchema();
		if (logicSchema == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		auto schemaDetailsSection = ReportSection::create(tr("Schema Details: %1").arg(logicSchema->schemaId()), pageLayout);
		schemaDetailsSection->setTag(tr("%1 - %2 [Details]").arg(logicSchema->schemaId()).arg(logicSchema->caption()));

		createLogicSchemaIOSignalsDetails(schemaDetailsSection, logicSchema, allSchemas, detailsSet);

		createLogicSchemaLoopbacksDetails(schemaDetailsSection, logicSchema, allSchemas, detailsSet);

		createLogicSchemaConnectionsDetails(schemaDetailsSection, logicSchema, allSchemas, detailsSet);

		if (schemaDetailsSection->objectCount() > 0)
		{
			report->addSection(schemaDetailsSection);
		}

		return;
	}

	void SchemasReportGenerator::createLogicSchemaIOSignalsDetails(const std::shared_ptr<ReportLib::ReportSection> section,
																   const VFrame30::LogicSchema* logicSchema,
																   const std::vector<SchemaInfo>& allSchemas,
																   const VFrame30::SchemaDetailsSet& detailsSet)
	{
		auto table = ReportTable::create({m_tableFont,
										  {tr("Signal ID"), tr("Caption"), tr("Type"), tr("Schemas")},
										  {20, 30, 20, 30},
										  Qt::AlignLeft});
		table->setHtmlEscaped(false);

		// Get list of signals for current schema
		//

		std::vector<SchemaSignalInfo> tableContents;

		auto f = [this, &tableContents, &detailsSet, &logicSchema, &allSchemas](const QString& signalId, const VFrame30::FblItemRect* item)
		{
			// Get list of schemas which contain this signal (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByAppSignalId(signalId);
			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue; // Skip current schema
				}

				// Get other schema
				//
				auto otherSchemaIt = std::find_if(allSchemas.begin(), allSchemas.end(), [otherSchemaId](const SchemaInfo& si)
												  {
													  return si.schema()->schemaId() == otherSchemaId;
												  });
				if (otherSchemaIt == allSchemas.end())
				{
					// No such schema in current album
					continue;
				}
				const SchemaInfo& otherSchemaInfo = *otherSchemaIt;
				if (otherSchemaInfo.schema() == nullptr)
				{
					Q_ASSERT(otherSchemaInfo.schema());
					continue;
				}
				if (otherSchemaInfo.schema()->isLogicSchema() == false)
				{
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchemaInfo.schema()->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					continue;
				}

				// Item is SchemaItemSignal* element
				//
				if (const VFrame30::SchemaItemSignal* signalElement = item->toSignalElement();
					signalElement != nullptr)
				{
					auto otherItemSignalsMap = otherLogicSchema->getSignalItemsMap();
					auto r = std::find_if(otherItemSignalsMap.begin(), otherItemSignalsMap.end(), [signalElement, signalId](const auto& it)
										  {
											  // Find a signal item on other schema that has opposite type (input vs output)
											  //
											  const QString& otherSignalId = it.first;
											  const VFrame30::SchemaItemSignal* otherItem = it.second;
											  return otherItem->isInputSignalElement() != signalElement->isInputSignalElement() &&
													 otherSignalId == signalId;
										  });
					if (r != otherItemSignalsMap.end())
					{
						otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
													  .arg(otherSchemaId)
													  .arg(ReportTagStorage::tagSectionStartPage)
													  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
					}
				}

				// Item is SchemaItemReceiver* element
				//
				if (const VFrame30::SchemaItemReceiver* receiverElement = item->toReceiverElement();
					receiverElement != nullptr)
				{
					const auto otherItemSignalSet = otherLogicSchema->getSignalList();
					auto r = std::find_if(otherItemSignalSet.begin(), otherItemSignalSet.end(), [signalId](const auto& it)
										  {
											  // Find an item on other schema with this signal
											  //
											  const QString& otherSignalId = it;
											  return otherSignalId == signalId;
										  });
					if (r != otherItemSignalSet.end())
					{
						otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
													  .arg(otherSchemaId)
													  .arg(ReportTagStorage::tagSectionStartPage)
													  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
					}
				}
			}

			// Fill signal info
			//

			SchemaSignalInfo ssi(item, signalId, otherSchemasIds, m_appSignalProvider);
			tableContents.push_back(ssi);
		};

		// Get SchemaItemSignal* objects
		//
		std::map<QString, VFrame30::SchemaItemSignal*> itemSignalsMap = logicSchema->getSignalItemsMap();

		for (const auto& [signalId, item] : itemSignalsMap)
		{
			f(signalId, item);
		}

		// Get SchemaItemReceiver objects
		//
		std::map<QString, VFrame30::SchemaItemReceiver*> itemReceiversMap = logicSchema->getSignalReceiversMap();

		for (const auto& [signalId, item] : itemReceiversMap)
		{
			f(signalId, item);
		}

		// Sort signals: input first, then sort by x coordinate, then sort by y coordinate
		//
		std::sort(tableContents.begin(), tableContents.end(), SchemaSignalInfo::less);

		// Output data to a table
		//
		for (const SchemaSignalInfo& ssi : tableContents)
		{
			table->insertRow(ssi.toStringList());
		}

		if (table->rowCount() > 0)
		{
			section->addText(tr("Schema '%1 - %2' Signals").arg(logicSchema->schemaId()).arg(logicSchema->caption()),
							 {m_textFont, Qt::AlignHCenter});
			section->addTable(table);
		}
	}

	void SchemasReportGenerator::createLogicSchemaLoopbacksDetails(const std::shared_ptr<ReportLib::ReportSection> section,
																   const VFrame30::LogicSchema* logicSchema,
																   const std::vector<SchemaInfo>& allSchemas,
																   const VFrame30::SchemaDetailsSet& detailsSet)
	{

		auto table = ReportTable::create({m_tableFont,
										  {tr("Loopback ID"), tr("Type"), tr("Schemas")},
										  {30, 20, 50},
										  Qt::AlignLeft});
		table->setHtmlEscaped(false);

		// Get list of loopbacks for current schema
		//

		std::map<QString, VFrame30::SchemaItemLoopback*> loopbacksMap = logicSchema->getLoopbacksMap();

		std::vector<SchemaLoopbackInfo> tableContents;
		tableContents.reserve(loopbacksMap.size());

		for (const auto& [loopbackId, loopbackItem] : loopbacksMap)
		{
			// Get list of schemas which contain this loopbackId (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByLoopbackId(loopbackItem->loopbackId());

			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue; // Skip current schema
				}

				// Get other schema
				//
				auto otherSchemaIt = std::find_if(allSchemas.begin(), allSchemas.end(), [otherSchemaId](const SchemaInfo& si)
												  {
													  return si.schema()->schemaId() == otherSchemaId;
												  });
				if (otherSchemaIt == allSchemas.end())
				{
					// No such schema in current album
					continue;
				}
				const SchemaInfo& otherSchemaInfo = *otherSchemaIt;
				if (otherSchemaInfo.schema() == nullptr)
				{
					Q_ASSERT(otherSchemaInfo.schema());
					continue;
				}
				if (otherSchemaInfo.schema()->isLogicSchema() == false)
				{
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchemaInfo.schema()->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					continue;
				}

				auto otherLoopbackSet = otherLogicSchema->getLoopbacksMap();

				auto r = std::find_if(otherLoopbackSet.begin(), otherLoopbackSet.end(), [loopbackItem](const auto& it)
									  {
										  // Find a loopback on other schema that has opposite type
										  //
										  const VFrame30::SchemaItemLoopback* item = it.second;
										  return loopbackItem->isLoopbackSourceElement() != item->isLoopbackSourceElement() &&
												 item->loopbackId() == loopbackItem->loopbackId();
									  });
				if (r != otherLoopbackSet.end())
				{
					otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
												  .arg(otherSchemaId)
												  .arg(ReportTagStorage::tagSectionStartPage)
												  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
				}
			}

			// Fill loopback info
			//

			SchemaLoopbackInfo ssi(loopbackItem, otherSchemasIds);
			tableContents.push_back(ssi);
		}

		// Sort loopbacks:  by x coordinate, then sort by y coordinate
		//
		std::sort(tableContents.begin(), tableContents.end(), SchemaLoopbackInfo::less);

		// Output data to a table
		//
		for (const SchemaLoopbackInfo& ssi : tableContents)
		{
			table->insertRow(ssi.toStringList());
		}

		if (table->rowCount() > 0)
		{
			section->addText(tr("\n\nSchema '%1 - %2' Loopbacks").arg(logicSchema->schemaId()).arg(logicSchema->caption()),
							 {m_textFont, Qt::AlignHCenter});
			section->addTable(table);
		}
	}

	void SchemasReportGenerator::createLogicSchemaConnectionsDetails(const std::shared_ptr<ReportLib::ReportSection> section,
																	 const VFrame30::LogicSchema* logicSchema,
																	 const std::vector<SchemaInfo>& allSchemas,
																	 const VFrame30::SchemaDetailsSet& detailsSet)
	{
		auto table = ReportTable::create({m_tableFont,
										  {tr("Connection ID"), tr("Type"), tr("Schemas")},
										  {30, 20, 50},
										  Qt::AlignLeft});
		table->setHtmlEscaped(false);

		// Get list of signals for current schema
		//

		std::map<QString, VFrame30::SchemaItemTransmitter*> transmitersMap = logicSchema->getTransmittersMap();
		std::map<QString, VFrame30::SchemaItemReceiver*> receiversMap = logicSchema->getReceiversMap();

		std::vector<SchemaConnectionInfo> tableContents;
		tableContents.reserve(transmitersMap.size() + receiversMap.size());

		for (const auto& [connectionId, transmitterItem] : transmitersMap)
		{
			// Get list of connections which contain this connectionId (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByConnectionId(connectionId);

			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue; // Skip current schema
				}

				// Get other schema
				//
				auto otherSchemaIt = std::find_if(allSchemas.begin(), allSchemas.end(), [otherSchemaId](const SchemaInfo& si)
												  {
													  return si.schema()->schemaId() == otherSchemaId;
												  });
				if (otherSchemaIt == allSchemas.end())
				{
					// No such schema in current album
					continue;
				}
				const SchemaInfo& otherSchemaInfo = *otherSchemaIt;
				if (otherSchemaInfo.schema() == nullptr)
				{
					Q_ASSERT(otherSchemaInfo.schema());
					continue;
				}
				if (otherSchemaInfo.schema()->isLogicSchema() == false)
				{
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchemaInfo.schema()->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					continue;
				}

				auto otherReceiversMap = otherLogicSchema->getReceiversMap();

				auto r = std::find_if(otherReceiversMap.begin(), otherReceiversMap.end(), [connectionId](const auto& it)
									  {
										  // Find a loopback on other schema that has opposite type
										  //
										  const VFrame30::SchemaItemReceiver* item = it.second;
										  return item->connectionIdsAsList().contains(connectionId);
									  });
				if (r != otherReceiversMap.end())
				{
					otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
												  .arg(otherSchemaId)
												  .arg(ReportTagStorage::tagSectionStartPage)
												  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
				}
			}

			// Fill transmitter info
			//

			SchemaConnectionInfo ssi(transmitterItem, connectionId, otherSchemasIds);
			tableContents.push_back(ssi);
		}

		for (const auto& [connectionId, receiverItem] : receiversMap)
		{
			// Get list of connections which contain this connectionId (other schemas)
			//
			QStringList otherSchemasIds;
			QStringList otherSchemasList = detailsSet.schemasByConnectionId(connectionId);

			for (const QString& otherSchemaId : otherSchemasList)
			{
				if (otherSchemaId == logicSchema->schemaId())
				{
					continue; // Skip current schema
				}

				// Get other schema
				//
				auto otherSchemaIt = std::find_if(allSchemas.begin(), allSchemas.end(), [otherSchemaId](const SchemaInfo& si)
												  {
													  return si.schema()->schemaId() == otherSchemaId;
												  });
				if (otherSchemaIt == allSchemas.end())
				{
					// No such schema in current album
					continue;
				}

				const SchemaInfo& otherSchemaInfo = *otherSchemaIt;
				if (otherSchemaInfo.schema() == nullptr)
				{
					Q_ASSERT(otherSchemaInfo.schema());
					continue;
				}
				if (otherSchemaInfo.schema()->isLogicSchema() == false)
				{
					continue;
				}
				VFrame30::LogicSchema* otherLogicSchema = otherSchemaInfo.schema()->toLogicSchema();
				if (otherLogicSchema == nullptr)
				{
					Q_ASSERT(false);
					continue;
				}

				auto otherTransmittersMap = otherLogicSchema->getTransmittersMap();

				auto r = std::find_if(otherTransmittersMap.begin(), otherTransmittersMap.end(), [connectionId](const auto& it)
									  {
										  // Find a loopback on other schema that has opposite type
										  //
										  const VFrame30::SchemaItemTransmitter* item = it.second;
										  return item->connectionIdsAsList().contains(connectionId);
									  });
				if (r != otherTransmittersMap.end())
				{
					otherSchemasIds.push_back(tr("%1 [p.%2(%1)]%3")
												  .arg(otherSchemaId)
												  .arg(ReportTagStorage::tagSectionStartPage)
												  .arg(otherLogicSchema->excludeFromBuild() ? " (excluded)" : ""));
				}
			}

			// Fill receiver info
			//

			SchemaConnectionInfo ssi(receiverItem, connectionId, otherSchemasIds);
			tableContents.push_back(ssi);
		}

		// Sort connections: transmitters first, then sort by x coordinate, then sort by y coordinate
		//
		std::sort(tableContents.begin(), tableContents.end(), SchemaConnectionInfo::less);

		// Output data to a table
		//
		for (const SchemaConnectionInfo& ssi : tableContents)
		{
			table->insertRow(ssi.toStringList());
		}

		if (table->rowCount() > 0)
		{
			section->addText(tr("\n\nSchema '%1 - %2' Connections").arg(logicSchema->schemaId()).arg(logicSchema->caption()),
							 {m_textFont, Qt::AlignHCenter});
			section->addTable(table);
		}
	}
} // namespace Builder
