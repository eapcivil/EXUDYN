/** ***********************************************************************************************
* @class        CMarkerSuperElementRigidParameters
* @brief        Parameter class for CMarkerSuperElementRigid
*
* @author       Gerstmayr Johannes
* @date         2019-07-01 (generated)
* @date         2020-06-01  20:10:12 (last modfied)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See "LICENSE.txt" for more details.
* @note         Bug reports, support and further information:
                - email: johannes.gerstmayr@uibk.ac.at
                - weblink: https://github.com/jgerstmayr/EXUDYN
                
************************************************************************************************ */
#pragma once

#include <ostream>

#include "Utilities/ReleaseAssert.h"
#include "Utilities/BasicDefinitions.h"


//! AUTO: Parameters for class CMarkerSuperElementRigidParameters
class CMarkerSuperElementRigidParameters // AUTO: 
{
public: // AUTO: 
    Index bodyNumber;                             //!< AUTO: body number to which marker is attached to
    Vector3D referencePosition;                   //!< AUTO: local marker SuperElement reference position used to compute average displacement and average rotation; currently, this must be the center of weighted nodes of the marker
    ArrayIndex meshNodeNumbers;                   //!< AUTO: a list of \f$n_m\f$ mesh node numbers of superelement (=interface nodes) which are used to compute the body-fixed marker position and orientation; the related nodes must provide 3D position information, such as NodePoint, NodePoint2D, NodeRigidBody[..]; in order to retrieve the global node number, the generic body needs to convert local into global node numbers
    Vector weightingFactors;                      //!< AUTO: a list of \f$n_m\f$ weighting factors per node to compute the final local position and orientation; these factors could be based on surface integrals of the constrained mesh faces
    //! AUTO: default constructor with parameter initialization
    CMarkerSuperElementRigidParameters()
    {
        bodyNumber = EXUstd::InvalidIndex;
        referencePosition = Vector3D({0.,0.,0.});
        meshNodeNumbers = ArrayIndex();
        weightingFactors = Vector();
    };
};


/** ***********************************************************************************************
* @class        CMarkerSuperElementRigid
* @brief        A position and orientation (rigid-body) marker attached to a SuperElement, such as ObjectFFRF, ObjectGenericODE2 and ObjectFFRFreducedOrder (for which it may be inefficient). The marker acts on the mesh nodes, not on the underlying nodes of the object. Note that in contrast to the MarkerSuperElementPosition, this marker needs a set of interface nodes which are not aligned at one line, such that the can represent rigid body motion. Note that definitions of marker positions are slightly different from MarkerSuperElementPosition.
*
* @author       Gerstmayr Johannes
* @date         2019-07-01 (generated)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See "LICENSE.txt" for more details.
* @note         Bug reports, support and further information:
                - email: johannes.gerstmayr@uibk.ac.at
                - weblink: https://github.com/jgerstmayr/EXUDYN
                
************************************************************************************************ */
#pragma once

#include <ostream>

#include "Utilities/ReleaseAssert.h"
#include "Utilities/BasicDefinitions.h"

//! AUTO: CMarkerSuperElementRigid
class CMarkerSuperElementRigid: public CMarker // AUTO: 
{
protected: // AUTO: 
    CMarkerSuperElementRigidParameters parameters; //! AUTO: contains all parameters for CMarkerSuperElementRigid

public: // AUTO: 

    // AUTO: access functions
    //! AUTO: Write (Reference) access to parameters
    virtual CMarkerSuperElementRigidParameters& GetParameters() { return parameters; }
    //! AUTO: Read access to parameters
    virtual const CMarkerSuperElementRigidParameters& GetParameters() const { return parameters; }

    //! AUTO:  general access to object number
    virtual Index GetObjectNumber() const override
    {
        return parameters.bodyNumber;
    }

    //! AUTO:  return marker type (for node treatment in computation)
    virtual Marker::Type GetType() const override
    {
        return (Marker::Type)(Marker::Body + Marker::Object + Marker::Position + Marker::Orientation + Marker::SuperElement);
    }

    //! AUTO:  return dimension of connector, which an attached connector would have; for coordinate markers, it gives the number of coordinates used by the marker
    virtual Index GetDimension(const CSystemData& cSystemData) const override
    {
        return 3;
    }

    //! AUTO:  return position of marker
    virtual void GetPosition(const CSystemData& cSystemData, Vector3D& position, ConfigurationType configuration = ConfigurationType::Current) const override;

    //! AUTO:  return velocity of marker
    virtual void GetVelocity(const CSystemData& cSystemData, Vector3D& velocity, ConfigurationType configuration = ConfigurationType::Current) const override;

    //! AUTO:  return configuration dependent rotation matrix of node; returns always a 3D Matrix
    virtual void GetRotationMatrix(const CSystemData& cSystemData, Matrix3D& rotationMatrix, ConfigurationType configuration = ConfigurationType::Current) const override;

    //! AUTO:  return configuration dependent angular velocity of node; returns always a 3D Vector
    virtual void GetAngularVelocity(const CSystemData& cSystemData, Vector3D& angularVelocity, ConfigurationType configuration = ConfigurationType::Current) const override;

    //! AUTO:  return configuration dependent local (=body-fixed) angular velocity of node; returns always a 3D Vector
    virtual void GetAngularVelocityLocal(const CSystemData& cSystemData, Vector3D& angularVelocity, ConfigurationType configuration = ConfigurationType::Current) const override;

    //! AUTO:  Compute marker data (e.g. position and positionJacobian) for a marker
    virtual void ComputeMarkerData(const CSystemData& cSystemData, bool computeJacobian, MarkerData& markerData) const override;

    //! AUTO:  return parameters of underlying floating frame node (or default values for case that no frame exists)
    void GetFloatingFrameNodeData(const CSystemData& cSystemData, Vector3D& framePosition, Matrix3D& frameRotationMatrix, Vector3D& frameVelocity, Vector3D& frameAngularVelocityLocal, ConfigurationType configuration = ConfigurationType::Current) const;

    //! AUTO:  return weighted (linearized) rotation from local mesh displacements
    void GetWeightedRotations(const CSystemData& cSystemData, Vector3D& weightedRotations, ConfigurationType configuration = ConfigurationType::Current) const;

    //! AUTO:  return weighted angular velocity from local mesh velocities
    void GetWeightedAngularVelocity(const CSystemData& cSystemData, Vector3D& weightedAngularVelocity, ConfigurationType configuration = ConfigurationType::Current) const;

};

