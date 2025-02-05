#include "VduLuaScript.h"

extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace
{
	int dumpCallBack(lua_State* L, const void* p, size_t size, void* ud)
	{
		QByteArray* bytecode = static_cast<QByteArray*>(ud);
		bytecode->append(static_cast<const char*>(p), size);
		return LUA_OK;
	}
} // namespace

namespace Builder
{
	bool VduLuaScript::checkLuaScript(QString luaScript, QString& outErrorMessage)
	{
		outErrorMessage.clear();

		lua_State* L = luaL_newstate(); // Create a new Lua state
		luaL_openlibs(L);               // Open standard libraries if needed

		// Load the string (parse/compile it) but do not run it
		//
		int status = luaL_loadstring(L, luaScript.toUtf8());
		if (status != LUA_OK)
		{
			const char* err_msg = lua_tostring(L, -1);
			outErrorMessage = QString::fromUtf8(err_msg ? err_msg : "Unknown error");

			lua_pop(L, 1); // Pop error message
		}
		else
		{
			lua_pop(L, 1); // Pop the compiled chunk
		}

		lua_close(L);
		return status == LUA_OK;
	}

	QByteArray VduLuaScript::compile(QString luaScript, QString& outErrorMessage)
	{
		lua_State* L = luaL_newstate();

		QByteArray bytecode;
		QByteArray scriptUtf8 = luaScript.toUtf8();

		try
		{
			if (luaL_loadstring(L, scriptUtf8.constData()) != LUA_OK)
			{
				throw std::runtime_error{lua_tostring(L, -1)};
			}

			if (lua_dump(L, dumpCallBack, &bytecode, 0) != LUA_OK)
			{
				throw std::runtime_error{"Failed to dump bytecode"};
			}
		}
		catch (std::runtime_error& e)
		{
			outErrorMessage = QString::fromUtf8(e.what());
			bytecode.clear();
		}

		lua_close(L);
		return bytecode;
	}
} // namespace Builder