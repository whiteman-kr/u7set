#include "BuildCompImpl.h"

#include <HardwareLib/ModuleFirmware.h>

#include <QFile>

namespace
{
	bool compareFirmware(const Hardware::ModuleFirmware& left, const Hardware::ModuleFirmware& right)
	{
		if (left.subsysId() != right.subsysId() || left.ssKey() != right.ssKey() || left.lmDescriptionFile() != right.lmDescriptionFile() ||
			left.lmDescriptionNumber() != right.lmDescriptionNumber())
		{
			return false;
		}

		auto leftUarts = left.uartList();
		auto rightUarts = right.uartList();

		std::sort(leftUarts.begin(), leftUarts.end());
		std::sort(rightUarts.begin(), rightUarts.end());

		if (leftUarts != rightUarts)
		{
			return false;
		}

		for (const auto& [uartId, uartName] : leftUarts)
		{
			bool leftDataOk = false;
			const Hardware::ModuleFirmwareData& leftData = left.firmwareData(uartId, &leftDataOk);

			bool rightDataOk = false;
			const Hardware::ModuleFirmwareData& rightData = right.firmwareData(uartId, &rightDataOk);

			if (leftDataOk == false || rightDataOk == false)
			{
				return false;
			}

			if (leftData.eepromFramePayloadSize != rightData.eepromFramePayloadSize ||
				leftData.eepromFrameSize != rightData.eepromFrameSize || leftData.maxFrameIndex != rightData.maxFrameIndex ||
				leftData.uartId != rightData.uartId || leftData.uartType != rightData.uartType)
			{
				return false;
			}

			auto leftFrames = leftData.frames;
			auto rightFrames = rightData.frames;

			if (leftFrames.size() != rightFrames.size())
			{
				return false;
			}

			for (size_t frameIndex = 0; frameIndex < leftFrames.size(); frameIndex++)
			{
				if (frameIndex == 1)
				{
					// Mask BuildNo and checksum in the end (8 bytes)
					//
					std::vector<quint8> leftFrame = leftFrames[frameIndex];
					std::vector<quint8> rightFrame = rightFrames[frameIndex];

					leftFrame[6] = 0;
					leftFrame[7] = 0;

					assert(leftFrame.size() > 8);
					leftFrame[leftFrame.size() - 8] = 0;
					leftFrame[leftFrame.size() - 7] = 0;
					leftFrame[leftFrame.size() - 6] = 0;
					leftFrame[leftFrame.size() - 5] = 0;
					leftFrame[leftFrame.size() - 4] = 0;
					leftFrame[leftFrame.size() - 3] = 0;
					leftFrame[leftFrame.size() - 2] = 0;
					leftFrame[leftFrame.size() - 1] = 0;

					rightFrame[6] = 0;
					rightFrame[7] = 0;

					assert(rightFrame.size() > 8);
					rightFrame[rightFrame.size() - 8] = 0;
					rightFrame[rightFrame.size() - 7] = 0;
					rightFrame[rightFrame.size() - 6] = 0;
					rightFrame[rightFrame.size() - 5] = 0;
					rightFrame[rightFrame.size() - 4] = 0;
					rightFrame[rightFrame.size() - 3] = 0;
					rightFrame[rightFrame.size() - 2] = 0;
					rightFrame[rightFrame.size() - 1] = 0;

					if (leftFrame != rightFrame)
					{
						return false;
					}

					continue;
				}

				if (leftFrames[frameIndex] != rightFrames[frameIndex])
				{
					return false;
				}
			}
		}

		return true;
	}
} // namespace

namespace BuildCompLib
{
	BuildCompImpl::BuildCompImpl() = default;
	BuildCompImpl::~BuildCompImpl() = default;

	tl::expected<bool, QString> BuildCompImpl::setFileLeft(QString fileName)
	{
		QFile file{fileName};
		if (file.open(QIODevice::ReadOnly) == false)
		{
			return tl::make_unexpected(QString("Open file %1 error: %2").arg(fileName).arg(file.errorString()));
		}

		return setFileLeft(file.readAll());
	}

