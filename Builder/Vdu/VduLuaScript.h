#pragma once

namespace Builder
{
	class VduLuaScript
	{
	public:
		static bool checkLuaScript(QString luaScript, QString& outErrorMessage);

		static QByteArray compile(QString luaScript, QString& outErrorMessage);
	};
} // namespace Builder