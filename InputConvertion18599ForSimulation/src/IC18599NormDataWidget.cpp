#include "IC18599NormDataWidget.h"

#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

IC18599NormDataWidget::IC18599NormDataWidget(QWidget *parent) :
	QWidget(parent)
{
	setupUI();
}


void IC18599NormDataWidget::setupUI() {
	QVBoxLayout *layout = new QVBoxLayout(this);

	m_infoLabel = new QLabel(tr("Please load a CSV file with DIN 18599 norm data (File > Load Norm Data)."), this);
	layout->addWidget(m_infoLabel);

	m_tableWidget = new QTableWidget(this);
	m_tableWidget->setAlternatingRowColors(true);
	m_tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);  // read-only
	m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_tableWidget->verticalHeader()->setVisible(false);
	layout->addWidget(m_tableWidget);
}


bool IC18599NormDataWidget::loadCSV(const QString &fname) {
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream in(&file);
	in.setEncoding(QStringConverter::Utf8);

	QVector<QStringList> data;
	int maxCols = 0;

	while (!in.atEnd()) {
		QString line = in.readLine();
		QStringList fields = line.split(';');
		if (fields.size() > maxCols)
			maxCols = fields.size();
		data.append(fields);
	}
	file.close();

	if (data.isEmpty())
		return false;

	// Store raw data for row-index based access
	m_rawData = data;

	// First row is header (Parameter; Zone1; Zone2; ...)
	QStringList headerRow = data.first();

	// Hide last 2 data rows (derived values: indices 40, 41 relative to first data row)
	QSet<int> hiddenDataRows = {40, 41};

	// Collect display rows (skip hidden parameters)
	QVector<int> displayRows;  // indices into data (1-based, skipping header)
	for (int r = 1; r < data.size(); ++r) {
		int dataRowIdx = r - 1;  // 0-based data row index
		if (!hiddenDataRows.contains(dataRowIdx))
			displayRows.append(r);
	}

	int numRows = displayRows.size();
	int numCols = maxCols;

	m_tableWidget->clear();
	m_tableWidget->setRowCount(numRows);
	m_tableWidget->setColumnCount(numCols);

	// Set horizontal header from first row, wrapping long names into multiple lines
	int maxLineCount = 1;
	QStringList hHeaders;
	for (int c = 0; c < numCols; ++c) {
		QString raw = (c < headerRow.size()) ? headerRow[c].trimmed() : QString();
		// Replace underscores with spaces in profile names
		if (c > 0)
			raw.replace('_', ' ');
		// Wrap long profile names at word boundaries (~22 chars per line)
		if (c > 0 && raw.length() > 24) {
			QString wrapped;
			int lineLen = 0;
			int lineCount = 1;
			for (int i = 0; i < raw.length(); ++i) {
				wrapped += raw[i];
				lineLen++;
				if (lineLen >= 22 && raw[i] == ' ' && i + 1 < raw.length()) {
					wrapped += '\n';
					lineLen = 0;
					lineCount++;
				}
			}
			if (lineCount > maxLineCount)
				maxLineCount = lineCount;
			hHeaders << wrapped;
		}
		else {
			hHeaders << raw;
		}
	}
	m_tableWidget->setHorizontalHeaderLabels(hHeaders);

	// Size header height to fit wrapped text
	int lineHeight = m_tableWidget->fontMetrics().height();
	m_tableWidget->horizontalHeader()->setFixedHeight(maxLineCount * lineHeight + 8);

	// Fill data rows
	for (int dr = 0; dr < displayRows.size(); ++dr) {
		const QStringList &fields = data[displayRows[dr]];
		for (int c = 0; c < numCols; ++c) {
			QString text;
			if (c < fields.size())
				text = fields[c].trimmed();

			QTableWidgetItem *item = new QTableWidgetItem(text);

			// Section headers (rows with empty data cells) get bold formatting
			bool isSectionHeader = (fields.size() <= 2) ||
				(c == 0 && fields.size() > 1 && fields[1].trimmed().isEmpty() && fields.size() > 2 && fields[2].trimmed().isEmpty());

			if (isSectionHeader && c == 0) {
				QFont f = item->font();
				f.setBold(true);
				item->setFont(f);
				item->setBackground(QColor(220, 230, 240));
			}
			else if (isSectionHeader) {
				item->setBackground(QColor(220, 230, 240));
			}

			m_tableWidget->setItem(dr, c, item);
		}
	}

	// Resize first column (parameter names) to content
	m_tableWidget->resizeColumnToContents(0);

	// Set a reasonable width for profile columns
	for (int c = 1; c < numCols; ++c) {
		m_tableWidget->setColumnWidth(c, 160);
	}

	m_infoLabel->setText(tr("DIN V 18599-10: %1 parameters, %2 usage profiles")
		.arg(numRows).arg(numCols - 1));

	return true;
}


QStringList IC18599NormDataWidget::profileNames() const {
	if (m_rawData.isEmpty())
		return {};
	QStringList names;
	const QStringList &header = m_rawData.first();
	for (int c = 1; c < header.size(); ++c) {
		QString name = header[c].trimmed();
		name.replace('_', ' ');
		name = name.simplified();
		if (!name.isEmpty())
			names << name;
	}
	return names;
}


int IC18599NormDataWidget::findProfileColumn(const QString &profileName) const {
	if (m_rawData.isEmpty())
		return -1;
	const QStringList &header = m_rawData.first();
	// Normalize the lookup name
	QString normLookup = profileName;
	normLookup.replace('_', ' ');
	normLookup.replace('\n', ' ');
	normLookup = normLookup.simplified();

	for (int c = 1; c < header.size(); ++c) {
		QString normHeader = header[c].trimmed();
		normHeader.replace('_', ' ');
		normHeader = normHeader.simplified();
		if (normHeader == normLookup)
			return c;
	}
	return -1;
}


QString IC18599NormDataWidget::getProfileStringByRow(const QString &profileName, int dataRowIndex) const {
	int profCol = findProfileColumn(profileName);
	if (profCol < 0)
		return {};

	// dataRowIndex is 0-based; row 0 in m_rawData is the header, so offset by 1
	int rawRow = dataRowIndex + 1;
	if (rawRow < 0 || rawRow >= m_rawData.size())
		return {};

	const QStringList &fields = m_rawData[rawRow];
	if (profCol < fields.size())
		return fields[profCol].trimmed();
	return {};
}


double IC18599NormDataWidget::getProfileValueByRow(const QString &profileName, int dataRowIndex) const {
	QString s = getProfileStringByRow(profileName, dataRowIndex);
	if (s.isEmpty())
		return 0.0;
	bool ok;
	double val = s.toDouble(&ok);
	return ok ? val : 0.0;
}
