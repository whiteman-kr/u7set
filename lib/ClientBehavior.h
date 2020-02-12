#ifndef CLIENTBEHAVIOR_H
#define CLIENTBEHAVIOR_H

#include "../lib/PropertyObject.h"
#include "../lib/DbController.h"

//
// ClientBehavior
//

class ClientBehavior : public PropertyObject
{
public:
	ClientBehavior();
	virtual ~ClientBehavior();

	ClientBehavior& operator=(const ClientBehavior& That)
	{
		m_behaviorId = That.m_behaviorId;

		return *this;
	}

public:
	bool isMonitorBehavior() const;
	bool isTuningClientBehavior() const;

	QString behaviorId() const;
	void setBehaviorId(const QString& behaviorId);

public:
	virtual void save(QXmlStreamWriter& writer);
	virtual bool load(QXmlStreamReader& reader);

protected:
	virtual void saveToXml(QXmlStreamWriter& writer) = 0;
	virtual bool loadFromXml(QXmlStreamReader& reader) = 0;

private:
	QString m_behaviorId;
};

//
// MonitorBehavior
//
class MonitorBehavior : public ClientBehavior
{
public:
	MonitorBehavior();

	MonitorBehavior& operator=(const MonitorBehavior& That)
	{
		m_signalTagToColor = That.m_signalTagToColor;

		ClientBehavior::operator= (That);
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
// TuningClientBehavior
//
class TuningClientBehavior : public ClientBehavior
{
public:
	TuningClientBehavior();

	TuningClientBehavior& operator=(const TuningClientBehavior& That)
	{
		m_tagToColor = That.m_tagToColor;

		ClientBehavior::operator= (That);
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
// ClientBehaviorStorage
//
class ClientBehaviorStorage
{
public:
	ClientBehaviorStorage();

	QString dbFileName() const;

	void add(std::shared_ptr<ClientBehavior> behavoiur);

	bool remove(int index);

	int count() const;

	std::shared_ptr<ClientBehavior> get(int index) const;
	std::shared_ptr<ClientBehavior> get(const QString& id) const;

	void clear();

	const std::vector<std::shared_ptr<ClientBehavior>>& behavoiurs();

	std::vector<std::shared_ptr<MonitorBehavior>> monitorBehavoiurs();
	std::vector<std::shared_ptr<TuningClientBehavior>> tuningClientBehavoiurs();

	void save(QByteArray& data);
	bool load(const QByteArray& data, QString* errorCode);

private:
	std::vector<std::shared_ptr<ClientBehavior>> m_behavoiurs;
	QString m_fileName = "ClientBehavior.xml";
};

#endif // CLIENTBEHAVIOR_H
