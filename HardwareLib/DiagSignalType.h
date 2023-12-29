#pragma once

#include "DeviceObject.h"

namespace Hardware
{
	//
	//
	// DiagSignalType
	//
	//
	class DiagSignalType final : public PropertyObject,
								 public Proto::ObjectSerialization<DiagSignalType>,
								 public std::enable_shared_from_this<DiagSignalType>
	{
		Q_OBJECT

	protected:
		DiagSignalType(QObject* parent = nullptr);

	public:
		virtual ~DiagSignalType() = default;

		// Serialization
		//
	protected:
		friend Proto::ObjectSerialization<DiagSignalType>; // for call CreateObject from Proto::ObjectSerialization

	public:
		[[nodiscard]] static std::shared_ptr<DiagSignalType> CreateObject(QObject* parent = nullptr);
		[[nodiscard]] static std::shared_ptr<DiagSignalType> CreateObject(const Proto::Envelope& message);

	protected:
		virtual bool SaveData(Proto::Envelope* message) const final;
		virtual bool LoadData(const Proto::Envelope& message) final;

		// Properties
		//
	public:
		[[nodiscard]] QUuid uuid() const;
		void setUuid(QUuid uuid);

		[[nodiscard]] const QString& signalTypeId() const;
		void setSignalTypeId(const QString& value);

		// Data
		//
	private:
		QUuid m_uuid;
		QString m_siganlTypeId;

	public:
		static const char* mimeType; // = "application/x-radiydiagsignaltype";
	};

} // namespace Hardware