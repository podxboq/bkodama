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

#include "bkodama.h"

#include "brand.h"

#include <KStandardDirs>
#include <KConfigDialog>
#include <KWindowSystem>
#include <KApplication>

#include <QTimer>
#include <QPainter>
#include <QSvgRenderer>
#include <QDesktopWidget>
#include <QGraphicsSceneMouseEvent>

static const QStringList cWalkOutList = QStringList() << "walk00-test.svg" << "walk01-test.svg" << "walk02-test.svg" << "walk03-test.svg" <<  "walk04-test.svg" << "walk05-test.svg" << "walk06-test.svg";
static const int cWalkOutTime = 800; //ms

static const QStringList cFadeInList = QStringList() << "sitting01-test.svg";
static const int cFadeInTime = 800; //ms

static const QStringList cFadeInStandList = QStringList() << "stand00-test.svg";
static const int cFadeInStandTime = 800; //ms

static const QStringList cFadeOutSittingList = QStringList() << "sitting01-test.svg";
static const int cFadeOutSittingTime = 800; //ms

static const QStringList cFadeOutStandingList = QStringList() << "stand00-test.svg";
static const int cFadeOutStandingTime = 800; //ms

static const QStringList cStandUpList = QStringList() << "sitting01-test.svg" << "stand03-test.svg" << "stand02-test.svg" << "stand01-test.svg" << "stand01-test.svg" << "stand00-test.svg";
static const int cStandUpTime = 720; //ms

static const QStringList cSitDownList = QStringList() << "stand00-test.svg" << "stand01-test.svg" << "stand02-test.svg" << "stand03-test.svg" << "sitting01-test.svg";
static const int cSitDownTime = 720; //ms

static const QStringList cHeadSpinList = QStringList() << "spin00-test.svg" << "spin00-test.svg" << "spin01-test.svg" << "spin01-test.svg" << "spin02-test.svg" << "spin02-test.svg" << "spin03-test.svg" << "spin04-test.svg" << "spin03-test.svg" << "spin05-test.svg" << "spin02-test.svg"  << "sitting01-test.svg";
static const int cHeadSpinTime = 1800; //ms

static const QStringList cSpecialChanceList = QStringList() << "stand00-test.svg" << "special00-test.svg" << "special01-test.svg" << "special02-test.svg" << "special03-test.svg" << "special04-test.svg" \
                                                      << "special01-test.svg" << "special02-test.svg" << "special03-test.svg" << "special04-test.svg" << "special01-test.svg";
static const int cSpecialChanceTime = 2400; //ms

static const QStringList cSpecialOverList = QStringList() << "special01-test.svg" << "special00-test.svg" << "stand00-test.svg";
static const int cSpecialOverTime = 500; //ms

static const QStringList cSpecialFailList = QStringList() << "specialfail01-test.svg" << "specialfail02-test.svg" << "specialfail03-test.svg" << "specialfail04-test.svg" << "specialfail05-test.svg" << "specialfail06-test.svg" << "specialfail07-test.svg" << "specialfail08-test.svg" << "specialfail09-test.svg" << "special01-test.svg";
static const int cSpecialFailTime = 600; //ms

//msecs
static const int cTimerFast = 30;
static const int cShortBreak = 2000;
static const int cLongBreak = 5000;
//static const int cVeryLongBreak = 35000;

using namespace Plasma;

bkodamaapplet::bkodamaapplet(QObject * parent, const QVariantList & args)
    : Plasma::Applet(parent, args)
{
    m_sound = 0;
    m_audioOutput = 0;
    m_soundEnabled = true;
    m_soundVolume = 1.0f;
    m_alpha = 100;
    m_alphaModifier = 0;
    m_X_modifier = 0;
    m_ImageIndex = 0;
    m_mousePressed = false;
    m_screenNumber = 0;

    m_animationPos = 0;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animation()));

    loadSVGList();
    initStandUp();
    setImage(cStandUpList.first());

    setHasConfigurationInterface(true);

    setBackgroundHints(NoBackground);
    setMinimumSize(QSizeF(18.75f, 30.0f));
    setMaximumSize(QSizeF(156.25f, 250.0f));
    setAspectRatioMode(Plasma::KeepAspectRatio);

    connect(&m_circleDawer, SIGNAL(done()), this, SLOT(initSpecialOver()), Qt::QueuedConnection);
}

