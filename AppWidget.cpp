/***************************************************************************
 * Copyright (c) 2009 Enrico Ros                                           *
 *         2009 Enrico Ros <enrico.ros@email.it>                           *
 *         2009 Alberto Scarpa <skaal.sl@gmail.com>                        *
 *                                                                         *
 * Permission is hereby granted, free of charge, to any person             *
 * obtaining a copy of this software and associated documentation          *
 * files (the "Software"), to deal in the Software without                 *
 * restriction, including without limitation the rights to use,            *
 * copy, modify, merge, publish, distribute, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the               *
 * Software is furnished to do so, subject to the following                *
 * conditions:                                                             *
 *                                                                         *
 * The above copyright notice and this permission notice shall be          *
 * included in all copies or substantial portions of the Software.         *
 *                                                                         *
 ***************************************************************************/

#include "AppWidget.h"
#include "AbstractGame.h"
#include "InputUtils.h"
#include "ScreenCapture.h"
#include "FVGame.h"
#include "ui_AppWidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QSettings>
#include <QDirIterator>
#include <QTimer>
#include <QDebug>

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 240


class RegionWidget : public QWidget {
    public:
        RegionWidget() : QWidget(0, Qt::CustomizeWindowHint | Qt::FramelessWindowHint)
        {
            setAttribute(Qt::WA_TranslucentBackground);
            setAttribute(Qt::WA_OpaquePaintEvent);
            hide();
        }
        int rX, rY, rWidth, rHeight;
        void setStartPos( const QPoint & pos )
        {
            startPos = pos;
        }
        void setEndPos( const QPoint & pos )
        {
            endPos = pos;
            rX = qMin( startPos.x(), endPos.x() );
            rY = qMin( startPos.y(), endPos.y() );
            rWidth = qMax( startPos.x(), endPos.x() ) - rX + 1;
            rHeight = qMax( startPos.y(), endPos.y() ) - rY + 1;
            setShown( rWidth > 0 && rHeight > 0 );
            setGeometry( rX, rY, rWidth, rHeight );
        }
    protected:
        void paintEvent(QPaintEvent * event)
        {
            QPainter p(this);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(event->rect(), QColor(255, 0, 0, 64));
            p.drawRect(0, 0, width() - 1, height() - 1);
        }
    private:
        QPoint startPos, endPos;
};


AppWidget::AppWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AppWidgetClass)
#if defined(Q_OS_WIN)
    , m_settings( new QSettings( "villecheat.ini", QSettings::IniFormat ) )
#else
    , m_settings( new QSettings() )
#endif
    , m_game( 0 )
    , m_capture( 0 )
    , m_pickingRegion( 0 )
{
    // create ui
    ui->setupUi( this );
    QDesktopWidget dw;
    ui->left->setMaximum( dw.width() );
    ui->top->setMaximum( dw.height() );
    ui->width->setMaximum( dw.width() );
    ui->height->setMaximum( dw.height() );
    // init defaults..
    if ( !m_settings->contains( "rect/left" ) ) {
        ui->left->setValue( (dw.width() - DEFAULT_WIDTH) / 3 );
        ui->top->setValue( (dw.height() - DEFAULT_HEIGHT) / 3 );
        ui->width->setValue( DEFAULT_WIDTH );
        ui->height->setValue( DEFAULT_HEIGHT );
    }
    // ..or reload previous values
    else {
        ui->left->setValue( m_settings->value( "rect/left" ).toInt() );
        ui->top->setValue( m_settings->value( "rect/top" ).toInt() );
        ui->width->setValue( m_settings->value( "rect/width" ).toInt() );
        ui->height->setValue( m_settings->value( "rect/height" ).toInt() );
        ui->frequency->setValue( m_settings->value( "rect/frequency" ).toInt() );
        ui->onTop->setChecked( m_settings->value( "rect/ontop" ).toBool() );
        ui->fvCellsX->setValue( m_settings->value( "cells/x" ).toInt() );
        ui->fvCellsY->setValue( m_settings->value( "cells/y" ).toInt() );
        ui->saferBox->setChecked( m_settings->value( "cells/safer" ).toBool() );
    }
    connect( ui->left, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->top, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->width, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->height, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->frequency, SIGNAL(valueChanged(int)), this, SLOT(slotCapParamsChanged()) );
    connect( ui->onTop, SIGNAL(toggled(bool)), this, SLOT(slotOnTopChanged()) );
    connect( ui->fvCellsX, SIGNAL(valueChanged(int)), this, SLOT(slotCellsChanged()));
    connect( ui->fvCellsY, SIGNAL(valueChanged(int)), this, SLOT(slotCellsChanged()));
    slotOnTopChanged();

    // create the capture
    m_capture = new ScreenCapture( this );
    connect( m_capture, SIGNAL(gotPixmap(const QPixmap &, const QPoint &)),
             this, SLOT(slotProcessPixmap(const QPixmap &, const QPoint &)) );
    slotCapParamsChanged();

    // ### TEMP
    m_capture->setEnabled( true );
}

