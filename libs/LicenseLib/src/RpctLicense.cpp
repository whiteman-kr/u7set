
#include <License.pb.h>
#include <LicenseLib/RpctLicense.h>
#include <ProtoCommonHelper.h>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>

#ifdef _WIN32
	#include <openssl/applink.c>
#endif

#include <sstream>

#include <QTemporaryFile>

namespace LicenseLib
{
	RpctLicense::RpctLicense() :
		m_data{std::make_unique<Proto::License>()}
	{
		Proto::Write(m_data->mutable_uuid(), QUuid{});
	}

	RpctLicense::RpctLicense(const RpctLicense& rhs) :
		m_data{std::make_unique<Proto::License>(*rhs.m_data)}
	{
	}

	RpctLicense::RpctLicense(RpctLicense&& rhs) noexcept = default;

	RpctLicense::~RpctLicense() = default;

	RpctLicense& RpctLicense::operator=(const RpctLicense& rhs)
	{
		m_data = std::make_unique<Proto::License>(*rhs.m_data);
		return *this;
	}

	RpctLicense& RpctLicense::operator=(RpctLicense&& rhs) noexcept = default;

	void RpctLicense::clear()
	{
		*this = {};
	}

	bool RpctLicense::isNull() const
	{
		Q_ASSERT(m_data);
		QUuid uuid = Proto::Read(m_data->uuid());
		return uuid.isNull();
	}

	QByteArray RpctLicense::toRawData(QString privateKeyFileName, QString keyPassword, QString* errorMessage) const
	{
		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return {};
		}

		QByteArray rawData;
		QByteArray compressed;
		QByteArray signature;

		if (isNull() == true)
		{
			return compressed;
		}

		Proto::LicenseContainter container;
		*container.mutable_rpct_license() = *m_data;

		rawData.resize(container.ByteSizeLong());

		bool ok = container.SerializeToArray(rawData.data(), rawData.size());
		Q_ASSERT(ok);
		Q_UNUSED(ok);

		compressed = qCompress(rawData);

		try
		{
			OpenSSL_add_all_algorithms();
			ERR_load_crypto_strings();

			FILE* keyFile = fopen(privateKeyFileName.toStdString().c_str(), "r");
			if (keyFile == nullptr)
			{
				throw "Cannot open file " + privateKeyFileName;
			}

			RSA* rsa = PEM_read_RSAPrivateKey(keyFile, nullptr, nullptr, (void*)keyPassword.toStdString().c_str());
			fclose(keyFile);
			if (rsa == nullptr)
			{
				throw false;
			}

			std::array<unsigned char, SHA256_DIGEST_LENGTH> hash{};
			SHA256(reinterpret_cast<const unsigned char*>(compressed.constData()), compressed.size(), hash.data());

			signature.resize(RSA_size(rsa));

			unsigned int sigLen = 0;
			if (RSA_sign(NID_sha256, hash.data(), SHA256_DIGEST_LENGTH, reinterpret_cast<unsigned char*>(signature.data()), &sigLen, rsa) ==
				0)
			{
				throw false;
			}

			Q_ASSERT(sigLen != 0);

			RSA_free(rsa);
		}
		catch (QString err)
		{
			*errorMessage = err;
			return {};
		}
		catch (...)
		{
			char buffer[1024]{};
			ERR_error_string(ERR_get_error(), buffer);
			*errorMessage = buffer;
			return {};
		}

		// Forming result.
		//
		Proto::LicenseBinary resultMessage;
		resultMessage.set_rpct_license(compressed.constData(), compressed.size());
		resultMessage.set_signature(signature.constData(), signature.size());

		QByteArray resultData;
		resultData.resize(resultMessage.ByteSizeLong());
		resultMessage.SerializeToArray(resultData.data(), resultData.size());

