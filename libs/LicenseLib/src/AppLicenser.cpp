#include <License.pb.h>
#include <LicenseLib/AppLicenser.h>

#include <QAbstractButton>
#include <QCoreApplication>
#include <QFile>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>

#ifdef _WIN32
	#define FAST_SMBIOS_SYSTEM_UUID_WIN32

	#ifdef FAST_SMBIOS_SYSTEM_UUID_WIN32
		#include <comdef.h>
		#include <vector>
		#include <wbemidl.h>
		#include <windows.h>
		#pragma comment(lib, "wbemuuid.lib")
	#endif
#endif

namespace
{
	// Functions to get hardware identifiers
	//
	QString getHardwareId()
	{
#ifdef _WIN32
	#ifdef FAST_SMBIOS_SYSTEM_UUID_WIN32
		//
		// AI generated: Claude Sonnet 3.7 Thinking
		//
		UINT size = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
		if (size == 0)
		{
			return {};
		}
		std::vector<BYTE> buffer(size);
		if (GetSystemFirmwareTable('RSMB', 0, buffer.data(), size) != size)
		{
			return {};
		}

		BYTE* p = buffer.data();
		BYTE* end = p + size;

		while (p + 4 < end)
		{
			BYTE type = p[0];
			BYTE length = p[1];

			// Type 1 is the System Information structure which contains UUID at offset 0x08
			if (type == 1 && length >= 0x19)
			{
				BYTE* uuid = p + 0x08; // UUID starts at offset 8 in the Type 1 structure

				// Format UUID in standard format: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
				QString result =
					QString("%1-%2-%3-%4-%5")
						.arg(QString::asprintf("%02X%02X%02X%02X", uuid[3], uuid[2], uuid[1], uuid[0])) // first 4 bytes are little-endian
						.arg(QString::asprintf("%02X%02X", uuid[5], uuid[4]))                           // next 2 bytes are little-endian
						.arg(QString::asprintf("%02X%02X", uuid[7], uuid[6]))                           // next 2 bytes are little-endian
						.arg(QString::asprintf("%02X%02X", uuid[8], uuid[9]))                           // next 2 bytes are big-endian
						.arg(QString::asprintf("%02X%02X%02X%02X%02X%02X",
											   uuid[10],
											   uuid[11],
											   uuid[12],
											   uuid[13],
											   uuid[14],
											   uuid[15])); // last 6 bytes are big-endian

				return result;
			}

			// Move to next structure
			p += length;
			while (p + 1 < end && (p[0] != 0 || p[1] != 0))
				++p;
			if (p + 1 < end)
				p += 2; // Skip double null terminator
		}
		return QString{};
	#else
		QProcess process;
		QString program = "powershell";
		QStringList arguments;
		arguments << "-NoProfile"
				  << "-Command"
				  << "Get-CimInstance -ClassName Win32_ComputerSystemProduct | Select-Object -ExpandProperty UUID";
		process.start(program, arguments);
		process.waitForFinished(5000);
		QString output = process.readAllStandardOutput();
		QStringList lines = output.split("\n", Qt::SkipEmptyParts);
		if (lines.size() >= 1)
		{
			return lines[0].trimmed();
		}
	#endif
#elif __linux__
		QProcess process;
		process.start("cat /var/lib/dbus/machine-id");
		process.waitForFinished();
		QString output = process.readAllStandardOutput();
		return output.trimmed();
#endif
	}

	// Functions to get hardware identifiers
	// AI generated: Claude Sonnet 3.7 Thinking!
	//
	QString getCpuInfo()
	{
#ifdef _WIN32
	#ifdef FAST_SMBIOS_SYSTEM_UUID_WIN32
		QString result;
		bool comInitializedHere = false;

		// Initialize COM
		HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (hres == RPC_E_CHANGED_MODE)
		{
			// COM is already initialized with a different threading model
			// Continue anyway since we can still use COM, just with the existing threading model
		}
		else if (FAILED(hres) && hres != S_FALSE && hres != S_OK)
		{
			// Handle other initialization failures
			return result;
		}
		else
		{
			// We successfully initialized COM, so we need to uninitialize it later
			comInitializedHere = true;
		}

		// Initialize security
		hres = CoInitializeSecurity(nullptr,
									-1,
									nullptr,
									nullptr,
									RPC_C_AUTHN_LEVEL_DEFAULT,
									RPC_C_IMP_LEVEL_IMPERSONATE,
									nullptr,
									EOAC_NONE,
									nullptr);

		// Create WMI locator
		IWbemLocator* pLoc = nullptr;
		hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

		if (FAILED(hres) || !pLoc)
		{
			if (comInitializedHere)
			{
				CoUninitialize();
			}
			return result;
		}

		// Connect to WMI
		IWbemServices* pSvc = nullptr;
		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, &pSvc);

