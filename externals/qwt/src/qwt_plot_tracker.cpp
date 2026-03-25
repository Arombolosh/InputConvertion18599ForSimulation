/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_tracker.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_graphic.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_picker_machine.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qvariant.h>
#include <qlayoutitem.h>
#include <qpainter.h>
#include <QtGlobal>


static inline QwtText qwtText( const QList< QVariant >& values )
{
    QwtText text;

    for ( int i = 0; i < values.size(); i++ )
    {
        const QVariant& v = values[i];

        if ( v.canConvert< QwtText >() )
            return qvariant_cast< QwtText >( v );

        if ( v.canConvert< QString >() )
            return qvariant_cast< QString >( v );
    }

    return QwtText();
}

static inline QwtGraphic qwtGraphic( const QList< QVariant >& values )
{
    for ( int i = 0; i < values.size(); i++ )
    {
        const QVariant& v = values[i];

        if ( v.canConvert< QwtGraphic >() )
            return qvariant_cast< QwtGraphic >( v );
    }

    return QwtGraphic();
}



class Layout : public QwtDynGridLayout
{
  public:
    Layout()
    {
        setMaxColumns( 2 );
        setSpacing( 0 );
        setContentsMargins( 0, 0, 0, 0 );
    }

    ~Layout() override
    {
        clear();
    }

    void clear()
    {
        for ( int i = count() - 1; i >= 0; i-- )
            delete takeAt( i );
    }

};

class QwtPlotTracker::PrivateData
{
  public:
    PrivateData()
        : borderRadius( 0 )
        , borderPen( Qt::NoPen )
        , itemMargin( 4 )
        , itemSpacing( 4 )
        , trackerAttributes( PositionInfo )
    {
    }

    double borderRadius;
    QPen borderPen;

    QBrush backgroundBrush;

    QFont titleFont;

    int itemMargin;
    int itemSpacing;

    TrackerAttributes trackerAttributes;
    Layout layout;
};

QwtPlotTracker::QwtPlotTracker( QWidget* canvas )
    : QwtPicker( canvas )
{
    m_data = new PrivateData;

    setStateMachine( new QwtPickerTrackerMachine() );
    setTrackerMode( AlwaysOn );
}

QwtPlotTracker::~QwtPlotTracker()
{
    delete m_data;
}

//! \return Observed plot canvas
QWidget* QwtPlotTracker::canvas()
{
    return parentWidget();
}

//! \return Observed plot canvas
const QWidget* QwtPlotTracker::canvas() const
{
    return parentWidget();
}

//! \return Plot widget, containing the observed plot canvas
QwtPlot* QwtPlotTracker::plot()
{
    QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< QwtPlot* >( w );
}

//! \return Plot widget, containing the observed plot canvas
const QwtPlot* QwtPlotTracker::plot() const
{
    const QWidget* w = canvas();
    if ( w )
        w = w->parentWidget();

    return qobject_cast< const QwtPlot* >( w );
}

void QwtPlotTracker::setMaxColumns( uint numColums )
{
    m_data->layout.setMaxColumns( numColums );
}

uint QwtPlotTracker::maxColumns() const
{
    return m_data->layout.maxColumns();
}

void QwtPlotTracker::setTrackerAttribute( TrackerAttribute attribute, bool on )
{
    if ( on )
        m_data->trackerAttributes |= attribute;
    else
        m_data->trackerAttributes &= ~attribute;
}

bool QwtPlotTracker::testTrackerAttribute( TrackerAttribute attribute ) const
{
    return m_data->trackerAttributes.testFlag( attribute );
}

void QwtPlotTracker::setBorderRadius( double radius )
{
    m_data->borderRadius = std::max( 0.0, radius );
}

double QwtPlotTracker::borderRadius() const
{
    return m_data->borderRadius;
}

void QwtPlotTracker::setBorderPen( const QPen& pen )
{
    m_data->borderPen = pen;
}

QPen QwtPlotTracker::borderPen() const
{
    return m_data->borderPen;
}

void QwtPlotTracker::setBackgroundBrush( const QBrush& brush )
{
    m_data->backgroundBrush = brush;
}

QBrush QwtPlotTracker::backgroundBrush() const
{
    return m_data->backgroundBrush;
}

