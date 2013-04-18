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

#include "pjsengine.h"

#include <Qt>
#include <QApplication>
#include <QDebug>
#include <iostream>

namespace JS
{

// public:

PJSEngine::PJSEngine(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
    , m_terminated(false)
    , m_js((QJSEngine *) NULL)
    , m_console((JS::Console *) NULL)
    , m_timers()
{
}

PJSEngine::~PJSEngine()
{
    qDeleteAll(m_timers);
}

bool PJSEngine::init()
{
    // Bail out if already initialized
    if (m_initialized) {
        return false;
    }

    if ((QJSEngine *) NULL == m_js) {
        m_js = new QJSEngine(this);
    }

    if ((JS::Console *) NULL == m_console) {
        m_console = new JS::Console(this);
        QJSValue console = m_js->newQObject(m_console);
        m_js->globalObject().setProperty("console", console);
    }

    QJSValue me = m_js->newQObject(this);

    m_js->globalObject().setProperty("_createSingleShotTimer", me.property("createSingleShotTimer"));
    m_js->globalObject().setProperty("_createRepeatingTimer", me.property("createRepeatingTimer"));
    QJSValue timerFuncs = m_js->evaluate(
        "(function () {"
            "function timerFunc(timerFactory) {"
                "return function (cb, ms) {"
                    "var t = timerFactory(ms);"
                    "t.timeout.connect(cb);"
                    "return t.timerId;"
                "};"
            "}"
            "return {"
                " setTimeout: timerFunc(_createSingleShotTimer)"
                ",setInterval: timerFunc(_createRepeatingTimer)"
            "};"
        "}())"
    );
    m_js->globalObject().setProperty("setTimeout", timerFuncs.property("setTimeout"));
    m_js->globalObject().setProperty("setInterval", timerFuncs.property("setInterval"));
    m_js->globalObject().setProperty("clearTimeout", me.property("clearTimer"));
    m_js->globalObject().setProperty("clearInterval", me.property("clearTimer"));

    QJSValue phantom = m_js->newObject();
    phantom.setProperty("exit", me.property("exit"));
    m_js->globalObject().setProperty("phantom", phantom);

    m_initialized = true;

    return true;
}

void PJSEngine::evaluate(const QString &src, const QString &file)
{
    if (!m_initialized) {
        return;
    }

    QJSValue result = m_js->evaluate(src, file);
    if (result.isError()) {
        qDebug() << "uncaught exception:" << result.toString();
    }
}

// public slots:

void PJSEngine::exit(int code)
{
    // TODO
    std::cerr << "EXIT(" << code << ")" << std::endl;
    QApplication::instance()->exit(code);
}

bool PJSEngine::isTerminated() const
{
    return m_terminated;
}

QObject *PJSEngine::createSingleShotTimer(int ms)
{
    return _createTimer(ms, true);
}

QObject *PJSEngine::createRepeatingTimer(int ms)
{
    return _createTimer(ms, false);
}

void PJSEngine::clearTimer(int id)
{
    // TODO: how to prevent double kills?
    QObject::killTimer(id);
    m_timers.remove(id);
}

void PJSEngine::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();
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

QObject *PJSEngine::_createTimer(int ms, bool isSingleShot)
{
    // TODO: Use Qt::CoarseTimer for long timeouts?
    Qt::TimerType timerType = Qt::PreciseTimer;
    JS::TimerContext *tc = new JS::TimerContext(QObject::startTimer(ms, timerType), isSingleShot, this);
    m_timers.insert(tc->timerId(), tc);
    return tc;
}

};

// vim:ts=4:sw=4:sts=4:et:
