#pragma once

#include <optional>
#include "../CommonLib/PropertyObject.h"


//
// ClientBehavior
//
class ClientBehavior : public PropertyObject
{
public:
	ClientBehavior();
	ClientBehavior(const ClientBehavior& src) noexcept;
	ClientBehavior(ClientBehavior&& src) noexcept;
	virtual ~ClientBehavior();

	ClientBehavior& operator=(const ClientBehavior& src);
	ClientBehavior& operator=(ClientBehavior&& src);

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

	MonitorBehavior& operator=(const MonitorBehavior& src);
	MonitorBehavior& operator=(MonitorBehavior&& src);

public:

	QStringList tags() const;

	void setTag(int index, const QString& tag);

	void insertTagToColors(int index, const QString& tag, std::pair<QRgb, QRgb> colors);	// Inserts a tag, if tag already exists, does not modify the color
	bool removeTagToColors(int index);
	bool moveTagToColors(int index, int step);

	std::optional<std::pair<QRgb, QRgb>> tagToColors(const QString& tag) const;
	void setTagToColors(const QString& tag, std::pair<QRgb, QRgb> colors);	// Sets tag color, if tag does not exist, adds it

	std::optional<std::pair<QRgb, QRgb>> tagToColors(const std::set<QString>& tags) const;	// Return the most periority color
	std::optional<std::pair<QRgb, QRgb>> tagToColors(const QStringList& tags) const;		// Return the most periority color

private:
	void addBaseTagToColors();

	virtual void saveToXml(QXmlStreamWriter& writer) override;
	virtual bool loadFromXml(QXmlStreamReader& reader) override;


public:
	static const QString criticalTag;
	static const QString attentionTag;
	static const QString generalTag;
	static const QString nonValidityTag;
	static const QString simulatedTag;
	static const QString blockedTag;
	static const QString mismatchTag;
	static const QString outOfLimitsTag;

private:
	struct TagToColorsType
	{
		QString tag;
		std::pair<QRgb, QRgb> colors;
	};

	std::vector<TagToColorsType> m_tagToColors;		// The lower position - the higher priority of the tag
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


