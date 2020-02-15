#pragma once

#include <optional>
#include "../lib/PropertyObject.h"
#include "../lib/DbController.h"

//
// ClientBehavior
//

class ClientBehavior : public PropertyObject
{
public:
	ClientBehavior();
	ClientBehavior(const ClientBehavior& src) noexcept;
	virtual ~ClientBehavior();

	ClientBehavior& operator=(const ClientBehavior& src);

public:
	bool isMonitorBehavior() const;
	bool isTuningClientBehavior() const;

	const QString& behaviorId() const;
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
	MonitorBehavior(const MonitorBehavior& src) noexcept;
	virtual ~MonitorBehavior() = default;

	MonitorBehavior& operator=(const MonitorBehavior& That);

public:
	QColor tagCriticalToColor() const;
	void setTagCriticalToColor(const QColor& color);

	QColor tagAttentionToColor() const;
	void setTagAttentionToColor(const QColor& color);

	QColor tagGeneralToColor() const;
	void setTagGeneralToColor(const QColor& color);

	std::optional<QColor> tagToColor(const QString& tag) const;

private:
	virtual void saveToXml(QXmlStreamWriter& writer) override;
	virtual bool loadFromXml(QXmlStreamReader& reader) override;

private:
	QHash<QString, QColor> m_tagToColor;
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
	ClientBehaviorStorage(const ClientBehaviorStorage& src);
	ClientBehaviorStorage(ClientBehaviorStorage&& src) noexcept;

	~ClientBehaviorStorage() = default;

	ClientBehaviorStorage& operator=(const ClientBehaviorStorage& scr);
	ClientBehaviorStorage& operator=(ClientBehaviorStorage&& scr);

public:
	QString dbFileName() const;

	void add(std::shared_ptr<ClientBehavior> behavoiur);
	bool remove(int index);

	int count() const;

	std::shared_ptr<ClientBehavior> get(int index) const;
	std::shared_ptr<ClientBehavior> get(const QString& id) const;

	void clear();

	const std::vector<std::shared_ptr<ClientBehavior>>& behaviors();

	std::vector<std::shared_ptr<MonitorBehavior>> monitorBehaviors();
	std::vector<std::shared_ptr<TuningClientBehavior>> tuningClientBehaviors();

	void save(QByteArray* data) const;
	bool load(const QByteArray& data, QString* errorCode);

private:
	std::vector<std::shared_ptr<ClientBehavior>> m_behaviors;
	QString m_fileName = "ClientBehavior.xml";
};


