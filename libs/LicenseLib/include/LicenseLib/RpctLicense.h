#pragma once

#include <QByteArray>
#include <QDate>
#include <QUuid>

#include <memory>


namespace Proto
{
	class License;
}

namespace LicenseLib
{
	Q_NAMESPACE

	enum class WorkplaceCheckType
	{
		Strict = 0,
		Relaxed = 1,
		NoCheck = 2
	};
	Q_ENUM_NS(WorkplaceCheckType)

	class RpctLicense
	{
	public:
		RpctLicense();
		RpctLicense(const RpctLicense& rhs);
		RpctLicense(RpctLicense&& rhs) noexcept;
		~RpctLicense();

		RpctLicense& operator=(const RpctLicense& rhs);
		RpctLicense& operator=(RpctLicense&& rhs) noexcept;

	public:
		void clear();
		bool isNull() const;

		// openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048
		// openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048 -aes256 -pass pass:your_password
		// openssl rsa -pubout -in private_key.pem -out public_key.pem
		//
		QByteArray toRawData(QString privateKey, QString keyPassword, QString* errorMessage) const;

		static RpctLicense fromRawData(const QByteArray& data, QString publicKeyFileName, QString* errorMessage);
		static RpctLicense fromRawData(const QByteArray& data, const QByteArray& publicKeyData, QString* errorMessage);

	public:
		bool revoked() const;
		void setRevoked(bool value);

		QUuid uuid() const;
		void setUuid(const QUuid& value);

		QString caption() const;
		void setCaption(const QString& value);

		QString organization() const;
		void setOrganization(const QString& value);

		QString firstName() const;
		void setFirstName(const QString& value);

		QString lastName() const;
		void setLastName(const QString& value);

		QString contactInfo() const;
		void setContactInfo(const QString& value);

		QDate startDate() const;
		void setStartDate(const QDate& value);

		QDate endDate() const;
		void setEndDate(const QDate& value);

		QString notes() const;
		void setNotes(const QString& value);

		QString licenserVersion() const;
		void setLicenserVersion(const QString& value);

		QDate issueDate() const;
		void setIssueDate(const QDate& value);

		// WorkplaceLicense
		//
		WorkplaceCheckType workplaceCheckType() const;
		void setWorkplaceCheckType(WorkplaceCheckType value);

		QString workplaceId() const;
		void setWorkplaceId(const QString& value);

		// ModuleConfiguratorLicense
		//
		bool allowedModuleConfigurator() const;
		void setAllowedModuleConfigurator(bool value);

		bool mcWriteServiceEeprom() const;
		void setMcWriteServiceEeprom(bool value);

		bool mcLimitModuleUartUuids() const;
		void setMcLimitModuleUartUuids(bool value);

		QString mcAllowedModuleUartUuids() const;
		void setMcAllowedModuleUartUuids(const QString& value);

		// u7
		//
		bool allowedU7() const;
		void setAllowedU7(bool value);

		// Simulator application
		//
		bool allowedSimulatorApp() const;
		void setSimulatorApp(bool value);

	private:
		std::unique_ptr<Proto::License> m_data;
	};
} // namespace LicenseLib