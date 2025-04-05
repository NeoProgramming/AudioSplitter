#include "HorizontalBarChart.h"
#include <QPainter>
#include <QMouseEvent>

HorizontalBarChart::HorizontalBarChart(QWidget *parent)
	: QWidget(parent)
{
}

void HorizontalBarChart::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
		
	int w = width()/2;
	int h = height();
	int n = m_values.size();

	// vertical lines
	painter.drawLine(w, 0, w, h);

	// Max widths
	m_totalWidths[0] = m_totalWidths[1] = 0;
	double p = 0;
	for (int i = 0; i < n; i++) {
		double c = m_values[i].v;
		m_totalWidths[i&1] = std::max(m_totalWidths[i&1], c-p);
		p = c;
	}

	// Draw bars
	m_lastY = hdrHeight;
	p = 0;
	painter.setPen(Qt::black);
	for (int i = 0; i < n; i++) {
		double c = m_values[i].v;
		if (i & 1) {
			// right
			painter.setBrush(i==m_index ? Qt::red : (m_values[i].st & 1) ? Qt::darkCyan : Qt::cyan);
			int scaledWidth = (c-p) * w / m_totalWidths[1];
			painter.drawRect(w, m_lastY, scaledWidth - 1, barHeight - 1);
			
			if (m_values[i].st & 1) {
				painter.setPen(Qt::blue);
				painter.drawLine(w, m_lastY+ hdrHeight/2, w*2, m_lastY + hdrHeight / 2);
				painter.setPen(Qt::yellow);
				painter.drawLine(w, m_lastY + hdrHeight / 2 - 1, w * 2, m_lastY + hdrHeight / 2 - 1);
				painter.drawLine(w, m_lastY + hdrHeight / 2 + 1, w * 2, m_lastY + hdrHeight / 2 + 1);
				painter.setPen(Qt::black);
			}

			// next line
			m_lastY += barHeight;
		}
		else {
			// left
			painter.setBrush(i == m_index ? Qt::red : Qt::green);
			int scaledWidth = (c-p) * w / m_totalWidths[0];
			painter.drawRect(0, m_lastY, scaledWidth - 1, barHeight - 1);
		}		
		p = c;
	}	

	// Draw Widths Info
	painter.drawText(0, 0, w, hdrHeight, 0, QString::asprintf(" Sounds: Max=%.5f s", m_totalWidths[0]));
	painter.drawText(w, 0, w, hdrHeight, 0, QString::asprintf(" Pauses: Max=%.5f s", m_totalWidths[1]));
}

void HorizontalBarChart::update()
{
	setFixedHeight(m_lastY);
	QWidget::update();
}

void HorizontalBarChart::clear()
{
	m_values.clear();
	m_index = -1;
	update();
}

void HorizontalBarChart::append(double v)
{
	Data d;
	d.v = v;
	d.st = 0;
	m_values.push_back(d);
	update();
}

int HorizontalBarChart::size()
{
	return m_values.size();
}

double HorizontalBarChart::getValue(int i)
{
	return m_values[i].v;
}

unsigned int HorizontalBarChart::getState(int i)
{
	return m_values[i].st;
}

int HorizontalBarChart::currentIndex()
{
	return m_index;
}

int HorizontalBarChart::getIndexByMouse(QMouseEvent *event)
{
	QPoint pt = event->pos();

	int x = pt.x();
	int y = pt.y();
	int w = width() / 2;
	int i = (y - hdrHeight) / barHeight * 2 + (x > w);
	if (i >= m_values.size())
		i = -1;
	if (y < hdrHeight)
		i = -1;
	return i;
}

void HorizontalBarChart::mousePressEvent(QMouseEvent *event)
{
	int i = getIndexByMouse(event);	
	if (event->button() == Qt::LeftButton) {
		m_index = i;
	}
	else if (event->button() == Qt::RightButton) {
		if ((i >= 0) && (i & 1))
			m_values[i].st ^= 1;
	}
		
	QWidget::update();
}

void HorizontalBarChart::mouseDoubleClickEvent(QMouseEvent * event)
{
	int i = getIndexByMouse(event);
	if (event->button() == Qt::LeftButton) {
		emit doubleClicked(i);
	}
}
