/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_TRACKER_H
#define QWT_PLOT_TRACKER_H

#include "qwt_global.h"
#include "qwt_picker.h"

#include <QLayoutItem>
#include <QVariant>

class QwtPlot;
class QwtPlotItem;
class QVariant;

class QBrush;
class QPen;
class QwtDynGridLayout;

template< typename T > class QList;

class QWT_EXPORT QwtPlotTracker : public QwtPicker
{
    Q_OBJECT

    Q_PROPERTY( double borderRadius READ borderRadius WRITE setBorderRadius )
    Q_PROPERTY( QPen borderPen READ borderPen WRITE setBorderPen )
    Q_PROPERTY( QBrush backgroundBrush READ backgroundBrush WRITE setBackgroundBrush )

  public:
    enum TrackerAttribute
    {
        PositionInfo = 0x01,
        ItemInfo     = 0x02
    };

    Q_DECLARE_FLAGS( TrackerAttributes, TrackerAttribute )

    explicit QwtPlotTracker( QWidget* );
    virtual ~QwtPlotTracker() override;

    QwtPlot* plot();
    const QwtPlot* plot() const;

    QWidget* canvas();
    const QWidget* canvas() const;

    void setTrackerAttribute( TrackerAttribute, bool on = true );
    bool testTrackerAttribute( TrackerAttribute ) const;

    void setMaxColumns( uint numColums );
    uint maxColumns() const;

    void setBorderRadius( double );
    double borderRadius() const;

    void setBorderPen( const QPen& );
    QPen borderPen() const;

    void setBackgroundBrush( const QBrush& );
    QBrush backgroundBrush() const;

    void setTitleFont( const QFont& );
    QFont titleFont() const;

    virtual QwtText title() const;

    virtual QRegion trackerMask() const override;
    virtual QwtText trackerText( const QPoint& ) const override;

    virtual void drawTracker( QPainter* ) const override;

    virtual int heightForWidth( const QwtPlotItem*,
        const QList< QVariant >&, int width ) const;

    virtual QSize minimumSize(
        const QwtPlotItem*, const QList< QVariant >& ) const;

    virtual QList< QVariant > trackerDataAt( const QwtPlotItem*, const QPointF& ) const;

  protected:


    class LayoutItem  : public QLayoutItem
    {
      public:
        LayoutItem( const QwtPlotTracker* tracker)
            : m_tracker( tracker )
            , m_plotItem( nullptr)
        {}

        virtual Qt::Orientations expandingDirections() const override { return Qt::Horizontal; }
        virtual QRect geometry() const override { return m_rect; }
        virtual bool hasHeightForWidth() const override { return true; }
        virtual int heightForWidth( int width) const override { return m_tracker->heightForWidth( m_plotItem, m_trackerData, width ); }
        virtual bool isEmpty() const override { return false; }
        virtual QSize maximumSize() const override { return QSize( QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX ); }
        virtual int minimumHeightForWidth( int width) const override { return m_tracker->heightForWidth( m_plotItem, m_trackerData, width ); }
        virtual QSize minimumSize() const override { return m_tracker->minimumSize( m_plotItem, m_trackerData ); }
        virtual void setGeometry( const QRect& rect ) override { m_rect = rect; }
        virtual QSize sizeHint() const override { return minimumSize(); }

        void setTrackerData( const QList< QVariant >& trackerData) { m_trackerData = trackerData; }
        const QList< QVariant >& trackerData() const { return m_trackerData; }

        void setPlotItem( const QwtPlotItem* plotItem) { m_plotItem = plotItem; }
        const QwtPlotItem* plotItem() const { return m_plotItem; }

      private:
        const QwtPlotTracker* m_tracker;

        const QwtPlotItem* m_plotItem;
        QList< QVariant > m_trackerData;

        QRect m_rect;
    };


    virtual void drawTrackerData( QPainter*, const QwtPlotItem*,
        const QList< QVariant >&, const QRect& ) const;
    virtual void updateLayoutItems(const QPointF& trackerPosition, QwtDynGridLayout& layout) const;



  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
