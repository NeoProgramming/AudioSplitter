#include "AudioSplitter.h"
#include <QScrollBar>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QThread>
#include <QDirIterator>
#include "SettingsDlg.h"
#include "id3v1.h"

const char pathIni[] = "audiosplitter.ini";

#define SETTINGS_LIST	\
	ITEM(pathFfmpeg, "ffmpeg")	\
	ITEM(pathFfplay, "ffplay")	\
	ITEM(pathOutput, ".")		\
	ITEM(pathBell, "i.mp3")		\
	ITEM(timeSilence, 3.0)		\
	ITEM(codeOutput, 0)			\
	ITEM(namePrefix, "")		\
	ITEM(numOfDigits, 3)		\
	ITEM(origName, 0)

AudioSplitter::AudioSplitter(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	ui.scrollArea->setWidgetResizable(true);
	
	chart = new HorizontalBarChart(ui.scrollArea);	
	ui.scrollArea->setWidget(chart);
	ui.scrollArea->setVerticalScrollBar(new QScrollBar(Qt::Vertical));
		
	QList<int> Sizes;
	Sizes.append(0.5 * sizeHint().height());
	Sizes.append(0.5 * sizeHint().height());
	ui.splitter->setSizes(Sizes);
	
	connect(ui.actionOpen, &QAction::triggered, this, &AudioSplitter::onOpen);
	connect(ui.actionPlay, &QAction::triggered, this, &AudioSplitter::onPlay);
	connect(ui.actionStop, &QAction::triggered, this, &AudioSplitter::onStop);
	connect(ui.actionScan, &QAction::triggered, this, &AudioSplitter::onScan);
	connect(ui.actionSplit, &QAction::triggered, this, &AudioSplitter::onConv);
	connect(ui.actionTags, &QAction::triggered, this, &AudioSplitter::onTags);
	connect(ui.actionBell, &QAction::triggered, this, &AudioSplitter::onBell);
	connect(ui.actionSettings, &QAction::triggered, this, &AudioSplitter::onSettings);

	connect(&ffmpegProcess, &QProcess::readyReadStandardOutput, this, &AudioSplitter::onReadStdOut);
	connect(&ffmpegProcess, &QProcess::readyReadStandardError, this, &AudioSplitter::onReadStdErr);
	connect(&ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &AudioSplitter::onProcessFinished);

	connect(chart, &HorizontalBarChart::doubleClicked, this, &AudioSplitter::onDoubleClicked);

	loadSettings();

	ui.textEdit->append("<h3>AudioSplitter usage</h3>");
	ui.textEdit->append("0. Configure: select paths to ffmpeg and ffplay, pause duration etc.");
	ui.textEdit->append("1. Open audio file");
	ui.textEdit->append("2. Start pause search.The diagram on the left shows durations of sections with sound, on the right - durations of pauses.");
	ui.textEdit->append("3. Listen to the resulting fragments(double - click on the pause) and mark those pauses by which the audio should be cut(right - click on the pause).");
	ui.textEdit->append("4. Start conversion to a set of *.mp3 files");
}

AudioSplitter::~AudioSplitter()
{
	onStop();
	saveSettings();
}

template<typename T>
void toValue(T&value, const QVariant &var)
{
	value = var.value<T>();
}

void AudioSplitter::loadSettings()
{
	QSettings settings(pathIni, QSettings::IniFormat);

#define ITEM(name, defv)	toValue(name, settings.value(#name, defv));
	SETTINGS_LIST
#undef ITEM
}

void AudioSplitter::saveSettings()
{
	QSettings settings(pathIni, QSettings::IniFormat);

#define ITEM(name, defv)	settings.setValue(#name, name);
	SETTINGS_LIST
#undef ITEM
}

void AudioSplitter::onOpen()
{
	QString selectedFilePath = QFileDialog::getOpenFileName(this, "Select audio file", "", "Files (*.*)");
	if (!selectedFilePath.isEmpty()) {
		pathSource = selectedFilePath;
		setWindowTitle(pathSource + " - AudioSplitter");
		chart->clear();
		ui.textEdit->clear();
		ui.textEdit->append("<b>SOURCE: " + pathSource + "</b>");
	}
}