AppWidget::~AppWidget()
{
    saveSettings();
    delete m_pickingRegion;
    delete m_settings;
    delete m_game;
    delete m_capture;
    delete ui;
}

void AppWidget::mousePressEvent( QMouseEvent * event )
{
    if ( m_pickingRegion )
        m_pickingRegion->setStartPos( event->globalPos() );
}

void AppWidget::mouseMoveEvent( QMouseEvent * event )
{
    if ( m_pickingRegion )
        m_pickingRegion->setEndPos( event->globalPos() );
}

void AppWidget::mouseReleaseEvent( QMouseEvent * event )
{
    if ( m_pickingRegion ) {
        m_pickingRegion->setEndPos( event->globalPos() );
        ui->left->setValue( m_pickingRegion->rX );
        ui->top->setValue( m_pickingRegion->rY );
        ui->width->setValue( m_pickingRegion->rWidth );
        ui->height->setValue( m_pickingRegion->rHeight );
        slotCapParamsChanged();
        delete m_pickingRegion;
        m_pickingRegion = 0;
        releaseMouse();
    }
}

void AppWidget::saveSettings()
{
    m_settings->setValue( "rect/left", ui->left->value() );
    m_settings->setValue( "rect/top", ui->top->value() );
    m_settings->setValue( "rect/width", ui->width->value() );
    m_settings->setValue( "rect/height", ui->height->value() );
    m_settings->setValue( "rect/frequency", ui->frequency->value() );
    m_settings->setValue( "rect/ontop", ui->onTop->isChecked() );
    m_settings->setValue( "cells/x", ui->fvCellsX->value() );
    m_settings->setValue( "cells/y", ui->fvCellsY->value() );
    m_settings->setValue( "cells/safer", ui->saferBox->isChecked() );
}

void AppWidget::on_gameNo_toggled(bool checked)
{
    if ( checked ) {
        delete m_game;
        m_game = 0;
    }
}

void AppWidget::on_gameClick_toggled(bool checked)
{
    if ( checked ) {
        delete m_game;
        m_game = new FVGame(this);
        slotCellsChanged();
    }
}

void AppWidget::on_pickRegionButton_clicked()
{
    m_pickingRegion = new RegionWidget();
    grabMouse( Qt::CrossCursor );
}

void AppWidget::slotOnTopChanged()
{
    Qt::WindowFlags flags = windowFlags();
    if ( ui->onTop->isChecked() )
        flags |= Qt::WindowStaysOnTopHint;
    else
        flags &= ~Qt::WindowStaysOnTopHint;
    setWindowFlags( flags );
    show();
}

void AppWidget::slotCapParamsChanged()
{
    QRect captureRect( ui->left->value(), ui->top->value(), ui->width->value(), ui->height->value() );
    m_capture->setGeometry( captureRect );
    m_capture->setFrequency( ui->frequency->value() );
    ui->capDisplay->setFixedSize( captureRect.size() );
}

void AppWidget::slotCellsChanged()
{
    if (m_game) {
        m_game->setProperty("hCells", ui->fvCellsX->value());
        m_game->setProperty("vCells", ui->fvCellsY->value());
    }
}

void AppWidget::slotProcessPixmap( const QPixmap & pixmap, const QPoint & /*cursor*/ )
{
    // highlight pixmap
    if ( m_game )
        ui->capDisplay->setPixmap( m_game->highlightPixmap( pixmap ) );
    else
        ui->capDisplay->setPixmap( pixmap );

    // make da move!
    if ( m_game )
        m_game->run( ui, m_capture );
}
