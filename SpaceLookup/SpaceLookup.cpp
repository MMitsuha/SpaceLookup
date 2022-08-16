#include "stdafx.h"
#include "SpaceLookup.h"

SpaceLookup::SpaceLookup(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.statusBar->showMessage("Initializing...");

	// 设置卷选择
	Volumes = GetLogicalDrives();
	for (char i = 0; i < Volumes.size(); i++)
	{
		if (Volumes[i])
		{
			auto VolStr = fmt::format("{}:\\", (char)('A' + i));

			ui.ComboVolSel->addItem(QString::fromStdString(VolStr));
		}
	}

	// 初始化PieChart
	Series = new QPieSeries(this);
	Chart = new QChart();

	Series->setHorizontalPosition(0.5);
	Series->setVerticalPosition(0.5);
	Series->setHoleSize(0.2);
	Series->setPieSize(0.8);
	Chart->addSeries(Series);
	Chart->setAnimationOptions(QChart::AllAnimations);
	Chart->legend()->setAlignment(Qt::AlignRight);

	ChartView = new QChartView(this);
	ChartView->setChart(Chart);
	ChartView->setRenderHint(QPainter::Antialiasing);

	ui.PieLayout->insertWidget(0, ChartView, 1);

	// 初始化Log
	SpaceLookupLogger = spdlog::basic_logger_mt("SpaceLookup", L"SpaceLookup.log");
	spdlog::set_default_logger(SpaceLookupLogger);
	spdlog::critical(L"Program started!");

	// 初始化槽
	connect(ui.ComboVolSel, SIGNAL(currentTextChanged(const QString&)), this, SLOT(onVolSelChanged(const QString&)));
	connect(Series, SIGNAL(hovered(QPieSlice*, bool)), this, SLOT(onPieSliceHovered(QPieSlice*, bool)));
	connect(Series, SIGNAL(clicked(QPieSlice*)), this, SLOT(onPieSliceClicked(QPieSlice*)));
	connect(ui.actionAbout_Me, SIGNAL(triggered()), this, SLOT(onAboutMeTriggered()));

	ui.statusBar->showMessage("Ready");
}

void SpaceLookup::onVolSelChanged(const QString& Text)
{
	ui.statusBar->showMessage("Running,please wait...");

	auto SearchRoot = Text.toStdString();
	Free(Space);
	ResolveDir(Space, SearchRoot);

	uint16_t Run = 1;
	uintmax_t TotalSize = 0;

	for (const auto& Item : Space[0])
	{
		TotalSize += Item->Size;
	}

	Series->clear();

	for (const auto& Item : Space[0])
	{
		auto Slice = new QPieSlice(QString::fromStdWString(Item->Name), (double)Item->Size / TotalSize, this);
		Slice->setLabelVisible(false);
		Slice->setColor(QColor::fromHsv(259.0 * Run / Space[0].size(), Random() % 255, Random() % 255));
		Slice->setExploded(true);
		Slice->setExplodeDistanceFactor(0.1);
		Slice->setProperty("filesearcher_item", (uint64_t)Item);
		Series->append(Slice);
		++Run;
	}

	ChartView->show();

	ui.statusBar->showMessage("Ready");
}

void SpaceLookup::onPieSliceHovered(QPieSlice* Slice, bool state)
{
	Slice->setLabelPosition(QPieSlice::LabelInsideNormal);
	auto Font = Slice->labelFont();
	Font.setBold(true);
	Slice->setLabelFont(Font);
	if (state)
	{
		Slice->setLabelVisible(true);
		Slice->setBorderWidth(Slice->borderWidth() / 10);

		auto Item = (SpaceItem*)Slice->property("filesearcher_item").value<uint64_t>();
		ui.statusBar->showMessage(QString::fromStdWString(Item->Name));
	}
	else
	{
		Slice->setLabelVisible(false);
		Slice->setBorderWidth(Slice->borderWidth() * 10);

		ui.statusBar->showMessage("Ready");
	}
}

void SpaceLookup::onPieSliceClicked(QPieSlice* Slice)
{
	ui.statusBar->showMessage("Running,please wait...");

	auto ParentItem = (SpaceItem*)Slice->property("filesearcher_item").value<uint64_t>();

	uint16_t Run = 1;
	uintmax_t TotalSize = 0;

	for (const auto& Item : ParentItem->BlinkItems)
	{
		TotalSize += Item->Size;
	}

	Series->clear();

	for (const auto& Item : ParentItem->BlinkItems)
	{
		auto Slice = new QPieSlice(QString::fromStdWString(Item->Name), (double)Item->Size / TotalSize, this);
		Slice->setLabelVisible(false);
		Slice->setColor(QColor::fromHsv(259.0 * Run / ParentItem->BlinkItems.size(), Random() % 255, Random() % 255));
		Slice->setExploded(true);
		Slice->setExplodeDistanceFactor(0.1);
		Slice->setProperty("filesearcher_item", (uint64_t)Item);
		Series->append(Slice);
		++Run;
	}

	ui.statusBar->showMessage("Ready");
}

void SpaceLookup::onAboutMeTriggered()
{
	QFont Font;
	QMessageBox AboutMe;
	Font.setBold(true);
	AboutMe.setWindowTitle("About Me");
	AboutMe.setText("Author:Mitsuha\n\nPowered by Qt\n\nThanks for using!");
	AboutMe.setStyleSheet("QLabel{min-width: 300px;}");
	AboutMe.setIconPixmap(QPixmap(":/SpaceLookup/logo.jpg"));
	AboutMe.setFont(Font);
	AboutMe.exec();
}

SpaceLookup::~SpaceLookup()
{
	spdlog::critical(L"Program exited!");
}