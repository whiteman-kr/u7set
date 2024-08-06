#pragma once

#include <LicenseLib/RpctLicense.h>

namespace LicenseLib
{
	enum class ValidationResult
	{
		Valid,    // License is valid
		Invalid,  // License is invalid
		NotFound, // License not found
		Expired   // License is expired, but still valid
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

		ValidationResult validateDate(const QDate& currentDate, const QDate& softwareReleaseDate) const;
		ValidationResult validateWorkplace() const;

		ValidationResult validateModuleConfigurator() const;
		ValidationResult validateModuleConfiguratorModule(QUuid uartUuid) const;

		const RpctLicense& license() const;

	private:
		std::unique_ptr<RpctLicense> m_license;
	};
} // namespace LicenseLib