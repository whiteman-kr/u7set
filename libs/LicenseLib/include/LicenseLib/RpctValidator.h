#pragma once

#include <LicenseLib/RpctLicense.h>

namespace LicenseLib
{
	class Blacklist;

	enum class ValidationResult
	{
		Valid,    // License is valid
		Invalid,  // License is invalid
		NotFound, // License not found
		Expired,  // License is expired, but still valid
		Revoked   // License is revoked
	};

	class RpctValidator final
	{
	public:
		RpctValidator();
		~RpctValidator();

	public:
		bool loadFromDir(QString directory,
						 QString* errorMessage,
						 QString publicKeyFileName = QString{":/LicenseLib/public_key_inst1.pem"});
		bool loadFromFile(QString fileName,
						  QString* errorMessage,
						  QString publicKeyFileName = QString{":/LicenseLib/public_key_inst1.pem"});

		bool isRevoked() const;

		ValidationResult isBlacklisted() const;

		ValidationResult validateDate(const QDate& currentDate, const QDate& softwareReleaseDate) const;
		ValidationResult validateWorkplace() const;

		ValidationResult validateAppU7() const;
		ValidationResult validateAppSimulator() const;

		ValidationResult validateModuleConfigurator() const;
		ValidationResult validateServiceEepromWrite() const;
		ValidationResult validateModuleConfiguratorModule(QUuid uartUuid) const;

		const RpctLicense& license() const;

		static QString dumpWorkplaceId(const QString& workplaceId);

	private:
		std::unique_ptr<RpctLicense> m_license;
		std::unique_ptr<Blacklist> m_blacklist;
	};
} // namespace LicenseLib