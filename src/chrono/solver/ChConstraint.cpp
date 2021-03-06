// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Alessandro Tasora, Radu Serban
// =============================================================================

#include "chrono/solver/ChConstraint.h"

namespace chrono {

// Register into the object factory, to enable run-time dynamic creation and persistence
ChClassRegisterABSTRACT<ChConstraint> a_registration_ChConstraint;

ChConstraint::ChConstraint(const ChConstraint& other) {
    c_i = other.c_i;
    g_i = other.g_i;
    b_i = other.b_i;
    l_i = other.l_i;
    cfm_i = other.cfm_i;
    valid = other.valid;
    disabled = other.disabled;
    redundant = other.redundant;
    broken = other.broken;
    mode = other.mode;
}

ChConstraint& ChConstraint::operator=(const ChConstraint& other) {
    if (&other == this)
        return *this;

    c_i = other.c_i;
    g_i = other.g_i;
    b_i = other.b_i;
    l_i = other.l_i;
    cfm_i = other.cfm_i;
    valid = other.valid;
    _active = other._active;
    disabled = other.disabled;
    redundant = other.redundant;
    broken = other.broken;
    mode = other.mode;

    return *this;
}

bool ChConstraint::operator==(const ChConstraint& other) const {
    return other.cfm_i == cfm_i && other.valid == valid && other._active == _active && other.disabled == disabled &&
           other.redundant == redundant && other.broken == broken && other.mode == mode;
}

void ChConstraint::Project() {
    if (mode == CONSTRAINT_UNILATERAL) {
        if (l_i < 0.)
            l_i = 0.;
    }
}

double ChConstraint::Violation(double mc_i) {
    if (mode == CONSTRAINT_UNILATERAL) {
        if (mc_i > 0.)
            return 0.;
    }

    return mc_i;
}

void ChConstraint::StreamOUT(ChStreamOutBinary& mstream) {
    // class version number
    mstream.VersionWrite(1);

    // stream useful member data
    mstream << cfm_i;
    mstream << valid;
    mstream << disabled;
    mstream << redundant;
    mstream << broken;
    mstream << (int)mode;
}

void ChConstraint::StreamIN(ChStreamInBinary& mstream) {
    // class version number
    int version = mstream.VersionRead();

    // stream in member data
    int mint;
    mstream >> cfm_i;
    mstream >> valid;
    mstream >> disabled;
    mstream >> redundant;
    mstream >> broken;
    mstream >> mint;
    mode = (eChConstraintMode)mint;
    this->UpdateActiveFlag();
}

}  // end namespace chrono