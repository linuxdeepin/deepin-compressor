// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_COMPRESSOR_METHOD_H
#define ORG_DEEPIN_COMPRESSOR_METHOD_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_compressor_method : public QObject {
    Q_OBJECT

    Q_PROPERTY(double special7zCompressor READ special7zCompressor WRITE setSpecial7zCompressor NOTIFY special7zCompressorChanged)
    Q_PROPERTY(double specialCompressorType READ specialCompressorType WRITE setSpecialCompressorType NOTIFY specialCompressorTypeChanged)
    Q_PROPERTY(double specialCpuTarGzCompressor READ specialCpuTarGzCompressor WRITE setSpecialCpuTarGzCompressor NOTIFY specialCpuTarGzCompressorChanged)
    Q_PROPERTY(double specialJarCompressor READ specialJarCompressor WRITE setSpecialJarCompressor NOTIFY specialJarCompressorChanged)
    Q_PROPERTY(double specialTar7zCompressor READ specialTar7zCompressor WRITE setSpecialTar7zCompressor NOTIFY specialTar7zCompressorChanged)
    Q_PROPERTY(double specialTarCompressor READ specialTarCompressor WRITE setSpecialTarCompressor NOTIFY specialTarCompressorChanged)
    Q_PROPERTY(double specialTarGzCompressor READ specialTarGzCompressor WRITE setSpecialTarGzCompressor NOTIFY specialTarGzCompressorChanged)
    Q_PROPERTY(double specialTarLzCompressor READ specialTarLzCompressor WRITE setSpecialTarLzCompressor NOTIFY specialTarLzCompressorChanged)
    Q_PROPERTY(double specialTarLzmaCompressor READ specialTarLzmaCompressor WRITE setSpecialTarLzmaCompressor NOTIFY specialTarLzmaCompressorChanged)
    Q_PROPERTY(double specialTarLzoCompressor READ specialTarLzoCompressor WRITE setSpecialTarLzoCompressor NOTIFY specialTarLzoCompressorChanged)
    Q_PROPERTY(double specialTarXzCompressor READ specialTarXzCompressor WRITE setSpecialTarXzCompressor NOTIFY specialTarXzCompressorChanged)
    Q_PROPERTY(double specialTarZCompressor READ specialTarZCompressor WRITE setSpecialTarZCompressor NOTIFY specialTarZCompressorChanged)
    Q_PROPERTY(double specialTarbz2Compressor READ specialTarbz2Compressor WRITE setSpecialTarbz2Compressor NOTIFY specialTarbz2CompressorChanged)
    Q_PROPERTY(double specialZipCompressor READ specialZipCompressor WRITE setSpecialZipCompressor NOTIFY specialZipCompressorChanged)
