#include "stdafx.h"
#include "ConfigWnd.h"

CConfigWnd::CConfigWnd(QWidget* pParent)
	: QMainWindow(pParent)
{
	ui.setupUi(this);

	statusBar()->hide();

	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(close()));
	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(OnOk()));

	ui.txtName->setText(theConf->value("Chat/Name").toString());
}

CConfigWnd::~CConfigWnd()
{
}

void CConfigWnd::OnOk()
{
	theConf->setValue("Chat/Name", ui.txtName->text());

	close();
}

void CConfigWnd::closeEvent(QCloseEvent* event)
{
	QMainWindow::closeEvent(event);
	this->deleteLater();
}