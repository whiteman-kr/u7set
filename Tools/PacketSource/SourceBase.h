#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include "BuildOpt.h"

#include "SourceWorker.h"
#include "SignalBase.h"
#include "FrameBase.h"

#include "../../lib/HostAddressPort.h"

// ==============================================================================================

namespace PS
{
	//
	//

	#define PS_SUPPORT_VERSION		5

	const int SUPPORT_VERSION		= PS_SUPPORT_VERSION;		// last known version of Rup::VERSION

	const int SIM_FRAME_VERSION		= 1;						// last version of SimFrame

	const int UDP_PORT				= 10000;

	const int SEND_TIMEOUT			= 5;						// 5 ms

	//
	//

	struct SourceInfo
	{
	public:

		SourceInfo();

		void				clear();

		int					index = -1;

		QString				caption;
		QString				equipmentID;

		int					moduleType = 0;
		QString				subSystem;
		int					frameCount = 0;
		quint32				dataID = 0;

		HostAddressPort		lmAddress;				// get from xml file
		HostAddressPort		serverAddress;			// get from options for send UDP (this is AppDataReceivingIP of AppDataSrv)

		int					signalCount = 0;
	};

	//
	//

	class Source : public QObject
	{
		Q_OBJECT

	public:

		Source();
		Source(const Source& from);
		Source(const PS::SourceInfo& si);
		virtual ~Source();

	private:

		mutable QMutex		m_sourceMutex;

		QThread*			m_pThread = nullptr;
		SourceWorker*		m_pWorker = nullptr;

		PS::SourceInfo		m_si;
		QStringList			m_associatedSignalList;
		QVector<PS::Signal>	m_signalList;
		FrameBase			m_frameBase;

	public:

		void					clear();

		//

		PS::SourceInfo&			info() { return m_si; }
		QStringList&			associatedSignalList()  { return m_associatedSignalList; }
		QVector<PS::Signal>&	signalList()  { return m_signalList; }
		FrameBase&				frameBase() { return m_frameBase; }

		//
		//
		bool					run();
		bool					stop();

		bool					isRunning();
		int						sentFrames();

		//
		//
		void					loadSignals(const SignalBase& signalBase);
		void					initSignalsState();

		bool					createWorker();
		void					deleteWorker();

		//
		//
		Source&					operator=(const Source& from);

	signals:

	public slots:

	};
}

// ==============================================================================================

class SourceBase : public QObject
{
	Q_OBJECT

public:

	explicit SourceBase(QObject *parent = nullptr);
	virtual ~SourceBase();

private:

	mutable QMutex			m_sourceMutex;
	QVector<PS::Source>		m_sourceList;

public:

	void					clear();
	int						count() const;

	int						readFromFile(const BuildInfo& buildInfo);

	int						append(const PS::Source &source);
	void					remove(int index);

	PS::Source				source(int index) const;
	PS::Source*				sourcePtr(int index);
	PS::Source*				sourcePtr(const QString& equipmentID);

	void					setSource(int index, const PS::Source& source);

	SourceBase&				operator=(const SourceBase& from);

	// run stop send udp trhread
	//
	void					runSourece(int index);
	void					stopSourece(int index);

	void					runAllSoureces();
	void					stopAllSoureces();

	// source id for run
	//
	void					runSources(const QStringList& sourceIDList);

signals:

	void					sourcesLoaded();

public slots:

};

// ==============================================================================================

#endif // SOURCEBASE_H
