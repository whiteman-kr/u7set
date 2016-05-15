#pragma once

#include "../TuningService/TuningService.h"

namespace Tuning
{

	class TuningIPENServiceWorker : public TuningServiceWorker
	{
	private:
		virtual TuningServiceWorker* createInstance() override;
		virtual void requestPreprocessing(Tuning::SocketRequest& sr) override;
		virtual void replyPreprocessing(Tuning::SocketReply& sr) override;

	public:
		TuningIPENServiceWorker(const QString& serviceStrID,
							const QString& cfgServiceIP1,
							const QString& cfgServiceIP2,
							const QString& cfgFileName);
	};

}