		if (FAILED(hres) || !pSvc)
		{
			pLoc->Release();
			if (comInitializedHere)
			{
				CoUninitialize();
			}
			return result;
		}

		// Set security levels
		hres = CoSetProxyBlanket(pSvc,
								 RPC_C_AUTHN_WINNT,
								 RPC_C_AUTHZ_NONE,
								 nullptr,
								 RPC_C_AUTHN_LEVEL_CALL,
								 RPC_C_IMP_LEVEL_IMPERSONATE,
								 nullptr,
								 EOAC_NONE);

		// Execute WQL query
		IEnumWbemClassObject* pEnumerator = nullptr;
		hres = pSvc->ExecQuery(bstr_t("WQL"),
							   bstr_t("SELECT Name FROM Win32_Processor"),
							   WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
							   nullptr,
							   &pEnumerator);

		if (FAILED(hres) || !pEnumerator)
		{
			pSvc->Release();
			pLoc->Release();
			if (comInitializedHere)
			{
				CoUninitialize();
			}
			return result;
		}

		// Get the data from the query
		IWbemClassObject* pclsObj = nullptr;
		ULONG uReturn = 0;

		while (pEnumerator)
		{
			hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

			if (uReturn == 0)
				break;

			VARIANT vtProp;
			VariantInit(&vtProp);

			// Get the value of the Name property
			hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
			if (SUCCEEDED(hres))
			{
				// Convert BSTR to QString
				if (vtProp.vt == VT_BSTR)
				{
					std::wstring wstr(vtProp.bstrVal);
					result = QString::fromStdWString(wstr).trimmed();
				}
				VariantClear(&vtProp);
			}

			pclsObj->Release();

			// We only need the first processor in most cases
			break;
		}

		// Cleanup
		pEnumerator->Release();
		pSvc->Release();
		pLoc->Release();
		if (comInitializedHere)
		{
			CoUninitialize();
		}

