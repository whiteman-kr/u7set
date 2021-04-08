#ifndef FRAMEBASE_H
#define FRAMEBASE_H

#include <QMutex>

#include "../../OnlineLib/DataProtocols.h"

// ==============================================================================================

namespace PS
{
	class FrameData : public QObject
	{
		Q_OBJECT

	public:

		FrameData();
		FrameData(const FrameData& from);
		virtual ~FrameData() override;

	public:

		void clear();

		Rup::Data& data() { return m_data; }
		FrameData& operator=(const FrameData& from);

	private:

		mutable QMutex m_frameMutex;
		Rup::Data m_data;
	};
}

// ==============================================================================================

class FrameBase : public QObject
{
	Q_OBJECT

public:

	explicit FrameBase(QObject *parent = nullptr);
	virtual ~FrameBase() override;

public:

	void clear();
	int count() const;

	bool setFrameCount(int count);

	PS::FrameData frameData(int index) const;
	PS::FrameData* frameDataPtr(int index);
	void setFrameData(int index, const PS::FrameData& frameData);

	FrameBase& operator=(const FrameBase& from);

private:

	mutable QMutex m_frameMutex;
	std::vector<PS::FrameData> m_frameList;

};

// ==============================================================================================

#endif // FRAMEBASE_H
