/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef KJOB_H
#define KJOB_H

#include <QObject>
#include <QPair>


class KJobPrivate;

class  KJob : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int error READ error NOTIFY result)
    Q_PROPERTY(QString errorText READ errorText NOTIFY result)
    Q_PROPERTY(QString errorString READ errorString NOTIFY result)
    Q_PROPERTY(unsigned long percent READ percent NOTIFY percent)
    Q_PROPERTY(Capabilities capabilities READ capabilities CONSTANT)

public:
    enum Unit { Bytes, Files, Directories };
    Q_ENUM(Unit)

    enum Capability { NoCapabilities = 0x0000,
                      Killable       = 0x0001,
                      Suspendable    = 0x0002
                    };
    Q_ENUM(Capability)

    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)

    explicit KJob(QObject *parent = nullptr);
    ~KJob() override;

    Capabilities capabilities() const;
    bool isSuspended() const;
    Q_SCRIPTABLE virtual void start() = 0;

    enum KillVerbosity { Quietly, EmitResult };
    Q_ENUM(KillVerbosity)

public Q_SLOTS:
    bool kill(KillVerbosity verbosity = Quietly);
    bool suspend();
    bool resume();

protected:
    virtual bool doKill();
    virtual bool doSuspend();
    virtual bool doResume();
    void setCapabilities(Capabilities capabilities);

public:
    bool exec();

    enum {
        /*** Indicates there is no error */
        NoError = 0,
        /*** Indicates the job was killed */
        KilledJobError = 1,
        LoadError = 2,
        /*** Subclasses should define error codes starting at this value */
        UserSkiped = 3,
        UserDefinedError = 100
    };

    int error() const;
    QString errorText() const;
    virtual QString errorString() const;
    Q_SCRIPTABLE qulonglong processedAmount(Unit unit) const;
    Q_SCRIPTABLE qulonglong totalAmount(Unit unit) const;
    unsigned long percent() const;
    void setAutoDelete(bool autodelete);
    bool isAutoDelete() const;

Q_SIGNALS:
    void finished(KJob *job
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
                  , QPrivateSignal
#endif
                 );
    void suspended(KJob *job
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
                   , QPrivateSignal
#endif
                  );

    void resumed(KJob *job
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
                 , QPrivateSignal
#endif
                );

    void result(KJob *job
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
                , QPrivateSignal
#endif
               );

    void description(KJob *job, const QString &title,
                     const QPair<QString, QString> &field1 = QPair<QString, QString>(),
                     const QPair<QString, QString> &field2 = QPair<QString, QString>());
    void infoMessage(KJob *job, const QString &plain, const QString &rich = QString());
    void warning(KJob *job, const QString &plain, const QString &rich = QString());

Q_SIGNALS:
    void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount);
    void totalSize(KJob *job, qulonglong size);
    void processedSize(KJob *job, qulonglong size);
    void percent(KJob *job, unsigned long percent);
    void percentfilename(KJob *job, const QString &filename);
    void speed(KJob *job, unsigned long speed);

protected:
    void setError(int errorCode);
    void setErrorText(const QString &errorText);
    void setProcessedAmount(Unit unit, qulonglong amount);
    void setTotalAmount(Unit unit, qulonglong amount);
    void setPercent(unsigned long percentage);
    void setPercentFilename(const QString &filename);
    void emitResult();
    void emitPercent(qulonglong processedAmount, qulonglong totalAmount);
    void emitSpeed(unsigned long speed);

protected:
    KJobPrivate *const d_ptr;
    KJob(KJobPrivate &dd, QObject *parent);

private:
    void finishJob(bool emitResult);

    Q_DECLARE_PRIVATE(KJob)
};

Q_DECLARE_METATYPE(KJob::Unit)
Q_DECLARE_OPERATORS_FOR_FLAGS(KJob::Capabilities)

#endif
