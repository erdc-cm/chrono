// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2016 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Hammad Mazhar, Radu Serban
// =============================================================================
//
// Description: The definition of a parallel ChSystem, pretty much everything is
// done manually instead of using the functions used in ChSystem. This is to
// handle the different data structures present in the parallel implementation
// =============================================================================

#pragma once

#include <stdlib.h>
#include <float.h>
#include <memory.h>
#include <algorithm>

#include "physics/ChSystem.h"
#include "physics/ChBody.h"
#include "physics/ChBodyAuxRef.h"
#include "physics/ChContactDEM.h"
#include "physics/ChGlobal.h"

#include "chrono_parallel/collision/ChCollisionModelParallel.h"
#include "chrono_parallel/ChDataManager.h"
#include "chrono_parallel/ChParallelDefines.h"
#include "chrono_parallel/math/real3.h"
#include "chrono_parallel/ChSettings.h"
#include "chrono_parallel/ChMeasures.h"

#if defined(CHRONO_FEA)
#include "chrono_fea/ChMesh.h"
#endif

namespace chrono {

class ChParallelDataManager;
class settings_container;

class CH_PARALLEL_API ChSystemParallel : public ChSystem {
    CH_RTTI(ChSystemParallel, ChSystem);

  public:
    ChSystemParallel(unsigned int max_objects);
    ChSystemParallel(const ChSystemParallel& other);
    ~ChSystemParallel();

    virtual int Integrate_Y();
    virtual void AddBody(std::shared_ptr<ChBody> newbody) override;
    virtual void AddOtherPhysicsItem(std::shared_ptr<ChPhysicsItem> newitem) override;

    void ClearForceVariables();
    void Update();
    void UpdateBilaterals();
    void UpdateLinks();
    void UpdateOtherPhysics();
    void UpdateRigidBodies();
    void UpdateShafts();
    void Update3DOFBodies();
    void RecomputeThreads();

    virtual void AddMaterialSurfaceData(std::shared_ptr<ChBody> newbody) = 0;
    virtual void UpdateMaterialSurfaceData(int index, ChBody* body) = 0;
    virtual void Setup();
    virtual void ChangeCollisionSystem(COLLISIONSYSTEMTYPE type);

    virtual void PrintStepStats();
    int GetNumBodies();
    int GetNumShafts();
    int GetNumContacts();
    int GetNumBilaterals();

    /// Gets the time (in seconds) spent for computing the time step
    virtual double GetTimerStep();
    /// Gets the fraction of time (in seconds) for the solution of the solver, within the time step
    virtual double GetTimerSolver();
    /// Gets the fraction of time (in seconds) for finding collisions, within the time step
    virtual double GetTimerCollisionBroad();
    /// Gets the fraction of time (in seconds) for finding collisions, within the time step
    virtual double GetTimerCollisionNarrow();
    /// Gets the fraction of time (in seconds) for updating auxiliary data, within the time step
    virtual double GetTimerUpdate();

    /// Gets the total time for the collision detection step
    double GetTimerCollision();

    /// Calculate cummulative contact forces for all bodies in the system.
    virtual void CalculateContactForces() {}

    /// Get the contact force on the body with specified id.
    virtual real3 GetBodyContactForce(uint body_id) const = 0;
    /// Get the contact torque on the body with specified id.
    virtual real3 GetBodyContactTorque(uint body_id) const = 0;
    /// Get the contact force on the specified body.
    real3 GetBodyContactForce(std::shared_ptr<ChBody> body) const { return GetBodyContactForce(body->GetId()); }
    /// Get the contact torque on the specified body.
    real3 GetBodyContactTorque(std::shared_ptr<ChBody> body) const { return GetBodyContactTorque(body->GetId()); }

    settings_container* GetSettings();

    // based on the passed logging level and the state of that level, enable or
    // disable logging level
    void SetLoggingLevel(LOGGINGLEVEL level, bool state = true);

    /// Calculate the (linearized) bilateral constraint violations.
    /// Return the maximum constraint violation.
    double CalculateConstraintViolation(std::vector<double>& cvec);

    ChParallelDataManager* data_manager;

    int current_threads;

  protected:
    double old_timer, old_timer_cd;
    bool detect_optimal_threads;

    int detect_optimal_bins;
    std::vector<double> timer_accumulator, cd_accumulator;
    uint frame_threads, frame_bins, counter;
    std::vector<ChLink*>::iterator it;

    COLLISIONSYSTEMTYPE collision_system_type;

  private:
    void AddShaft(std::shared_ptr<ChShaft> shaft);
#ifdef CHRONO_FEA
    void AddMesh(std::shared_ptr<fea::ChMesh> mesh);
#endif

    std::vector<ChShaft*> shaftlist;
};
//====================================================================================================
class CH_PARALLEL_API ChSystemParallelDVI : public ChSystemParallel {
    CH_RTTI(ChSystemParallelDVI, ChSystemParallel);

  public:
    ChSystemParallelDVI(unsigned int max_objects = 1000);
    ChSystemParallelDVI(const ChSystemParallelDVI& other);

    /// "Virtual" copy constructor (covariant return type).
    virtual ChSystemParallelDVI* Clone() const override { return new ChSystemParallelDVI(*this); }

    void ChangeSolverType(SOLVERTYPE type);
    void Initialize();

    virtual ChMaterialSurfaceBase::ContactMethod GetContactMethod() const { return ChMaterialSurfaceBase::DVI; }
    virtual ChBody* NewBody() override;
    virtual ChBodyAuxRef* NewBodyAuxRef() override;
    virtual void AddMaterialSurfaceData(std::shared_ptr<ChBody> newbody) override;
    virtual void UpdateMaterialSurfaceData(int index, ChBody* body) override;

    void CalculateContactForces();
    real CalculateKineticEnergy();
    real CalculateDualObjective();

    virtual real3 GetBodyContactForce(uint body_id) const;
    virtual real3 GetBodyContactTorque(uint body_id) const;
    using ChSystemParallel::GetBodyContactForce;
    using ChSystemParallel::GetBodyContactTorque;

    virtual void AssembleSystem();
    virtual void SolveSystem();
};
//====================================================================================================
class CH_PARALLEL_API ChSystemParallelDEM : public ChSystemParallel {
    CH_RTTI(ChSystemParallelDEM, ChSystemParallel);

  public:
    ChSystemParallelDEM(unsigned int max_objects = 1000);
    ChSystemParallelDEM(const ChSystemParallelDEM& other);

    /// "Virtual" copy constructor (covariant return type).
    virtual ChSystemParallelDEM* Clone() const override { return new ChSystemParallelDEM(*this); }

    virtual ChMaterialSurface::ContactMethod GetContactMethod() const { return ChMaterialSurfaceBase::DEM; }
    virtual ChBody* NewBody() override;
    virtual ChBodyAuxRef* NewBodyAuxRef() override;
    virtual void AddMaterialSurfaceData(std::shared_ptr<ChBody> newbody) override;
    virtual void UpdateMaterialSurfaceData(int index, ChBody* body) override;

    virtual void Setup();
    virtual void ChangeCollisionSystem(COLLISIONSYSTEMTYPE type);

    virtual real3 GetBodyContactForce(uint body_id) const;
    virtual real3 GetBodyContactTorque(uint body_id) const;
    using ChSystemParallel::GetBodyContactForce;
    using ChSystemParallel::GetBodyContactTorque;

    virtual void PrintStepStats();

    double GetTimerProcessContact() const {
        return data_manager->system_timer.GetTime("ChIterativeSolverParallelDEM_ProcessContact");
    }
};

}  // end namespace chrono
