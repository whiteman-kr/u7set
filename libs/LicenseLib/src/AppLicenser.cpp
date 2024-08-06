#include <License.pb.h>
#include <LicenseLib/AppLicenser.h>

#include <QAbstractButton>
#include <QCoreApplication>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QProcess>


namespace
{
	// Functions to get hardware identifiers
	//
	QString getHardwareId()
	{
#ifdef _WIN32
		QProcess process;
		process.start("wmic",
					  QStringList{} << "csproduct"
									<< "get"
									<< "uuid");
		process.waitForFinished(3000);
		QString output = process.readAllStandardOutput();
		QStringList lines = output.split("\n", Qt::SkipEmptyParts);
		if (lines.size() > 1)
		{
			return lines[1].trimmed();
		}
#elif __linux__
		QProcess process;
		process.start("cat /var/lib/dbus/machine-id");
		process.waitForFinished();
		QString output = process.readAllStandardOutput();
		return output.trimmed();
#endif
		return QString();
	}

	// Functions to get hardware identifiers
	//
	QString getCpuInfo()
	{
#ifdef _WIN32
		QProcess process;
		process.start("wmic",
					  QStringList{} << "cpu"
									<< "get"
									<< "name");
		process.waitForFinished(3000);
		QString output = process.readAllStandardOutput();
		QStringList lines = output.split("\n", Qt::SkipEmptyParts);
		if (lines.size() > 1)
		{
			return lines[1].trimmed();
		}
#elif __linux__
		QProcess process;
		process.start("cat /proc/cpuinfo | grep 'model name' | uniq");
		process.waitForFinished();
		QString output = process.readAllStandardOutput();

		int colonIndex = output.indexOf(':');
		if (colonIndex != -1)
		{
			return output.mid(colonIndex + 1).trimmed();
		}
		return output;
#endif
		return QString();
	}


	QStringList getMacAddresses()
	{
		QStringList result;

		foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
		{
			if ((netInterface.flags() & QNetworkInterface::IsLoopBack) == 0 &&
				(netInterface.type() == QNetworkInterface::Ethernet || netInterface.type() == QNetworkInterface::Wifi) &&
				netInterface.hardwareAddress().startsWith("00:") == false)
			{
				result << netInterface.hardwareAddress();
			}
		}

		result.sort();
		return result;
	}
} // namespace


namespace LicenseLib
{
	AppLicenser::AppLicenser(QString publicKeyFileName /* = QString{":/LicenseLib/public_key_inst1.pem"}*/)
	{
		QString errorMessage;
		loadAppLicense(errorMessage, publicKeyFileName);
	}

	bool AppLicenser::guiAppStartValidation(QDate buildDate, QWidget* parent)
	{
#ifndef NDEBUG
		if (AppLicenser::noLicenseCheck() == true)
		{
			return true;
		}
#endif
		QString licenseLoadError;

		AppLicenser appLicenser;
		bool licenseLoaded = appLicenser.loadAppLicense(licenseLoadError);

		if (licenseLoaded == false)
		{
			AppLicenser::showRestrictionMessageBox(parent, LicenseLib::ValidationResult::NotFound, licenseLoadError);
			return false;
		}

		// Validate license restrictions - date
		//
		auto dateValidationResult = appLicenser.validator().validateDate(QDate::currentDate(), buildDate);

		if (AppLicenser::showRestrictionMessageBox(parent, dateValidationResult) == false)
		{
			return false;
		}

		// Validate license restrictions - workplace
		//
		if (auto workplaceValidationResult = appLicenser.validator().validateWorkplace();
			AppLicenser::showRestrictionMessageBox(parent, workplaceValidationResult) == false)
		{
			return false;
		}

		return true;
	}

