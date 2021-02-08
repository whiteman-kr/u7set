TEMPLATE = subdirs

CONFIG += ordered

win32 {
    SUBDIRS += Protobuf \
	    qtservice \
		qtpropertybrowser \
		QScintilla \
		TrendView \
		VFrame30 \
		ServiceControlManager \
		Metrology \
		Simulator \
		SimulatorConsole \
		Builder \
		BuilderConsole \
		u7 \
		BaseService \
		ConfigurationService \
		ArchivingService \
		Monitor \
		PacketViewer \
		TuningService \
		AppDataService \
		DiagDataService \
		TuningClient \
		TuningIPEN \
		mconf \
		./Tools/CommView \
		./Tools/PacketSource \
		./Test/UalTester \
		./Test/SimulatorTests \
		./Test/u7databaseTests
}
unix {
    SUBDIRS += Protobuf \
	    qtservice \
		qtpropertybrowser \
		QScintilla \
		TrendView \
		VFrame30 \
		#ServiceControlManager \
		#Metrology \
		Simulator \
		#SimulatorConsole \
		Builder \
		#BuilderConsole \
		#u7 \
		#BaseService \
		#ConfigurationService \
		#ArchivingService \
		#Monitor \
		#PacketViewer \
		#3TuningService \
		#AppDataService \
		#DiagDataService \
		#TuningClient \
		#TuningIPEN \
		#mconf \
		#./Tools/CommView \
		#./Tools/PacketSource \
		#./Test/UalTester \
		#./Test/SimulatorTests \
		./Test/u7databaseTests
}
