#pragma once

#include <QtWidgets/QMainWindow>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include "ui_SpaceLookup.h"
#include "FileSearcher.h"

class SpaceLookup : public QMainWindow
{
	Q_OBJECT

public:
	SpaceLookup(QWidget* parent = nullptr);
	~SpaceLookup();

private:
	SpaceInfo Space;
	std::bitset<32> Volumes;
	std::default_random_engine Random;
	std::shared_ptr<spdlog::logger> SpaceLookupLogger;

	Ui::SpaceLookupClass ui;

	QPieSeries* Series = nullptr;
	QChart* Chart = nullptr;
	QChartView* ChartView = nullptr;

private Q_SLOTS:
	void onVolSelChanged(const QString& Text);
	void onPieSliceHovered(QPieSlice* Slice, bool state);
	void onPieSliceClicked(QPieSlice* Slice);
	void onAboutMeTriggered();
};
