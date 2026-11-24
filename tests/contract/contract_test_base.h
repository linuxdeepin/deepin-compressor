// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTRACT_TEST_BASE_H
#define CONTRACT_TEST_BASE_H

#include "backend_factory.h"
#include "fakes.h"

#include <gtest/gtest.h>

#include <QTemporaryDir>

#include <memory>

namespace dcarchive {

class ContractTestBase : public ::testing::TestWithParam<BackendFactory>
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_workspace.isValid());
        m_backend = GetParam()();
        ASSERT_NE(m_backend, nullptr);
    }

    void TearDown() override
    {
        m_backend.reset();
    }

    std::unique_ptr<ArchiveBackend> m_backend;
    FakeObserver m_observer;
    QTemporaryDir m_workspace;
};

} // namespace dcarchive

#endif // CONTRACT_TEST_BASE_H
