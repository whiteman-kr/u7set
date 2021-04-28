#include "../Builder/ModuleFirmwareWriter.h"
#include "../UtilsLib/CsvFile.h"
#include "../UtilsLib/Crc.h"
#include "../UtilsLib/WUtils.h"

#include <qqml.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QJsonDocument>
#include <QtEndian>
#include <QQmlEngine>

#if __has_include("../gitlabci_version.h")
#	include "../gitlabci_version.h"
#endif

//
// JsVariantList
//

JsVariantList::JsVariantList(QObject* parent):
	QObject(parent)
{

}

void JsVariantList::append(QVariant v)
{
	m_list.append(v);
}

int JsVariantList::jsSize()
{
	return m_list.size();
}

QVariant JsVariantList::jsAt(int i)
{
	return m_list.at(i);
}


namespace Hardware
{

	//
	// ModuleFirmwareWriter
	//

	ModuleFirmwareWriter::ModuleFirmwareWriter()
	{
	}


	void ModuleFirmwareWriter::setDescriptionFields(const QString& subsysId, int uartId, int descriptionVersion, const QStringList& fields)
	{
		UartChannelData& channelData = uartChannelData(subsysId, uartId);

		channelData.descriptionFieldsVersion = descriptionVersion;
		channelData.descriptionFields = fields;
	}

	bool ModuleFirmwareWriter::setChannelData(const QString& subsysId, int uartId, QString equipmentID, int lmNumber, int frameSize, int frameCount, quint64 uniqueID, const QByteArray& binaryData, const std::vector<QVariantList>& descriptionData, Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		// Check for correct firmware parameters

		bool ok = false;

		const ModuleFirmware& sf = firmware(subsysId, &ok);

		if (ok == false)
		{
			log->errINT1000(QString("FirmwareWriter::setChannelData error, subsystem %1 does not exist.").arg(subsysId));
			return false;
		}

		if (sf.eepromFramePayloadSize(uartId) != frameSize)
		{
			log->errINT1000(QString("FirmwareWriter::setChannelData error, LM number %1: wrong frameSize (%2), expected %3.").arg(lmNumber).arg(frameSize).arg(sf.eepromFramePayloadSize(uartId)));
			return false;
		}

		if (sf.eepromFrameCount(uartId) != frameCount)
		{
			log->errINT1000(QString("FirmwareWriter::setChannelData error, LM number %1: wrong frameCount (%2), expected %3.").arg(lmNumber).arg(frameSize).arg(sf.eepromFramePayloadSize(uartId)));
			return false;
		}

		// Set the data for channel

		UartChannelData& channelData = uartChannelData(subsysId, uartId);

		// Check if data for this channel already exists

		if (channelData.binaryDataMap.find(lmNumber) != channelData.binaryDataMap.end())
		{
			log->errCFG3003(lmNumber, equipmentID);
			return false;
		}

		channelData.uniqueIdMap[lmNumber] = uniqueID;

		channelData.descriptonDataMap[lmNumber] = descriptionData;

		channelData.binaryDataMap[lmNumber] = binaryData;

		return true;
	}

	UartChannelData& ModuleFirmwareWriter::uartChannelData(const QString& subsysId, int uartId)
	{
		ModuleChannelData& subsystemChannelData = m_moduleChannelData[subsysId];

		UartChannelData& moduleChannelData = subsystemChannelData[uartId];

		return moduleChannelData;
	}

	bool ModuleFirmwareWriter::writeFirmwareStatistics(QStringList &dest, Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}


		auto reportWriter = [&dest](const QStringList& header, const std::vector<QStringList>& strings)
			{

			std::vector<int> columnWidth;

			// Count column width

			for (const QString& s : header)
			{
				columnWidth.push_back(s.length());
			}

			for (const QStringList& reportText : strings)
			{

				for (int i = 0; i < static_cast<int>(reportText.size()); i++)
				{
					const QString& s = reportText[i];
					columnWidth[i] = std::max(columnWidth[i], s.length());
				}

			}

			// Write strings

			QString text = "   ";

			for (int i = 0; i < static_cast<int>(header.size()); i++)
			{
				const QString& s = header[i];
				text += tr("| %1 ").arg(s.rightJustified(columnWidth[i], ' '));
			}

			dest << text;

			text = "---";

			for (int i = 0; i < static_cast<int>(header.size()); i++)
			{
				QString d(columnWidth[i], '-');

				text += tr("+-%1-").arg(d.rightJustified(columnWidth[i], ' '));
			}

			dest << text;

			for (const QStringList& reportText : strings)
			{
				text = "   ";

				for (int i = 0; i < static_cast<int>(reportText.size()); i++)
				{
					const QString& s = reportText[i];
					text += tr("| %1 ").arg(s.rightJustified(columnWidth[i], ' '));
				}

				dest << text;
			}
		};

