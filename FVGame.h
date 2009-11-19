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

#ifndef __FVGame_h__
#define __FVGame_h__

#include "AbstractGame.h"
#include <QList>
#include <QTime>

class FVGame : public AbstractGame
{
    Q_OBJECT
    Q_PROPERTY(int hCells READ hCells WRITE setHCells)
    Q_PROPERTY(int vCells READ vCells WRITE setVCells)
    public:
        FVGame( QObject * parent );
        ~FVGame();

        // properties
        int hCells() const;
        void setHCells(int);
        int vCells() const;
        void setVCells(int);

        // ::AbstractGame
        QPixmap highlightPixmap( const QPixmap & pixmap ) const;
        void run( Ui::AppWidgetClass * ui, const ScreenCapture * capture );

    private:
        QList<QPoint> points(const QRect & rect) const;
        QTime m_time;
        int m_hCells;
        int m_vCells;
        QList<QPoint> m_nextPoints;
};

#endif