void AudioSplitter::onSettings()
{
	SettingsDlg dlg(pathFfmpeg, pathFfplay, pathOutput, pathBell, origName, namePrefix, numOfDigits, codeOutput, timeSilence, this);
	
	if (dlg.exec() == QDialog::Accepted) {
		pathFfmpeg = dlg.pathFfmpeg;
		pathFfplay = dlg.pathFfplay;
		pathOutput = dlg.pathOutput;
		pathBell = dlg.pathBell;
		origName = dlg.origName;
		namePrefix = dlg.namePrefix;
		numOfDigits = dlg.numOfDigits;
		codeOutput = dlg.codeOutput;
		timeSilence = dlg.timeSilence;
	}
}

QString quotePath(const QString &path)
{
	return "\"" + path + "\"";
}

void AudioSplitter::onPlay()
{
	// play
	if (state == State::PLAY)
		onStop();
	if (state != State::IDLE)
		return;
	if (pathSource.isEmpty()) {
		QMessageBox::information(0, "AudioSplitter", "File not loaded");
		return;
	}

	state = State::PLAY;
	double t = 0;
	int i = chart->currentIndex();
	if (i >= 0)
		t = chart->getValue(i);

	// ffplay c:/MyNotes/ABooks/sm.m4b -ss 44.0 -nodisp
	
	QString args;
	args += quotePath(pathSource);
	args += " -nodisp";
	args += " -loglevel warning";
	args += QString::asprintf(" -ss %.5f", t);

	ui.textEdit->clear();
	ui.textEdit->append("<b>FFPLAY: " + pathFfplay + "</b>");
	ui.textEdit->append("<b>ARGS: " + args + "</b>");

	ffmpegProcess.setProgram(pathFfplay);
	ffmpegProcess.setNativeArguments(args);
	ffmpegProcess.start();
}

void AudioSplitter::onStop()
{
	// stop
	if (state == State::IDLE)
		return;
	ffmpegProcess.kill();
	while (!ffmpegProcess.waitForFinished()) {
		QThread::msleep(30);
	}
	state = State::IDLE;
	ui.textEdit->append("<b>Kill process</b>");
}

void AudioSplitter::onScan()
{
	// start analyse
	if (state != State::IDLE)
		return;
	if (pathSource.isEmpty()) {
		QMessageBox::information(0, "AudioSplitter", "File not loaded");
		return;
	}
	state = State::SCAN;

	// ffplay c:/MyNotes/ABooks/sm.m4b -ss 44.0 -nodisp
	
	QString args;
	args += " -i ";
	args += quotePath(pathSource);
	args += QString::asprintf(" -af silencedetect=d=%.5f", timeSilence);
	args += " -f null - ";

	chart->clear();
	ui.textEdit->clear();
	ui.textEdit->append("<b>FFMPEG: " + pathFfmpeg  + "</b>");
	ui.textEdit->append("<b>ARGS: " + args + "</b>");

	ffmpegProcess.setProgram(pathFfmpeg);
	ffmpegProcess.setNativeArguments(args);
	ffmpegProcess.start();
}

void AudioSplitter::onConv()
{
	// start conversion
	if (state != State::IDLE)
		return;
	if (pathSource.isEmpty()) {
		QMessageBox::information(0, "AudioSplitter", "File not loaded");
		return;
	}

	int last_i = -1;
	for (int i = 0, n = chart->size(); i < n; i++) {
		if (chart->getState(i) & 1)
			last_i = i;
	}
	if (last_i < 0) {
		QMessageBox::information(0, "AudioSplitter", "Fragments not selected");
		return;
	}

	state = State::CONV;
	// ffmpeg -i input.mp3 -f segment -segment_times 1.20837,1.92546,3.51778,4.0881,6.40315,7.7922 -reset_timestamps 1 -map 0:a -c:a copy output_%03d.mp3
	QString args;
	args += " -i ";
	args += quotePath(pathSource);
	args += " -f segment -map_metadata -1 -segment_times ";	

	for (int i = 0, n = chart->size(); i < n; i++) {
		if (chart->getState(i) & 1) {
			double silence_middle = (chart->getValue(i) + chart->getValue(i - 1)) / 2;
			args += QString::asprintf("%.5f", silence_middle);
			if (i != last_i)
				args += ",";
		}
	}

	QString fmt;
	if (origName) {
		QFileInfo fi(pathSource);
		fmt = fi.baseName();
	}
	fmt += namePrefix + "%0" + QString::number(numOfDigits) + "d.mp3";

	args += " -reset_timestamps 1 ";
	if (codeOutput == 2)
		args += quotePath(pathOutput + "\\" + fmt);
	else if (codeOutput == 1)
		args += quotePath(QFileInfo(pathSource).absolutePath() + "\\" + fmt);
	else
		args += fmt;
			
	ui.textEdit->clear();
	ui.textEdit->append("<b>FFMPEG: " + pathFfmpeg + "</b>");
	ui.textEdit->append("<b>ARGS: " + args + "</b>");

	ffmpegProcess.setProgram(pathFfmpeg);
	ffmpegProcess.setNativeArguments(args);
	ffmpegProcess.start();
}

