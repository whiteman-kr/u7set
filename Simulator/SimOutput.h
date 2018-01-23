#ifndef OUTPUT_H
#define OUTPUT_H
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(u7sim)

namespace Sim
{
	class Output
	{
	public:
		explicit Output(QString outputScope = "");
		virtual ~Output();

	protected:
		void writeMessage(const QString& text);
		void writeWaning(const QString& text);
		void writeError(const QString& text);

		const QString& outputScope() const;
		void setOutputScope(QString value);

	private:
		QString m_scope;
	};

}

#endif // OUTPUT_H
