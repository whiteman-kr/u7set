#pragma once
#include <QtWidgets>
#include "../Simulator/SimRam.h"

class MemoryView;
class MemoryHexView;

class SimulatorMemoryWidget : public QWidget
{
	Q_OBJECT
public:
	explicit SimulatorMemoryWidget(const Sim::Ram ram, QWidget* parent = nullptr);
	virtual ~SimulatorMemoryWidget();

protected:
	void updateAreaCombo();
	void updateAreaInfo();
	void updateQuickWatch();

protected slots:
	void currentAreaChanged(int index);

private:
	Sim::Ram m_ram;

	QComboBox* m_ramAreaCombo = nullptr;
	QLabel* m_areaInfoLabel = nullptr;

	QSplitter* m_splitter = nullptr;
	MemoryView* m_memoryWidget = nullptr;
	QLabel* m_quickWatchLabel = nullptr;
};


class MemoryView : public QWidget
{
	Q_OBJECT
public:
	MemoryView();

public:
	void setAreaInfo(const Sim::RamAreaInfo& areaInfo);
	const Sim::RamAreaInfo& areaInfo() const;

protected:
	void setScrollrange();
	virtual void showEvent(QShowEvent*) override;
	virtual void resizeEvent(QResizeEvent *event) override;

private:
	Sim::RamAreaInfo m_areaInfo;
	MemoryHexView* m_hexView = nullptr;
	QScrollBar* m_scroll = nullptr;
};

class MemoryHexView : public QWidget
{
	Q_OBJECT
public:
	MemoryHexView(Sim::RamAreaInfo* memoryArea, QScrollBar* scroll);

	int wordsInLine() const;		// Calc number of words in the line depending on the widget width, can be power of 2: [1, 2, 4, 8, 16]
	int lineCount() const;			// Calc number of lines depending on the widget height

protected:
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;

private:
	Sim::RamAreaInfo* m_memoryArea = nullptr;
	QScrollBar* m_scroll = nullptr;
};