QList< QVariant > QwtPlotTracker::trackerDataAt(
     const QwtPlotItem* plotItem, const QPointF& pos ) const
{
    return plotItem->trackerData( 0, pos );
}

void QwtPlotTracker::drawTracker( QPainter* painter ) const
{
    const Layout& layout = m_data->layout;

    const QwtText title = this->title();

    int titleHeight = 0.0;
    if ( !title.isEmpty() )
    {
        titleHeight = QFontMetricsF( titleFont() ).height();
        titleHeight += 2 * m_data->itemMargin;
    }

    const QRect rect = layout.geometry();

    QRect backgroundRect = rect;
    backgroundRect.setY( rect.y() - titleHeight );

    if ( backgroundRect.isEmpty() )
        return;

    if ( m_data->borderPen != Qt::NoPen
        || m_data->backgroundBrush.style() != Qt::NoBrush )
    {
        // expanding rect by borderPen/borderRadius. TODO ...

        painter->save();

        painter->setPen( m_data->borderPen );
        painter->setBrush( m_data->backgroundBrush );

        if ( m_data->borderRadius == 0. )
        {
            QwtPainter::drawRect( painter, backgroundRect );
        }
        else
        {
            painter->setRenderHint( QPainter::Antialiasing, true );
            painter->drawRoundedRect( backgroundRect,
                m_data->borderRadius, m_data->borderRadius );
        }

        painter->restore();
    }

    if ( titleHeight > 0 )
    {
        painter->save();

        painter->setPen( trackerPen() );
        painter->setFont( titleFont() );

        QRect titleRect = backgroundRect;
        titleRect.setHeight( titleHeight );

        const int m = m_data->itemMargin;
        titleRect.adjust( m, m, -m, -m );

        title.draw( painter, titleRect );

        painter->restore();
    }

    if ( !rect.isEmpty() )
    {
        for ( int i = 0; i < layout.count(); i++ )
        {
            const LayoutItem* layoutItem =
                static_cast< LayoutItem* >( layout.itemAt( i ) );

            painter->save();

            drawTrackerData( painter, layoutItem->plotItem(),
                layoutItem->trackerData(), layoutItem->geometry() );

            painter->restore();
        }
    }
}

void QwtPlotTracker::drawTrackerData( QPainter* painter,
    const QwtPlotItem* plotItem, const QList< QVariant >& data,
    const QRect& rect ) const
{
    Q_UNUSED( plotItem )

    const int m = m_data->itemMargin;

    const QRect r = rect.adjusted( m, m, -m, -m );

    painter->setClipRect( r, Qt::IntersectClip );

    int titleOff = 0;

    const QwtGraphic graphic = qwtGraphic( data );
    if ( !graphic.isEmpty() )
    {
        QRectF iconRect( r.topLeft(), graphic.defaultSize() );

        iconRect.moveCenter(
            QPoint( iconRect.center().x(), rect.center().y() ) );

        graphic.render( painter, iconRect, Qt::KeepAspectRatio );

        titleOff += iconRect.width() + m_data->itemSpacing;
    }

    const QwtText text = qwtText( data );
    if ( !text.isEmpty() )
    {
        painter->setPen( trackerPen() );
        painter->setFont( trackerFont() );

        const QRect textRect = r.adjusted( titleOff, 0, 0, 0 );
        text.draw( painter, textRect );
    }
}