bkodamaapplet::~bkodamaapplet()
{
    while (!m_SVGRenderList.isEmpty())
    {
        delete m_SVGRenderList.takeFirst();
    }

    if (m_timer)
    {
        disconnect(m_timer, 0, 0, 0);
        if (m_timer->isActive())
            m_timer->stop();
        delete m_timer;
    }

    delete m_audioOutput;
    delete m_sound;
}

void bkodamaapplet::init()
{
    QStringList soundFiles = KGlobal::dirs()->findAllResources("data", "bkodama/head-spin3.ogg");
    if (soundFiles.isEmpty())
    {
        setFailedToLaunch(true, i18n("Unabled to find soundfile: head-spin3.ogg"));
        return;
    }
    m_soundUrl = soundFiles.first();

    readConfiguration();

    initSound();

    //kDebug() << KApplication::desktop()->numScreens();
    //kDebug() << KApplication::desktop()->isVirtualDesktop();
    if (KApplication::desktop()->numScreens() > 1 || KApplication::desktop()->isVirtualDesktop())
    {
        //int tScreen = KApplication::desktop()->screenNumber( view()->parentWidget() ); //always screen 0
        m_screenNumber = KApplication::desktop()->screenNumber(QCursor::pos());   //unreliable
        //kDebug() << "tScreen" << tScreen;

        m_screen = KWindowSystem::workArea().intersect(KApplication::desktop()->screenGeometry(m_screenNumber));
        //m_screen = KApplication::desktop()->screen()->geometry();
    }
    else
    {
        m_screen = KWindowSystem::workArea();
    }
    //m_screen = screenRect();

    kDebug() << "width: " << m_screen.width();
    kDebug() << "height: " << m_screen.height();

    // 500/800 -> svg size
    setMaximumSize(QSizeF(m_screen.height() * 500.0f / 800.0f / 5.0f * 2.0f, m_screen.height() / 5.0 * 2.0f));
    KConfigGroup cg = config();
    resize(cg.readEntry("Size", QSizeF(maximumWidth() / 2.0f, maximumHeight() / 2.0f)));

    updateScaledImage();

    //for some unknown reason I bever receive a workAreaResized or resized from desktop
    //comment out and remember for later
//     connect(KApplication::desktop(), SIGNAL(workAreaResized(int)), this, SLOT(screenResized(int)));

    m_timer->start(brand(cLongBreak) + cShortBreak);
}

