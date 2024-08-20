#include <memory>
#include <QUuid>
#include <QVariant>

class Property; // "../CommonLib/PropertyObject.h"

namespace Proto
{
	class Uuid;
	class wstring;
	class qvariant;
	class Property;

	// Helper serialization functions
	//
	QUuid Read(const Proto::Uuid& message);
	void Write(Proto::Uuid* pMessage, const QUuid& guid);

	// Read/write wstring message
	//
	void Read(const Proto::wstring& message, QString* dst);
	void Write(Proto::wstring* pMessage, const QString& str);

	// Read/write qVariant message
	//
	const QVariant Read(const Proto::qvariant& message);
	void Write(Proto::qvariant* pMessage, const QVariant& value);

	void saveProperty(::Proto::Property* protoProperty, const std::shared_ptr<::Property>& property);
	void saveProperty(::Proto::Property* protoProperty, const ::Property* property);

	bool loadProperty(const ::Proto::Property& protoProperty, const std::shared_ptr<::Property>& property);
	bool loadProperty(const ::Proto::Property& protoProperty, ::Property* property);
}