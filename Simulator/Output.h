#ifndef OUTPUT_H
#define OUTPUT_H
#include <memory>
#include <QTextStream>
#include <QMutex>

namespace Sim
{

	class Output
	{
	protected:
		Output(QTextStream* outputStream, QString outputScope, std::shared_ptr<QMutex> mutex);

	public:
		Output() = delete;
		explicit Output(QTextStream* outputStream, QString outputScope = "");
		Output(const Output&);
		Output(const Output&, QString outputScope);
		virtual ~Output();

	private:
		QTextStream& output();
		QTextStream& output() const;

		QString curentTime() const;

	protected:
		void writeMessage(const QString& text);
		void writeWaning(const QString& text);
		void writeError(const QString& text);

		const QString& outputScope() const;
		void setOutputScope(QString value);

	private:
		QString m_scope;
		QTextStream* m_textStream = nullptr;			// Store input param

		std::shared_ptr<QMutex> m_mutex;

		mutable QTextStream* m_currentStream = nullptr;	// Is m_textStream or m_nullTextStream if m_textStream == null
		FILE* m_nullDevice = nullptr;					// If m_textStream is null, then m_nullTextStream is used instead
		mutable QTextStream m_nullTextStream;
	};

}

#endif // OUTPUT_H
