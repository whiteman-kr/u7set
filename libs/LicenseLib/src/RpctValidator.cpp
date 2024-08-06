#include <License.pb.h>
#include <LicenseLib/AppLicenser.h>
#include <LicenseLib/RpctLicense.h>
#include <LicenseLib/RpctValidator.h>

#include <QDir>
#include <QRegularExpression>

namespace
{
	Proto::InternalWorkplaceId parsedWorkplaceId(QString workplaceId)
	{
		Proto::InternalWorkplaceId result;
		if (workplaceId.startsWith("ZY") == false || workplaceId.endsWith("BA") == false)
		{
			return result;
		}

		workplaceId = workplaceId.mid(2, workplaceId.size() - 4);
		QByteArray decoded = QByteArray::fromBase64(workplaceId.toUtf8());
		QByteArray uncompressed = qUncompress(decoded);

		if (uncompressed.isEmpty() == true)
		{
			return result;
		}

#if 0 // It makes code much longer ((
		char mask = 0x7;
		for (qsizetype i = 0; i < uncompressed.size(); i++)
		{
			uncompressed[i] = uncompressed[i] ^ mask;
			mask = mask >= 100 ? 0x7 : mask + 1;
		}
#endif
		result.ParseFromArray(uncompressed.constData(), uncompressed.size());
#if 0
		qDebug() << workplaceId;
		qDebug() << "Parsed workplace id: " << QString::fromStdString(result.machine_id());
		qDebug() << "Parsed workplace id: " << QString::fromStdString(result.hardware_id());
		qDebug() << "Parsed workplace id: " << QString::fromStdString(result.cpu());
		qDebug() << "Parsed workplace id: " << QString::fromStdString(result.macs());
#endif
		return result;
	}
} // namespace

namespace LicenseLib
{
	RpctValidator::RpctValidator() :
		m_license{std::make_unique<RpctLicense>()}
	{
	}

	RpctValidator::~RpctValidator() = default;

	bool RpctValidator::loadFromDir(QString directory,
									QString* errorMessage,
									QString publicKeyFileName /* = QString{":/LicenseLib/public_key_inst1.pem"}*/)
	{
		// Find the only file with extension .rls in a folder,
		//
		QDir dir{directory};
		dir.setNameFilters(QStringList{} << "*.rls");

		if (dir.count() != 1)
		{
			if (errorMessage != nullptr)
			{
				*errorMessage = "Failed to find a single license file in directory: " + directory;
			}

			return false;
		}

		return loadFromFile(dir.filePath(dir.entryList().first()), errorMessage, publicKeyFileName);
	}

	bool RpctValidator::loadFromFile(QString fileName, QString* errorMessage, QString publicKeyFileName)
	{
		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return false;
		}

		// If any error occurs, set errorMessage and return false
		// If file is loaded successfully, return true
		//
		QFile file{fileName};

		if (file.open(QIODevice::ReadOnly) == false)
		{
			if (errorMessage != nullptr)
			{
				*errorMessage = "Failed to open file: " + file.fileName() + ", error: " + file.errorString();
			}

			return false;
		}

		QByteArray fileData = file.readAll();
		file.close();

		*m_license = RpctLicense::fromRawData(fileData, QString{publicKeyFileName}, errorMessage);

		if (m_license->isNull() == true)
		{
			*errorMessage = "Failed to parse file: " + file.fileName() + ", Error: " + *errorMessage;
		}

		return m_license->isNull() == false;
	}

	ValidationResult RpctValidator::validateDate(const QDate& currentDate, const QDate& softwareReleaseDate) const
	{
#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return ValidationResult::Valid;
		}
#endif

		Q_ASSERT(m_license);

		if (m_license->isNull() == true)
		{
			return ValidationResult::NotFound;
		}

		if (currentDate < softwareReleaseDate)
		{
			return ValidationResult::Invalid;
		}

		if (currentDate > m_license->endDate())
		{
			return ValidationResult::Expired;
		}

		return ValidationResult::Valid;
	}

	ValidationResult RpctValidator::validateWorkplace() const
	{
#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return ValidationResult::Valid;
		}
#endif

		// Decode workplaceId
		//
		Proto::InternalWorkplaceId lhs = parsedWorkplaceId(AppLicenser::workplaceId());
		Proto::InternalWorkplaceId rhs = parsedWorkplaceId(m_license->workplaceId());

		int matches = 0;
		int expectedMatches = 2;

		if (lhs.machine_id().empty() == false || rhs.machine_id().empty() == false)
		{
			matches += (lhs.machine_id() == rhs.machine_id());
		}

		if (lhs.hardware_id().empty() == false || rhs.hardware_id().empty() == false)
		{
			matches += (lhs.hardware_id() == rhs.hardware_id());
		}

		if (lhs.cpu().empty() == false || rhs.cpu().empty() == false)
		{
			matches += (lhs.cpu() == rhs.cpu());
		}

		QStringList lhsMacs = QString::fromStdString(lhs.macs()).split(" ");
		QStringList rhsMacs = QString::fromStdString(rhs.macs()).split(" ");

		bool macMatched = false;
		for (const QString& lm : lhsMacs)
		{
			for (const QString& rm : rhsMacs)
			{
				if (lm == rm)
				{
					macMatched = true;
					break;
				}
			}

			if (macMatched == true)
			{
				break;
			}
		}

		matches += macMatched;
		return (matches >= expectedMatches) ? ValidationResult::Valid : ValidationResult::Invalid;
	}

	ValidationResult RpctValidator::validateModuleConfigurator() const
	{
#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return ValidationResult::Valid;
		}
#endif

		return m_license->allowedModuleConfigurator() ? ValidationResult::Valid : ValidationResult::Invalid;
	}

	ValidationResult RpctValidator::validateModuleConfiguratorModule(QUuid uartUuid) const
	{
#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return ValidationResult::Valid;
		}
#endif

		if (m_license->allowedModuleConfigurator() == false)
		{
			// Configuration is not allowed at all
			//
			return ValidationResult::Invalid;
		}

		if (m_license->mcLimitModuleUartUuids() == false)
		{
			// Allowed configuration for all modules
			//
			return ValidationResult::Valid;
		}

		QStringList allowedUuids = m_license->mcAllowedModuleUartUuids().split(QRegularExpression{"[ \n]+"}, Qt::SkipEmptyParts);

		std::set<QUuid> uuids;
		for (const QString& uuidStr : allowedUuids)
		{
			QUuid uuid{uuidStr};
			if (uuid.isNull() == false)
			{
				uuids.insert(uuid);
			}
		}

		return uuids.contains(uartUuid) ? ValidationResult::Valid : ValidationResult::Invalid;
	}

	const RpctLicense& RpctValidator::license() const
	{
		Q_ASSERT(m_license);
		return *m_license;
	}

} // namespace LicenseLib
