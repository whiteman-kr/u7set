#pragma once

#include "../include/ProtoSerialization.h"
#include "../include/DebugInstCounter.h"

namespace VFrame30
{
	class Schema;

	class VFRAME30LIBSHARED_EXPORT Configuration : 
		public QObject,
		public Proto::ObjectSerialization<Configuration>,
		public DebugInstCounter<Configuration>
	{
		Q_OBJECT

	public:
		Configuration();
		virtual ~Configuration();

		// Serialization
		//
		friend Proto::ObjectSerialization<Configuration>;

	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

	private:
		// Use this func ONLY for serialization
		//
		static Configuration* CreateObject(const Proto::Envelope& message);

		// Properties and Datas
		//
	public:
		const QUuid& guid() const;
		void setGuid(const QUuid& guid);

		const QString& strID() const;
		void setStrID(const QString& strID);

		const QString& caption() const;
		void setCaption(const QString& caption);

		const QString& variables() const;
		void setVariables(const QString& variables);

		const QString& globals() const;
		void setGlobals(const QString& globals);

		const std::vector<QUuid>& schemesIDs() const;
		std::vector<QUuid>* mutableSchemesIDs();

		const std::vector<std::shared_ptr<Schema>>& schemes() const;
		std::vector<std::shared_ptr<Schema>>* mutableSchemes();

	private:
		QUuid m_guid;												// Configuration ID
		QString m_strID;											// Configuration string ID's
		QString m_caption;											// The configuration name
		QString m_variables;										// Global scripts varibales
		QString m_globals;											// Global scripts functions
		
		std::vector<QUuid> m_schemeIds;								// The list of Scheme's ID's
		std::vector<std::shared_ptr<Schema>> m_schemes;				// The list of Scheme's
	};


	class VFRAME30LIBSHARED_EXPORT ConfigurationSharedPtr
	{
	public:
		ConfigurationSharedPtr(const std::shared_ptr<Configuration>& sp) : m_sp(sp)
		{
		}

		std::shared_ptr<Configuration> get()
		{
			return m_sp;
		}
		
	private:
		std::shared_ptr<Configuration> m_sp;
	};
}