void bkodamaapplet::animation()
{
    if (m_mousePressed)
    {
        return;
    }

    bool animationEnded = false;
    setTimerSpeed(cTimerFast);

    switch (m_animation)
    {
        case FadeOutWalk:
        {
            m_alpha += m_alphaModifier;
            animationEnded = animationHandling(cWalkOutList, cWalkOutTime);
            setAlphaToPercent(m_alpha);
            QRectF tGeometry = geometry();
            setPos(tGeometry.x() - m_X_modifier, tGeometry.y());
        }
        break;
        case FadeOutSitting:
            animationEnded = fadeOutHandling(cFadeOutSittingList);
            break;
        case FadeOutStanding:
            animationEnded = fadeOutHandling(cFadeOutStandingList);
            break;
        case SpecialFadeIn:
        case FadeInStanding:
            animationEnded = fadeInHandling(cFadeInStandList);
            break;
        case FadeIn:
            animationEnded = fadeInHandling(cFadeInList);
            break;
        case StandUp:
            animationEnded = animationHandling(cStandUpList, cStandUpTime);
            break;
        case SitDown:
            animationEnded = animationHandling(cSitDownList, cSitDownTime);
            break;
        case HeadSpin:
            if (m_soundEnabled && m_animationPos == 0)
            {
                kDebug() << "play";
                m_sound->play();
            }
            animationEnded = animationHandling(cHeadSpinList, cHeadSpinTime);
            break;
        case SpecialChance:
            animationEnded = animationHandling(cSpecialChanceList, cSpecialChanceTime);
            break;
        case Special:
            break;
        case SpecialOver:
            animationEnded = animationHandling(cSpecialOverList, cSpecialOverTime);
            break;
        case SpecialFail:
            animationEnded = animationHandling(cSpecialFailList, cSpecialFailTime);
            break;
        default:
            kDebug() << "unknown animation" << m_animation;
            break;
    }

    if (animationEnded)
    {
        m_animationPos = 0;
        m_X_modifier = 0;
        m_alphaModifier = 0;

        int tRandom;

        switch (m_animation)
        {
            case FadeOutWalk:
            case FadeOutSitting:
            case FadeOutStanding:
                tRandom = brand(100);
                if (tRandom > 50)
                {
                    initFadeIn();
                }
                else
                {
                    initFadeInStanding();
                }
                break;
            case FadeInStanding:
                tRandom = brand(100);
                if (tRandom > 90)
                {
                    initFadeOutStanding();
                }
                else
                {
                    initSitDown();
                }
                break;
            case FadeIn:
                tRandom = brand(100);
                if (tRandom > 96)
                {
                    initFadeOutSitting();
                }
                else if (tRandom > 17)
                {
                    initHeadSpin();
                }
                else
                {
                    initStandUp();
                }
                break;
            case HeadSpin:
                tRandom = brand(100);
                if (tRandom > 97)
                {
                    initHeadSpin();
                }
                else if (tRandom > 30)
                {
                    initStandUp();
                }
                else
                {
                    initFadeOutSitting();
                }
                break;
            case StandUp:
                tRandom = brand(100);
                if (tRandom > 95 || walkOutDistance() > geometry().x())
                {
                    initFadeOutStanding();
                }
                else
                {
                    initFadeOutWalk();
                }
                break;
            case SitDown:
                tRandom = brand(100);
                if (tRandom > 55)
                {
                    initFadeOutSitting();
                }
                else
                {
                    initHeadSpin();
                }
                break;
            case SpecialFadeIn:
                initSpecialChance();
                break;
            case Special:
                break;
            case SpecialChance:
                tRandom = brand(100);
                if (tRandom > 98)
                {
                    initSpecial();
                }
                else
                {
                    initSpecialFail();
                }
                break;
            case SpecialOver:
                initFadeOutStanding();
                break;
            case SpecialFail:
                initSpecialOver();
                break;

            default:
                kDebug() << "unknown animation end" << m_animation;
                break;
        }
    }

    update();
}

void bkodamaapplet::setAlphaToPercent(float aAlphaPercent)
{
    //QTime tTime = QTime::currentTime();
    if (aAlphaPercent >= 100.0f)
    {
        m_alphaImage = m_scaledImage;
        return;
    }

    if (aAlphaPercent <= 0.0f)
    {
        m_alphaImage.fill(Qt::transparent);
        return;
    }

    int tBytePerLine = m_alphaImage.bytesPerLine();
    int tHeight = m_alphaImage.height();
//     uchar* dataAlpha = m_alphaImage.bits();
//     uchar* dataScaled = m_alphaImage.bits();

    for (int i = 0; i < tHeight; ++i)
    {
        QRgb* tRgb = (QRgb*)m_scaledImage.scanLine(i);
        QRgb* tLineAlpha = (QRgb*)m_alphaImage.scanLine(i);
//causes flicker, I wonder why...
//         QRgb* tRgb = (QRgb*)(dataAlpha + i * tBytePerLine);
//         QRgb* tLineAlpha = (QRgb*)(dataScaled + i * tBytePerLine);

        for (int j = 0; j < tBytePerLine; j += sizeof(QRgb))
        {
            int tAlpha = qAlpha(*tRgb);
            if (tAlpha > 0)
            {
                *tLineAlpha = qRgba(qRed(*tRgb), qGreen(*tRgb), qBlue(*tRgb), ((float)tAlpha / 100.0f) * aAlphaPercent);
                //kDebug() << "alpha: " << ((float)qAlpha(*tRgb)/100.0) * aAlphaPercent;
            }
            ++tRgb;
            ++tLineAlpha;
        }
    }
    //kDebug() << tTime.elapsed();
}

void bkodamaapplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint)
    {
        setBackgroundHints(NoBackground);
    }

    if (constraints & Plasma::SizeConstraint)
    {
        if (m_animation != Special)
        {
            updateScaledImage();
            setAlphaToPercent(m_alpha);
            update();
        }
    }
}

void bkodamaapplet::saveState(KConfigGroup& cg) const
{
    //Sound
    cg.writeEntry("SoundEnabled", m_soundEnabled);
    cg.writeEntry("SoundVolume", m_soundVolume * 100);

    //Animation
    cg.writeEntry("TimeBewteenAppearance", m_veryLongBreakTime);
    cg.writeEntry("SpecialEvent", m_specialEvent);

    //Size
    cg.writeEntry("Size", size());
    kDebug() << "saved.";
}

void bkodamaapplet::updateScaledImage()
{
    //QTime tTime = QTime::currentTime();
    QRectF tRect = geometry();
    m_scaledImage = QImage(tRect.width(), tRect.height(), QImage::Format_ARGB32);
    m_scaledImage.fill(Qt::transparent);
    m_SVGRenderList[m_ImageIndex]->resize(tRect.width(), tRect.height());
    QPainter tPaint;
    tPaint.begin(&m_scaledImage);
    m_SVGRenderList[m_ImageIndex]->paint(&tPaint, m_scaledImage.rect());
    tPaint.end();
    //kDebug() << tTime.elapsed();

    m_alphaImage = m_scaledImage;
}

void bkodamaapplet::paintInterface(QPainter * p, const QStyleOptionGraphicsItem * option, const QRect & contentsRect)
{
    Q_UNUSED(option);
    Q_UNUSED(contentsRect);

    if (m_animation == Special)
    {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::Antialiasing);
        m_circleDawer.drawCircleEffect(p);

        //p->setRenderHint(QPainter::Antialiasing);
        QSizeF sizef(size());
        p->drawImage((sizef.width()-m_oldSpecialGeometry.width())/2.0f,
                     (sizef.height()-m_oldSpecialGeometry.height())/2.0f,
                     m_alphaImage);
    }
    else
    {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::Antialiasing);

        p->drawImage(0, 0, m_alphaImage);
    }
}

void bkodamaapplet::setTimerSpeed(int aMSecs)
{
    if (m_timer->interval() != aMSecs)
    {
        m_timer->start(aMSecs);
    }
}

bool bkodamaapplet::fadeInHandling(const QStringList& animationList)
{
    if (m_alpha == 0)
    {
        setImage(animationList.first());
    }
    m_alpha += m_alphaModifier;
    setAlphaToPercent(m_alpha);
    if (m_alpha >= 100.0f)
    {
        return true;
    }
    return false;
}

bool bkodamaapplet::fadeOutHandling(const QStringList& animationList)
{
    if (m_alpha == 100)
    {
        setImage(animationList.first());
    }
    m_alpha += m_alphaModifier;
    setAlphaToPercent(m_alpha);
    if (m_alpha <= 0.0f)
    {
        return true;
    }
    return false;
}

bool bkodamaapplet::animationHandling(const QStringList& animationList, int animationTime)
{
    ++m_animationPos;
    if ((m_animationPos * cTimerFast) >= animationTime)
    {
        return true;
    }
    int tImage = int((m_animationPos * (float)cTimerFast * (float)animationList.size()) / (float)animationTime);
    setImage(animationList.at(tImage));
    return false;
}

void bkodamaapplet::loadSVGList()
{
    QStringList tList = KGlobal::dirs()->findAllResources("data", "bkodama/*.svg");

    foreach(const QString& tFile, tList)
    {
        kDebug() << tFile;
        Svg* tRender = new Svg();
        tRender->setImagePath(tFile);
        if (tRender->isValid())
        {
            m_SVGRenderList.append(tRender);
        }
        else
        {
            kDebug() << "invalid svg: " << tFile;
            delete tRender;
            continue;
        }
    }
}