	bool AppLicenser::loadAppLicense(QString& errorMessage, QString publicKeyFileName /*= QString{":/LicenseLib/public_key_inst1.pem"}*/)
	{
		bool loaded = m_validator.loadFromDir(licensePath(), &errorMessage, publicKeyFileName);

		if (loaded == false)
		{
			qWarning() << "Failed to load license: " << errorMessage;
		}

		return loaded;
	}

	QString AppLicenser::licensePath()
	{
		return QCoreApplication::applicationDirPath() + "/license";
	}

	QString AppLicenser::workplaceId()
	{
		Proto::InternalWorkplaceId proto;

		proto.set_machine_id(QSysInfo::machineUniqueId().toStdString());
		proto.set_hardware_id(getHardwareId().toStdString());
		proto.set_cpu(getCpuInfo().toStdString());
		proto.set_macs(getMacAddresses().join(" ").toStdString());

		proto.set_os(QSysInfo::productType().toStdString());
		proto.set_host(QSysInfo::machineHostName().toStdString());

		QByteArray data;
		data.resize(proto.ByteSizeLong());
		proto.SerializeToArray(data.data(), data.size());

#if 0 // It makes code much longer ((
		char mask = 0x7;
		for (qsizetype i = 0; i < data.size(); i++)
		{
			data[i] = data[i] ^ mask;
			mask = mask >= 100 ? 0x7 : mask + 1;
		}
#endif

		QByteArray compressed = qCompress(data, 9);
		return "ZY" + compressed.toBase64() + "BA";
	}

	bool AppLicenser::showRestrictionMessageBox(QWidget* parent, LicenseLib::ValidationResult validationResult, const QString& extraInfo)
	{
		if (validationResult == ValidationResult::Valid)
		{
			return true;
		}

		QMessageBox mb{parent};
		mb.setIcon(QMessageBox::Critical);
		mb.setDetailedText(AppLicenser::workplaceId());

		switch (validationResult)
		{
		case ValidationResult::Invalid:
			mb.setText(QString{"The license is invalid. %1"}.arg(extraInfo));
			mb.setInformativeText("Please, contact the software vendor to obtain a valid license.");
			break;

		case LicenseLib::ValidationResult::NotFound:
			mb.setText(QString{"Failed to load application license. %1"}.arg(extraInfo));
			mb.setInformativeText(QString{"Please, contact the software vendor to obtain a valid license. If you have a valid license, "
										  "please, check the license directory is %1"}
									  .arg(AppLicenser::licensePath()));
			break;

		case LicenseLib::ValidationResult::Expired:
			mb.setText(QString{"The license has expired. %1"}.arg(extraInfo));
			mb.setInformativeText("Please, contact the software vendor to obtain a valid license.");
			break;

		default:
			mb.setText(QString{"Unknown license error. %1"}.arg(extraInfo));
			mb.setInformativeText("Please, contact the software vendor to obtain a valid license.");
			break;
		}

		foreach (auto button, mb.buttons())
		{
			if (mb.buttonRole(button) == QMessageBox::ActionRole)
			{
				button->click(); // click it to expand the text
				break;
			}
		}

		mb.exec();

		return false;
	}

	const LicenseLib::RpctValidator& AppLicenser::validator() const
	{
		return m_validator;
	}

#ifndef NDEBUG
	bool AppLicenser::noLicenseCheck()
	{
		// Check environment variable U7_NO_LICENSE_CHECK
		//
		QByteArray env = qgetenv("U7_NO_LICENSE_CHECK");
		if (env.isEmpty() == false)
		{
			return true;
		}

		return false;
	}
#endif // NDEBUG

	QUuid AppLicenser::uuid() const
	{
		return m_validator.license().uuid();
	}

	QString AppLicenser::organization() const
	{
		return m_validator.license().organization();
	}

	QString AppLicenser::person() const
	{
		return m_validator.license().firstName() + " " + m_validator.license().lastName();
	}

	QDate AppLicenser::endDate() const
	{
		return m_validator.license().endDate();
	}

} // namespace LicenseLib