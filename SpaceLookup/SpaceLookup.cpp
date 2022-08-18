#include "stdafx.h"
#include "SpaceLookup.h"
#include "Donate.h"

#define __WIDE(x) L##x
#define L(x) __WIDE(x)

#define SL_VER L"0.0.0.3"

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
	ui.ComboVolSel->setCurrentIndex(-1);

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
	ChartView->show();

	ui.PieLayout->insertWidget(0, ChartView, 1);

	// 初始化Log
	SpaceLookupLogger = spdlog::basic_logger_mt("SpaceLookup", L"SpaceLookup.log");
	spdlog::set_default_logger(SpaceLookupLogger);
	spdlog::set_level(spdlog::level::info);
	spdlog::critical(L"Program started!");

	// 初始化槽
	connect(ui.ComboVolSel, SIGNAL(currentTextChanged(const QString&)), this, SLOT(onVolSelChanged(const QString&)));
	connect(Series, SIGNAL(hovered(QPieSlice*, bool)), this, SLOT(onPieSliceHovered(QPieSlice*, bool)));
	connect(Series, SIGNAL(clicked(QPieSlice*)), this, SLOT(onPieSliceClicked(QPieSlice*)));
	connect(ui.BtnBack, SIGNAL(pressed()), this, SLOT(onBtnBackPressed()));
	connect(ui.BtnOpen, SIGNAL(pressed()), this, SLOT(onBtnOpenPressed()));
	connect(ui.actionAbout_Me, SIGNAL(triggered()), this, SLOT(onAboutMeTriggered()));
	connect(ui.actionAbout_Program, SIGNAL(triggered()), this, SLOT(onAboutProgramTriggered()));
	connect(ui.actionDonate, SIGNAL(triggered()), this, SLOT(onDonateTriggered()));

	ui.BtnBack->setEnabled(false);
	ui.BtnOpen->setEnabled(false);

	ui.statusBar->showMessage("Ready");
}

void SpaceLookup::onVolSelChanged(const QString& Text)
{
	ui.statusBar->showMessage("Running,please wait...");

	ui.BtnBack->setEnabled(false);
	ui.BtnOpen->setEnabled(false);
	ui.ComboVolSel->setEnabled(false);
	Series->clear();

	auto SearchRoot = Text.toStdString();
	auto& Space = AllSpaces[SearchRoot[0]];

	auto WorkThread = QThread::create([this, &Space, SearchRoot]()
		{
			if (Space.empty()) ResolveDir(Space, SearchRoot);
		});
	WorkThread->start();

	connect(WorkThread, &QThread::finished, this, [this, &Space, SearchRoot]()
		{
			auto& Space = AllSpaces[SearchRoot[0]];
			uint16_t Run = 1;
			uintmax_t TotalSize = 0;

			for (const auto& Item : Space[0])
			{
				TotalSize += Item->Size;
			}

			Series->clear();

			for (const auto& Item : Space[0])
			{
				auto Slice = new QPieSlice(QString::fromStdWString(Item->FileObj.path().filename()), (double)Item->Size / TotalSize, this);
				Slice->setLabelVisible(false);
				Slice->setColor(QColor::fromHsv(259.0 * Run / Space[0].size(), Random() % 255, Random() % 255));
				Slice->setExploded(true);
				Slice->setExplodeDistanceFactor(0.1);
				Slice->setProperty("current_item", (ULONG_PTR)Item);
				Slice->setProperty("parent_item", (ULONG_PTR)nullptr);
				Series->append(Slice);
				++Run;
			}

			ui.ComboVolSel->setEnabled(true);
			ui.BtnOpen->setEnabled(true);

			ui.statusBar->showMessage("Ready");
		});
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
		Slice->setExplodeDistanceFactor(0.2);

		auto Item = (SpaceItem*)Slice->property("current_item").value<ULONG_PTR>();
		auto TipString = fmt::format(L"Name:{}\nSize:{:.2f} MB\nFileCount:{}\nDirCount:{}", Item->FileObj.path().filename().wstring(), (double)Item->Size / 1024 / 1024, Item->FileCount, Item->DirCount);
		QToolTip::showText(QCursor().pos(), QString::fromStdWString(TipString), this);

		auto ShowString = fmt::format(L"{} | {}", Item->FileObj.path().filename().wstring(), Item->FileObj.path().wstring());
		ui.statusBar->showMessage(QString::fromStdWString(ShowString));
	}
	else
	{
		Slice->setLabelVisible(false);
		Slice->setExplodeDistanceFactor(0.1);

		QToolTip::hideText();

		ui.statusBar->showMessage("Ready");
	}
}

