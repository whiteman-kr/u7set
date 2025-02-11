#pragma once

#include <BuildCompLib/CompareResult.h>
#include <CommonLib/expected.hpp>

#include <memory>


namespace Hardware
{
	class ModuleFirmwareStorage;
}


namespace BuildCompLib
{
	inline constexpr size_t BuildNoOffset = 0x0006; // Offset in bytes of build number in firmware, always in frame 1
	inline constexpr size_t BuildNoSize = 0x0002;   // Size in bytes of build number in firmware, always in frame 1


	class BuildCompImpl final
	{
	public:
		BuildCompImpl();
		BuildCompImpl(const BuildCompImpl&) = delete;
		BuildCompImpl& operator=(const BuildCompImpl&) = delete;

		~BuildCompImpl();

	public:
		tl::expected<bool, QString> setFileLeft(QString fileName);
		tl::expected<bool, QString> setFileRight(QString fileName);

		tl::expected<bool, QString> setFileLeft(const QByteArray& data);
		tl::expected<bool, QString> setFileRight(const QByteArray& data);

		[[nodiscard]] CompareResult compare() const;

	private:
		std::unique_ptr<Hardware::ModuleFirmwareStorage> m_left;
		std::unique_ptr<Hardware::ModuleFirmwareStorage> m_right;
	};
} // namespace BuildCompLib