#ifndef SETTINGS_H
#define SETTINGS_H

namespace TrendLib
{

	class Settings
	{
	public:
		Settings();
		virtual ~Settings();

		// Public methods
		//
	public:
		void write() const;
		void load();

		void writeUserScope() const;
		void loadUserScope();

		void writeSystemScope() const;
		void loadSystemScope();

		// Properties
		//
	public:

		// Data	-- DO NOT FORGET TO ADD NEW MEMBERS TO ASSIGN OPERATOR
		//
	public:

		// MainWindow settings -- user scope
		//
		QPoint m_mainWindowPos;
		QByteArray m_mainWindowGeometry;
		QByteArray m_mainWindowState;		// Toolbars/dock's

		int m_viewType = 0;
		int m_laneCount = 0;

	private:
		mutable QMutex m_mutex;
	};

	extern Settings theSettings;

}

#endif // SETTINGS_H
