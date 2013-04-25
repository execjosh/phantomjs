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

#ifndef JS_PJSENGINE_H
#define JS_PJSENGINE_H

#include <QObject>
#include <QList>
#include <QPointer>
#include <QJSEngine>
#include <QJSValue>

#include "console.h"
#include "timers.h"
#include "nativemodules.h"

#include "../webpage.h"

namespace JS
{

class PJSEngine : public QObject
{
    Q_OBJECT

public:
    explicit PJSEngine(QObject *parent = 0);
    virtual ~PJSEngine();

    bool init();
    QJSValue evaluate(const QString &src, const QString &file = "");

signals:

public slots:
    void exit(int code = 0);
    bool isTerminated() const;

    // Module stuff
    QJSValue loadModule(const QString &moduleSource, const QString &filename);
    QObject *createWebPage();

private:
    bool m_initialized;
    bool m_terminated;
    QJSEngine *m_js;
    JS::Console *m_console;
    Timers *m_timers;
    NativeModules *m_nativemodules;
    QList<QPointer<WebPage> > m_pages;
};

};

#endif // JS_PJSENGINE_H

// vim:ts=4:sw=4:sts=4:et:
