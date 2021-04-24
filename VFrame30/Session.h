#ifndef SESSION_H
#define SESSION_H
#include "../CommonLib/PropertyObject.h"

namespace VFrame30
{

	class Session : public PropertyObject
	{
		Q_OBJECT

	public:
		Session();
		Session(QString project, QString username, QString host);

		Session& operator= (const Session&);

	public:
		QString project() const;
		void setProject(QString value);

		QString username() const;
		void setUsername(QString value);

		QString host() const;
		void setHost(QString value);

		QString date() const;
		QString time() const;

	private:
		// !!! Pay Attention to operator = !!!
		//
		QString m_project;
		QString m_username;
		QString m_host;
		// !!! Pay Attention to operator = !!!
		//
	};

}

#endif // SESSION_H
