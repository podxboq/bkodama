/*
 * Copyright 2009  Bernd Buschinski b.buschinski@web.de
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

#ifndef BRAND_H
#define BRAND_H

#include <time.h>
#include <stdlib.h>
#include <QtGlobal>

static bool sRandInit = false;

/**
 *
 * @param aRand the max number
 * @return a Value between 0 and aRand
 */
static inline unsigned int brand( unsigned int aRand )
{
    if (!sRandInit)
    {
        qsrand ( time(NULL) );
        sRandInit = true;
    }
    return uint( (double(aRand) * (qrand() / (RAND_MAX + 1.0f))) + 0.5f );
}

/**
 *
 * @param aRand the max number
 * @return a Value between -aRand and aRand
 */
static inline int negbrand( int aRand )
{
    return (brand( aRand*2 ) - aRand);
}

#endif //BRAND_H
