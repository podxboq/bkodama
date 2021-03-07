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


#ifndef CIRCLEEFFECTDRAWER_H
#define CIRCLEEFFECTDRAWER_H

#include <QObject>
#include <QTime>
#include <QList>

class QPainter;

class CircleEffectDrawer : public QObject
{
Q_OBJECT
public:
    enum CircleState
    {
        Uninitialized,
        Growing,
        Rotating,
        Sinking,
        Done
    };

    CircleEffectDrawer(QObject* parent = 0);
    virtual ~CircleEffectDrawer();

    void start(int width, int height);
    void stop();

    void drawCircleEffect(QPainter* painter);

signals:
    void done();

private:
    float maxLengthPerIndex(float length, int index);

    QTime m_time;
    CircleState m_state;
    int m_width;
    int m_height;
    QList<int> m_lines;
    float m_baseRotation;
    float m_outerCircleSize;
};

#endif // CIRCLEEFFECTDRAWER_H
