#ifndef SESSION_H
#define SESSION_H

namespace VFrame30
{

	class Session : public PropertyObject
	{
		Q_OBJECT

	public:
		Session();
		Session(QString project, QString username, QString host);

		Session& operator=(const Session&);

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
		struct Data
		{
			// Separate struct to make it copyable
			//
			QString project;
			QString username;
			QString host;
		} m_data;
	};

}

#endif // SESSION_H
