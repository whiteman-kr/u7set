#include <License.pb.h>
#include <LicenseLib/AppLicenser.h>
#include <LicenseLib/RpctLicense.h>
#include <LicenseLib/RpctValidator.h>

#include "Blacklist.h"

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
		m_license{std::make_unique<RpctLicense>()},
		m_blacklist{std::make_unique<Blacklist>()}
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

	bool RpctValidator::isRevoked() const
	{
		if (m_license == nullptr)
		{
			Q_ASSERT(m_license);
			return false;
		}

		return m_license->revoked();
	}

	ValidationResult RpctValidator::isBlacklisted() const
	{
		Q_ASSERT(m_license);
		if (m_license == nullptr)
		{
			return ValidationResult::Invalid;
		}

		auto r = m_blacklist->check(m_license->uuid());
		if (r.has_value() == true)
		{
			Q_ASSERT(r.value());
			return ValidationResult::Valid;
		}

		switch (r.error())
		{
		case BlacklistReason::Revoked:
			return ValidationResult::Revoked;
		case BlacklistReason::Expired:
			return ValidationResult::Expired;
		default:
			break;
		}

		return ValidationResult::Invalid;
	}

	ValidationResult RpctValidator::validateDate(const QDate& currentDate, const QDate& softwareReleaseDate) const
	{
		Q_ASSERT(m_license);

#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return ValidationResult::Valid;
		}
#endif
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
		Proto::InternalWorkplaceId lhs = parsedWorkplaceId(m_license->workplaceId());
		Proto::InternalWorkplaceId rhs;
		switch (lhs.version())
		{
		case 0:
			rhs = parsedWorkplaceId(AppLicenser::workplaceIdV0());
			break;
		case 1:
			rhs = parsedWorkplaceId(AppLicenser::workplaceIdV1());
			break;
		default:
			rhs = parsedWorkplaceId(AppLicenser::workplaceId());
		}

		if (lhs.version() == 0)
		{
			int matches = 0;
			const int expectedMatches = 2;

			if (lhs.machine_id().empty() == false || rhs.machine_id().empty() == false)
			{
				QString l = QString::fromStdString(lhs.machine_id()).toLower();
				QString r = QString::fromStdString(rhs.machine_id()).toLower();
				matches += (l == r);
			}

			if (lhs.hardware_id().empty() == false || rhs.hardware_id().empty() == false)
			{
				QString l = QString::fromStdString(lhs.hardware_id()).toLower();
				QString r = QString::fromStdString(rhs.hardware_id()).toLower();
				matches += (l == r);
			}

			if (lhs.cpu().empty() == false || rhs.cpu().empty() == false)
			{
				QString l = QString::fromStdString(lhs.cpu()).toLower();
				QString r = QString::fromStdString(rhs.cpu()).toLower();
				matches += (l == r);
			}

			QStringList lhsMacs = QString::fromStdString(lhs.macs()).toLower().split(" ");
			QStringList rhsMacs = QString::fromStdString(rhs.macs()).toLower().split(" ");

			bool macMatched = std::any_of(lhsMacs.begin(),
										  lhsMacs.end(),
										  [&](const QString& leftMac)
										  {
											  return rhsMacs.contains(leftMac);
										  });

			matches += macMatched;
			return (matches >= expectedMatches) ? ValidationResult::Valid : ValidationResult::Invalid;
		}

		if (lhs.version() == 1)
		{
			double matches = 0;
			constexpr double expectedMatches = 0.59;

			if (lhs.motherboard_info_hash() == rhs.motherboard_info_hash())
			{
				matches += 0.5;
			}

			if (lhs.machine_id().empty() == false || rhs.machine_id().empty() == false)
			{
				QString l = QString::fromStdString(lhs.machine_id()).toLower();
				QString r = QString::fromStdString(rhs.machine_id()).toLower();
				matches += (l == r) ? 0.25 : 0.0;
			}

			if (lhs.hardware_id().empty() == false || rhs.hardware_id().empty() == false)
			{
				QString l = QString::fromStdString(lhs.hardware_id()).toLower();
				QString r = QString::fromStdString(rhs.hardware_id()).toLower();
				matches += (l == r) ? 0.25 : 0.0;
			}

			if (lhs.cpu().empty() == false || rhs.cpu().empty() == false)
			{
				QString l = QString::fromStdString(lhs.cpu()).toLower();
				QString r = QString::fromStdString(rhs.cpu()).toLower();
				matches += (l == r) ? 0.1 : 0.0;
			}

			QStringList lhsMacs = QString::fromStdString(lhs.macs()).toLower().split(" ");
			QStringList rhsMacs = QString::fromStdString(rhs.macs()).toLower().split(" ");

			bool macMatched = std::any_of(lhsMacs.begin(),
										  lhsMacs.end(),
										  [&](const QString& leftmac)
										  {
											  return rhsMacs.contains(leftmac);
										  });

			matches += macMatched ? 0.25 : 0.0;

			// Check threshold of matches.
			//
			return (matches >= expectedMatches) ? ValidationResult::Valid : ValidationResult::Invalid;
		}

		Q_ASSERT(false);
		return ValidationResult::Invalid;
	}

	ValidationResult RpctValidator::validateAppU7() const
	{
#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return ValidationResult::Valid;
		}
#endif
		return m_license->allowedU7() ? ValidationResult::Valid : ValidationResult::Invalid;
	}

	ValidationResult RpctValidator::validateAppSimulator() const
	{
#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return ValidationResult::Valid;
		}
#endif
		return m_license->allowedSimulatorApp() ? ValidationResult::Valid : ValidationResult::Invalid;
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

	QString RpctValidator::dumpWorkplaceId(const QString& workplaceId)
	{
		Proto::InternalWorkplaceId proto = parsedWorkplaceId(workplaceId);

		QString result;
		result += "Version: " + QString::number(proto.version()) + "\n";
		result += "MachineId: " + QString::fromStdString(proto.machine_id()) + "\n";
		result += "HardwareId: " + QString::fromStdString(proto.hardware_id()) + "\n";
		result += "CPU: " + QString::fromStdString(proto.cpu()) + "\n";
		result += "MACs: " + QString::fromStdString(proto.macs()) + "\n";
		result += "MotherboardInfoHash: " + QString("0x%1").arg(proto.motherboard_info_hash(), 8, 16, QChar('0')) + "\n";

		result += "OS: " + QString::fromStdString(proto.os()) + "\n";
		result += "Host: " + QString::fromStdString(proto.host()) + "\n";

		return result;
	}

} // namespace LicenseLib
