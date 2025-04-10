#include "ApertureFile.h"
#include "../UtilsLib/WUtils.h"

ApertureFile::ApertureFile()
{
}

void ApertureFile::clear()
{
	m_appPath.clear();
	m_apertures.clear();
}

bool ApertureFile::load(const QString& appPath)
{
	QFileInfo fi(appPath);

	m_appPath = fi.path();

	if (loadFile(m_appPath + Separator::DIR + APERTURES_CSV))
	{
		return true;
	}

	if (loadFile(m_appPath + Separator::DIR + APERTURES_BAK))
	{
		return true;
	}

	return false;
}

bool ApertureFile::save()
{
	if (m_appPath.isEmpty() == true)
	{
		return false;
	}

	QString csvFileName = m_appPath + Separator::DIR + APERTURES_CSV;
	QString bakFileName = m_appPath + Separator::DIR + APERTURES_BAK;

	QString fileData;

	fileData.append(QString("%1;%2;%3;%3\n").
						arg(SIGNAL_ID).arg(APERTURE_TYPE).arg(COARSE_APERTURE).arg(FINE_APERTURE));

	for(const auto& [signalID, apertureRecord] : m_apertures)
	{
		fileData.append(apertureRecord.toString());
	}

	QFile::remove(bakFileName);
	QFile::rename(csvFileName, bakFileName);

	QFile csvFile(csvFileName);

	if (csvFile.open(QIODevice::WriteOnly) == false)
	{
		return false;
	}

	csvFile.write(fileData.toUtf8());

	csvFile.close();

	return true;
}

void ApertureFile::updateAperture(const ApertureRecord& ar)
{
	auto it = m_apertures.find(ar.signalID);

	if (it == m_apertures.end())
	{
		m_apertures.emplace(ar.signalID, ar);
	}
	else
	{
		it->second = ar;
	}
}

bool ApertureFile::loadFile(const QString& fileName)
{
	QFile file(fileName);

	if (file.open(QIODevice::ReadOnly | QIODeviceBase::Text) == false)
	{
		return false;
	}

	QStringList fileData = QString(file.readAll()).split(Separator::NEW_LINE, Qt::SkipEmptyParts);

	file.close();

	for(const QString& str : fileData)
	{
		if (str == fileData.constFirst())
		{
			QStringList fields = str.split(Separator::NEW_LINE);

			if (fields.size() != 4)
			{
				return false;
			}

			if (fields[0] != SIGNAL_ID ||
				fields[1] != APERTURE_TYPE ||
				fields[2] != COARSE_APERTURE ||
				fields[3] != FINE_APERTURE)
			{
				return false;
			}

			continue;
		}

		ApertureRecord ar;

		ar.fromString(str);

		if (ar.signalID.isEmpty() == false)
		{
			m_apertures.emplace(ar.signalID, ar);
		}
	}

	return true;
}


