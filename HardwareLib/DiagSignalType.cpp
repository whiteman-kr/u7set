#include "DiagSignalType.h"

namespace Hardware
{
	//
	// DiagSignalType
	//
	const char* DiagSignalType::mimeType = "application/x-radiydiagsignaltype";

	DiagSignalType::DiagSignalType(QObject* parent) :
		PropertyObject(parent)
	{
		addProperty<QUuid, DiagSignalType, &DiagSignalType::uuid, &DiagSignalType::setUuid>(PropertyNames::uuid, {}, true)
			->setReadOnly(true)
			.setExpert(true);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::diagSignalTypeId, PropertyNames::categoryDiagSignal, true, DiagSignalType::signalTypeId, DiagSignalType::setSignalTypeId);

		return;
	}

	std::shared_ptr<DiagSignalType> DiagSignalType::CreateObject(QObject* parent)
	{
		return std::shared_ptr<DiagSignalType>(new DiagSignalType{parent}); // cannot use make_shared as constructor is proteced ((
	}

	std::shared_ptr<DiagSignalType> DiagSignalType::CreateObject(const Proto::Envelope& message)
	{
		std::shared_ptr<DiagSignalType> dst = DiagSignalType::CreateObject();

		bool ok = dst->LoadData(message);

		return ok ? dst : std::shared_ptr<DiagSignalType>();
	}

	bool DiagSignalType::SaveData(Proto::Envelope* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = ::ClassNameHashCode(className);

		message->set_classnamehash(classnamehash);

		Proto::DiagSignalType* m = message->mutable_diagsignaltype();

		Proto::Write(m->mutable_uuid(), m_uuid);
		m->set_signaltypeid(m_siganlTypeId.toStdString());

		return true;
	}

	bool DiagSignalType::LoadData(const Proto::Envelope& message)
	{
		if (message.has_diagsignaltype() == false)
		{
			Q_ASSERT(message.has_diagsignaltype());
			return false;
		}

		const Proto::DiagSignalType& m = message.diagsignaltype();

		m_uuid = Proto::Read(m.uuid());
		Q_ASSERT(m_uuid.isNull() == false);

		m_siganlTypeId = QString::fromStdString(m.signaltypeid());

		return true;
	}

	QUuid DiagSignalType::uuid() const
	{
		return m_uuid;
	}

	void DiagSignalType::setUuid(QUuid uuid)
	{
		m_uuid = uuid;
	}

	const QString& DiagSignalType::signalTypeId() const
	{
		return m_siganlTypeId;
	}

	void DiagSignalType::setSignalTypeId(const QString& value)
	{
		m_siganlTypeId = value;
	}

} // namespace Hardware