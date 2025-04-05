#pragma once

#include <QtWidgets/QMainWindow>
#include <QProcess>
#include "ui_AudioSplitter.h"
#include "HorizontalBarChart.h"

class AudioSplitter : public QMainWindow
{
	Q_OBJECT
public:
	enum class State {
		IDLE,
		SCAN,
		PLAY,
		CONV
	} state = State::IDLE;

public:
    AudioSplitter(QWidget *parent = nullptr);
    ~AudioSplitter();
private:
	void loadSettings();
	void saveSettings();
	void handleOut(const QString &str);
private slots:

	void onOpen();
	void onSettings();
	void onPlay();
	void onStop();
	void onScan();
	void onConv();
	void onTags();
	void onBell();

	void onReadStdOut();
	void onReadStdErr();
	void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onDoubleClicked(int index);
private:
    Ui::AudioSplitterClass ui;
	HorizontalBarChart* chart;
	QProcess ffmpegProcess;
	QString pathSource;
	QString pathOutput;
	QString pathFfmpeg;
	QString pathFfplay;
	double timeSilence;
	int codeOutput;
	QString pathBell;
	QString namePrefix;
	int numOfDigits;
};

// todo:
// save & load CUE files https://en.wikipedia.org/wiki/Cue_sheet_(computing)
// 