public:
    explicit org_deepin_compressor_method(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(appId, name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    explicit org_deepin_compressor_method(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(backend, appId, name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    explicit org_deepin_compressor_method(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    explicit org_deepin_compressor_method(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(backend, name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    ~org_deepin_compressor_method() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    double special7zCompressor() const {
        return p_special7zCompressor;
    }
    void setSpecial7zCompressor(const double &value) {
        auto oldValue = p_special7zCompressor;
        p_special7zCompressor = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("special7zCompressor"), value);
            });
        }
        if (p_special7zCompressor != oldValue) {
            Q_EMIT special7zCompressorChanged();
        }
    }
    double specialCompressorType() const {
        return p_specialCompressorType;
    }
    void setSpecialCompressorType(const double &value) {
        auto oldValue = p_specialCompressorType;
        p_specialCompressorType = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialCompressorType"), value);
            });
        }
        if (p_specialCompressorType != oldValue) {
            Q_EMIT specialCompressorTypeChanged();
        }
    }
    double specialCpuTarGzCompressor() const {
        return p_specialCpuTarGzCompressor;
    }
    void setSpecialCpuTarGzCompressor(const double &value) {
        auto oldValue = p_specialCpuTarGzCompressor;
        p_specialCpuTarGzCompressor = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialCpuTarGzCompressor"), value);
            });
        }
        if (p_specialCpuTarGzCompressor != oldValue) {
            Q_EMIT specialCpuTarGzCompressorChanged();
        }
    }
    double specialJarCompressor() const {
        return p_specialJarCompressor;
    }
    void setSpecialJarCompressor(const double &value) {
        auto oldValue = p_specialJarCompressor;
        p_specialJarCompressor = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialJarCompressor"), value);
            });
        }
        if (p_specialJarCompressor != oldValue) {
            Q_EMIT specialJarCompressorChanged();
        }
    }
    double specialTar7zCompressor() const {
        return p_specialTar7zCompressor;
    }
    void setSpecialTar7zCompressor(const double &value) {
        auto oldValue = p_specialTar7zCompressor;
        p_specialTar7zCompressor = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTar7zCompressor"), value);
            });
        }
        if (p_specialTar7zCompressor != oldValue) {
            Q_EMIT specialTar7zCompressorChanged();
        }
    }
    double specialTarCompressor() const {
        return p_specialTarCompressor;
    }
    void setSpecialTarCompressor(const double &value) {
        auto oldValue = p_specialTarCompressor;
        p_specialTarCompressor = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarCompressor"), value);
            });
        }
        if (p_specialTarCompressor != oldValue) {
            Q_EMIT specialTarCompressorChanged();
        }
    }
    double specialTarGzCompressor() const {
        return p_specialTarGzCompressor;
    }
    void setSpecialTarGzCompressor(const double &value) {
        auto oldValue = p_specialTarGzCompressor;
        p_specialTarGzCompressor = value;
        markPropertySet(6);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarGzCompressor"), value);
            });
        }
        if (p_specialTarGzCompressor != oldValue) {
            Q_EMIT specialTarGzCompressorChanged();
        }
    }
    double specialTarLzCompressor() const {
        return p_specialTarLzCompressor;
    }
    void setSpecialTarLzCompressor(const double &value) {
        auto oldValue = p_specialTarLzCompressor;
        p_specialTarLzCompressor = value;
        markPropertySet(7);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarLzCompressor"), value);
            });
        }
        if (p_specialTarLzCompressor != oldValue) {
            Q_EMIT specialTarLzCompressorChanged();
        }
    }
    double specialTarLzmaCompressor() const {
        return p_specialTarLzmaCompressor;
    }
    void setSpecialTarLzmaCompressor(const double &value) {
        auto oldValue = p_specialTarLzmaCompressor;
        p_specialTarLzmaCompressor = value;
        markPropertySet(8);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarLzmaCompressor"), value);
            });
        }
        if (p_specialTarLzmaCompressor != oldValue) {
            Q_EMIT specialTarLzmaCompressorChanged();
        }
    }
    double specialTarLzoCompressor() const {
        return p_specialTarLzoCompressor;
    }
    void setSpecialTarLzoCompressor(const double &value) {
        auto oldValue = p_specialTarLzoCompressor;
        p_specialTarLzoCompressor = value;
        markPropertySet(9);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarLzoCompressor"), value);
            });
        }
        if (p_specialTarLzoCompressor != oldValue) {
            Q_EMIT specialTarLzoCompressorChanged();
        }
    }
    double specialTarXzCompressor() const {
        return p_specialTarXzCompressor;
    }
    void setSpecialTarXzCompressor(const double &value) {
        auto oldValue = p_specialTarXzCompressor;
        p_specialTarXzCompressor = value;
        markPropertySet(10);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarXzCompressor"), value);
            });
        }
        if (p_specialTarXzCompressor != oldValue) {
            Q_EMIT specialTarXzCompressorChanged();
        }
    }
    double specialTarZCompressor() const {
        return p_specialTarZCompressor;
    }
    void setSpecialTarZCompressor(const double &value) {
        auto oldValue = p_specialTarZCompressor;
        p_specialTarZCompressor = value;
        markPropertySet(11);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarZCompressor"), value);
            });
        }
        if (p_specialTarZCompressor != oldValue) {
            Q_EMIT specialTarZCompressorChanged();
        }
    }
    double specialTarbz2Compressor() const {
        return p_specialTarbz2Compressor;
    }
    void setSpecialTarbz2Compressor(const double &value) {
        auto oldValue = p_specialTarbz2Compressor;
        p_specialTarbz2Compressor = value;
        markPropertySet(12);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialTarbz2Compressor"), value);
            });
        }
        if (p_specialTarbz2Compressor != oldValue) {
            Q_EMIT specialTarbz2CompressorChanged();
        }
    }
    double specialZipCompressor() const {
        return p_specialZipCompressor;
    }
    void setSpecialZipCompressor(const double &value) {
        auto oldValue = p_specialZipCompressor;
        p_specialZipCompressor = value;
        markPropertySet(13);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("specialZipCompressor"), value);
            });
        }
        if (p_specialZipCompressor != oldValue) {
            Q_EMIT specialZipCompressorChanged();
        }
    }
