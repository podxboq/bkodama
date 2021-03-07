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

#ifndef BKODAMA_H
#define BKODAMA_H

#include <QImage>
#include <QRectF>
#include <QList>
#include <QVariant>
#include <QSize>
#include <QString>

#include <Plasma/Applet>
#include <Plasma/Svg>

#include <Phonon/MediaObject>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>

#include "ui_bkodamaConfig.h"

#include "circleeffectdrawer.h"

class QTimer;
class QSvgRenderer;
class QGraphicsSceneMouseEvent;
class KConfigDialog;

class bkodamaapplet : public Plasma::Applet
{
        Q_OBJECT

        enum Animation
        {
            FadeOutWalk,
            FadeIn,
            FadeInStanding,
            StandUp,
            SitDown,
            HeadSpin,
            FadeOutSitting,
            FadeOutStanding,
            SpecialChance,
            SpecialFadeIn,
            Special,
            SpecialFail,
            SpecialOver
        };

    public:
        bkodamaapplet(QObject * parent, const QVariantList & args);
        ~bkodamaapplet();

        virtual void init();
        virtual void paintInterface(QPainter * painter, const QStyleOptionGraphicsItem * option, const QRect & contentsRect);

        virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
        virtual void resizeEvent(QGraphicsSceneResizeEvent * event);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    public slots:
        void animation();

        void createConfigurationInterface(KConfigDialog *parent);

    protected slots:
        void configAccepted();
//         void screenResized(int screen);

    protected:
        virtual void constraintsEvent(Plasma::Constraints constraints);
        virtual void saveState(KConfigGroup& config) const;

    private slots:
        void initSpecialOver();

    private:
        inline void readConfiguration();

        inline void loadSVGList();
        inline void updateScaledImage();

        inline void setAlphaToPercent(float aAlphaPercent);
        inline bool setImage(const QString& aImageName);
        inline void setTimerSpeed(int aMSecs);

//default animation handler
        inline bool fadeInHandling(const QStringList& animationList);
        inline bool fadeOutHandling(const QStringList& animationList);
        inline bool animationHandling(const QStringList& animationList, int animationTime);

//don't use setImage in this functions, wait for animation, it can produce weird no-animation images
        inline void initFadeOutWalk();
        inline float walkOutDistance();
        inline void initFadeOutSitting();
        inline void initFadeOutStanding();
        inline void initFadeIn();
        inline void initFadeInStanding();
        inline void initHeadSpin();
        inline void initStandUp();
        inline void initSitDown();
        inline void initSpecialChance();
        inline void initSpecial();
        inline void initSpecialFail();

        inline void checkSpecialEventTime(int aMecs);

        int m_ImageIndex;

        float m_alpha;
        float m_alphaModifier;

        int m_X_modifier;

        QList<Plasma::Svg*> m_SVGRenderList;

        Animation m_animation;
        int m_animationPos;

        QRectF m_screen;
        int m_screenNumber;
        QImage m_scaledImage;
        QImage m_alphaImage;

        bool m_mousePressed;
        QTimer *m_timer;

        // Config dialog
        Ui::bkodamaConfig ui;

        // config sound
        QString m_soundUrl;
        float m_soundVolume;
        bool m_soundEnabled;

        // config animation
        int m_veryLongBreakTime;
        bool m_specialEvent;

        // animation special
        CircleEffectDrawer m_circleDawer;
        QRectF m_oldSpecialGeometry;
        QSizeF m_oldMaxSize;

        Phonon::AudioOutput * m_audioOutput;
        Phonon::MediaObject * m_sound;

        inline void initSound();
};

K_EXPORT_PLASMA_APPLET(bkodama, bkodamaapplet)

#endif //BKODAMA_H