		return resultData;
	}

	RpctLicense RpctLicense::fromRawData(const QByteArray& data, QString publicKeyFileName, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return RpctLicense{};
		}

		QFile file{publicKeyFileName};
		if (file.open(QIODevice::ReadOnly) == false)
		{
			*errorMessage = "Failed to open file: " + file.fileName() + ", error: " + file.errorString();
			return {};
		}

		QByteArray publicKeyData = file.readAll();
		file.close();

		return fromRawData(data, publicKeyData, errorMessage);
	}

	RpctLicense RpctLicense::fromRawData(const QByteArray& data, const QByteArray& publicKeyData, QString* errorMessage)
	{
		// Verify signature
		//
		if (errorMessage == nullptr)
		{
			Q_ASSERT(errorMessage);
			return RpctLicense{};
		}

		Proto::LicenseBinary message;
		bool parsed = message.ParseFromArray(data.constData(), data.size());
		if (parsed == false)
		{
			*errorMessage = "Failed to parse license binary";
			return {};
		}

		QByteArray licenseData;
		QByteArray signature;

		licenseData.resize(message.rpct_license().size());
		std::copy(message.rpct_license().begin(), message.rpct_license().end(), licenseData.begin());

		signature.resize(message.signature().size());
		std::copy(message.signature().begin(), message.signature().end(), signature.begin());

		try
		{
			OpenSSL_add_all_algorithms();
			ERR_load_crypto_strings();

			// Convert QString to BIO
			BIO* bio = BIO_new_mem_buf(publicKeyData.constData(), -1);
			if (bio == nullptr)
			{
				throw "Failed to create BIO for public key";
			}

			RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
			BIO_free(bio);
			if (rsa == nullptr)
			{
				throw "Failed to read RSA public key";
			}

			std::array<unsigned char, SHA256_DIGEST_LENGTH> hash{};
			SHA256(reinterpret_cast<const unsigned char*>(licenseData.constData()), licenseData.size(), hash.data());

			int result = RSA_verify(NID_sha256,
									hash.data(),
									SHA256_DIGEST_LENGTH,
									reinterpret_cast<const unsigned char*>(signature.constData()),
									signature.size(),
									rsa);

			RSA_free(rsa);

			if (result == 0)
			{
				throw "Signature verification failed";
			}
		}
		catch (const char* err)
		{
			*errorMessage = err;

			char buffer[1024]{};
			ERR_error_string(ERR_get_error(), buffer);

			*errorMessage += ": ";
			*errorMessage += buffer;

			return {};
		}
		catch (...)
		{
			char buffer[1024]{};
			ERR_error_string(ERR_get_error(), buffer);
			*errorMessage = buffer;
			return {};
		}

		// --
		//
		RpctLicense result;

		QByteArray uncompressed = qUncompress(licenseData);

		Proto::LicenseContainter container;
		parsed = container.ParseFromArray(uncompressed.constData(), uncompressed.size());

		if (parsed == false || container.has_rpct_license() == false)
		{
			*errorMessage = "Failed to parse license";
			return {};
		}

		*result.m_data = container.rpct_license();
		return result;
	}

	QUuid RpctLicense::uuid() const
	{
		Q_ASSERT(m_data);
		return Proto::Read(m_data->uuid());
	}

	void RpctLicense::setUuid(const QUuid& value)
	{
		Q_ASSERT(m_data);
		Proto::Write(m_data->mutable_uuid(), value);
	}

	QString RpctLicense::caption() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->caption());
	}

	void RpctLicense::setCaption(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->set_caption(value.toStdString());
	}

	QString RpctLicense::organization() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->organization());
	}

	void RpctLicense::setOrganization(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->set_organization(value.toStdString());
	}

	QString RpctLicense::firstName() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->first_name());
	}

	void RpctLicense::setFirstName(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->set_first_name(value.toStdString());
	}

	QString RpctLicense::lastName() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->last_name());
	}

	void RpctLicense::setLastName(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->set_last_name(value.toStdString());
	}

	QString RpctLicense::contactInfo() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->contact_info());
	}

	void RpctLicense::setContactInfo(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->set_contact_info(value.toStdString());
	}

	QDate RpctLicense::startDate() const
	{
		Q_ASSERT(m_data);
		return QDate::fromJulianDay(m_data->start_date());
	}

	void RpctLicense::setStartDate(const QDate& value)
	{
		Q_ASSERT(m_data);
		m_data->set_start_date(value.toJulianDay());
	}

	QDate RpctLicense::endDate() const
	{
		Q_ASSERT(m_data);
		return QDate::fromJulianDay(m_data->end_date());
	}

	void RpctLicense::setEndDate(const QDate& value)
	{
		Q_ASSERT(m_data);
		m_data->set_end_date(value.toJulianDay());
	}

	QString RpctLicense::notes() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->notes());
	}

	void RpctLicense::setNotes(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->set_notes(value.toStdString());
	}

	QString RpctLicense::licenserVersion() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->licenser_version());
	}

	void RpctLicense::setLicenserVersion(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->set_licenser_version(value.toStdString());
	}

	QDate RpctLicense::issueDate() const
	{
		Q_ASSERT(m_data);
		return QDate::fromJulianDay(m_data->issue_date());
	}

	void RpctLicense::setIssueDate(const QDate& value)
	{
		Q_ASSERT(m_data);
		m_data->set_issue_date(value.toJulianDay());
	}

	WorkplaceCheckType RpctLicense::workplaceCheckType() const
	{
		Q_ASSERT(m_data);
		return static_cast<WorkplaceCheckType>(m_data->workplace().check_type());
	}

	void RpctLicense::setWorkplaceCheckType(WorkplaceCheckType value)
	{
		Q_ASSERT(m_data);
		m_data->mutable_workplace()->set_check_type(static_cast<int>(value));
	}

	QString RpctLicense::workplaceId() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->workplace().workplace_id());
	}

	void RpctLicense::setWorkplaceId(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->mutable_workplace()->set_workplace_id(value.toStdString());
	}

	bool RpctLicense::allowedModuleConfigurator() const
	{
		Q_ASSERT(m_data);
		return m_data->module_configurator().allowed();
	}

	void RpctLicense::setAllowedModuleConfigurator(bool value)
	{
		Q_ASSERT(m_data);
		m_data->mutable_module_configurator()->set_allowed(value);
	}

	bool RpctLicense::mcLimitModuleUartUuids() const
	{
		Q_ASSERT(m_data);
		return m_data->module_configurator().limit_module_uart_uuids();
	}

	void RpctLicense::setMcLimitModuleUartUuids(bool value)
	{
		Q_ASSERT(m_data);
		m_data->mutable_module_configurator()->set_limit_module_uart_uuids(value);
	}

	QString RpctLicense::mcAllowedModuleUartUuids() const
	{
		Q_ASSERT(m_data);
		return QString::fromStdString(m_data->module_configurator().allowed_module_uart_uuids());
	}

	void RpctLicense::setMcAllowedModuleUartUuids(const QString& value)
	{
		Q_ASSERT(m_data);
		m_data->mutable_module_configurator()->set_allowed_module_uart_uuids(value.toStdString());
	}
} // namespace LicenseLib