void AudioSplitter::onTags()
{
	QString selectedDirPath = QFileDialog::getExistingDirectory(this, 
		"Select directory with audio files", pathOutput, QFileDialog::ShowDirsOnly);
	   
	ui.textEdit->clear();
	ui.textEdit->append("<b>MAKE TAGS: " + selectedDirPath + "</b>");
	
	if (!selectedDirPath.isEmpty()) {
		QDirIterator it(selectedDirPath, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);

		while (it.hasNext()) {
			QString filePath = it.next();
			if(gen(filePath.toLocal8Bit().constData()))
				ui.textEdit->append("<font color=blue>tags changed: " + filePath + "</font>");
			else
				ui.textEdit->append("<font color=red>TAGS ERROR: " + filePath + "</font>");
			QCoreApplication::processEvents();
		}
		ui.textEdit->append("<b>Done!</b>");
	}
}

void AudioSplitter::onBell()
{
	QString selectedDirPath = QFileDialog::getExistingDirectory(this,
		"Select directory with audio files", pathOutput, QFileDialog::ShowDirsOnly);

	ui.textEdit->clear();
	ui.textEdit->append("<b>MAKE BELLS: " + selectedDirPath + "</b>");

	if (!QFile::exists(pathBell)) {
		ui.textEdit->append("<font color=red>BELL FILE NOT FOUND: " + pathBell + "</font>");
		return;
	}

	if (!selectedDirPath.isEmpty()) {
		QDirIterator it(selectedDirPath, QStringList() << "*.mp3", QDir::Files, QDirIterator::Subdirectories);

		while (it.hasNext()) {
			QFileInfo fi = it.fileInfo();
			QString path = fi.absolutePath() + fi.baseName() + "i.mp3";
			if (QFile::copy(pathBell, path)) 
				ui.textEdit->append("<font color=blue>file copied: " + path + "</font>");
			else 
				ui.textEdit->append("<font color=red>COPY ERROR: " + path + "</font>");
			QCoreApplication::processEvents();
			it.next();
		}
		ui.textEdit->append("<b>Done!</b>");
	}
}

void AudioSplitter::onReadStdOut()
{
	QString output = ffmpegProcess.readAllStandardOutput();
	ui.textEdit->append(output);
	if(state == State::SCAN)
		handleOut(output);	
}

void AudioSplitter::onReadStdErr()
{
	QString error = ffmpegProcess.readAllStandardError();
	ui.textEdit->append("<font color=red>" + error + "</font>");
	if (state == State::SCAN)
		handleOut(error);	
}

void AudioSplitter::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
		ui.textEdit->append("<b>Process completed successfully!</b>");
	}
	else {
		ui.textEdit->append("<b>Process failed with exit code " + QString::number(exitCode) + "</b>");
	}
	state = State::IDLE;

	chart->update();
}

void AudioSplitter::onDoubleClicked(int index)
{
	if (index >= 0) {
		onPlay();
	}
}

void AudioSplitter::handleOut(const QString &str)
{
	// one string can contain some 'start' and 'end' tags
	int i = 0;
	bool ok;
	double d;

	while ((i = str.indexOf("silence_", i)) >= 0) {
		i += 8;
		if (str.midRef(i, 7) == "start: ") {
			i += 7;
			QStringRef r = str.midRef(i);
			const QChar*p = r.constData();
			d = _wtof((wchar_t*)p);
			ui.textEdit->append("<font color=blue>##start: " + QString::number(d)+"</font>");
			chart->append(d);
		}
		else if (str.midRef(i, 5) == "end: ") {
			i += 5;
			QStringRef r = str.midRef(i);
			const QChar*p = r.constData();
			d = _wtof((wchar_t*)p);
			ui.textEdit->append("<font color=blue>##end: " + QString::number(d) + "</font>");
			chart->append(d);
		}
	}
}


