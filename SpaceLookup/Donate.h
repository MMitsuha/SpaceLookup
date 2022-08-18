#pragma once

#include <QtWidgets/QDialog>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>
#include "ui_SpaceLookup.h"
#include "FileSearcher.h"

class Donate : public QDialog
{
	Q_OBJECT

public:
	Donate(QWidget* parent = nullptr);
	~Donate();

private:
	QVBoxLayout* VRootLayout;
	QHBoxLayout* HPicLayout;
	QSpacerItem* HSpacer_1;
	QLabel* Alipay;
	QSpacerItem* HSpacer_2;
	QFrame* MidLine;
	QSpacerItem* HSpacer_3;
	QLabel* Wechat;
	QSpacerItem* HSpacer_4;

	QFrame* DownLine;

	QLabel* ThankText;

private Q_SLOTS:
};
