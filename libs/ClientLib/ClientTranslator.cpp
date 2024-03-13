#include "./include/ClientLib/ClientTranslator.h"


namespace ClientLib
{
	ClientTranslator::ClientTranslator()
	{
	}

	ClientTranslator::~ClientTranslator()
	{
		for (QTranslator* translator : m_translators)
		{
			delete translator;
		}
		m_translators.clear();
	}

	void ClientTranslator::addLanguage(const QString& code, const QString& name)
	{
		m_languages[code] = name;
	}

	QStringList ClientTranslator::languagesList() const
	{
		QStringList result;
		for (const auto& it : m_languages)
		{
			result.push_back(it.first);
		}
		return result;
	}

	QString ClientTranslator::languageName(const QString& code) const
	{
		const auto it = m_languages.find(code);
		if (it == m_languages.end())
		{
			return QString();
		}
		return it->second;
	}

	bool ClientTranslator::addTranslationFile(const QString& code, const QString& fileName)
	{
		if (m_languages.find(code) == m_languages.end())
		{
			Q_ASSERT(false);
			return false;
		}

		QStringList& translations = m_translations[code];
		translations.push_back(fileName);

		return true;
	}

	bool ClientTranslator::setLanguage(const QString& code, QStringList& failedFiles)
	{
		failedFiles.clear();

		if (m_languages.find(code) == m_languages.end())
		{
			Q_ASSERT(false);
			return false;
		}

		QLocale loc = QLocale(code);
		QLocale::setDefault(loc);

		bool ok = true;

		// Remove previous translators
		//
		for (QTranslator* translator : m_translators)
		{
			ok &= qApp->removeTranslator(translator);
		}

		for (QTranslator* translator : m_translators)
		{
			delete translator;
		}
		m_translators.clear();

		// Create and set new translators
		//
		const QStringList& translations = m_translations[code];

		for (const QString& fileName : translations)
		{
			QTranslator* translator = new QTranslator();
			m_translators.push_back(translator);

			if(translator->load(fileName) == true)
			{
				ok &= qApp->installTranslator(translator);
			}
			else
			{
				failedFiles.push_back(fileName);
				ok = false;
			}
		}

		return ok;
	}
}