		// Subsystems logic modules report

		dest << "Subsystems Logic Modules";
		dest << "";

		{
			QStringList reportHeader;
			reportHeader << "Subsystem ID";
			reportHeader << "LM Equipment ID";
			reportHeader << "LM Number";

			std::vector<QStringList> reportStrings;

			for (const auto& moduleChannelDataIt : m_moduleChannelData)
			{
				QStringList reportText;

				const QString subsystemId = moduleChannelDataIt.first;
				reportText << subsystemId;

				bool ok = false;
				const ModuleFirmware& sf = firmware(subsystemId, &ok);
				if (ok == false)
				{
					Q_ASSERT(ok);
					return false;
				}

				const std::vector<LogicModuleInfo>& logicModulesInfo = sf.logicModulesInfo();

				for (const LogicModuleInfo& lmInfo : logicModulesInfo)
				{
					reportText << lmInfo.equipmentId;
					reportText << QString::number(lmInfo.lmNumber);

					reportStrings.push_back(reportText);

					reportText.clear();
					reportText << "";
				}
			}

			reportWriter(reportHeader, reportStrings);
		}

		dest << "";

		// UART usage report

		dest << "UART Usage Statistics";
		dest << "";

		{

			QStringList reportHeader;
			reportHeader << "Subsystem ID";
			reportHeader << "Uart ID";
			reportHeader << "Uart Type";

			reportHeader << "Frames Used";
			reportHeader << "Frames Used, %";

			std::vector<QStringList> reportStrings;

			for (const auto& moduleChannelDataIt : m_moduleChannelData)
			{
				QStringList reportText;

				const QString subsystemId = moduleChannelDataIt.first;

				reportText << subsystemId;

				bool ok = false;
				const ModuleFirmware& sf = firmware(subsystemId, &ok);
				if (ok == false)
				{
					Q_ASSERT(ok);
					return false;
				}

				std::vector<UartPair> uartList = sf.uartList();

				for (const auto& uart : uartList)
				{
					int uartId = uart.first;
					const QString uartType = uart.second;

					const ModuleFirmwareData& moduleFirmwareData = sf.firmwareData(uartId, &ok);
					if (ok == false)
					{
						Q_ASSERT(ok);
						return false;
					}

					reportText << tr("%1h").arg(QString::number(uartId, 16));
					reportText << uartType;

					int framesCount = static_cast<int>(moduleFirmwareData.frames.size());
					int framesUsed = moduleFirmwareData.maxFrameIndex + 1;

					reportText << tr("%1 / %2").arg(framesUsed).arg(framesCount);

					double framesUsedPercent = (double)framesUsed / framesCount * 100;

					QString framesUsedPercentString = QString::number(framesUsedPercent, 'f', 1);

					if (framesUsedPercent > 95)
					{
						framesUsedPercentString = "!" + framesUsedPercentString;
					}

					reportText << framesUsedPercentString;

					reportStrings.push_back(reportText);

					reportText.clear();
					reportText << "";
				}
			}

			reportWriter(reportHeader, reportStrings);
		}