void SpaceLookup::onPieSliceClicked(QPieSlice* Slice)
{
	ui.statusBar->showMessage("Running,please wait...");

	auto CurrentItem = (SpaceItem*)Slice->property("current_item").value<ULONG_PTR>();

	if (!CurrentItem->BlinkItems.empty())
	{
		ui.BtnBack->setEnabled(false);
		ui.ComboVolSel->setEnabled(false);

		uint16_t Run = 1;
		Series->clear();

		for (const auto& Item : CurrentItem->BlinkItems)
		{
			auto Slice = new QPieSlice(QString::fromStdWString(Item->FileObj.path().filename()), (double)Item->Size / CurrentItem->Size, this);
			Slice->setLabelVisible(false);
			Slice->setColor(QColor::fromHsv(259.0 * Run / CurrentItem->BlinkItems.size(), Random() % 255, Random() % 255));
			Slice->setExploded(true);
			Slice->setExplodeDistanceFactor(0.1);
			Slice->setProperty("current_item", (ULONG_PTR)Item);
			Slice->setProperty("parent_item", (ULONG_PTR)CurrentItem);
			Series->append(Slice);
			++Run;
		}

		ui.BtnBack->setEnabled(true);
		ui.ComboVolSel->setEnabled(true);
	}

	ui.statusBar->showMessage("Ready");
}

void SpaceLookup::onAboutMeTriggered()
{
	QFont Font;
	QMessageBox AboutMe;
	QFile Html(":/SpaceLookup/AboutMe.html");
	QString HtmlString = "Error when opening html file!";
	if (Html.open(QIODevice::ReadOnly | QIODevice::Text))
		HtmlString = Html.readAll();

	Font.setBold(true);
	Font.setPointSize(15);
	AboutMe.setTextFormat(Qt::AutoText);
	AboutMe.setWindowTitle("About Me");
	AboutMe.setText(HtmlString);
	AboutMe.setStyleSheet("QLabel{min-width: 300px;}");
	AboutMe.setIconPixmap(QPixmap(":/SpaceLookup/logo.jpg"));
	AboutMe.setFont(Font);
	AboutMe.exec();
}

void SpaceLookup::onAboutProgramTriggered()
{
	QFont Font;
	QMessageBox AboutProgram;

	Font.setBold(true);
	Font.setPointSize(10);
	AboutProgram.setTextFormat(Qt::AutoText);
	AboutProgram.setWindowTitle("About Program");
	AboutProgram.setText(QString::fromStdWString(fmt::format(L"Version:{}\n\nBuild Time:{} {} GMT+0800", SL_VER, L(__DATE__), L(__TIME__))));
	AboutProgram.setStyleSheet("QLabel{min-width: 400px;}");
	AboutProgram.setFont(Font);
	AboutProgram.exec();
}

void SpaceLookup::onDonateTriggered()
{
	auto DonateWindow = new Donate(this);
	DonateWindow->setWindowTitle("Donate Me");
	DonateWindow->setAttribute(Qt::WA_DeleteOnClose);
	DonateWindow->setModal(true);
	DonateWindow->show();
}

void SpaceLookup::onBtnBackPressed()
{
	ui.statusBar->showMessage("Running,please wait...");

	auto ParentItem = (SpaceItem*)Series->slices().first()->property("parent_item").value<ULONG_PTR>();
	if (ParentItem)
	{
		ui.ComboVolSel->setEnabled(false);

		uintmax_t TotalSize = 0;
		auto CurrentList = ParentItem->FlinkItem ? &ParentItem->FlinkItem->BlinkItems : nullptr;
		TotalSize = ParentItem->FlinkItem ? ParentItem->FlinkItem->Size : 0;
		if (!CurrentList)
		{
			auto RootPath = ui.ComboVolSel->currentText().toStdString();
			CurrentList = &AllSpaces[RootPath[0]][0];

			ui.BtnBack->setEnabled(false);
		}
		if (!TotalSize)
		{
			for (const auto& Item : *CurrentList)
			{
				TotalSize += Item->Size;
			}
		}

		uint16_t Run = 1;
		Series->clear();

		for (const auto& Item : *CurrentList)
		{
			auto Slice = new QPieSlice(QString::fromStdWString(Item->FileObj.path().filename()), (double)Item->Size / TotalSize, this);
			Slice->setLabelVisible(false);
			Slice->setColor(QColor::fromHsv(259.0 * Run / CurrentList->size(), Random() % 255, Random() % 255));
			Slice->setExploded(true);
			Slice->setExplodeDistanceFactor(0.1);
			Slice->setProperty("current_item", (ULONG_PTR)Item);
			Slice->setProperty("parent_item", (ULONG_PTR)ParentItem->FlinkItem);
			Series->append(Slice);
			++Run;
		}

		ui.ComboVolSel->setEnabled(true);
	}

	ui.statusBar->showMessage("Ready");
}

void SpaceLookup::onBtnOpenPressed()
{
	ui.statusBar->showMessage("Running,please wait...");

	std::wstring OpenPath;
	auto ParentItem = (SpaceItem*)Series->slices().first()->property("parent_item").value<ULONG_PTR>();
	if (ParentItem)
		OpenPath = ParentItem->FileObj.path().wstring();
	else
		OpenPath = ui.ComboVolSel->currentText().toStdWString();

	::ShellExecuteW((HWND)this->winId(), L"explore", OpenPath.c_str(), nullptr, nullptr, SW_SHOW);

	ui.statusBar->showMessage("Ready");
}

SpaceLookup::~SpaceLookup()
{
	spdlog::critical(L"Program exited!");
}