//searches for the aImageName in the list of svgs and sets m_ImageIndex if found
bool bkodamaapplet::setImage(const QString& aImageName)
{
    for (int i = 0; i < m_SVGRenderList.size(); ++i)
    {
        if (m_SVGRenderList.at(i)->imagePath().endsWith(aImageName))
        {
            //nothing changed
            if (i == m_ImageIndex)
            {
                return true;
            }
            m_ImageIndex = i;
            updateScaledImage();
            return true;
        }
    }
    kDebug() << "could not find imageindex for " << aImageName;
    return false;
}

void bkodamaapplet::initFadeOutWalk()
{
    m_animation = FadeOutWalk;
    m_alpha = 100;
    m_alphaModifier = -(float(cTimerFast) * 100.0f) / (float)cWalkOutTime;
    m_X_modifier = int(((float)cTimerFast * (219.48f / 500.0f) * (float)geometry().width()) / (float)cWalkOutTime + 0.5f);
    setTimerSpeed(brand(cShortBreak) + cShortBreak);
}

float bkodamaapplet::walkOutDistance()
{
//100/(100/(800/30)) * ((219.48/500)*100)/(800/30)

//original stuff:
//m_alphaModifier = (100.0f / ( (float)cWalkOutTime / cTimerFast ));
//m_X_modifier = int((float((219.48f/500.0f)*m_position.width())/float((float)cWalkOutTime / (float)cTimerFast ))+0.5);

// yes this is ugly
// m_X_modifier = distance per tick
// m_alphaModifier = 100/m_alphaModifier how many ticks
//     float tAlphaTicks = (float)cWalkOutTime / float(cTimerFast);
//     float tXModifier = ((float)cTimerFast * (219.48f/500.0f) * m_position.width()) / (float)cWalkOutTime;
    //return tAlphaTicks * tXModifier + m_position.width()/8.0f + 0.5f;

//(a/b) * (b*d*e)/a + e/8
//d * e + e / 8
//d * e + e * 0.125
// d = (219.48f/500.0f)
// d = 0.43896f
//mathimized
    float tWidth = geometry().width();
    return 0.43896f * tWidth + tWidth * 0.125f + 0.5f;
}

void bkodamaapplet::initFadeOutSitting()
{
    m_animation = FadeOutSitting;
    m_alpha = 100;
    m_alphaModifier = -(float)cTimerFast * 100.0f / (float)cFadeOutSittingTime;
    setTimerSpeed(brand(cLongBreak) + cShortBreak);
}

void bkodamaapplet::initFadeOutStanding()
{
    m_animation = FadeOutStanding;
    m_alpha = 100;
    m_alphaModifier = -(float)cTimerFast * 100.0f / (float)cFadeOutStandingTime ;
    setTimerSpeed(brand(cLongBreak) + cShortBreak);
}

void bkodamaapplet::initFadeIn()
{
    m_animation = FadeIn;
    m_alphaModifier = (float)cTimerFast * 100.0f / (float)cFadeInTime;
    m_alpha = 0;
    //setTimerSpeed(brand(cVeryLongBreak) + cLongBreak);
    //setTimerSpeed( brand( 1 ) + cLongBreak );

    unsigned int x = brand(m_screen.width() - geometry().width());
    unsigned int y = brand(m_screen.height() - geometry().height());

    setPos(x, y);
//     kDebug() << "x: " << x;
//     kDebug() << "y: " << y;
//     kDebug() << "screen width: " << m_screen.width();
//     kDebug() << "screen height: " << m_screen.height();
    checkSpecialEventTime(brand(m_veryLongBreakTime) + cLongBreak);
}

