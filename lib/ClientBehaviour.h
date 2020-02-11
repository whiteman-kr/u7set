#ifndef CLIENTBEHAVIOUR_H
#define CLIENTBEHAVIOUR_H

#include "../lib/PropertyObject.h"
#include "../lib/DbController.h"

//
// ClientBehaviour
//

class ClientBehaviour : public PropertyObject
{
public:
	ClientBehaviour();
	virtual ~ClientBehaviour();

	ClientBehaviour& operator=(const ClientBehaviour& That)
	{
		m_behaviourId = That.m_behaviourId;

		return *this;
	}

public:
	bool isMonitorBehaviour() const;
	bool isTuningClientBehaviour() const;

	QString behaviourId() const;
	void setBehaviourId(const QString& behaviourId);

public:
	virtual void save(QXmlStreamWriter& writer);
	virtual bool load(QXmlStreamReader& reader);

protected:
	virtual void saveToXml(QXmlStreamWriter& writer) = 0;
	virtual bool loadFromXml(QXmlStreamReader& reader) = 0;

private:
	QString m_behaviourId;
};

//
// MonitorBehaviour
//
class MonitorBehaviour : public ClientBehaviour
{
public:
	MonitorBehaviour();

	MonitorBehaviour& operator=(const MonitorBehaviour& That)
	{
		m_signalTagToColor = That.m_signalTagToColor;

		ClientBehaviour::operator= (That);
		return *this;
	}

public:
	QColor signalToTagCriticalColor() const;
	void setSignalToTagCriticalColor(const QColor& color);

	QColor signalToTagAttentionColor() const;
	void setSignalToTagAttentionColor(const QColor& color);

	QColor signalToTagGeneralColor() const;
	void setSignalToTagGeneralColor(const QColor& color);

private:
	virtual void saveToXml(QXmlStreamWriter& writer) override;
	virtual bool loadFromXml(QXmlStreamReader& reader) override;


private:
	QHash<QString, QColor> m_signalTagToColor;

};

//
// TuningClientBehaviour
//
class TuningClientBehaviour : public ClientBehaviour
{
public:
	TuningClientBehaviour();

	TuningClientBehaviour& operator=(const TuningClientBehaviour& That)
	{
		m_tagToColor = That.m_tagToColor;

		ClientBehaviour::operator= (That);
		return *this;
	}

public:
	QColor defaultMismatchBackColor() const;
	void setDefaultMismatchBackColor(const QColor& color);

	QColor defaultMismatchTextColor() const;
	void setDefaultMismatchTextColor(const QColor& color);

	QColor unappliedBackColor() const;
	void setUnappliedBackColor(const QColor& color);

	QColor defaultUnappliedTextColor() const;
	void setDefaultUnappliedTextColor(const QColor& color);

private:
	virtual void saveToXml(QXmlStreamWriter& writer) override;
	virtual bool loadFromXml(QXmlStreamReader& reader) override;

private:
	QHash<QString, QColor> m_tagToColor;
};

//
// ClientBehaviourStorage
//
class ClientBehaviourStorage
{
public:
	ClientBehaviourStorage();

	QString dbFileName() const;

	void add(std::shared_ptr<ClientBehaviour> behavoiur);

	bool remove(int index);

	int count() const;

	std::shared_ptr<ClientBehaviour> get(int index) const;
	std::shared_ptr<ClientBehaviour> get(const QString& id) const;

	void clear();

	const std::vector<std::shared_ptr<ClientBehaviour>>& behavoiurs();

	std::vector<std::shared_ptr<MonitorBehaviour>> monitorBehavoiurs();
	std::vector<std::shared_ptr<TuningClientBehaviour>> tuningClientBehavoiurs();

	void save(QByteArray& data);
	bool load(const QByteArray& data, QString* errorCode);

private:
	std::vector<std::shared_ptr<ClientBehaviour>> m_behavoiurs;
	QString m_fileName = "ClientBehaviour.xml";
};

#endif // CLIENTBEHAVIOUR_H
