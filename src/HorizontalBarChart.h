#pragma once

#include <QWidget>


class HorizontalBarChart :
	public QWidget
{
	Q_OBJECT

	struct Data {
		double v;
		unsigned int st;
	};
public:
	static const int hdrHeight = 20;
	static const int barHeight = 20;
public:
	HorizontalBarChart(QWidget *parent);
	void update();
	void clear();
	void append(double v);
	int size();
	double getValue(int i);
	unsigned int getState(int i);
	int currentIndex();
signals:
	int  doubleClicked(int index);
protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseDoubleClickEvent(QMouseEvent * e) override;

private:
	int getIndexByMouse(QMouseEvent *event);
	void drawDuration(QPainter *painter, int x, int y, int t);
private:
	QVector<Data> m_values;

	int m_lastY;
	int m_index = -1;
	double m_totalWidths[2];
};

