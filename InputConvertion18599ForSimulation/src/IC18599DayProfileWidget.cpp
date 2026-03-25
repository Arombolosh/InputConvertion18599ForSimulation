#include "IC18599DayProfileWidget.h"

#include <QPainter>
#include <QMouseEvent>

static const char* DAY_NAMES[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

IC18599DayProfileWidget::IC18599DayProfileWidget(QWidget *parent) :
	QWidget(parent),
	m_weekValues(168, 0.0),
	m_yAxisLabel("%"),
	m_barColor(70, 130, 180)
{
	setMouseTracking(true);
}


void IC18599DayProfileWidget::setWeekValues(const std::vector<double> &vals) {
	m_weekValues = vals;
	m_weekValues.resize(168, 0.0);
	update();
}


QRectF IC18599DayProfileWidget::plotArea() const {
	int mt = marginTop();
	return QRectF(MARGIN_LEFT, mt,
				  width() - MARGIN_LEFT - MARGIN_RIGHT,
				  height() - mt - MARGIN_BOTTOM);
}


int IC18599DayProfileWidget::hourFromPos(const QPointF &pos) const {
	QRectF pa = plotArea();
	if (!pa.contains(pos))
		return -1;
	double barW = pa.width() / 168.0;
	int h = (int)((pos.x() - pa.left()) / barW);
	return std::max(0, std::min(167, h));
}


double IC18599DayProfileWidget::pctFromPos(const QPointF &pos) const {
	QRectF pa = plotArea();
	double range = m_maxValue - m_minValue;
	double val = m_minValue + range * (pa.bottom() - pos.y()) / pa.height();
	return std::max(m_minValue, std::min(m_maxValue, val));
}


void IC18599DayProfileWidget::paintEvent(QPaintEvent * /*event*/) {
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, false);

	QRectF pa = plotArea();

	// Background
	p.fillRect(rect(), palette().window());
	p.fillRect(pa.toRect(), QColor(250, 250, 250));

	// Title above plot area
	if (!m_title.isEmpty()) {
		QFont titleFont(font().family(), 9, QFont::Bold);
		p.setFont(titleFont);
		p.setPen(Qt::black);
		p.drawText(MARGIN_LEFT, 2, width() - MARGIN_LEFT - MARGIN_RIGHT, 16,
				   Qt::AlignLeft | Qt::AlignTop, m_title);
	}

	// Day backgrounds (alternating)
	double dayW = pa.width() / 7.0;
	for (int d = 0; d < 7; ++d) {
		bool selected = false;
		for (int sd : m_selectedDays) {
			if (sd == d) { selected = true; break; }
		}
		QRectF dayRect(pa.left() + d * dayW, pa.top(), dayW, pa.height());
		if (selected) {
			p.fillRect(dayRect.toRect(), QColor(220, 235, 255));
		}
		else if (d >= 5) {
			p.fillRect(dayRect.toRect(), QColor(255, 245, 240));
		}
		else if (d % 2 == 1) {
			p.fillRect(dayRect.toRect(), QColor(245, 245, 245));
		}
	}

	// Grid lines Y
	QFont smallFont = font();
	smallFont.setPointSize(8);
	p.setFont(smallFont);

	double range = m_maxValue - m_minValue;
	int numGridLines = 5;
	for (int i = 0; i <= numGridLines; ++i) {
		double val = m_minValue + range * i / numGridLines;
		double y = pa.bottom() - pa.height() * i / numGridLines;
		p.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
		p.drawLine(QPointF(pa.left(), y), QPointF(pa.right(), y));
		p.setPen(Qt::black);
		QString label = (range <= 2.0) ? QString::number(val, 'f', 1) : QString::number((int)val);
		p.drawText(QRectF(0, y - 8, MARGIN_LEFT - 5, 16),
				   Qt::AlignRight | Qt::AlignVCenter, label);
	}

	// Day separator lines
	p.setPen(QPen(QColor(180, 180, 180), 1, Qt::SolidLine));
	for (int d = 1; d < 7; ++d) {
		double x = pa.left() + d * dayW;
		p.drawLine(QPointF(x, pa.top()), QPointF(x, pa.bottom()));
	}

	// Bars (168 total)
	double barW = pa.width() / 168.0;
	for (int h = 0; h < 168; ++h) {
		double val = std::max(m_minValue, std::min(m_maxValue, m_weekValues[(size_t)h]));
		double fraction = (val - m_minValue) / range;
		if (fraction <= 0)
			continue;
		double barH = pa.height() * fraction;
		double x = pa.left() + h * barW;
		double y = pa.bottom() - barH;

		QColor col = m_barColor;
		// Dim bars not in selected days
		int day = h / 24;
		bool daySelected = false;
		for (int sd : m_selectedDays) {
			if (sd == day) { daySelected = true; break; }
		}
		if (!m_selectedDays.empty() && !daySelected)
			col = col.lighter(150);

		p.setPen(Qt::NoPen);
		p.setBrush(col);
		p.drawRect(QRectF(x, y, std::max(barW - 0.5, 1.0), barH));
	}

	// Day labels at bottom
	p.setPen(Qt::black);
	QFont dayFont = font();
	dayFont.setPointSize(9);
	dayFont.setBold(true);
	p.setFont(dayFont);
	for (int d = 0; d < 7; ++d) {
		double x = pa.left() + d * dayW;
		p.drawText(QRectF(x, pa.bottom() + 2, dayW, 20),
				   Qt::AlignCenter, DAY_NAMES[d]);
	}

	// Frame
	p.setPen(QPen(Qt::black, 1));
	p.setBrush(Qt::NoBrush);
	p.drawRect(pa);
}


void IC18599DayProfileWidget::mousePressEvent(QMouseEvent *event) {
	if (m_readOnly)
		return;
	int h = hourFromPos(event->position());
	if (h < 0)
		return;

	double pct = pctFromPos(event->position());
	pct = std::round(pct);
	pct = std::max(m_minValue, std::min(m_maxValue, pct));
	m_weekValues[(size_t)h] = pct;
	m_selectedDays.clear();
	m_selectedDays.push_back(h / 24);
	update();
	emit barClicked(h, pct);
}


void IC18599DayProfileWidget::mouseMoveEvent(QMouseEvent *event) {
	if (m_readOnly)
		return;
	if (!(event->buttons() & Qt::LeftButton))
		return;

	int h = hourFromPos(event->position());
	if (h < 0)
		return;

	double pct = pctFromPos(event->position());
	pct = std::round(pct);
	pct = std::max(m_minValue, std::min(m_maxValue, pct));
	m_weekValues[(size_t)h] = pct;
	update();
	emit barClicked(h, pct);
}
