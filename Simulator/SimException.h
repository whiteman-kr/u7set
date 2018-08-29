#ifndef SIMEXCEPTION_H
#define SIMEXCEPTION_H

#include <QException>

namespace Sim
{
	class SimException : public QException
	{
	public:
		static void raise(QString message, QString where = QString(), QString scope = QString());

	protected:
		virtual void raise() const override;
	public:
		virtual SimException* clone() const override;

	public:
		const QString& message() const;
		const QString& where() const;
		const QString& scope() const;

	private:
		QString m_message;
		QString m_where;
		QString m_scope;
	};
}

#endif // SIMEXCEPTION_H
