#include "SettingsDlg.h"
#include <QFileDialog>

SettingsDlg::SettingsDlg(const QString& ffmpeg, const QString &ffplay, const QString &output, const QString &bell,
	bool origname, const QString &prefix, int digits, int code, double silence, QWidget *parent)
	: QDialog(parent)
	, pathFfmpeg(ffmpeg)
	, pathFfplay(ffplay)
	, pathOutput(output)
	, pathBell(bell)
	, origName(origname)
	, namePrefix(prefix)
	, numOfDigits(digits)
	, codeOutput(code)
	, timeSilence(silence)
{
	ui.setupUi(this);

	ui.lineFfmpeg->setText(pathFfmpeg);
	ui.lineFfplay->setText(pathFfplay);
	ui.lineOutput->setText(pathOutput);
	ui.lineBell->setText(pathBell);
	ui.linePrefix->setText(namePrefix);
	ui.comboDigits->setCurrentIndex(numOfDigits-1);
	ui.comboOutput->setCurrentIndex(codeOutput);
	ui.lineSilence->setText(QString::number(timeSilence));
	ui.checkOrigName->setChecked(origName);
	
	connect(ui.pushFfmpeg, &QPushButton::clicked, this, &SettingsDlg::onOverviewFfmpeg);
	connect(ui.pushFfplay, &QPushButton::clicked, this, &SettingsDlg::onOverviewFfplay);
	connect(ui.pushOutput, &QPushButton::clicked, this, &SettingsDlg::onOverviewOutput);
	connect(ui.pushBell, &QPushButton::clicked, this, &SettingsDlg::onOverviewBell);
	connect(ui.pushOk, &QPushButton::clicked, this, &SettingsDlg::onOk);
}

SettingsDlg::~SettingsDlg()
{}

void SettingsDlg::onOverviewFfmpeg()
{
	QString path = QFileDialog::getOpenFileName(this, "Select FFMPEG binary file", pathFfmpeg, "Files (*.*)");
	if (!path.isEmpty()) {
		pathFfmpeg = path;
		ui.lineFfmpeg->setText(pathFfmpeg);
	}
}

void SettingsDlg::onOverviewFfplay()
{
	QString path = QFileDialog::getOpenFileName(this, "Select FFPLAY binary file", pathFfplay, "Files (*.*)");
	if (!path.isEmpty()) {
		pathFfplay = path;
		ui.lineFfplay->setText(pathFfplay);
	}
}

void SettingsDlg::onOverviewOutput()
{
	QString path = QFileDialog::getExistingDirectory(this, "Select output directory", pathOutput);
	if (!path.isEmpty()) {
		pathOutput = path;
		ui.lineOutput->setText(pathOutput);
	}
}

void SettingsDlg::onOverviewBell()
{
	QString path = QFileDialog::getOpenFileName(this, "Select BELL mp3 file", pathBell, "Files (*.mp3)");
	if (!path.isEmpty()) {
		pathBell = path;
		ui.lineBell->setText(pathBell);
	}
}


void SettingsDlg::onOk()
{
	pathFfmpeg = ui.lineFfmpeg->text();
	pathFfplay = ui.lineFfplay->text();
	pathOutput = ui.lineOutput->text();
	pathBell = ui.lineBell->text();
	namePrefix = ui.linePrefix->text();
	numOfDigits = ui.comboDigits->currentIndex() + 1;
	codeOutput = ui.comboOutput->currentIndex();
	timeSilence = ui.lineSilence->text().toDouble();
	origName = ui.checkOrigName->isChecked();

	accept();
}
