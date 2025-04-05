#pragma once

#include <QDialog>
#include "ui_SettingsDlg.h"

class SettingsDlg : public QDialog
{
	Q_OBJECT

public:
	SettingsDlg(const QString& ffmpeg, const QString &ffplay, const QString &output, 
		const QString &bell, const QString &prefix, int digits, int code, double silence, QWidget *parent = nullptr);
	~SettingsDlg();
public:
	QString pathFfmpeg;
	QString pathFfplay;
	QString pathOutput;
	QString pathBell;
	QString namePrefix;
	int numOfDigits;
	double timeSilence;
	int codeOutput;
	
private slots:
	void onOverviewFfmpeg();
	void onOverviewFfplay();
	void onOverviewOutput();
	void onOverviewBell();
	void onOk();
private:
	Ui::SettingsDlgClass ui;
};
