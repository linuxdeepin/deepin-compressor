// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KNOWN_DIFFERENCES_H
#define KNOWN_DIFFERENCES_H

#include <QString>
#include <QList>

namespace archive {

// ---------------------------------------------------------------------------
// KnownDifference — documents a known behavioural difference between the
// old KDE-plugin-based archive interface and the new ArchiveBackend /
// BackendObserver interface.
//
// The migration plan (§阶段2) requires a baseline list of old-implementation
// defects, old-vs-new differences, and unsupported requests / events / error
// codes, each with verification status or follow-up issue ownership.
//
// Categories:
//   OldDefect       — a defect in the old implementation carried forward
//                     or addressed by the new interface.
//   BehaviourChange — a deliberate change in the new interface that differs
//                     from the old behaviour.
//   Unsupported     — a request, event, or error code that the new interface
//                     does not yet support (planned for a later phase).
// ---------------------------------------------------------------------------

enum class DifferenceCategory {
    OldDefect,
    BehaviourChange,
    Unsupported
};

enum class VerificationStatus {
    Verified,       // verified by test or manual inspection
    AcceptedRisk,   // known and accepted, no immediate fix
    PendingFollowup // requires a follow-up issue
};

struct KnownDifference {
    QString id;                  // stable identifier, e.g. "KD-001"
    DifferenceCategory category;
    QString title;               // short description
    QString oldBehavior;         // how the old interface behaves
    QString newBehavior;         // how the new interface behaves (or "N/A")
    QString impact;              // user-visible or API-level impact
    VerificationStatus verification;
    QString verificationNote;    // test name, inspection result, or follow-up issue
};

// Returns the full baseline list of known differences.
// The list is static — it documents the state at the time of the V-8285
// migration phase 2 delivery.
QList<KnownDifference> knownDifferences();

// Convenience: find a difference by id.
const KnownDifference *findDifference(const QString &id);

} // namespace archive

#endif // KNOWN_DIFFERENCES_H
