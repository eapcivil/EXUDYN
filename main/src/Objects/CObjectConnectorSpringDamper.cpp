/** ***********************************************************************************************
* @brief        Implementation of CObjectConnectorSpringDamper
*
* @author       Gerstmayr Johannes
* @date         2018-05-06 (generated)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See "LICENSE.txt" for more details.
* @note         Bug reports, support and further information:
                - email: johannes.gerstmayr@uibk.ac.at
                - weblink: missing
                
************************************************************************************************ */

#include "Utilities/ExceptionsTemplates.h"
#include "Main/CSystemData.h"
#include "Autogenerated/CObjectConnectorSpringDamper.h"


//compute the properties which are needed for computation of RHS and needed for OutputVariables
void ComputeConnectorProperties(const MarkerDataStructure& markerData, const CObjectConnectorSpringDamperParameters& parameters,
	Vector3D& relPos, Vector3D& relVel, Real& force, Vector3D& forceDirection)
//void ComputeConnectorProperties(const MarkerDataStructure& markerData, Real referenceLength, Real stiffness, Real damping, Real springForce, 
//	bool activeConnector, const std::function<Real(Real, Real, Real, Real, Real)>& userFunction,
//	Vector3D& relPos, Vector3D& relVel, Real& force, Vector3D& forceDirection)
{
	relPos = (markerData.GetMarkerData(1).position - markerData.GetMarkerData(0).position);
	Real springLength = relPos.GetL2Norm();
	Real springLengthInv;

	if (springLength != 0.) { springLengthInv = 1. / springLength; }
	else { springLengthInv = 1.; SysError("CObjectConnectorSpringDamper::ComputeODE2RHS: springLength = 0"); }

	//unit direction and relative velocity of spring-damper
	forceDirection = springLengthInv * relPos;
	relVel = (markerData.GetMarkerData(1).velocity - markerData.GetMarkerData(0).velocity);

	//stiffness term; this is the term without the jacobian [delta l_vec]; compare Shabana MultibodyDynamics1998, page 119:
	force = 0;
	if (parameters.activeConnector)
	{
		if (!parameters.springForceUserFunction)
		{
			// delta W_spring = k*(l-l0)*(1/l)*l_vec* [delta l_vec]
			force += (parameters.stiffness * (springLength - parameters.referenceLength));

			//damping term + force:
			// delta W_damper = d*l_dot*(1/l)*l_vec* [delta l_vec] = d*(vVel*vPos)*(1/l^2)*l_vec* [delta l_vec]
			force += (parameters.damping * (relVel*forceDirection)) + parameters.force;
		}
		else
		{
			UserFunctionExceptionHandling([&] //lambda function to add consistent try{..} catch(...) block
			{
				//user function args:(deltaL, deltaL_t, Real stiffness, Real damping, Real springForce)
				force += parameters.springForceUserFunction(markerData.GetTime(), springLength - parameters.referenceLength, relVel*forceDirection, parameters.stiffness, parameters.damping, parameters.force);
			}, "ObjectConnectorSpringDamper::springForceUserFunction");
		}
	}
}

//! Computational function: compute right-hand-side (RHS) of second order ordinary differential equations (ODE) to "ode2rhs"
//  MODEL: f
void CObjectConnectorSpringDamper::ComputeODE2RHS(Vector& ode2Rhs, const MarkerDataStructure& markerData) const
{
	//relative position, spring length and inverse spring length
	CHECKandTHROW(markerData.GetMarkerData(1).velocityAvailable && markerData.GetMarkerData(0).velocityAvailable,
		"CObjectConnectorSpringDamper::ComputeODE2RHS: marker do not provide velocityLevel information");

	//link separate vectors to result (ode2Rhs) vector
	ode2Rhs.SetNumberOfItems(markerData.GetMarkerData(0).positionJacobian.NumberOfColumns() + markerData.GetMarkerData(1).positionJacobian.NumberOfColumns());
	ode2Rhs.SetAll(0.); //this is the default; used if !activeConnector

	if (parameters.activeConnector)
	{
		Real force;
		Vector3D relPos, relVel, forceDirection;
		ComputeConnectorProperties(markerData, parameters, relPos, relVel, force, forceDirection);
		Vector3D fVec = force * forceDirection;

		//now link ode2Rhs Vector to partial result using the two jacobians
		if (markerData.GetMarkerData(1).positionJacobian.NumberOfColumns()) //special case: COGround has (0,0) Jacobian
		{
			LinkedDataVector ldv1(ode2Rhs, markerData.GetMarkerData(0).positionJacobian.NumberOfColumns(), markerData.GetMarkerData(1).positionJacobian.NumberOfColumns());

			//ldv1 = (1.)*(markerData.GetMarkerData(1).positionJacobian.GetTransposed()*f); //slow version		
			EXUmath::MultMatrixTransposedVector(markerData.GetMarkerData(1).positionJacobian, fVec, ldv1);
		}

		if (markerData.GetMarkerData(0).positionJacobian.NumberOfColumns()) //special case: COGround has (0,0) Jacobian
		{
			LinkedDataVector ldv0(ode2Rhs, 0, markerData.GetMarkerData(0).positionJacobian.NumberOfColumns());

			//ldv0 = (-1.)*(jacobian0.GetTransposed()*f); //SLOW version
			EXUmath::MultMatrixTransposedVector(markerData.GetMarkerData(0).positionJacobian, fVec, ldv0);
			ldv0 *= -1.;
		}
	}
}

void CObjectConnectorSpringDamper::ComputeJacobianODE2_ODE2(ResizableMatrix& jacobian, ResizableMatrix& jacobian_ODE2_t, const MarkerDataStructure& markerData) const
{
	CHECKandTHROWstring("ERROR: illegal call to CObjectConnectorSpringDamper::ComputeODE2RHSJacobian");
}

////! Flags to determine, which output variables are available (displacment, velocity, stress, ...)
//OutputVariableType CObjectConnectorSpringDamper::GetOutputVariableTypes() const
//{
//	return (OutputVariableType)((Index)OutputVariableType::Distance + (Index)OutputVariableType::Displacement +
//		(Index)OutputVariableType::Velocity + (Index)OutputVariableType::Force);
//}

//! provide according output variable in "value"
void CObjectConnectorSpringDamper::GetOutputVariableConnector(OutputVariableType variableType, const MarkerDataStructure& markerData, Vector& value) const
{
	Real force;
	Vector3D relPos, relVel, forceDirection;
	ComputeConnectorProperties(markerData, parameters, relPos, relVel, force, forceDirection);

	switch (variableType)
	{
	case OutputVariableType::Distance: value.SetVector({ relPos.GetL2Norm() }); break;
	case OutputVariableType::Displacement: value.CopyFrom(relPos); break;
	case OutputVariableType::Velocity: value.CopyFrom(relVel); break;
	case OutputVariableType::Force: value.CopyFrom(force*forceDirection); break;
	default:
		SysError("CObjectConnectorSpringDamper::GetOutputVariable failed"); //error should not occur, because types are checked!
	}
}

