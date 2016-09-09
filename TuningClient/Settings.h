#ifndef SETTINGS_H
#define SETTINGS_H


class Settings
{
public:
	Settings();

	void StoreUser();
	void RestoreUser();





public:
	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

	QByteArray m_mainWindowSplitterState;

};

extern Settings theSettings;

#endif // SETTINGS_H
