#include "stdafx.h"
#include "Donate.h"

Donate::Donate(QWidget* parent)
	: QDialog(parent)
{
	VRootLayout = new QVBoxLayout(this);

	HPicLayout = new QHBoxLayout();
	HSpacer_1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	HPicLayout->addItem(HSpacer_1);

	Alipay = new QLabel();
	Alipay->setPixmap(QPixmap(":/SpaceLookup/alipay.jpg"));
	HPicLayout->addWidget(Alipay);

	HSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	HPicLayout->addItem(HSpacer_2);

	MidLine = new QFrame(this);
	MidLine->setObjectName(QString::fromUtf8("MidLine"));
	MidLine->setFrameShape(QFrame::VLine);
	MidLine->setFrameShadow(QFrame::Sunken);
	HPicLayout->addWidget(MidLine);

	HSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	HPicLayout->addItem(HSpacer_3);

	Wechat = new QLabel();
	Wechat->setPixmap(QPixmap(":/SpaceLookup/wechat.png"));
	HPicLayout->addWidget(Wechat);

	HSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	HPicLayout->addItem(HSpacer_4);
	VRootLayout->addLayout(HPicLayout);

	DownLine = new QFrame(this);
	DownLine->setObjectName(QString::fromUtf8("DownLine"));
	DownLine->setFrameShape(QFrame::HLine);
	DownLine->setFrameShadow(QFrame::Sunken);
	VRootLayout->addWidget(DownLine);

	ThankText = new QLabel();
	ThankText->setText("Thank you for donating!");
	VRootLayout->addWidget(ThankText);
}

Donate::~Donate()
{
}