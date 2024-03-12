#pragma once

class AppSignal;
class DbController;
class DbFile;

namespace Hardware
{
	class DeviceObject;
	class DeviceAppSignal;
}

class DbControllerTools
{
public:
	static std::pair<int, std::vector<int>> showSelectFolderDialog(DbController* db, int parentFileId, int currentSelectionFileId, bool showRootFile, QWidget* parentWidget);

	// Hardware Configuration
	//
	[[nodiscard]] static std::shared_ptr<Hardware::DeviceObject> deviceObjectFromDbFile(const DbFile& file);
	[[nodiscard]] static std::vector<std::shared_ptr<Hardware::DeviceObject>> deviceObjectFromDbFiles(const std::vector<std::shared_ptr<DbFile>>& files);

	static QString initAppSignalFromDeviceAppSignal(const Hardware::DeviceAppSignal& deviceSignal, AppSignal* appSignal);
};


