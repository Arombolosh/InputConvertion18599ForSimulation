#ifndef SCI_BarLabelScaleDrawH
#define SCI_BarLabelScaleDrawH

#include <qwt_text.h>
#include <qwt_scale_draw.h>

namespace SCI {

class BarLabelScaleDraw: public QwtScaleDraw
{
public:
	BarLabelScaleDraw( Qt::Orientation orientation = Qt::Vertical)
	{
		setTickLength( QwtScaleDiv::MinorTick, 0 );
		setTickLength( QwtScaleDiv::MediumTick, 0 );
		setTickLength( QwtScaleDiv::MajorTick, 2 );

		enableComponent( QwtScaleDraw::Backbone, false );

		if ( orientation == Qt::Vertical )
		{
			setLabelRotation( -90.0 );
		}
		else
		{
			setLabelRotation( -20.0 );
		}

		setLabelAlignment( Qt::AlignLeft | Qt::AlignVCenter );
	}

	void setLabels(const QStringList &labels) {
		m_labels = labels;
	}

	virtual QwtText label( double value ) const {
		QwtText lbl;

		const int index = qRound( value );
		if ( index >= 0 && index < m_labels.size() ) {
			lbl = m_labels[ index ];
		}

		return lbl;
	}

private:
	QStringList m_labels;
};

} // end namespace

#endif // SCI_BarLabelScaleDrawH
