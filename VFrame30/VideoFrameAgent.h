#ifndef VIDEOFRAMEAGENT_H
#define VIDEOFRAMEAGENT_H

#include <QObject>

namespace VFrame30
{
	class CVideoFrame;

	class VideoFrameAgent : public QObject, protected QScriptable
	{
		Q_OBJECT
		Q_PROPERTY(QString strID READ strID)

	public:
		explicit VideoFrameAgent(QObject* pParent, const std::shared_ptr<const CVideoFrame>& videoFrame);
		~VideoFrameAgent();

		// Properties
		//
	public:
		QString strID();

		// Functions avaible for scripts
		//
	public slots:

		// Data
		//
	private:
		std::shared_ptr<const CVideoFrame> m_videoFrame;
	};
}

Q_DECLARE_METATYPE(VFrame30::VideoFrameAgent*)





#endif // VIDEOFRAMEAGENT_H
