#pragma once
#include <LicenseLib/RpctValidator.h>

namespace LicenseLib
{
	class AppLicenser
	{
	public:
		AppLicenser(QString publicKeyFileName = QString{":/LicenseLib/public_key_inst1.pem"});

		static bool guiAppStartValidation(QDate buildDate, QWidget* parent = nullptr);

	public:
		bool loadAppLicense(QString& errorMessage,
							QString publicKeyFileName = QString{
								":/LicenseLib/public_key_inst1.pem"}); // Tries to load license from application directory

		void showLicenseDialog();                                      // Shows license dialog with license information
		QString getLicenseInfo();                                      // Returns license information in a arbitrary string format

		static QString licensePath();                                  // Returns the path to the license file for the application
		static QString workplaceId();                                  // Generates workplace id, which is used to identify the workplace
		static QString workplaceIdV0();
		static QString workplaceIdV1();

		static bool showRestrictionMessageBox(QWidget* parent,
											  LicenseLib::ValidationResult validationResult,
											  const QString& extraInfo = {});

		const LicenseLib::RpctValidator& validator() const;

#ifndef NDEBUG
		static bool noLicenseCheck();
#endif // NDEBUG

	public:
		QUuid uuid() const;
		QString organization() const;
		QString person() const;
		QDate endDate() const;

	private:
		LicenseLib::RpctValidator m_validator;
	};
} // namespace LicenseLib