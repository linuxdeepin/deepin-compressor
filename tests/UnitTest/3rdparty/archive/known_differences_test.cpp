// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "known_differences.h"

#include <QSet>

using namespace archive;

// ===========================================================================
// KnownDifference baseline list is non-empty
// ===========================================================================
TEST(KnownDifferencesTest, BaselineListIsNonEmpty)
{
    QList<KnownDifference> diffs = knownDifferences();
    EXPECT_GT(diffs.size(), 0);
}

// ===========================================================================
// Each entry has a unique non-empty id
// ===========================================================================
TEST(KnownDifferencesTest, EachEntryHasUniqueId)
{
    QList<KnownDifference> diffs = knownDifferences();
    QSet<QString> seen;
    for (const KnownDifference &d : diffs) {
        EXPECT_FALSE(d.id.isEmpty()) << "Found entry with empty id";
        EXPECT_FALSE(seen.contains(d.id)) << "Duplicate id: " << d.id.toStdString();
        seen.insert(d.id);
    }
}

// ===========================================================================
// Each entry has all required fields populated
// ===========================================================================
TEST(KnownDifferencesTest, EachEntryHasRequiredFields)
{
    QList<KnownDifference> diffs = knownDifferences();
    for (const KnownDifference &d : diffs) {
        EXPECT_FALSE(d.title.isEmpty()) << "Empty title in " << d.id.toStdString();
        EXPECT_FALSE(d.oldBehavior.isEmpty()) << "Empty oldBehavior in " << d.id.toStdString();
        EXPECT_FALSE(d.impact.isEmpty()) << "Empty impact in " << d.id.toStdString();
        EXPECT_FALSE(d.verificationNote.isEmpty()) << "Empty verificationNote in " << d.id.toStdString();
    }
}

// ===========================================================================
// findDifference returns the correct entry
// ===========================================================================
TEST(KnownDifferencesTest, FindDifferenceReturnsCorrectEntry)
{
    QList<KnownDifference> diffs = knownDifferences();
    ASSERT_GT(diffs.size(), 0);

    const KnownDifference *found = findDifference(diffs[0].id);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->id, diffs[0].id);
    EXPECT_EQ(found->title, diffs[0].title);
}

// ===========================================================================
// findDifference returns nullptr for unknown id
// ===========================================================================
TEST(KnownDifferencesTest, FindDifferenceReturnsNullForUnknownId)
{
    const KnownDifference *found = findDifference(QStringLiteral("KD-NONEXIST"));
    EXPECT_EQ(found, nullptr);
}

// ===========================================================================
// Each entry has a valid category
// ===========================================================================
TEST(KnownDifferencesTest, EachEntryHasValidCategory)
{
    QList<KnownDifference> diffs = knownDifferences();
    for (const KnownDifference &d : diffs) {
        bool validCategory = (d.category == DifferenceCategory::OldDefect ||
                              d.category == DifferenceCategory::BehaviourChange ||
                              d.category == DifferenceCategory::Unsupported);
        EXPECT_TRUE(validCategory) << "Invalid category in " << d.id.toStdString();
    }
}

// ===========================================================================
// Each entry has a valid verification status
// ===========================================================================
TEST(KnownDifferencesTest, EachEntryHasValidVerificationStatus)
{
    QList<KnownDifference> diffs = knownDifferences();
    for (const KnownDifference &d : diffs) {
        bool validStatus = (d.verification == VerificationStatus::Verified ||
                            d.verification == VerificationStatus::AcceptedRisk ||
                            d.verification == VerificationStatus::PendingFollowup);
        EXPECT_TRUE(validStatus) << "Invalid verification status in " << d.id.toStdString();
    }
}

// ===========================================================================
// PendingFollowup entries mention a follow-up issue
// ===========================================================================
TEST(KnownDifferencesTest, PendingFollowupEntriesMentionFollowup)
{
    QList<KnownDifference> diffs = knownDifferences();
    for (const KnownDifference &d : diffs) {
        if (d.verification == VerificationStatus::PendingFollowup) {
            EXPECT_TRUE(d.verificationNote.contains("Follow-up") ||
                        d.verificationNote.contains("follow-up") ||
                        d.verificationNote.contains("V-828"))
                << "PendingFollowup entry " << d.id.toStdString()
                << " should reference a follow-up issue";
        }
    }
}
