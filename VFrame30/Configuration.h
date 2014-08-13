#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "VFrame30.pb.h"
#include "DebugInstCounter.h"

namespace VFrame30
{
	class CVideoFrame;

	class VFRAME30LIBSHARED_EXPORT Configuration : 
		public QObject,
		public Proto::CVFrameObjectSerialization<Configuration>,
		public DebugInstCounter<Configuration>
	{
		Q_OBJECT

	public:
		Configuration();
		virtual ~Configuration();

		// Serialization
		//
		friend Proto::CVFrameObjectSerialization<Configuration>;

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

		const std::vector<QUuid>& videoFramesIDs() const;
		std::vector<QUuid>* mutableVideoFramesIDs();

		const std::vector<std::shared_ptr<CVideoFrame>>& videoFrames() const;
		std::vector<std::shared_ptr<CVideoFrame>>* mutableVideoFrames();

	private:
		QUuid m_guid;												// Configuration ID
		QString m_strID;											// Configuration string ID's
		QString m_caption;											// The configuration name
		QString m_variables;										// Global scripts varibales
		QString m_globals;											// Global scripts functions
		
		std::vector<QUuid> m_videoFramesIDs;						// The list of VideoFrame's ID's
		std::vector<std::shared_ptr<CVideoFrame>> m_videoFrames;	// The list of VideoFrame's
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

#endif // CONFIGURATION_H
