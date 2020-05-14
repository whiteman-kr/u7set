#pragma once
//#include "../Simulator/SimRam.h"
//#include "../Simulator/Simulator.h"


//class MemoryView;
//class MemoryHexView;
//class SimMemoryWidget;


//class SimMemoryDialog : public QDialog
//{
//	Q_OBJECT

//private:
//	SimMemoryDialog(Sim::AppSignalManager* appSignalManager, QString lmEquipmentId, QWidget* parent);
//	~SimMemoryDialog();

//public:
//	static void showDialog(Sim::AppSignalManager* appSignalManager, QString lmEquipmentId, QWidget* parent);

//protected:
//	virtual void timerEvent(QTimerEvent* event) override;

//private:
//	QString m_lmEquipmentId;
//	Sim::AppSignalManager* m_appSignalManager = nullptr;

//	SimMemoryWidget* m_memoryWidget = nullptr;
//	int m_timerId = -1;
//};


//class SimMemoryWidget : public QWidget
//{
//	Q_OBJECT

//public:
//	explicit SimMemoryWidget(const Sim::Ram& ram, QWidget* parent = nullptr);
//	virtual ~SimMemoryWidget();

//	const Sim::Ram& ram() const;
//	Sim::Ram& ram();

//	void update();

//protected:
//	void updateAreaCombo();
//	void updateAreaInfo();
//	void updateQuickWatch();

//protected slots:
//	void currentAreaChanged(int index);

//private:
//	Sim::Ram m_ram;

//	QComboBox* m_ramAreaCombo = nullptr;
//	QLabel* m_areaInfoLabel = nullptr;

//	QSplitter* m_splitter = nullptr;
//	MemoryView* m_memoryWidget = nullptr;
//	QLabel* m_quickWatchLabel = nullptr;
//};


//class MemoryView : public QWidget
//{
//	Q_OBJECT

//public:
//	MemoryView();

//public:
//	void setArea(const Sim::RamArea& area);
//	const Sim::RamArea& area() const;

//protected:
//	void setScrollrange();
//	virtual void showEvent(QShowEvent*) override;
//	virtual void resizeEvent(QResizeEvent* event) override;

//private:
//	Sim::RamArea m_area;
//	MemoryHexView* m_hexView = nullptr;
//	QScrollBar* m_scroll = nullptr;
//};


//class MemoryHexView : public QWidget
//{
//	Q_OBJECT

//public:
//	MemoryHexView(QScrollBar* scroll);

//	void setMemoryArea(const Sim::RamArea* area);

//	int wordsInLine() const;		// Calc number of words in the line depending on the widget width, can be power of 2: [1, 2, 4, 8, 16]
//	int lineCount() const;			// Calc number of lines depending on the widget height

//protected:
//	virtual void wheelEvent(QWheelEvent* event) override;
//	virtual void paintEvent(QPaintEvent* event) override;

//private:
//	const Sim::RamArea* m_memoryArea = nullptr;	// MemoryHexView IS NOT OWNER OF THIS DATA
//	QScrollBar* m_scroll = nullptr;
//};