void bkodamaapplet::initFadeInStanding()
{
    m_animation = FadeInStanding;
    m_alphaModifier = (float)cTimerFast * 100.0f / (float)cFadeInStandTime;
    m_alpha = 0;

    unsigned int x = brand(m_screen.width() - geometry().width());
    unsigned int y = brand(m_screen.height() - geometry().height());

    setPos(x, y);
    checkSpecialEventTime(brand(m_veryLongBreakTime) + cLongBreak);
}

void bkodamaapplet::initHeadSpin()
{
    m_animation = HeadSpin;
    m_alpha = 100;
    setTimerSpeed(brand(cLongBreak) + cShortBreak);
}

void bkodamaapplet::initStandUp()
{
    m_animation = StandUp;
    m_alpha = 100;
    setTimerSpeed(brand(cLongBreak) + cShortBreak);
}

void bkodamaapplet::initSitDown()
{
    m_animation = SitDown;
    m_alpha = 100;
    setTimerSpeed(brand(cLongBreak) + cShortBreak);
}

void bkodamaapplet::initSpecialChance()
{
    m_animation = SpecialChance;
    m_alpha = 100;
    setTimerSpeed(brand(cShortBreak) + cShortBreak);
}

void bkodamaapplet::initSpecial()
{
    m_animation = Special;
    m_oldSpecialGeometry = geometry();
    m_oldMaxSize = maximumSize();
    m_circleDawer.start(m_screen.width(), m_screen.height());
    setPos(0,0);
    setMaximumSize(m_screen.width(), m_screen.height());
    resize(m_screen.width(), m_screen.height());
    //don't wait, start as drawer->start was already called
    //setTimerSpeed(brand(cShortBreak) + cShortBreak);
}

void bkodamaapplet::initSpecialOver()
{
    if (m_animation == Special)
    {
        setGeometry(m_oldSpecialGeometry);
        setMaximumSize(m_oldMaxSize);
    }
    else
    {
        setTimerSpeed(brand(cShortBreak) + cShortBreak);
    }
    //special set m_animation AFTER resize, as resize causes update
    //and image rescale, but it is blocked for "Special", which comes before this
    m_animation = SpecialOver;
}

void bkodamaapplet::initSpecialFail()
{
    m_animation = SpecialFail;
    setTimerSpeed(brand(cShortBreak) + cShortBreak);
}

void bkodamaapplet::checkSpecialEventTime(int aMecs)
{
    //REMOVE ME
//     m_animation = SpecialFadeIn;
//     unsigned int x = (m_screen.width() - geometry().width())/2.0f;
//     unsigned int y = (m_screen.height() - geometry().height())/2.0f;
//     setPos(x,y);
    //REMOVE ME END
    if (!m_specialEvent)
    {
        setTimerSpeed(aMecs);
        return;
    }

    QDateTime time(QDateTime::currentDateTime());
    QDateTime timeTo = time.addMSecs(aMecs);

    if (time.time().hour() != timeTo.time().hour())
    {
        qint64 msecsTo = time.time().msecsTo(QTime(timeTo.time().hour(), 0));
        if (time.date().day() != timeTo.date().day())
        {
            msecsTo += 24 * 60 * 60 * 1000;
        }
        m_animation = SpecialFadeIn;
        m_alphaModifier = (float)cTimerFast * 100.0f / (float)cFadeInTime;
        m_alpha = 0;
        m_X_modifier = 0;
        unsigned int x = (m_screen.width() - geometry().width())/2.0f;
        unsigned int y = (m_screen.height() - geometry().height())/2.0f;
        setPos(x,y);
        setTimerSpeed(msecsTo);
    }
    else
    {
        setTimerSpeed(aMecs);
    }
}

void bkodamaapplet::createConfigurationInterface(KConfigDialog *aParent)
{
    QWidget *widget = new QWidget;
    ui.setupUi(widget);

    //Sound
    ui.soundEnabled->setChecked(m_soundEnabled);
    ui.soundVolumeLabel->setEnabled(m_soundEnabled);
    ui.soundVolume->setEnabled(m_soundEnabled);
    ui.soundVolume->setSliderPosition(m_soundVolume * 100);

    //Animation
    ui.m_timeBetweenAppearance->setValue((double)m_veryLongBreakTime / (60.0 * 1000.0));
    ui.m_specialHourlyEventCheckbox->setChecked(m_specialEvent);

    aParent->addPage(widget, i18n("General"), icon());
    connect(aParent, SIGNAL(accepted()), this, SLOT(configAccepted()));
}