Q_SIGNALS:
    void special7zCompressorChanged();
    void specialCompressorTypeChanged();
    void specialCpuTarGzCompressorChanged();
    void specialJarCompressorChanged();
    void specialTar7zCompressorChanged();
    void specialTarCompressorChanged();
    void specialTarGzCompressorChanged();
    void specialTarLzCompressorChanged();
    void specialTarLzmaCompressorChanged();
    void specialTarLzoCompressorChanged();
    void specialTarXzCompressorChanged();
    void specialTarZCompressorChanged();
    void specialTarbz2CompressorChanged();
    void specialZipCompressorChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("special7zCompressor"), QVariant::fromValue(p_special7zCompressor));
        } else {
            updateValue(QStringLiteral("special7zCompressor"), QVariant::fromValue(p_special7zCompressor));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("specialCompressorType"), QVariant::fromValue(p_specialCompressorType));
        } else {
            updateValue(QStringLiteral("specialCompressorType"), QVariant::fromValue(p_specialCompressorType));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("specialCpuTarGzCompressor"), QVariant::fromValue(p_specialCpuTarGzCompressor));
        } else {
            updateValue(QStringLiteral("specialCpuTarGzCompressor"), QVariant::fromValue(p_specialCpuTarGzCompressor));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("specialJarCompressor"), QVariant::fromValue(p_specialJarCompressor));
        } else {
            updateValue(QStringLiteral("specialJarCompressor"), QVariant::fromValue(p_specialJarCompressor));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("specialTar7zCompressor"), QVariant::fromValue(p_specialTar7zCompressor));
        } else {
            updateValue(QStringLiteral("specialTar7zCompressor"), QVariant::fromValue(p_specialTar7zCompressor));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("specialTarCompressor"), QVariant::fromValue(p_specialTarCompressor));
        } else {
            updateValue(QStringLiteral("specialTarCompressor"), QVariant::fromValue(p_specialTarCompressor));
        }
        if (testPropertySet(6)) {
            config->setValue(QStringLiteral("specialTarGzCompressor"), QVariant::fromValue(p_specialTarGzCompressor));
        } else {
            updateValue(QStringLiteral("specialTarGzCompressor"), QVariant::fromValue(p_specialTarGzCompressor));
        }
        if (testPropertySet(7)) {
            config->setValue(QStringLiteral("specialTarLzCompressor"), QVariant::fromValue(p_specialTarLzCompressor));
        } else {
            updateValue(QStringLiteral("specialTarLzCompressor"), QVariant::fromValue(p_specialTarLzCompressor));
        }
        if (testPropertySet(8)) {
            config->setValue(QStringLiteral("specialTarLzmaCompressor"), QVariant::fromValue(p_specialTarLzmaCompressor));
        } else {
            updateValue(QStringLiteral("specialTarLzmaCompressor"), QVariant::fromValue(p_specialTarLzmaCompressor));
        }
        if (testPropertySet(9)) {
            config->setValue(QStringLiteral("specialTarLzoCompressor"), QVariant::fromValue(p_specialTarLzoCompressor));
        } else {
            updateValue(QStringLiteral("specialTarLzoCompressor"), QVariant::fromValue(p_specialTarLzoCompressor));
        }
        if (testPropertySet(10)) {
            config->setValue(QStringLiteral("specialTarXzCompressor"), QVariant::fromValue(p_specialTarXzCompressor));
        } else {
            updateValue(QStringLiteral("specialTarXzCompressor"), QVariant::fromValue(p_specialTarXzCompressor));
        }
        if (testPropertySet(11)) {
            config->setValue(QStringLiteral("specialTarZCompressor"), QVariant::fromValue(p_specialTarZCompressor));
        } else {
            updateValue(QStringLiteral("specialTarZCompressor"), QVariant::fromValue(p_specialTarZCompressor));
        }
        if (testPropertySet(12)) {
            config->setValue(QStringLiteral("specialTarbz2Compressor"), QVariant::fromValue(p_specialTarbz2Compressor));
        } else {
            updateValue(QStringLiteral("specialTarbz2Compressor"), QVariant::fromValue(p_specialTarbz2Compressor));
        }
        if (testPropertySet(13)) {
            config->setValue(QStringLiteral("specialZipCompressor"), QVariant::fromValue(p_specialZipCompressor));
        } else {
            updateValue(QStringLiteral("specialZipCompressor"), QVariant::fromValue(p_specialZipCompressor));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("special7zCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_special7zCompressor != newValue) {
                    p_special7zCompressor = newValue;
                    Q_EMIT special7zCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialCompressorType")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialCompressorType != newValue) {
                    p_specialCompressorType = newValue;
                    Q_EMIT specialCompressorTypeChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialCpuTarGzCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialCpuTarGzCompressor != newValue) {
                    p_specialCpuTarGzCompressor = newValue;
                    Q_EMIT specialCpuTarGzCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialJarCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialJarCompressor != newValue) {
                    p_specialJarCompressor = newValue;
                    Q_EMIT specialJarCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTar7zCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTar7zCompressor != newValue) {
                    p_specialTar7zCompressor = newValue;
                    Q_EMIT specialTar7zCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarCompressor != newValue) {
                    p_specialTarCompressor = newValue;
                    Q_EMIT specialTarCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarGzCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarGzCompressor != newValue) {
                    p_specialTarGzCompressor = newValue;
                    Q_EMIT specialTarGzCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarLzCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarLzCompressor != newValue) {
                    p_specialTarLzCompressor = newValue;
                    Q_EMIT specialTarLzCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarLzmaCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarLzmaCompressor != newValue) {
                    p_specialTarLzmaCompressor = newValue;
                    Q_EMIT specialTarLzmaCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarLzoCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarLzoCompressor != newValue) {
                    p_specialTarLzoCompressor = newValue;
                    Q_EMIT specialTarLzoCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarXzCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarXzCompressor != newValue) {
                    p_specialTarXzCompressor = newValue;
                    Q_EMIT specialTarXzCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarZCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarZCompressor != newValue) {
                    p_specialTarZCompressor = newValue;
                    Q_EMIT specialTarZCompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialTarbz2Compressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialTarbz2Compressor != newValue) {
                    p_specialTarbz2Compressor = newValue;
                    Q_EMIT specialTarbz2CompressorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("specialZipCompressor")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_specialZipCompressor != newValue) {
                    p_specialZipCompressor = newValue;
                    Q_EMIT specialZipCompressorChanged();
                }
            });
            return;
        }
    }
    inline void markPropertySet(const int index) {
        if (index < 32) {
            m_propertySetStatus0.fetchAndOrOrdered(1 << (index - 0));
            return;
        }
        Q_UNREACHABLE();
    }
    inline bool testPropertySet(const int index) const {
        if (index < 32) {
            return (m_propertySetStatus0.loadRelaxed() & (1 << (index - 0)));
        }
        Q_UNREACHABLE();
    }
    QAtomicPointer<DTK_CORE_NAMESPACE::DConfig> m_config = nullptr;
    double p_special7zCompressor { 0 };
    double p_specialCompressorType { 11 };
    double p_specialCpuTarGzCompressor { 3 };
    double p_specialJarCompressor { 0 };
    double p_specialTar7zCompressor { 0 };
    double p_specialTarCompressor { 0 };
    double p_specialTarGzCompressor { 0 };
    double p_specialTarLzCompressor { 0 };
    double p_specialTarLzmaCompressor { 0 };
    double p_specialTarLzoCompressor { 0 };
    double p_specialTarXzCompressor { 0 };
    double p_specialTarZCompressor { 0 };
    double p_specialTarbz2Compressor { 0 };
    double p_specialZipCompressor { 0 };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_COMPRESSOR_METHOD_H
