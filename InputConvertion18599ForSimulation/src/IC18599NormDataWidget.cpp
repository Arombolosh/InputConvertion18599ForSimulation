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

	// First row is header (Parameter; Zone1; Zone2; ...)
	QStringList headerRow = data.first();

	// Rows to hide from display (derived values computed from other rows)
	QStringList hiddenParams = {
		"Personenlast [W/m2]",
		"Arbeitshilfen (Geräte) [W/m2]"
	};

	// Collect display rows (skip hidden parameters)
	QVector<int> displayRows;  // indices into data (1-based, skipping header)
	for (int r = 1; r < data.size(); ++r) {
		QString paramName = data[r].isEmpty() ? QString() : data[r][0].trimmed();
		if (!hiddenParams.contains(paramName))
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
	QStringList names;
	for (int c = 1; c < m_tableWidget->columnCount(); ++c) {
		QTableWidgetItem *item = m_tableWidget->horizontalHeaderItem(c);
		if (item && !item->text().trimmed().isEmpty())
			names << item->text().trimmed();
	}
	return names;
}


double IC18599NormDataWidget::getProfileValue(const QString &profileName, const QString &paramName) const {
	// Find the column for this profile
	int profCol = -1;
	for (int c = 1; c < m_tableWidget->columnCount(); ++c) {
		QTableWidgetItem *hdr = m_tableWidget->horizontalHeaderItem(c);
		if (hdr && hdr->text().trimmed() == profileName) {
			profCol = c;
			break;
		}
	}
	if (profCol < 0)
		return 0.0;

	// Find the row with this parameter name
	for (int r = 0; r < m_tableWidget->rowCount(); ++r) {
		QTableWidgetItem *item = m_tableWidget->item(r, 0);
		if (item && item->text().trimmed() == paramName) {
			QTableWidgetItem *valItem = m_tableWidget->item(r, profCol);
			if (valItem) {
				bool ok;
				double val = valItem->text().toDouble(&ok);
				if (ok)
					return val;
			}
			return 0.0;
		}
	}
	return 0.0;
}
