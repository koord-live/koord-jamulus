/******************************************************************************\
 * Copyright (c) 2004-2024
 *
 * Author(s):
 *  Volker Fischer
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
\******************************************************************************/

#pragma once

// these gotta go ....
// #include <QFrame>
// #include <QPixmap>
// #include <QLayout>
// #include <QProgressBar>
// --

#include <QTimer>
#include "util.h"
#include "global.h"

/* Definitions ****************************************************************/
#define NUM_LEDS_INCL_CLIP_LED ( NUM_STEPS_LED_BAR + 1 )
#define CLIP_IND_TIME_OUT_MS   20000

/* Classes ********************************************************************/
class CLevelMeterNu : public QObject
{
    Q_OBJECT

public:
    CLevelMeterNu ( QObject* parent = nullptr );
    virtual ~CLevelMeterNu();
    void SetValue ( const double dValue );

protected:
    // virtual void mousePressEvent ( QMouseEvent* ) override { ClipReset(); }
    void SetClipStatus ( const bool bIsClip );
    QTimer TimerClip;

public slots:
    void ClipReset();
};