	tl::expected<bool, QString> BuildCompImpl::setFileRight(QString fileName)
	{
		QFile file{fileName};
		if (file.open(QIODevice::ReadOnly) == false)
		{
			return tl::make_unexpected(QString("Open file %1 error: %2").arg(fileName).arg(file.errorString()));
		}

		return setFileRight(file.readAll());
	}

	tl::expected<bool, QString> BuildCompImpl::setFileLeft(const QByteArray& data)
	{
		try
		{
			m_left = std::make_unique<Hardware::ModuleFirmwareStorage>();

			QString errorMessage;

			if (m_left->load(data, &errorMessage) == false)
			{
				throw std::runtime_error(errorMessage.toStdString());
			}
		}
		catch (const std::exception& e)
		{
			m_left.reset();
			return tl::make_unexpected(QString("Error: %1").arg(e.what()));
		}

		return true;
	}

	tl::expected<bool, QString> BuildCompImpl::setFileRight(const QByteArray& data)
	{
		try
		{
			m_right = std::make_unique<Hardware::ModuleFirmwareStorage>();

			QString errorMessage;

			if (m_right->load(data, &errorMessage) == false)
			{
				throw std::runtime_error(errorMessage.toStdString());
			}
		}
		catch (const std::exception& e)
		{
			m_right.reset();
			return tl::make_unexpected(QString("Error: %1").arg(e.what()));
		}

		return true;
	}

	CompareResult BuildCompImpl::compare() const
	{
		CompareResult result;

		if (m_left == nullptr || m_right == nullptr)
		{
			return result;
		}

		result.projectName = m_left->projectName() == m_right->projectName();
		result.projectNameLeft = m_left->projectName();
		result.projectNameRight = m_right->projectName();

		result.userName = m_left->userName() == m_right->userName();
		result.userNameLeft = m_left->userName();
		result.userNameRight = m_right->userName();

		result.buildNumber = m_left->buildNumber() == m_right->buildNumber();
		result.buildNumberLeft = m_left->buildNumber();
		result.buildNumberRight = m_right->buildNumber();

		auto subsystems = m_left->subsystems() + m_right->subsystems();
		subsystems.sort();
		subsystems.removeDuplicates();

		for (const auto& subsystemId : subsystems)
		{
			CompareResult::Subsystem subsystemResult;
			subsystemResult.subsystemId = subsystemId;

			bool foundLeft = false;
			const Hardware::ModuleFirmware& leftFirmware = m_left->firmware(subsystemId, &foundLeft);

			bool foundRight = false;
			const Hardware::ModuleFirmware& rightFirmware = m_right->firmware(subsystemId, &foundRight);

			if (foundLeft == false)
			{
				subsystemResult.left = CompareResult::Subsystem::NotExists;
				subsystemResult.right = CompareResult::Subsystem::NotModified;
			}
			else if (foundRight == false)
			{
				subsystemResult.left = CompareResult::Subsystem::NotModified;
				subsystemResult.right = CompareResult::Subsystem::NotExists;
			}
			else
			{
				// Compare firmware
				//
				bool eq = compareFirmware(leftFirmware, rightFirmware);
				subsystemResult.left = eq ? CompareResult::Subsystem::NotModified : CompareResult::Subsystem::Modified;
				subsystemResult.right = subsystemResult.left;
			}

			result.subsystems.push_back(std::move(subsystemResult));
		}

		bool allSubsystemsAreEqual = std::all_of(result.subsystems.begin(),
												 result.subsystems.end(),
												 [](const auto& subsystem)
												 {
													 return subsystem.left == CompareResult::Subsystem::NotModified &&
															subsystem.right == CompareResult::Subsystem::NotModified;
												 });

		result.isSame = result.projectName && result.userName && /*result.buildNumber &&*/
						allSubsystemsAreEqual;

		return result;
	}
} // namespace BuildCompLib