void QwtPlotTracker::updateLayoutItems(const QPointF &trackerPosition, QwtDynGridLayout &layout) const {
    const QwtPlot* plt = plot();
    if ( plt == nullptr )
    {
        // just a precaution: this is normally not necessary, as without plot, this function won't be called at all
        for ( int i = layout.count() - 1; i >= 0; i-- )
            delete layout.takeAt( i );
        return;
    }

    int index = 0;

    double pos[ QwtPlot::axisCnt ];

    {

        const double x = trackerPosition.x();
        const double y = trackerPosition.y();

        pos[ QwtPlot::yLeft ] = plt->canvasMap( QwtPlot::yLeft ).invTransform( y );
        pos[ QwtPlot::yRight ] = plt->canvasMap( QwtPlot::yRight ).invTransform( y );
        pos[ QwtPlot::xBottom ] = plt->canvasMap( QwtPlot::xBottom ).invTransform( x );
        pos[ QwtPlot::xTop ] = plt->canvasMap( QwtPlot::xTop ).invTransform( x );
    }

    const QwtPlotItemList& items = plt->itemList();
    for ( QwtPlotItemIterator it = items.begin(); it != items.end(); ++it )
    {
        const QwtPlotItem* plotItem = *it;
        if ( plotItem->testItemAttribute( QwtPlotItem::Tracker ) )
        {
            const QPointF trackerPos(
                pos[ plotItem->xAxis() ], pos[ plotItem->yAxis() ] );

            const QList< QVariant > data
                = trackerDataAt( plotItem, trackerPos );

            if ( !data.isEmpty() )
            {
                LayoutItem* layoutItem = nullptr;
                if ( index < layout.count() )
                {
                    layoutItem = static_cast< LayoutItem* >( layout.itemAt( index ) );
                }
                else
                {
                    layoutItem = new LayoutItem( this );
                    layout.addItem( layoutItem );
                }

                layoutItem->setPlotItem( plotItem );
                layoutItem->setTrackerData( data );

                index++;
            }
        }
    }

    for ( int i = layout.count() - 1; i >= index; i-- )
        delete layout.takeAt( i );

    layout.invalidate();
    layout.activate();
}

void QwtPlotTracker::setTitleFont( const QFont& font )
{
    m_data->titleFont = font;
}

QFont QwtPlotTracker::titleFont() const
{
    return m_data->titleFont;
}

QRegion QwtPlotTracker::trackerMask() const
{
    Layout& layout = m_data->layout;


    layout.clear();
    updateLayoutItems( trackerPosition(), layout );

    const QSize size = m_data->layout.sizeHint();

    const QwtText title = this->title();

    int titleHeight = 0.0;
    int titleWidth = 0.0;
    if ( !title.isEmpty() )
    {
        QRectF titleRect = QFontMetricsF( titleFont() ).boundingRect(title.text());
        titleHeight = qwtCeil(titleRect.height() + 2 * m_data->itemMargin);
        titleWidth = qwtCeil(titleRect.width() + 2 * m_data->itemMargin);
    }

    const QRect rect = trackerRect( QSize( std::max(size.width(), titleWidth), size.height() + titleHeight ) );

    const QRect layoutRect( rect.x(), rect.y() + titleHeight, rect.width(), rect.height() -  titleHeight );
    layout.setGeometry( layoutRect );

    // expanding rect by borderPen/borderRadius. TODO ...
    return rect;
}

QwtText QwtPlotTracker::title() const
{
    return QwtText();
}

QwtText QwtPlotTracker::trackerText( const QPoint& pos ) const
{
    return QwtPicker::trackerText( pos );
}

int QwtPlotTracker::heightForWidth( const QwtPlotItem* plotItem,
    const QList< QVariant >& data, int width ) const
{
    Q_UNUSED( plotItem );

    width -= 2 * m_data->itemMargin;

    const QwtGraphic graphic = qwtGraphic( data );
    const QwtText text = qwtText( data );

    if ( text.isEmpty() )
        return graphic.height();

    if ( graphic.width() > 0 )
        width -= graphic.width() + m_data->itemSpacing;

    int h = text.heightForWidth( width, trackerFont() );
    h += 2 * m_data->itemMargin;

    return qMax( graphic.height(), h );
}

QSize QwtPlotTracker::minimumSize( const QwtPlotItem* plotItem,
    const QList< QVariant >& data ) const
{
    Q_UNUSED( plotItem );

    QSize size( 2 * m_data->itemMargin, 2 * m_data->itemMargin );

    const QwtGraphic graphic = qwtGraphic( data );
    QwtText text = qwtText( data );

    int w = 0;
    int h = 0;

    if ( !graphic.isNull() )
    {
        w = graphic.width();
        h = graphic.height();
    }

    if ( !text.isEmpty() )
    {
        QFont f(trackerFont());
        if (text.font().bold()) {
            f.setBold(true);
            text.setFont(f);
        }
        const QSizeF sz = text.textSize( f );

        w += std::ceil( sz.width() );
        h = qMax<int>( h, std::ceil( sz.height() ) );
    }

    if ( graphic.width() > 0 && !text.isEmpty() )
        w += m_data->itemSpacing;

    size += QSize( w, h );
    return size;
}

#include "moc_qwt_plot_tracker.cpp"