void bkodamaapplet::configAccepted()
{
    KConfigGroup cg = config();

    //Sound
    m_soundEnabled = (ui.soundEnabled->checkState() == Qt::Checked);
    m_soundVolume = float(ui.soundVolume->value()) / 100.0f;

    //Animation
    m_veryLongBreakTime = ui.m_timeBetweenAppearance->value() * 60 * 1000;
    m_specialEvent = ui.m_specialHourlyEventCheckbox->checkState() == Qt::Checked;

    saveState(cg);
    emit configNeedsSaving();

    initSound();

    //mouse - undo the mouse clicked
    m_mousePressed = false;

    update();
}

// void bkodamaapplet::screenResized(int screen)
// {
//     kDebug() <<  screen << m_screenNumber;
//     if (screen == m_screenNumber)
//         return;
//
//     if (KApplication::desktop()->numScreens() > 1 || KApplication::desktop()->isVirtualDesktop())
//     {
//         m_screen = KWindowSystem::workArea().intersect(KApplication::desktop()->screenGeometry(m_screenNumber));
//     }
//     else
//     {
//         m_screen = KWindowSystem::workArea();
//     }
//
//     kDebug() << "width: " << m_screen.width();
//     kDebug() << "height: " << m_screen.height();
//     QSizeF oldSize = m_screen.size();
//
//     // 500/800 -> svg size
//     setMaximumSize(QSizeF(oldSize.height() * 500.0f / (800.0f * 5.0f), oldSize.height() / 5.0));
//     resize(qBound(minimumWidth(), oldSize.width(), maximumWidth()), qBound(minimumHeight(), oldSize.height(), maximumHeight()));
//
//     updateScaledImage();
// }

void bkodamaapplet::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    m_mousePressed = false;
    event->accept();
}

void bkodamaapplet::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    m_mousePressed = true;
    event->accept();
}

void bkodamaapplet::resizeEvent(QGraphicsSceneResizeEvent * event)
{
//it crashes if the don't stop the sound here, I wonder why
    if (m_sound && (m_sound->state() == Phonon::PlayingState || m_sound->state() == Phonon::BufferingState))
    {
        m_sound->stop();
    }
    Plasma::Applet::resizeEvent(event);
}

QVariant bkodamaapplet::itemChange(GraphicsItemChange change, const QVariant & value)
{
    //bypass Plasma::Applet when it comes to position changes, because otherwise
    //an locked applet can not move
    QVariant ret;
    if (change == QGraphicsItem::ItemPositionChange || change == QGraphicsItem::ItemPositionHasChanged)
    {
        ret = QGraphicsWidget::itemChange(change, value);
    }
    else
    {
        ret = Plasma::Applet::itemChange(change, value);
    }
    return ret;
}

void bkodamaapplet::readConfiguration()
{
    KConfigGroup cg = config();

    //Sound
    m_soundEnabled = cg.readEntry("SoundEnabled", true);
    m_soundVolume = float(cg.readEntry("SoundVolume", 100)) / 100.0f;

    //Animation
    m_veryLongBreakTime = cg.readEntry("TimeBewteenAppearance", 35000);
    m_specialEvent = cg.readEntry("SpecialEvent", true);
}

void bkodamaapplet::initSound()
{
    if (m_soundEnabled)
    {
        if (!m_sound)
        {
            m_sound = new Phonon::MediaObject(this);
            m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
            m_sound->setCurrentSource(m_soundUrl);
            createPath(m_sound, m_audioOutput);
        }
        m_audioOutput->setVolume(m_soundVolume);
    }
    else
    {
        delete m_sound;
        m_sound = 0;
        delete m_audioOutput;
        m_audioOutput = 0;
    }
}

#include "bkodama.moc"
