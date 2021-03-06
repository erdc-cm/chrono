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
// Authors: Radu Serban
// =============================================================================
//
// Various utility classes for vehicle subsystems.
//
// =============================================================================

#ifndef CH_SUBSYS_DEFS_H
#define CH_SUBSYS_DEFS_H

#include <vector>

#include "chrono/core/ChQuaternion.h"
#include "chrono/core/ChVector.h"
#include "chrono/physics/ChLinkSpringCB.h"

#include "chrono_vehicle/ChApiVehicle.h"

namespace chrono {
namespace vehicle {

/// @addtogroup vehicle
/// @{

/// Enum for the side (left/right) of a vehicle.
enum VehicleSide {
    LEFT = 0,  ///< left side of vehicle is always 0
    RIGHT = 1  ///< right side of vehicle is always 1
};

/// Class to encode the ID of a vehicle wheel.
/// By convention, wheels are counted front to rear and left to right. In other
/// words, for a vehicle with 2 axles, the order is: front-left, front-right,
/// rear-left, rear-right.
class WheelID {
  public:
    WheelID(int id) : m_id(id), m_axle(id / 2), m_side(VehicleSide(id % 2)) {}
    WheelID(int axle, VehicleSide side) : m_id(2 * axle + side), m_axle(axle), m_side(side) {}

    /// Return the wheel ID.
    int id() const { return m_id; }

    /// Return the axle index for this wheel ID.
    /// Axles are counted from the front of the vehicle.
    int axle() const { return m_axle; }

    /// Return the side for this wheel ID.
    /// By convention, left is 0 and right is 1.
    VehicleSide side() const { return m_side; }

  private:
    int m_id;            ///< wheel ID
    int m_axle;          ///< axle index (counted from the front)
    VehicleSide m_side;  ///< vehicle side (LEFT: 0, RIGHT: 1)
};

/// Global constant wheel IDs for the common topology of a 2-axle vehicle.
static const WheelID FRONT_LEFT(0, LEFT);
static const WheelID FRONT_RIGHT(0, RIGHT);
static const WheelID REAR_LEFT(1, LEFT);
static const WheelID REAR_RIGHT(1, RIGHT);

/// Structure to communicate a full body state.
struct BodyState {
    ChVector<> pos;      ///< global position
    ChQuaternion<> rot;  ///< orientation with respect to global frame
    ChVector<> lin_vel;  ///< linear velocity, expressed in the global frame
    ChVector<> ang_vel;  ///< angular velocity, expressed in the global frame
};

/// Vector of body state structures
typedef std::vector<BodyState> BodyStates;

/// Structure to communicate a full wheel body state.
/// In addition to the quantities communicated for a generic body, the wheel
/// state also includes the wheel angular speed about its axis of rotation.
struct WheelState {
    ChVector<> pos;      ///< global position
    ChQuaternion<> rot;  ///< orientation with respect to global frame
    ChVector<> lin_vel;  ///< linear velocity, expressed in the global frame
    ChVector<> ang_vel;  ///< angular velocity, expressed in the global frame
    double omega;        ///< wheel angular speed about its rotation axis
};

/// Vector of wheel state structures
typedef std::vector<WheelState> WheelStates;

/// Structure to communicate a set of generalized tire forces.
struct TireForce {
    ChVector<> force;   ///< force vector, epxressed in the global frame
    ChVector<> point;   ///< global location of the force application point
    ChVector<> moment;  ///< moment vector, expressed in the global frame
};

/// Vector of tire force structures.
typedef std::vector<TireForce> TireForces;

/// Structure to communicate a set of generalized track shoe forces.
struct TrackShoeForce {
    ChVector<> force;   ///< force vector, epxressed in the global frame
    ChVector<> point;   ///< global location of the force application point
    ChVector<> moment;  ///< moment vector, expressed in the global frame
};

/// Vector of tire force structures.
typedef std::vector<TrackShoeForce> TrackShoeForces;

/// Utility class for specifying a linear spring force.
class LinearSpringForce : public ChSpringForceCallback {
  public:
    LinearSpringForce(double k) : m_k(k) {}
    virtual double operator()(double time, double rest_length, double length, double vel) {
        return -m_k * (length - rest_length);
    }

  private:
    double m_k;
};

/// Utility class for specifying a linear damper force.
class LinearDamperForce : public ChSpringForceCallback {
  public:
    LinearDamperForce(double c) : m_c(c) {}
    virtual double operator()(double time, double rest_length, double length, double vel) { return -m_c * vel; }

  private:
    double m_c;
};

/// Utility class for specifying a linear spring-damper force.
class LinearSpringDamperForce : public ChSpringForceCallback {
  public:
    LinearSpringDamperForce(double k, double c) : m_k(k), m_c(c) {}
    virtual double operator()(double time, double rest_length, double length, double vel) {
        return -m_k * (length - rest_length) - m_c * vel;
    }

  private:
    double m_k;
    double m_c;
};

/// Utility class for specifying a linear spring-damper force with pre-tension.
class LinearSpringDamperActuatorForce : public ChSpringForceCallback {
  public:
    LinearSpringDamperActuatorForce(double k, double c, double f) : m_k(k), m_c(c), m_f(f) {}
    virtual double operator()(double time, double rest_length, double length, double vel) {
        return m_f - m_k * (length - rest_length) - m_c * vel;
    }

  private:
    double m_k;
    double m_c;
    double m_f;
};

/// Utility class for specifying a map spring force.
class MapSpringForce : public ChSpringForceCallback {
  public:
    MapSpringForce() {}
    MapSpringForce(const std::vector<std::pair<double, double>>& data) {
        for (unsigned int i = 0; i < data.size(); ++i) {
            m_map.AddPoint(data[i].first, data[i].second);
        }
    }
    void add_point(double x, double y) { m_map.AddPoint(x, y); }
    virtual double operator()(double time, double rest_length, double length, double vel) {
        return -m_map.Get_y(length - rest_length);
    }

