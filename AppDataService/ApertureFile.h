#pragma once

#include "../OnlineLib/ApertureRecord.h"

class ApertureFile
{
public:
	ApertureFile();

	void clear();

	bool load(const QString& appPath);
	bool save();

	void updateAperture(const ApertureRecord& ar);

	const std::map<QString, ApertureRecord> apertures() const { return m_apertures; }

private:
	bool loadFile(const QString& fileName);

private:
	QString m_appPath;
	std::map<QString, ApertureRecord> m_apertures;		// appSignalID => ApertureRecord

	inline static const QString APERTURES_CSV = QStringLiteral("Apertures.csv");
	inline static const QString APERTURES_BAK = QStringLiteral("Apertures.bak");

	inline static const QString SIGNAL_ID = QStringLiteral("SignalID");
	inline static const QString APERTURE_TYPE = QStringLiteral("ApertureType");
	inline static const QString COARSE_APERTURE = QStringLiteral("CoarseAperture");
	inline static const QString FINE_APERTURE = QStringLiteral("FineAperture");
};