		return result;
	#else
		QProcess process;
		QString program = "powershell";
		QStringList arguments;
		arguments << "-NoProfile"
				  << "-Command"
				  << "Get-CimInstance Win32_Processor | Select-Object -ExpandProperty Name";
		process.start(program, arguments);
		process.waitForFinished(5000);
		QString output = process.readAllStandardOutput();
		QStringList lines = output.split("\n", Qt::SkipEmptyParts);
		if (lines.size() >= 1)
		{
			return lines[0].trimmed();
		}
		return QString();
	#endif
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
#else
		// For unsupported platforms
		return QString();
#endif
	}

	QString getMotherboardInfo()
	{
		//
		// AI generated: Claude Sonnet 3.7 Thinking
		//
#ifdef _WIN32
		QString result;
		HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (hres == RPC_E_CHANGED_MODE)
		{
			// COM already initialized with different threading model, continue anyway
		}
		else if (FAILED(hres) && hres != S_FALSE && hres != S_OK)
		{
			return result;
		}

		// Initialize WMI connection (similar to your CPU info code)
		IWbemLocator* pLoc = nullptr;
		hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
		if (FAILED(hres) || !pLoc)
		{
			CoUninitialize();
			return result;
		}

		// Connect to WMI
		IWbemServices* pSvc = nullptr;
		hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, &pSvc);
		if (FAILED(hres) || !pSvc)
		{
			pLoc->Release();
			CoUninitialize();
			return result;
		}

		// Set security
		hres = CoSetProxyBlanket(pSvc,
								 RPC_C_AUTHN_WINNT,
								 RPC_C_AUTHZ_NONE,
								 nullptr,
								 RPC_C_AUTHN_LEVEL_CALL,
								 RPC_C_IMP_LEVEL_IMPERSONATE,
								 nullptr,
								 EOAC_NONE);

		// Query for motherboard info
		IEnumWbemClassObject* pEnumerator = nullptr;
		hres = pSvc->ExecQuery(bstr_t("WQL"),
							   bstr_t("SELECT Manufacturer, Product, SerialNumber FROM Win32_BaseBoard"),
							   WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
							   nullptr,
							   &pEnumerator);

		if (FAILED(hres) || !pEnumerator)
		{
			pSvc->Release();
			pLoc->Release();
			CoUninitialize();
			return result;
		}

		// Process query results
		IWbemClassObject* pclsObj = nullptr;
		ULONG uReturn = 0;
		QString manufacturer, product, serialNumber;

		if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == S_OK && uReturn != 0)
		{
			VARIANT vtProp;
			VariantInit(&vtProp);

			// Get manufacturer
			if (SUCCEEDED(pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR)
			{
				manufacturer = QString::fromStdWString(std::wstring(vtProp.bstrVal)).trimmed();
				VariantClear(&vtProp);
			}

			// Get product name
			if (SUCCEEDED(pclsObj->Get(L"Product", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR)
			{
				product = QString::fromStdWString(std::wstring(vtProp.bstrVal)).trimmed();
				VariantClear(&vtProp);
			}

			// Get serial number
			if (SUCCEEDED(pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR)
			{
				serialNumber = QString::fromStdWString(std::wstring(vtProp.bstrVal)).trimmed();
				VariantClear(&vtProp);
			}

			pclsObj->Release();
		}

		// Cleanup
		pEnumerator->Release();
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();

		// Combine information
		if (!manufacturer.isEmpty() || !product.isEmpty() || !serialNumber.isEmpty())
		{
			result = QString("%1|%2|%3").arg(manufacturer, product, serialNumber);
		}

		return result;

#elif __linux__
		// Linux implementation
		QString manufacturer, product, serialNumber;

		// Try reading from /sys/class/dmi/id/ (doesn't require root)
		QFile manufacturerFile("/sys/class/dmi/id/board_vendor");
		if (manufacturerFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			manufacturer = QString::fromUtf8(manufacturerFile.readAll()).trimmed();
			manufacturerFile.close();
		}

		QFile productFile("/sys/class/dmi/id/board_name");
		if (productFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			product = QString::fromUtf8(productFile.readAll()).trimmed();
			productFile.close();
		}

		QFile serialFile("/sys/class/dmi/id/board_serial");
		if (serialFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			serialNumber = QString::fromUtf8(serialFile.readAll()).trimmed();
			serialFile.close();
		}

		// If the above didn't work (maybe due to permissions), try dmidecode as fallback
		if (manufacturer.isEmpty() || product.isEmpty() || serialNumber.isEmpty())
		{
			// Try to run dmidecode with sudo (might require password - not ideal)
			QProcess process;
			process.start("sudo dmidecode -t 2"); // Type 2 is for motherboard
			process.waitForFinished(3000);

			QString output = process.readAllStandardOutput();

			// Parse the output
			if (!output.isEmpty())
			{
				QRegularExpression manufRegex("Manufacturer:\\s*(.+)");
				QRegularExpression productRegex("Product Name:\\s*(.+)");
				QRegularExpression serialRegex("Serial Number:\\s*(.+)");

				QRegularExpressionMatch manufMatch = manufRegex.match(output);
				QRegularExpressionMatch productMatch = productRegex.match(output);
				QRegularExpressionMatch serialMatch = serialRegex.match(output);

				if (manufacturer.isEmpty() && manufMatch.hasMatch())
					manufacturer = manufMatch.captured(1).trimmed();

				if (product.isEmpty() && productMatch.hasMatch())
					product = productMatch.captured(1).trimmed();

				if (serialNumber.isEmpty() && serialMatch.hasMatch())
					serialNumber = serialMatch.captured(1).trimmed();
			}
		}

		// If we still don't have data, try lshw as another fallback
		if (manufacturer.isEmpty() || product.isEmpty() || serialNumber.isEmpty())
		{
			QProcess process;
			process.start("lshw -C bus");
			process.waitForFinished(3000);

			QString output = process.readAllStandardOutput();

			if (!output.isEmpty())
			{
				QRegularExpression vendorRegex("vendor:\\s*(.+)");
				QRegularExpression productRegex("product:\\s*(.+)");
				QRegularExpression serialRegex("serial:\\s*(.+)");

				QRegularExpressionMatch vendorMatch = vendorRegex.match(output);
				QRegularExpressionMatch productMatch = productRegex.match(output);
				QRegularExpressionMatch serialMatch = serialRegex.match(output);

				if (manufacturer.isEmpty() && vendorMatch.hasMatch())
					manufacturer = vendorMatch.captured(1).trimmed();

				if (product.isEmpty() && productMatch.hasMatch())
					product = productMatch.captured(1).trimmed();

				if (serialNumber.isEmpty() && serialMatch.hasMatch())
					serialNumber = serialMatch.captured(1).trimmed();
			}
		}

		// Combine information
		if (!manufacturer.isEmpty() || !product.isEmpty() || !serialNumber.isEmpty())
		{
			return QString("%1|%2|%3").arg(manufacturer, product, serialNumber);
		}
	
		return QString();
#endif
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

		// Check if license is revoked
		//
		if (auto revocationValidationResult = appLicenser.validator().isRevoked() ? ValidationResult::Revoked : ValidationResult::Valid;
			AppLicenser::showRestrictionMessageBox(parent, revocationValidationResult) == false)
		{
			return false;
		}

		// Check if license is blacklisted
		//
		if (auto blacklistValidationResult = appLicenser.validator().isBlacklisted();
			AppLicenser::showRestrictionMessageBox(parent, blacklistValidationResult) == false)
		{
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
		// If env variable U7_LICENSE_PATH is set, use it, else use "application directory"/license
		//
		QByteArray env = qgetenv("U7_LICENSE_PATH");
		if (env.isEmpty() == false)
		{
			return QString::fromUtf8(env);
		}

		return QCoreApplication::applicationDirPath() + "/license";
	}

	QString AppLicenser::workplaceId()
	{
		return workplaceIdV1();
	}

	QString AppLicenser::workplaceIdV0()
	{
		QString hwId = getHardwareId();
		QString cpuInfo = getCpuInfo();

		Proto::InternalWorkplaceId proto;

		proto.set_version(0); // !!!

		proto.set_machine_id(QSysInfo::machineUniqueId().toStdString());
		proto.set_hardware_id(hwId.toStdString());
		proto.set_cpu(cpuInfo.toStdString());
		proto.set_macs(getMacAddresses().join(" ").toStdString());

		proto.set_os(QSysInfo::productType().toStdString());
		proto.set_host(QSysInfo::machineHostName().toStdString());

		QByteArray data;
		data.resize(proto.ByteSizeLong());
		proto.SerializeToArray(data.data(), data.size());

		QByteArray compressed = qCompress(data, 9);
		return "ZY" + compressed.toBase64() + "BA";
	}

	QString AppLicenser::workplaceIdV1()
	{
		std::string hwId = getHardwareId().toStdString();
		std::string cpuInfo = getCpuInfo().toStdString();
		std::string motherboardInfo = getMotherboardInfo().toStdString();

		Proto::InternalWorkplaceId proto;

		proto.set_version(1); // !!!

		proto.set_machine_id(QSysInfo::machineUniqueId().toStdString());
		proto.set_hardware_id(std::move(hwId));
		proto.set_cpu(std::move(cpuInfo));
		proto.set_macs(getMacAddresses().join(" ").toStdString());
		proto.set_motherboard_info_hash(::ClassNameHashCode(motherboardInfo));

		QByteArray data;
		data.resize(proto.ByteSizeLong());
		proto.SerializeToArray(data.data(), data.size());

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

		case LicenseLib::ValidationResult::Revoked:
			mb.setText(QString{"The license has been revoked. %1"}.arg(extraInfo));
			mb.setInformativeText("Please, contact the software vendor to obtain a new license.");
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
		// Decision: always perform license check in debug builds, to prevent unlicensed use cases.
		//
		return false;

		//// Check environment variable U7_NO_LICENSE_CHECK
		////
		// QByteArray env = qgetenv("U7_NO_LICENSE_CHECK");
		// if (env.isEmpty() == false)
		//{
		//	return true;
		// }

		// return false;
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