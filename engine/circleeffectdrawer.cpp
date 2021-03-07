/*
 * Copyright 2009,2010  Bernd Buschinski b.buschinski@web.de
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "circleeffectdrawer.h"

#include <cmath>
#include <QPainter>
#include <QDebug>

static const int cGrowTime = 2200; //msec
static const int cLastGrowTime = cGrowTime * 4.0f/6.0f; //msec
static const int cLineWidth = 6; //pixel

static const int cOuterCircleGrowTime = cGrowTime;
static const float cOuterCircleMaxSize = 0.75f;
static const float cOuterCircleMinSize = 0.0f;
static const float cInnerCircleMaxSize = 0.50f;
static const float cInnerInnerCircleMaxSize = 0.48f;

static const int cSinkTime = 1400; //msec
static const int cLastSinkTime = cSinkTime - 100; //msec
static const int cOuterCircleSinkTime = cSinkTime; //msec

static const int cRotationTime = 10000; //msec

static const float cRotationSpeed = 360.0/90000.0;

CircleEffectDrawer::CircleEffectDrawer(QObject* parent)
    : QObject(parent),
      m_state(CircleEffectDrawer::Uninitialized),
      m_baseRotation(0),
      m_outerCircleSize(0)
{
    const int size = 4*8;
#if QT_VERSION >= 0x040700
    m_lines.reserve(size);
#endif
    for (int i = 0; i < size; ++i)
        m_lines.append(0);
}

CircleEffectDrawer::~CircleEffectDrawer()
{
}

void CircleEffectDrawer::start(int width, int height)
{
    m_height = height;
    m_width = width;
    m_time = QTime::currentTime();
    m_state = Growing;
    m_baseRotation = 0;
    for (int i = 0; i < m_lines.count(); ++i)
        m_lines[i] = 0;
    m_outerCircleSize = 0;
}

void CircleEffectDrawer::stop()
{
    m_state = Uninitialized;
}

void CircleEffectDrawer::drawCircleEffect(QPainter* painter)
{
    if (m_state == Uninitialized || m_state == Done)
        return;

    const int msecs = m_time.msecsTo(QTime::currentTime());
    float rotation = m_baseRotation + (msecs * cRotationSpeed);
    while (rotation >= 360)
        rotation -= 360;
    switch (m_state)
    {
        case Rotating:
            if (msecs > cRotationTime)
            {
                m_state = Sinking;
                m_baseRotation = rotation;
                m_time = QTime::currentTime();
            }
            break;
        case Sinking:
        {
            if (msecs > cSinkTime)
            {
                m_state = Done;
                m_baseRotation = rotation;
                m_time = QTime::currentTime();
                emit done();
                stop();
                break;
            }

            const int maxLength = qMin(m_width, m_height)/2.0;
            static const int growSpeed = cSinkTime - cLastSinkTime;
            const int size = m_lines.count();
            static const float perItemTime = ((float)cLastSinkTime/(float)(size-1));
            for (int i = 0; i < size; ++i)
            {
                m_lines[i] = qBound(0.0f, 1.0f - (((float)msecs - (float)i * perItemTime) /(float)growSpeed), 1.0f) * maxLengthPerIndex(maxLength, i);
            }

            m_outerCircleSize = qBound(cOuterCircleMinSize, 1.0f - (float)msecs/(float)cOuterCircleSinkTime, cOuterCircleMaxSize);
        }

            break;
        case Growing:
        {
            const int maxLength = qMin(m_width, m_height)/2.0;
            static const int growSpeed = cGrowTime - cLastGrowTime;
            const int size = m_lines.count();
            static const float perItemTime = (float)cLastGrowTime/(float)(size-1);
            const int growToIndex = ((float)msecs/perItemTime)+1;
            for (int i = 0; i < size; ++i)
            {
                if (i == growToIndex)
                    break;
                m_lines[i] = qBound(0.0f, ((float)msecs - (float)i * perItemTime) /(float)growSpeed, 1.0f) * maxLengthPerIndex(maxLength, i);
            }

            m_outerCircleSize = qBound(cOuterCircleMinSize, (float)msecs/(float)cOuterCircleGrowTime, cOuterCircleMaxSize);

            if (msecs > cGrowTime)
            {
                m_state = Rotating;
                m_baseRotation = rotation;
                m_time = QTime::currentTime();
            }
        }
            break;
        case Done:
        case Uninitialized:
        default:
            break;
    }
    if (m_state == Uninitialized || m_state == Done)
        return;

    int centerWidth = m_width/2.0f;
    int centerHeight = m_height/2.0f;

    //painter->save();
    painter->setBrush(Qt::yellow);
    painter->setPen(Qt::red);
    painter->translate(centerWidth, centerHeight);
    painter->rotate(-rotation - 90);
    const int lineCount = m_lines.count();
    const float rotatePerItem = -360.0f/(float)lineCount;
    float rotated = 0;
    for (int i = 0; i < lineCount; ++i)
    {
        painter->rotate(rotatePerItem);
        rotated += rotatePerItem;

        const int value = m_lines.at(i);
        if (value <= 0)
        {
            if (m_state == Growing)
            {
                break;
            }
            continue;
        }
        painter->drawRect(0, -cLineWidth/2.0f, value, cLineWidth);
    }

    painter->rotate(rotation + 90 - rotated);
    painter->translate(-centerWidth, -centerHeight);

    if (m_outerCircleSize == 0)
        return;

    const float circleSize = qMin(m_width, m_height) * m_outerCircleSize;

    //draw inner circle with gray border
    painter->setBrush(Qt::lightGray);
    painter->drawEllipse(centerWidth-circleSize*cInnerCircleMaxSize/2.0f,
                         centerHeight-circleSize*cInnerCircleMaxSize/2.0f,
                         circleSize*cInnerCircleMaxSize,
                         circleSize*cInnerCircleMaxSize);
    painter->setBrush(Qt::yellow);
    painter->drawEllipse(centerWidth-circleSize*cInnerInnerCircleMaxSize/2.0f,
                         centerHeight-circleSize*cInnerInnerCircleMaxSize/2.0f,
                         circleSize*cInnerInnerCircleMaxSize,
                         circleSize*cInnerInnerCircleMaxSize);

    //draw outer circle with white circles
    QPainterPath path;
    const int diffX = centerWidth - circleSize/2.0f;
    const int diffY = centerHeight - circleSize/2.0f;
    const int outerBigCircleWidth = circleSize/12.0f;
    const int outerSmallCircleWidth = circleSize/80.0f;
    int diff = 1;
    path.addEllipse(diff+diffX, diff+diffY, circleSize-diff*2, circleSize-diff*2);
    diff = outerBigCircleWidth;
    path.addEllipse(diff+diffX, diff+diffY, circleSize-diff*2, circleSize-diff*2);
    painter->drawPath(path);

    QPainterPath path2;
    diff = 1;
    path2.addEllipse(diff+diffX, diff+diffY, circleSize-diff*2, circleSize-diff*2);
    diff = outerSmallCircleWidth;
    path2.addEllipse(diff+diffX, diff+diffY, circleSize-diff*2, circleSize-diff*2);
    painter->drawPath(path2);

    painter->translate(centerWidth, centerHeight);
    painter->rotate(rotation - 90 - rotatePerItem/2);
    painter->setBrush(Qt::white);
    for (int i = 0; i < lineCount; ++i)
    {
        painter->rotate(rotatePerItem);
        painter->drawEllipse(circleSize/2.0f-outerBigCircleWidth*0.95f,
                             -(outerBigCircleWidth*0.95f-outerSmallCircleWidth)/2.0f,
                             outerBigCircleWidth*0.9f-outerSmallCircleWidth,
                             outerBigCircleWidth*0.9f-outerSmallCircleWidth);
    }
    //rotate + translate back
    painter->rotate(-rotation + 90 + rotatePerItem/2 - rotatePerItem*lineCount);
    painter->translate(-centerWidth, -centerHeight);
}

float CircleEffectDrawer::maxLengthPerIndex(float length, int index)
{
    if ((index % 4) == 0)
    {
        return length*0.99f;
    }
    if ((index % 2) == 0)
    {
        return length*0.92f;
    }
    else
    {
        return length*0.84f;
    }
}

#include "circleeffectdrawer.moc"