		return true;
	}

	bool ModuleFirmwareWriter::save(QByteArray& dest, Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		if (storeChannelData(log) == false)
		{
			return false;
		}

		QJsonObject jObject;

		// Store Subsystems Info

		QJsonArray jSubsystemInfoArray;

		for (auto fwi : m_firmwares)
		{
			ModuleFirmware& fw = fwi.second;

			QJsonObject jSubsystemInfo;

			jSubsystemInfo.insert(QLatin1String("lmDescriptionFile"), fw.lmDescriptionFile());
			jSubsystemInfo.insert(QLatin1String("lmDescriptionNumber"), fw.lmDescriptionNumber());
			jSubsystemInfo.insert(QLatin1String("subsystemId"), fw.subsysId());
			jSubsystemInfo.insert(QLatin1String("subsystemKey"), fw.ssKey());

			const std::vector<LogicModuleInfo>& lmInfo = fw.logicModulesInfo();

			QJsonArray jModuleInfoArray;

			for (auto info : lmInfo)
			{
				QJsonObject jModuleInfo;

				jModuleInfo.insert(QLatin1String("equipmentId"), info.equipmentId);
				jModuleInfo.insert(QLatin1String("subsystemId"), info.subsystemId);
				jModuleInfo.insert(QLatin1String("lmNumber"), info.lmNumber);
				jModuleInfo.insert(QLatin1String("channel"), E::valueToString<E::Channel>(info.channel));
				jModuleInfo.insert(QLatin1String("moduleFamily"), info.moduleFamily);
				jModuleInfo.insert(QLatin1String("customModuleFamily"), info.customModuleFamily);
				jModuleInfo.insert(QLatin1String("moduleVersion"), info.moduleVersion);
				jModuleInfo.insert(QLatin1String("moduleType"), info.moduleType);

				jModuleInfoArray.push_back(jModuleInfo);
			}

			jSubsystemInfo.insert(QLatin1String("z_modules"), jModuleInfoArray);

			jSubsystemInfoArray.push_back(jSubsystemInfo);
		}

		jObject.insert(QLatin1String("z_i_subsystemsInfo"), jSubsystemInfoArray);

		// Store Subsystems Data

		QJsonArray jSubsystemDataArray;

		for (auto fwi : m_firmwares)
		{
			const QString& subsystemId = fwi.first;

			ModuleFirmware& fw = fwi.second;

			QJsonObject jSubsystemData;

			std::vector<UartPair> uarts = fw.uartList();

			QJsonArray jFirmwaresDataArray;

			for (auto it : uarts)
			{
				int	uartId = it.first;

				bool ok = false;
				ModuleFirmwareData& firmwareData = fw.firmwareData(uartId, &ok);
				if (ok == false)
				{
					assert(false);
					continue;
				}

				UartChannelData& channelData = uartChannelData(subsystemId, uartId);

				// Count CRC64 for all frames

				int framesCount = static_cast<int>(firmwareData.frames.size());

				for (int i = 0; i < framesCount; i++)
				{
					std::vector<quint8>& frame = firmwareData.frames[i];

					Crc::setDataBlockCrc(static_cast<quint16>(i), frame.data(), (int)frame.size());
				}

				// Save all frames to file

				QJsonObject jFirmwareData;

				const int frameStringWidth = 16;

				for (int i = 0; i < framesCount; i++)
				{
					const int numCharsCount = 4;					// number of symbols in number "0000" (4)
					const int recCharsCount = numCharsCount + 1;	// number of symbols in number "0000 " (with space)
					const int maxStringLength = recCharsCount * frameStringWidth;

					const std::vector<quint8>& frame = firmwareData.frames[i];

					int frameSize = (int)frame.size();

					int linesCount = static_cast<int>(ceil((float)frameSize / 2.0 / frameStringWidth));

					QJsonObject jFrame;

					// Frame data

					int dataBytePos = 0;

					QByteArray str;
					str.resize(maxStringLength);

					char buf[10];

					for (int l = 0; l < linesCount; l++)
					{
						// Check if line is full of zeroes
						//
						bool lineOfZeroes = true;

						for (int ip = 0; ip < frameStringWidth * sizeof(quint16); ip++)
						{
							if (dataBytePos + ip >= frameSize)
							{
								break;
							}

							if (frame[dataBytePos + ip] != 0)
							{
								lineOfZeroes = false;
								break;
							}
						}

						if (lineOfZeroes == true)
						{
							dataBytePos += frameStringWidth * sizeof(quint16);
							continue;
						}

						// Write line

						str.fill(' ');

						for (int ip = 0; ip < frameStringWidth; ip++)
						{
							quint16 value = ((quint16)frame[dataBytePos++] << 8);
							if (dataBytePos >= frameSize)
							{
								assert(false);
								break;
							}

							value |= frame[dataBytePos++];

							snprintf(buf, sizeof(buf), "%hx", value);

							int len = static_cast<int>(strlen(buf));

							memset(str.data() + ip * recCharsCount, '0', numCharsCount);
							memcpy(str.data() + ip * recCharsCount + (numCharsCount - len), buf, len);

							if (dataBytePos >= frameSize)
							{
								int pos = ip * recCharsCount + numCharsCount;
								if (pos >= maxStringLength)
								{
									Q_ASSERT(false);
									break;
								}

								str[pos] = 0;
								break;
							}
						}

						jFrame.insert("data" + QString().number(l * frameStringWidth, 16).rightJustified(4, '0'), QJsonValue(str.trimmed().data()));
					}

					jFrame.insert(QLatin1String("frameIndex"), i);

					jFirmwareData.insert("z_frame_" + QString().number(i).rightJustified(4, '0'), jFrame);
				}

				//description
				//
				if (channelData.descriptionFields.empty() == false)
				{
					for (auto channelDescription : channelData.descriptonDataMap)
					{
						QJsonObject jDescription;

						int channel = channelDescription.first;
						const std::vector<QVariantList>& descriptionItems = channelDescription.second;

						// Description header string

						QString descriptionHeaderString = "Version;" + CsvFile::getCsvString(channelData.descriptionFields, true);

						jDescription.insert("desc fields", descriptionHeaderString);

						// Description strings

						int diIndex = 0;
						for (auto di : descriptionItems)
						{
							if (di.size() != channelData.descriptionFields.size())
							{
								qDebug() << "number of data items (" << di.size() << ") must be equal to number of header items (" << channelData.descriptionFields.size() << ")";
								assert(false);
								return false;
							}

							QString descriptionString = QString("%1;%2").arg(channelData.descriptionFieldsVersion).arg(CsvFile::getCsvString(di, true));

							jDescription.insert("desc" + QString::number(diIndex++).rightJustified(8, '0'), descriptionString);
						}

						if (descriptionItems.empty() == false)
						{
							jFirmwareData.insert("z_description_channel_" + QString::number(channel).rightJustified(2, '0'), jDescription);
						}
					}

					jFirmwareData.insert(QLatin1String("uartId"), uartId);
					jFirmwareData.insert(QLatin1String("uartType"), firmwareData.uartType);
					jFirmwareData.insert(QLatin1String("eepromFramePayloadSize"), firmwareData.eepromFramePayloadSize);
					jFirmwareData.insert(QLatin1String("eepromFrameSize"), firmwareData.eepromFrameSize);
					jFirmwareData.insert(QLatin1String("eepromFrameCount"), static_cast<int>(firmwareData.frames.size()));

					jFirmwaresDataArray.push_back(jFirmwareData);
				}


				jSubsystemData.insert(QLatin1String("z_firmwareData"), jFirmwaresDataArray);

				jSubsystemData.insert(QLatin1String("subsystemId"), subsystemId);
			}

			jSubsystemDataArray.push_back(jSubsystemData);
		}

		jObject.insert(QLatin1String("z_s_subsystemsData"), jSubsystemDataArray);

		// properties
		//

		m_buildSoftware = qApp->applicationName() +" v" + qApp->applicationVersion();
#ifndef QT_DEBUG
		m_buildSoftware += ", release";
#else
		m_buildSoftware += ", debug";
#endif

#ifdef GITLAB_CI_BUILD
		m_buildSoftware += ", commit SHA1: " CI_COMMIT_SHA;
#else
		m_buildSoftware += ", commit SHA1: LOCALBUILD";
#endif


		QString m_buildTime = QDateTime().currentDateTime().toString("dd.MM.yyyy hh:mm:ss");

		jObject.insert(QLatin1String("userName"), m_userName);
		jObject.insert(QLatin1String("projectName"), m_projectName);
		jObject.insert(QLatin1String("buildNumber"), m_buildNumber);
		jObject.insert(QLatin1String("changesetId"), m_changesetId);
		jObject.insert(QLatin1String("fileVersion"), maxFileVersion());
		jObject.insert(QLatin1String("buildSoftware"), m_buildSoftware);
		jObject.insert(QLatin1String("buildTime"), m_buildTime);

		dest = QJsonDocument(jObject).toJson();

		return true;
	}

	void ModuleFirmwareWriter::setScriptFirmware(QString subsysId, int uartId)
	{
		scriptFirmware = nullptr;
		scriptFirmwareData = nullptr;

		bool ok = false;
		ModuleFirmware& fw = firmware(subsysId, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}


		ModuleFirmwareData& fd = fw.firmwareData(uartId, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		UartChannelData& channelData = uartChannelData(subsysId, uartId);

		scriptFirmware = &fw;
		scriptFirmwareData = &fd;
		scriptUartChannelData = &channelData;
	}

	bool ModuleFirmwareWriter::setData8(int frameIndex, int offset, quint8 data)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint8* ptr = static_cast<quint8*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = data;

		if (scriptFirmwareData->maxFrameIndex < frameIndex)
		{
			scriptFirmwareData->maxFrameIndex = frameIndex;
		}

		return true;
	}

	bool ModuleFirmwareWriter::setData16(int frameIndex, int offset, quint16 data)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint16 dataBE= qToBigEndian(data);

		quint16* ptr = reinterpret_cast<quint16*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = dataBE;

		if (scriptFirmwareData->maxFrameIndex < frameIndex)
		{
			scriptFirmwareData->maxFrameIndex = frameIndex;
		}

		return true;
	}

	bool ModuleFirmwareWriter::setData32(int frameIndex, int offset, quint32 data)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint32 dataBE = qToBigEndian(data);

		quint32* ptr = reinterpret_cast<quint32*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = dataBE;

		if (scriptFirmwareData->maxFrameIndex < frameIndex)
		{
			scriptFirmwareData->maxFrameIndex = frameIndex;
		}

		return true;
	}

	bool ModuleFirmwareWriter::setDataFloat(int frameIndex, int offset, float data)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		float dataBE = reverseFloat(data);

		float* ptr = reinterpret_cast<float*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = dataBE;

		if (scriptFirmwareData->maxFrameIndex < frameIndex)
		{
			scriptFirmwareData->maxFrameIndex = frameIndex;
		}

		return true;
	}

	bool ModuleFirmwareWriter::setData64(int frameIndex, int offset, quint64 data)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmware);
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(data)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return false;
		}

		quint64 dataBE = qToBigEndian(data);

		quint64* ptr = reinterpret_cast<quint64*>(scriptFirmwareData->frames[frameIndex].data() + offset);
		*ptr = dataBE;

		if (scriptFirmwareData->maxFrameIndex < frameIndex)
		{
			scriptFirmwareData->maxFrameIndex = frameIndex;
		}

		return true;
	}

	quint8 ModuleFirmwareWriter::data8(int frameIndex, int offset)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(quint8)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		return static_cast<quint8>(*(scriptFirmwareData->frames[frameIndex].data() + offset));

	}

	quint16 ModuleFirmwareWriter::data16(int frameIndex, int offset)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(quint16)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = scriptFirmwareData->frames[frameIndex];

		quint16 data = *(reinterpret_cast<quint16*>(frameData.data() + offset));
		return qFromBigEndian(data);
	}

	quint32 ModuleFirmwareWriter::data32(int frameIndex, int offset)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(quint32)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = scriptFirmwareData->frames[frameIndex];

		quint32 data = *(reinterpret_cast<quint32*>(frameData.data() + offset));
		return qFromBigEndian(data);
	}

	float ModuleFirmwareWriter::dataFloat(int frameIndex, int offset)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(float)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		std::vector<quint8>& frameData = scriptFirmwareData->frames[frameIndex];

		float data = *(reinterpret_cast<float*>(frameData.data() + offset));
		return reverseFloat(data);
	}

	JsVariantList* ModuleFirmwareWriter::calcHash64(QString dataString)
	{

		QByteArray bytes = dataString.toUtf8();

		quint64 result = ::calcHash(bytes.data(), bytes.size());

		quint32 h = (result >> 32) & 0xffffffff;
		quint32 l = result & 0xffffffff;

		JsVariantList* vl = new JsVariantList(this);
		vl->append(QVariant(h));
		vl->append(QVariant(l));
		return vl;
	}

	QString ModuleFirmwareWriter::storeCrc64(int frameIndex, int start, int count, int offset)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return QString();
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(quint64)) || start + count > scriptFirmwareData->eepromFramePayloadSize)
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return QString();
		}

		quint64 result = Crc::crc64(scriptFirmwareData->frames[frameIndex].data() + start, count);
		setData64(frameIndex, offset, result);

		return QString::number(result, 16);
	}

	QString ModuleFirmwareWriter::storeHash64(int frameIndex, int offset, QString dataString)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return QString();
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				offset > (int)(scriptFirmwareData->eepromFramePayloadSize - sizeof(quint64)))
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return QString("");
		}

		QByteArray bytes = dataString.toUtf8();

		quint64 result = ::calcHash(bytes.data(), bytes.size());
		setData64(frameIndex, offset, result);

		return QString::number(result, 16);

		//qDebug() << "Frame " << frameIndex << "Count " << count << "Offset" << offset << "CRC" << hex << result;

	}

	quint32 ModuleFirmwareWriter::calcCrc32(int frameIndex, int start, int count)
	{
		if (scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmwareData);
			return false;
		}

		//

		if (frameIndex >= static_cast<int>(scriptFirmwareData->frames.size()) ||
				start + count > scriptFirmwareData->eepromFramePayloadSize)
		{
			qDebug() << Q_FUNC_INFO << " ERROR: FrameIndex or Frame offset is too big";
			return 0;
		}

		quint64 result = Crc::crc64(scriptFirmwareData->frames[frameIndex].data() + start, count);

		return result & 0xffffffff;
	}

	void ModuleFirmwareWriter::jsSetDescriptionFields(int descriptionVersion, QString fields)
	{
		if (scriptFirmware == nullptr || scriptFirmwareData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmware);
			assert(scriptFirmwareData);
			return;
		}

		setDescriptionFields(scriptFirmware->subsysId(), scriptFirmwareData->uartId , descriptionVersion, fields.split(';'));
	}

	void ModuleFirmwareWriter::jsInsertDescription(int index, int channel, QString descriptionCSV)
	{
		if (scriptUartChannelData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptUartChannelData);
			return;
		}

		QStringList l = descriptionCSV.split(';');

		QVariantList v;
		for (auto s : l)
		{
			v.append(s);
		}

		//

		auto dd = scriptUartChannelData->descriptonDataMap.find(channel);
		if (dd == scriptUartChannelData->descriptonDataMap.end())
		{
			std::vector<QVariantList> descriptonData;
			descriptonData.push_back(v);

			scriptUartChannelData->descriptonDataMap[channel] = descriptonData;

		}
		else
		{
			std::vector<QVariantList>& descriptionData = dd->second;
			if (index == -1)
			{
				descriptionData.push_back(v);
			}
			else
			{
				if (index < 0 || index >= static_cast<int>(descriptionData.size()))
				{
					Q_ASSERT(false);
					return;
				}

				descriptionData.insert(descriptionData.begin() + index, v);
			}
		}
	}

	void ModuleFirmwareWriter::jsAddDescription(int channel, QString descriptionCSV)
	{
		return jsInsertDescription(-1, channel, descriptionCSV);
	}

	void ModuleFirmwareWriter::jsSetUniqueID(int lmNumber, quint64 uniqueID)
	{
		if (scriptUartChannelData == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptUartChannelData);
			return;
		}

		scriptUartChannelData->uniqueIdMap[lmNumber] = uniqueID;

	}

	UnitsConvertor* ModuleFirmwareWriter::jsGetUnitsConvertor()
	{
		QQmlEngine::setObjectOwnership(&m_unitsConvertor, QQmlEngine::ObjectOwnership::CppOwnership);
		return &m_unitsConvertor;
	}


	void ModuleFirmwareWriter::writeLog(QString logString)
	{
		if (scriptFirmware == nullptr)	//maybe setScriptFirmware is not called!
		{
			assert(scriptFirmware);
			return;
		}

		QStringList& log = m_scriptLog[scriptFirmware->subsysId()];

		log.push_back(logString);
	}

	void ModuleFirmwareWriter::replaceLog(const QString& subsysId, const QString& oldString, const QString& newString)
	{
		if (m_scriptLog.find(subsysId) == m_scriptLog.end())
		{
			Q_ASSERT(false);
			return;
		}

		QStringList& l = m_scriptLog.at(subsysId);

		l.replaceInStrings(oldString, newString);

		return;
	}

	QByteArray ModuleFirmwareWriter::scriptLog(const QString& subsysId) const
	{
static QByteArray err;
		if (m_scriptLog.find(subsysId) == m_scriptLog.end())
		{
			return err;
		}

		const QStringList& l = m_scriptLog.at(subsysId);

		return l.join(QString()).toUtf8();
	}

	quint64 ModuleFirmwareWriter::uniqueID(const QString& subsysId, int uartId, int lmNumber, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		UartChannelData& channelData = uartChannelData(subsysId, uartId);

		auto it = channelData.uniqueIdMap.find(lmNumber);
		if (it == channelData.uniqueIdMap.end())
		{
			assert(false);

			if (ok != nullptr)
			{
				*ok = false;
			}

			return 0;
		}

		return it->second;
	}

	void ModuleFirmwareWriter::setGenericUniqueId(const QString& subsysId, int lmNumber, quint64 genericUniqueId)
	{
		bool ok = false;

		ModuleFirmware& mf = firmware(subsysId, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		std::vector<UartPair> uarts = mf.uartList();

		for (auto uartIt : uarts)
		{
			int uartId = uartIt.first;

			UartChannelData& channelData = uartChannelData(subsysId, uartId);

			if (channelData.binaryDataMap.empty() == true)
			{
				// This case is used for Configuration data (data is stored in frames, no binary data exists)

				if (mf.uartExists(uartId) == false)
				{
					assert(false);
					return;
				}

				ModuleFirmwareData& data = mf.firmwareData(uartId, &ok);
				if (ok == false)
				{
					assert(false);
					return;
				}

				const int ConfigDataStartFrame = 2;

				const int LMNumberConfigFrameCount = 19;

				int frameNumber = ConfigDataStartFrame + LMNumberConfigFrameCount * (lmNumber - 1);

				const int UniqueIdOffset = 4;

				quint64 uidBE = qToBigEndian(genericUniqueId);

				if (frameNumber < 0 || frameNumber >= (int)data.frames.size())
				{
					assert(false);
					return;
				}

				std::vector<quint8>& frame = data.frames[frameNumber];

				quint8* pData = frame.data();

				quint64* pUniqueIdPtr = (quint64*)(pData + UniqueIdOffset);

				*pUniqueIdPtr = uidBE;

				// Write uniqueID description

				QString equipmentId;

				const std::vector<LogicModuleInfo>& mi = mf.logicModulesInfo();
				for (auto& lmi : mi)
				{
					if (lmi.lmNumber == lmNumber && lmi.subsystemId == subsysId)
					{
						equipmentId = lmi.equipmentId;
						break;
					}
				}

				const int UniqueIdDescriptionPos = 9;

				setScriptFirmware(subsysId, uartId);
				jsInsertDescription(UniqueIdDescriptionPos, lmNumber, tr("%1;%2;%3;0;64;UniqueID;0x%4").arg(equipmentId).arg(frameNumber).arg(UniqueIdOffset).arg(QString::number(genericUniqueId, 16)));

				QString oldString = tr("    [%1:%2] UniqueID = 0\r\n").arg(frameNumber).arg(UniqueIdOffset);
				QString newString = tr("    [%1:%2] UniqueID = 0x%3\r\n").arg(frameNumber).arg(UniqueIdOffset).arg(QString::number(genericUniqueId, 16));

				replaceLog(subsysId, oldString, newString);
			}
			else
			{
				// This case is used for Application and Tuning data (data is stored in Channel Data)

				if (channelData.uniqueIdMap.find(lmNumber) == channelData.uniqueIdMap.end())
				{
					assert(false);
					return;
				}

				channelData.uniqueIdMap[lmNumber] = genericUniqueId;
			}
		}
	}

	bool ModuleFirmwareWriter::storeChannelData(Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		const int storageConfigFrame = 1;
		const int startDataFrame = 2;

		for (auto& it : m_firmwares)
		{
			ModuleFirmware& firmware = it.second;

			quint16 ssKeyValue = firmware.ssKey() << 6;

			std::vector<UartPair> uarts = firmware.uartList();

			for (auto uartIt : uarts)
			{
				int uartId = uartIt.first;

				UartChannelData& channelData = uartChannelData(firmware.subsysId(), uartId);

				if (channelData.binaryDataMap.empty() == true)
				{
					// no channel data supplied
					//
					continue;
				}

				if (firmware.eepromFrameCount(uartId) < 3)
				{
					log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, subsystem %1: At least 3 frames needed.").arg(firmware.subsysId()));
					return false;
				}

				// sort channel data by growing channel number
				//
				const int LMNumber_Min = 1;
				const int LMNumber_Max = 64;

				std::vector<std::pair<int, int>> channelNumbersAndSize;
				for (auto channelDataIt = channelData.binaryDataMap.begin(); channelDataIt != channelData.binaryDataMap.end(); channelDataIt++)
				{
					int channel = channelDataIt->first;

					if (channel < LMNumber_Min || channel > LMNumber_Max)
					{
						log->errINT1000(QString("ModuleFirmwareWriter::storeChannelData error, LM number %1: Wrong LM number, expected %2..%3.").arg(channel).arg(LMNumber_Min).arg(LMNumber_Max));
						return false;
					}

					const QByteArray& data = channelDataIt->second;

					double fSize = (double)data.size() / firmware.eepromFramePayloadSize(uartId);
					fSize = ceil(fSize);
					int size = (int)fSize;

					channelNumbersAndSize.push_back(std::make_pair(channel, size));
				}
				std::sort(channelNumbersAndSize.begin(), channelNumbersAndSize.end(), [](std::pair<int, int> a, std::pair<int, int> b)
				{
					return a.first < b.first;
				});

				// place channel data to frames

				bool ok = false;

				ModuleFirmwareData& firmwareData = firmware.firmwareData(uartId, &ok);
				if (ok == false)
				{
					assert(false);
					return false;
				}

				std::vector<int> channelStartFrame;

				firmwareData.maxFrameIndex = storageConfigFrame;

				int frame = startDataFrame;
				int lmNumberCount = static_cast<int>(channelNumbersAndSize.size());

				for (int c = 0; c < lmNumberCount; c++)
				{
					int channel = channelNumbersAndSize[c].first;
					int size = (quint16)channelNumbersAndSize[c].second;

					int eepromFrameCount = firmware.eepromFrameCount(uartId);
					int eepromFrameCount95 = static_cast<int>(eepromFrameCount * 0.95);

					if (frame == eepromFrameCount95)
					{
						log->wrnALC5800(firmware.subsysId(), uartId);
					}

					if (frame >= firmware.eepromFrameCount(uartId))
					{
						log->errALC5801(firmware.subsysId(), channel, uartId);
						return false;
					}

					channelStartFrame.push_back(frame);

					// channel data
					//
					const QByteArray& binaryData = channelData.binaryDataMap.at(channel);

					quint64 uniqueId = channelData.uniqueIdMap.at(channel);

					// channel service information
					//

					quint8* ptr = firmwareData.frames[frame].data();

					*(quint16*)ptr = qToBigEndian((quint16)0x0001);		//Channel configuration version
					ptr += sizeof(quint16);

					*(quint16*)ptr = qToBigEndian((quint16)uartId);	//Data type (configuration)
					ptr += sizeof(quint16);

					if (uniqueId == 0)
					{
						QByteArray bytes = firmware.subsysId().toUtf8();

						*(quint64*)ptr = qToBigEndian(::calcHash(bytes.data(), bytes.size()));
						ptr += sizeof(quint64);
					}
					else
					{
						*(quint64*)ptr = qToBigEndian(uniqueId);
						ptr += sizeof(quint64);
					}

					*(quint16*)ptr = qToBigEndian((quint16)size);           // Frames count
					ptr += sizeof(quint16);

					firmwareData.maxFrameIndex = frame;

					frame++;

					if (size != 0)
					{
						// store channel data in frames
						//
						int index = 0;
						for (int i = 0; i < binaryData.size(); i++)
						{
							if (index >= firmware.eepromFramePayloadSize(uartId))
							{
								// data is bigger than frame - switch to the next frame
								//
								frame++;
								index = 0;
							}

							if (frame == eepromFrameCount95 && index == 0)
							{
								log->wrnALC5800(firmware.subsysId(), uartId);
							}

							if (frame >= firmware.eepromFrameCount(uartId))
							{
								log->errALC5801(firmware.subsysId(), channel, uartId);
								return false;
							}

							firmwareData.frames[frame][index++] = binaryData[i];

							firmwareData.maxFrameIndex = frame;
						}

						//switch to the next frame
						//
						frame++;
					}
				}


				// fill storage config frame
				//
				quint8* ptr = firmwareData.frames[storageConfigFrame].data();

				*(quint16*)ptr = qToBigEndian((quint16)0xCA70);	// Configuration reference mark
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)0x0001);	// Configuration structure version
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)ssKeyValue);	// Subsystem key
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)m_buildNumber);	// Build number
				ptr += sizeof(quint16);

				*(quint16*)ptr = qToBigEndian((quint16)firmware.lmDescriptionNumber());	// Description number
				ptr += sizeof(quint16);	//reserved

				ptr += sizeof(quint32);	//reserved

				*(quint16*)ptr = qToBigEndian((quint16)lmNumberCount);	// Configuration channels quantity
				ptr += sizeof(quint16);

				if (channelNumbersAndSize.size() != channelStartFrame.size())
				{
					assert(channelNumbersAndSize.size() == channelStartFrame.size());
					return false;
				}

				quint8* ptrChannelTable = ptr;

				for (size_t i = 0; i < channelNumbersAndSize.size(); i++)	// Start frames
				{
					int channel = channelNumbersAndSize[i].first;

					quint8* ptrChannel = ptrChannelTable + (sizeof(quint16) * 3) *(channel - 1);

					quint16 startFrame = (quint16)channelStartFrame[i];

					*(quint16*)ptrChannel = qToBigEndian(startFrame);
					ptrChannel += sizeof(quint16);

					ptrChannel += sizeof(quint32);
				}
			}
		}

		return true;
	}
}
