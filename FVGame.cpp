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

#include "FVGame.h"

#include "ScreenCapture.h"
#include "InputUtils.h"
#include "ui_AppWidget.h"
#include <QTimer>
#include <QPainter>

//BEGIN portableMSleep
#ifdef Q_OS_UNIX
#include <unistd.h>
void portableMSleep( int ms ) {
    ::usleep( ms * 1000 );
}
#else
#ifdef Q_OS_WIN32
#include <windows.h>
void portableMSleep( int ms ) {
    Sleep( ms );
}
#endif
#endif
//END portableMSleep


FVGame::FVGame( QObject * parent )
    : AbstractGame( parent )
    , m_hCells(12)
    , m_vCells(12)
{
}

FVGame::~FVGame()
{
}

int FVGame::hCells() const
{
    return m_hCells;
}

void FVGame::setHCells(int cells)
{
    m_hCells = cells;
}

int FVGame::vCells() const
{
    return m_vCells;
}

void FVGame::setVCells(int cells)
{
    m_vCells = cells;
}

QPixmap FVGame::highlightPixmap( const QPixmap & pixmap ) const
{
    QPixmap pix = pixmap;
    QPainter pp( &pix );
    pp.setRenderHint(QPainter::Antialiasing, true);
    pp.setPen(QPen(Qt::red, 3));
    foreach (const QPoint & point, points(pixmap.rect()))
        pp.drawPoint(point);
    pp.end();
    return pix;
}

void FVGame::run( Ui::AppWidgetClass * ui, const ScreenCapture * capture )
{
    if (!ui->goButton->isChecked() && !ui->tryButton->isChecked())
        return;

    // random noise
    if (ui->saferBox->isChecked())
        InputUtils::mouseMove(QCursor::pos() + QPoint(-10 + (qrand() % 20), -10 + (qrand() % 20)));

    // proceed every T
    if ( m_time.isNull() )
        m_time.start();
    if ( m_time.elapsed() < 200 )
        return;
    m_time.restart();

    // repopulate list if empty
    if (m_nextPoints.isEmpty()) {
        QRect rect = capture->geometry();
        m_nextPoints = points(rect);
    }

    // click on that point
    QPoint point = m_nextPoints.takeFirst();
    InputUtils::mouseMove(point);
    if (ui->goButton->isChecked())
        InputUtils::mouseLeftClick();
    if (m_nextPoints.isEmpty()) {
        ui->goButton->setChecked(false);
        ui->tryButton->setChecked(false);
    }
}

QList<QPoint> FVGame::points(const QRect & rect) const
{
    // setup matrix for affine projection (found out with paper & math)
    const qreal a11 = (qreal)rect.width() / (qreal)(m_hCells + m_vCells);
    const qreal a12 = -a11;
    const qreal a21 = (qreal)rect.height() / (qreal)(m_hCells + m_vCells);
    const qreal a22 =  a21;
    QList<QPoint> pts;
    for (int y = 0; y < m_vCells; y++) {
        for (int x = 0; x < m_hCells; x++) {
            QPoint point = rect.topLeft();
            point.rx() += a11 *  x        + a12 * (y - (qreal)m_vCells);
            point.ry() += a21 * (x + 1.0) + a22 *  y;
            pts.append(point);
        }
    }
    return pts;
}
