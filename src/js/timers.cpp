/*
  This file is part of the PhantomJS project from Ofi Labs.

  Copyright (C) 2013 execjosh, http://execjosh.blogspot.com

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "timers.h"

#include <Qt>
#include <QTimerEvent>

namespace JS
{

// public:

Timers::Timers(QObject *parent)
    : QObject(parent)
    , m_timers()
{
}

Timers::~Timers()
{
    qDeleteAll(m_timers);
}

// public slots:

QObject *Timers::createSingleShotTimer(int ms)
{
    return _createTimer(ms, true);
}

QObject *Timers::createRepeatingTimer(int ms)
{
    return _createTimer(ms, false);
}

void Timers::clearTimer(int id)
{
    // TODO: how to prevent double kills?
    QObject::killTimer(id);
    m_timers.remove(id);
}

// protected:

void Timers::timerEvent(QTimerEvent *evt)
{
    int id = evt->timerId();
    JS::TimerContext *tc = m_timers.value(id);
    if (-1 == tc->timerId()) {
        QObject::killTimer(id);
    } else if (tc->isSingleShot()) {
        clearTimer(id);
        tc->invokeTimeout();
        tc->deleteLater();
    } else {
        tc->invokeTimeout();
    }
}

// private:

QObject *Timers::_createTimer(int ms, bool isSingleShot)
{
    // TODO: Use Qt::CoarseTimer for long timeouts?
    Qt::TimerType timerType = Qt::PreciseTimer;
    JS::TimerContext *tc = new JS::TimerContext(QObject::startTimer(ms, timerType), isSingleShot, this);
    m_timers.insert(tc->timerId(), tc);
    return tc;
}

};

// vim:ts=4:sw=4:sts=4:et:
