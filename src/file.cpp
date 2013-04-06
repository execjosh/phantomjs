/*
  This file is part of the PhantomJS project from Ofi Labs.

  Copyright (C) 2011 Ivan De Marino <ivan.de.marino@gmail.com>
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

#include "file.h"

#include <QDebug>

#include "asyncreadrequest.h"

// public:
File::File(QFile *openfile, QTextCodec *codec, QObject *parent) :
    QObject(parent),
    m_file(openfile),
    m_fileStream(0)
{
    if ( codec ) {
        m_fileStream = new QTextStream(m_file);
        m_fileStream->setCodec(codec);
    }
}

File::~File()
{
    this->close();
}

//NOTE: for binary files we want to use QString instead of QByteArray as the
//      latter is not really useable in javascript and e.g. window.btoa expects a string
//      and we need special code required since fromAsci() would stop as soon as it
//      encounters \0 or similar

// public slots:
QString File::read(const QVariant &n)
{
    // Default to 1024 (used when n is "null")
    qint64 bytesToRead = 1024;

    // If parameter can be converted to a qint64, do so and use that value instead
    if (n.canConvert(QVariant::LongLong)) {
        bytesToRead = n.toLongLong();
    }

    const bool isReadAll = 0 > bytesToRead;

    if ( !m_file->isReadable() ) {
        qDebug() << "File::read - " << "Couldn't read:" << m_file->fileName();
        return QString();
    }
    if ( m_file->isWritable() ) {
        // make sure we write everything to disk before reading
        flush();
    }
    if ( m_fileStream ) {
        // text file
        QString ret;
        if (isReadAll) {
            // This code, for some reason, reads the whole file from 0 to EOF,
            // and then resets to the position the file was at prior to reading
            const qint64 pos = m_fileStream->pos();
            m_fileStream->seek(0);
            ret = m_fileStream->readAll();
            m_fileStream->seek(pos);
        } else {
            ret = m_fileStream->read(bytesToRead);
        }
        return ret;
    } else {
        // binary file
        QByteArray data;
        if (isReadAll) {
            // This code, for some reason, reads the whole file from 0 to EOF,
            // and then resets to the position the file was at prior to reading
            const qint64 pos = m_file->pos();
            m_file->seek(0);
            data = m_file->readAll();
            m_file->seek(pos);
        } else {
            data = m_file->read(bytesToRead);
        }
        QString ret(data.size());
        for(int i = 0; i < data.size(); ++i) {
            ret[i] = data.at(i);
        }
        return ret;
    }
}

QObject *File::_getAsyncReadRequest(const QVariant &n)
{
    return new AsyncReadRequest(this, n, this);
}

bool File::write(const QString &data)
{
    if ( !m_file->isWritable() ) {
        qDebug() << "File::write - " << "Couldn't write:" << m_file->fileName();
        return true;
    }
    if ( m_fileStream ) {
        // text file
        (*m_fileStream) << data;
        return true;
    } else {
        // binary file
        QByteArray bytes(data.size(), Qt::Uninitialized);
        for(int i = 0; i < data.size(); ++i) {
            bytes[i] = data.at(i).toAscii();
        }
        return m_file->write(bytes);
    }
}

bool File::seek(const qint64 pos)
{
    if (m_fileStream) {
        return m_fileStream->seek(pos);
    } else {
        return m_file->seek(pos);
    }
}

QString File::readLine()
{
    if ( !m_file->isReadable() ) {
        qDebug() << "File::readLine - " << "Couldn't read:" << m_file->fileName();
        return QString();
    }
    if ( m_file->isWritable() ) {
        // make sure we write everything to disk before reading
        flush();
    }
    if ( m_fileStream ) {
        // text file
        return m_fileStream->readLine();
    } else {
        // binary file - doesn't make much sense but well...
        return QString::fromAscii(m_file->readLine());
    }
}

bool File::writeLine(const QString &data)
{
    if ( write(data) && write("\n") ) {
        return true;
    }
    qDebug() << "File::writeLine - " << "Couldn't write:" << m_file->fileName();
    return false;
}

bool File::atEnd() const
{
    if ( m_file->isReadable() ) {
        if (m_fileStream) {
            // text file
            return m_fileStream->atEnd();
        } else {
            // binary file
            return m_file->atEnd();
        }
    }
    qDebug() << "File::atEnd - " << "Couldn't read:" << m_file->fileName();
    return false;
}

void File::flush()
{
    if ( m_file ) {
        if ( m_fileStream ) {
            // text file
            m_fileStream->flush();
        }
        // binary or text file
        m_file->flush();
    }
}

void File::close()
{
    flush();
    if ( m_fileStream ) {
        delete m_fileStream;
        m_fileStream = 0;
    }
    if ( m_file ) {
        m_file->close();
        delete m_file;
        m_file = NULL;
    }
    deleteLater();
}
