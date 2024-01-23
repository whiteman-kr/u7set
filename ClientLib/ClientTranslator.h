#pragma once

#include <QTranslator>
#include <QApplication>

namespace ClientLib
{
	class ClientTranslator
	{
	public:
		ClientTranslator();
		~ClientTranslator();

		// Languages functions
		//
		void addLanguage(const QString& code, const QString& name);

		QStringList languagesList() const;					// returns language codes
		QString languageName(const QString& code) const;	// returns language name by code

		// Translation files functions
		//
		bool addTranslationFile(const QString& code, const QString& fileName);

		// Language switching
		//
		bool setLanguage(const QString& code, QStringList& failedFiles);

	private:
		std::map<QString, QString> m_languages;			// Key is locale code, value is language name
		std::map<QString, QStringList> m_translations;	// Key is locale code, value is list of translation files

		std::vector<QTranslator*> m_translators;
	};
}
