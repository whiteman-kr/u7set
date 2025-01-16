class QApplication;

namespace UiLib
{
	// Override Windows11 style, the current implementation does not look well.
	void OverrideWindows11Style(QApplication& app, int argc, char* argv[]);
} // namespace UiLib