  private:
    ChFunction_Recorder m_map;
};

/// Utility class for specifying a map damper force.
class MapDamperForce : public ChSpringForceCallback {
  public:
    MapDamperForce() {}
    MapDamperForce(const std::vector<std::pair<double, double>>& data) {
        for (unsigned int i = 0; i < data.size(); ++i) {
            m_map.AddPoint(data[i].first, data[i].second);
        }
    }
    void add_point(double x, double y) { m_map.AddPoint(x, y); }
    virtual double operator()(double time, double rest_length, double length, double vel) { return -m_map.Get_y(vel); }

  private:
    ChFunction_Recorder m_map;
};

/// Utility class for specifying a map spring-damper force with pre-tension.
class MapSpringDamperActuatorForce : public ChSpringForceCallback {
  public:
    MapSpringDamperActuatorForce() {}
    MapSpringDamperActuatorForce(const std::vector<std::pair<double, double>>& dataK,
                                 const std::vector<std::pair<double, double>>& dataC,
                                 double f) {
        for (unsigned int i = 0; i < dataK.size(); ++i) {
            m_mapK.AddPoint(dataK[i].first, dataK[i].second);
        }
        for (unsigned int i = 0; i < dataC.size(); ++i) {
            m_mapC.AddPoint(dataC[i].first, dataC[i].second);
        }
    }
    void add_pointK(double x, double y) { m_mapK.AddPoint(x, y); }
    void add_pointC(double x, double y) { m_mapC.AddPoint(x, y); }
    void set_f(double f) { m_f = f; }
    virtual double operator()(double time, double rest_length, double length, double vel) {
        return m_f - m_mapK.Get_y(length - rest_length) - m_mapC.Get_y(vel);
    }

  private:
    ChFunction_Recorder m_mapK;
    ChFunction_Recorder m_mapC;
    double m_f;
};

/// Enum for visualization types.
enum class VisualizationType {
    NONE,        ///< no visualization
    PRIMITIVES,  ///< use primitve shapes
    MESH         ///< use meshes
};

/// Enum for available tire models.
enum class TireModelType {
    RIGID,       ///< rigid tire (cylindrical)
    RIGID_MESH,  ///< rigid tire (mesh)
    PACEJKA,     ///< Pacejka (magic formula) tire
    LUGRE,       ///< Lugre frition model tire
    FIALA,       ///< Fiala tire
    ANCF,        ///< ANCF shell element-based tire
    REISSNER,    ///< Reissner 6-field shell element-based tire
    FEA          ///< FEA co-rotational tire
};

/// Enum for available powertrain model templates.
enum class PowertrainModelType {
    SHAFTS,  ///< powertrain based on ChShaft elements
    SIMPLE   ///< simple powertrain model (similar to a DC motor)
};

/// Enum for available wheeled-vehicle suspension model templates.
enum class SuspensionType {
    DOUBLE_WISHBONE,          ///< double wishbone
    DOUBLE_WISHBONE_REDUCED,  ///< simplified double wishbone (constraint-based)
    SOLID_AXLE,               ///< solid axle
    MULTI_LINK,               ///< multi-link
    HENDRICKSON_PRIMAXX,      ///< Hendrickson PRIMAXX (walking beam)
    MACPHERSON_STRUT          ///< MacPherson strut
};

/// Enum for drive types.
enum class DrivelineType {
    FWD,  ///< front-wheel drive
    RWD,  ///< rear-wheel drive
    AWD,  ///< all-wheel drive
    SIMPLE
};

/// Enum for track shoe types.
enum class TrackShoeType {
    SINGLE_PIN,  ///< single-pin track shoe and sprocket
    DOUBLE_PIN   ///< double-pin track shoe and sprocket
};

/// Enum for guide pin (track shoe/roadwheel/idler).
enum class GuidePinType {
    CENTRAL_PIN,  ///< track shoes with central guiding pin and double wheels
    LATERAL_PIN   ///< track shoes with lateral guiding pins and single wheels
};

/// Enumerations for track collision flags.
namespace TrackCollide {
// Note: we cannot use strongly typed enums since these are used as integers
enum Enum {
    NONE = 0,
    SPROCKET_LEFT = 1 << 0,
    SPROCKET_RIGHT = 1 << 1,
    IDLER_LEFT = 1 << 2,
    IDLER_RIGHT = 1 << 3,
    WHEELS_LEFT = 1 << 4,
    WHEELS_RIGHT = 1 << 5,
    SHOES_LEFT = 1 << 6,
    SHOES_RIGHT = 1 << 7,
    ALL = 0xFFFF
};
}

/// Enumerations for track collision families.
namespace TrackCollisionFamily {
// Note: we cannot use strongly typed enums, since these are passed as integers
enum Enum {
    CHASSIS = 0,  ///< chassis collision family
    IDLERS = 1,   ///< collision family for idler subsystems
    WHEELS = 2,   ///< collision family for road-wheel assemblies
    SHOES = 3     ///< collision family for track shoe subsystems
};
}

/// Flags for output (log/debug).
/// These flags can be bit-wise ORed and used as a mask.
enum OutputInformation {
    OUT_SPRINGS = 1 << 0,      ///< suspension spring information
    OUT_SHOCKS = 1 << 1,       ///< suspension shock information
    OUT_CONSTRAINTS = 1 << 2,  ///< constraint violation information
    OUT_TESTRIG = 1 << 3       ///< test-rig specific information
};

/// @} vehicle

}  // end namespace vehicle
}  // end namespace chrono

#endif
