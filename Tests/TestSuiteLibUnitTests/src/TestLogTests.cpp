#include <TestSuiteLib/TestLog.h>

using ::testing::_;

class MockITestLogOutput : public TestSuite::ITestLogOutput
{
public:
	MOCK_METHOD(void, logItemArrived, (const TestSuite::TestLogItem& item), (override));
};

TEST(TestLog, LoggingHappens)
{
	MockITestLogOutput output;
	TestSuite::TestLog log{&output};

	EXPECT_CALL(output, logItemArrived(_)).Times(5);

	log.writeAlert("Alert", "TagAlert");
	log.writeError("Error", {});
	log.writeWarning("Warning", {});
	log.writeMessage("Message", {});
	log.writeText("Text", {});

	auto items = log.items();
	EXPECT_EQ(items.size(), 5);

	return;
}

TEST(TestLog, LogCanBeCleared)
{
	MockITestLogOutput output;
	TestSuite::TestLog log{&output};

	EXPECT_CALL(output, logItemArrived(_)).Times(5);

	log.writeAlert("Alert", "TagAlert");
	log.writeError("Error", {});
	log.writeWarning("Warning", {});
	log.writeMessage("Message", {});
	log.writeText("Text", {});

	auto items = log.items();
	EXPECT_EQ(items.size(), 5);

	log.clear();
	items = log.items();
	EXPECT_EQ(items.size(), 0);
	EXPECT_TRUE(items.empty());

	return;
}

TEST(TestLog, LogItemIsCorrect)
{
	MockITestLogOutput output;
	TestSuite::TestLog log{&output};

	EXPECT_CALL(output, logItemArrived(_))
		.Times(1)
		.WillOnce(
			[](const TestSuite::TestLogItem& item)
			{
				EXPECT_EQ(item.time().isValid(), true);
				EXPECT_EQ(item.message(), "Message");
				EXPECT_EQ(item.tag(), "Tag");
				EXPECT_EQ(item.type(), TestSuite::TestLogItemType::Message);
			});

	log.writeMessage("Message", "Tag");

	return;
}

TEST(TestLog, LogSavedAndrestoredAsCsv)
{
	MockITestLogOutput output;
	TestSuite::TestLog log{&output};
	EXPECT_CALL(output, logItemArrived(_)).Times(5);

	log.writeAlert("Alert", "TagAlert");
	log.writeError("Error", "TagError");
	log.writeWarning("Warning", "TagWarning");
	log.writeMessage("Message", "TagMessage");
	log.writeText("Text", "TagText");

	QDir dir{QDir::tempPath()};

	QString fileName = dir.filePath("test.csv");
	QString errorMsg;
	bool ok = log.saveToCSV(fileName, &errorMsg);

	EXPECT_EQ(ok, true);
	EXPECT_EQ(errorMsg.isEmpty(), true);


	TestSuite::TestLog log2{&output};

	EXPECT_TRUE(log2.empty());

	ok = log2.loadFromCSV(fileName, &errorMsg);

	EXPECT_EQ(ok, true);
	EXPECT_EQ(errorMsg.isEmpty(), true);

	EXPECT_FALSE(log2.empty());

	auto items = log2.items();
	EXPECT_EQ(items.size(), 5);

	EXPECT_EQ(items[0].message(), "Alert");
	EXPECT_EQ(items[0].tag(), "TagAlert");
	EXPECT_EQ(items[0].type(), TestSuite::TestLogItemType::Error);

	EXPECT_EQ(items[4].message(), "Text");
	EXPECT_EQ(items[4].tag(), "TagText");
	EXPECT_EQ(items[4].type(), TestSuite::TestLogItemType::Text);

	// Fails to load file
	//
	TestSuite::TestLog log3{&output};

	errorMsg.clear();
	ok = log2.loadFromCSV("FileDoesNotExis.csv", &errorMsg);

	EXPECT_EQ(ok, false);
	EXPECT_EQ(errorMsg.isEmpty(), false);

	// Falis to save file
	//
	TestSuite::TestLog log4{&output};

	errorMsg.clear();
	ok = log4.saveToCSV("", &errorMsg);

	EXPECT_EQ(ok, false);
	EXPECT_EQ(errorMsg.isEmpty(), false);

	return;
}

TEST(TestLog, TestLogItemSavedToText)
{
	TestSuite::TestLogItem item{
		10,
		TestSuite::TestLogItemType::Warning,
		"Warning",
		"Tag",
	};
	QString text = item.toText();

	EXPECT_TRUE(text.contains("WRN"));
	EXPECT_TRUE(text.contains("Warning"));

	return;
}

TEST(TestLog, TestLogItemSavedToHtml)
{
	TestSuite::TestLogItem itemErr{
		9,
		TestSuite::TestLogItemType::Error,
		"ErrorText",
		"ErrorTag",
	};
	TestSuite::TestLogItem itemWrn{10, TestSuite::TestLogItemType::Warning, "WarningText", "WarningTag"};
	TestSuite::TestLogItem itemMsg{10, TestSuite::TestLogItemType::Message, "MessageText", "MessageTag"};
	TestSuite::TestLogItem itemTxt{10, TestSuite::TestLogItemType::Text, "TextText", "TextTag"};

	EXPECT_TRUE(itemErr.isError());
	EXPECT_TRUE(itemWrn.isWarning());
	EXPECT_TRUE(itemMsg.isMessage());


	QString textErr = itemErr.toHtml();
	EXPECT_TRUE(textErr.startsWith("<"));
	EXPECT_TRUE(textErr.endsWith(">"));
	EXPECT_TRUE(textErr.contains("ErrorText"));

	QString textWrn = itemWrn.toHtml();
	EXPECT_TRUE(textWrn.startsWith("<"));
	EXPECT_TRUE(textWrn.endsWith(">"));
	EXPECT_TRUE(textWrn.contains("WarningText"));

	QString textMsg = itemMsg.toHtml();
	EXPECT_TRUE(textMsg.startsWith("<"));
	EXPECT_TRUE(textMsg.endsWith(">"));
	EXPECT_TRUE(textMsg.contains("MessageText"));

	QString textTxt = itemTxt.toHtml();
	EXPECT_TRUE(textTxt.startsWith("<"));
	EXPECT_TRUE(textTxt.endsWith(">"));
	EXPECT_TRUE(textTxt.contains("TextText"));

	return;
}

TEST(TestLog, TestLogItemSavedToStringList)
{
	TestSuite::TestLogItem item{
		10,
		TestSuite::TestLogItemType::Warning,
		"Warning",
		"Tag",
	};
	QStringList list = item.toStringList();

	EXPECT_TRUE(list.contains("Warning"));
	EXPECT_TRUE(list.contains("Tag"));

	// Convert back to TestLogItem
	//
	bool ok = false;
	TestSuite::TestLogItem item2 = TestSuite::TestLogItem::fromStringList(list.join(";"), &ok);

	EXPECT_TRUE(ok);

	EXPECT_EQ(item.time(), item2.time());
	EXPECT_EQ(item.message(), item2.message());
	EXPECT_EQ(item.tag(), item2.tag());
	EXPECT_EQ(item.type(), item2.type());

	TestSuite::TestLogItem item3 = TestSuite::TestLogItem::fromStringList("", &ok);
	EXPECT_FALSE(ok);

	return;
}
