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
    , m_timers((JS::Timers *) NULL)
    , m_nativemodules((JS::NativeModules *) NULL)
{
}

PJSEngine::~PJSEngine()
{
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
        QJSValue _console = m_js->newQObject(m_console);
        m_js->globalObject().setProperty("_console", _console);
        QJSValue console = m_js->evaluate(
            "(function () {"
                "var __slice = [].slice;"
                "function log() {"
                    "var args = 0 === arguments.length ? [] : __slice.apply(arguments);"
                    // TODO: `printf` type arguments?
                    "return _console.log(args.join(' '));"
                "}"
                "return {"
                    "log: log"
                "};"
            "}())"
        );
        m_js->globalObject().setProperty("console", console);
    }

    if ((JS::Timers *) NULL == m_timers) {
        m_timers = new JS::Timers(this);
    }
    QJSValue timers = m_js->newQObject(m_timers);

    if ((JS::NativeModules *) NULL == m_nativemodules) {
        m_nativemodules = new JS::NativeModules(this);
    }
    QJSValue nativeModules = m_js->newQObject(m_nativemodules);

    QJSValue me = m_js->newQObject(this);

    QJSValue createTimerFunc = m_js->evaluate(
        "(function timerFunc(timerFactory) {\n"
            "return function (cb, ms) {\n"
                "var t = timerFactory(ms)\n"
                "t.timeout.connect(cb)\n"
                "return t.timerId\n"
            "}\n"
        "})"
    , "createTimerFunc");
    m_js->globalObject().setProperty("setTimeout", createTimerFunc.call(QJSValueList() << timers.property("createSingleShotTimer")));
    m_js->globalObject().setProperty("setInterval", createTimerFunc.call(QJSValueList() << timers.property("createRepeatingTimer")));
    m_js->globalObject().setProperty("clearTimeout", timers.property("clearTimer"));
    m_js->globalObject().setProperty("clearInterval", timers.property("clearTimer"));

    QJSValue phantom = m_js->newObject();
    phantom.setProperty("exit", me.property("exit"));
    phantom.setProperty("loadModule", me.property("loadModule"));
    phantom.setProperty("_createChildProcess", nativeModules.property("getChildProcess"));
    phantom.setProperty("createFilesystem", nativeModules.property("getFileSystem"));
    phantom.setProperty("createSystem", nativeModules.property("getSystem"));
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

QJSValue PJSEngine::loadModule(const QString &moduleSource, const QString &filename)
{
    if (isTerminated()) {
        return QJSValue(QJSValue::UndefinedValue);
    }

   QString scriptSource =
      "(function(require, exports, module) {" +
      moduleSource +
      "}.call({},"
      "require.cache['" + filename + "']._getRequire(),"
      "require.cache['" + filename + "'].exports,"
      "require.cache['" + filename + "']"
      "));";
   QJSValue res = m_js->evaluate(scriptSource, filename);
   if (res.isError()) {
       std::cerr << "uncaught exception: " << qPrintable(res.property("stack").toString()) << std::endl;
   }
   return res;
}

};

// vim:ts=4:sw=4:sts=4:et:
