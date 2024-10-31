/******************************************************************************\
 * Copyright (c) 2004-2024
 *
 * Author(s):
 *  Volker Fischer
 *
 * Description:
 *  Implements a multi color LED bar
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

#include "levelmeternu.h"

/* Implementation *************************************************************/
CLevelMeterNu::CLevelMeterNu ( QObject* parent ) : QObject ( parent )
{
    // // initialize LED meter
    // QWidget*     pLEDMeter  = new QWidget();
    // QVBoxLayout* pLEDLayout = new QVBoxLayout ( pLEDMeter );
    // pLEDLayout->setAlignment ( Qt::AlignHCenter );
    // pLEDLayout->setContentsMargins ( 0, 0, 0, 0 );
    // pLEDLayout->setSpacing ( 0 );

    // create LEDs plus the clip LED
    // vecpLEDs.Init ( NUM_LEDS_INCL_CLIP_LED );

    // for ( int iLEDIdx = NUM_LEDS_INCL_CLIP_LED - 1; iLEDIdx >= 0; iLEDIdx-- )
    // {
    //     // create LED object
    //     vecpLEDs[iLEDIdx] = new cLED ( parent );

    //     // add LED to layout with spacer (do not add spacer on the bottom of the first LED)
    //     if ( iLEDIdx < NUM_LEDS_INCL_CLIP_LED - 1 )
    //     {
    //         pLEDLayout->addStretch();
    //     }

    //     pLEDLayout->addWidget ( vecpLEDs[iLEDIdx]->GetLabelPointer() );
    // }

    // initialize bar meter
    // pBarMeter = new QProgressBar();
    // pBarMeter->setOrientation ( Qt::Vertical );
    // pBarMeter->setRange ( 0, 100 * NUM_STEPS_LED_BAR ); // use factor 100 to reduce quantization (bar is continuous)
    // pBarMeter->setFormat ( "" );                        // suppress percent numbers

    // // setup stacked layout for meter type switching mechanism
    // pMinStackedLayout = new CMinimumStackedLayout ( this );
    // pMinStackedLayout->addWidget ( pLEDMeter );
    // pMinStackedLayout->addWidget ( pBarMeter );

    // // according to QScrollArea description: "When using a scroll area to display the
    // // contents of a custom widget, it is important to ensure that the size hint of
    // // the child widget is set to a suitable value."
    // pBarMeter->setMinimumSize ( QSize ( 1, 1 ) );
    // pLEDMeter->setMinimumSize ( QSize ( 1, 1 ) );

    // update the meter type (using the default value of the meter type)
    // SetLevelMeterType ( eLevelMeterType );

    // setup clip indicator timer
    TimerClip.setSingleShot ( true );
    TimerClip.setInterval ( CLIP_IND_TIME_OUT_MS );

    // Connections -------------------------------------------------------------
    QObject::connect ( &TimerClip, &QTimer::timeout, this, &CLevelMeterNu::ClipReset );
}

CLevelMeterNu::~CLevelMeterNu()
{
    // // clean up the LED objects
    // for ( int iLEDIdx = 0; iLEDIdx < NUM_LEDS_INCL_CLIP_LED; iLEDIdx++ )
    // {
    //     delete vecpLEDs[iLEDIdx];
    // }
}


void CLevelMeterNu::SetClipStatus ( const bool bIsClip )
{
    if ( bIsClip )
    {
      // pBarMeter->setRedSomehow();
    }
    else
    {
      // pBarMeter->setToNormalGreenSituation();
    }
}

void CLevelMeterNu::SetValue ( const double dValue )
{

    // pBarMeter->setValue ( 100 * dValue );

    // clip indicator management (note that in case of clipping, i.e. full
    // scale level, the value is above NUM_STEPS_LED_BAR since the minimum
    // value of int16 is -32768 but we normalize with 32767 -> therefore
    // we really only show the clipping indicator, if actually the largest
    // value of int16 is used)
    if ( dValue > NUM_STEPS_LED_BAR )
    {
        // vecpLEDs[NUM_STEPS_LED_BAR]->SetColor ( cLED::RL_RED );
        TimerClip.start();
    }
}

void CLevelMeterNu::ClipReset()
{
    // we manually want to reset the clipping indicator: stop timer and reset
    // clipping indicator GUI element
    TimerClip.stop();

    // vecpLEDs[NUM_STEPS_LED_BAR]->SetColor ( cLED::RL_BLACK );
}
