/** ***********************************************************************************************
* @brief        CObjectJointSliding2D implementation
*
* @author       Gerstmayr Johannes
* @date         2018-06-17 (generated)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See "LICENSE.txt" for more details.
* @note         Bug reports, support and further information:
                - email: johannes.gerstmayr@uibk.ac.at
                - weblink: missing
                
************************************************************************************************ */

#include "Main/CSystemData.h"
#include "Autogenerated/CNodeGenericData.h"
//#include "Autogenerated/CObjectJointSliding2D.h"

//for consistency checks:
#include "Main/MainSystem.h"
#include "Pymodules/PybindUtilities.h"
#include "Autogenerated/MainObjectJointSliding2D.h"
#include "Autogenerated/CObjectANCFCable2D.h" //for access to static functions of cable element

bool MainObjectJointSliding2D::CheckPreAssembleConsistency(const MainSystem& mainSystem, STDstring& errorString) const
{
	CObjectJointSliding2D* cObject = (CObjectJointSliding2D*)GetCObject();
	Index dataNodeNumber = cObject->GetNodeNumber(0);

	const MainNode& dataNode = mainSystem.GetMainSystemData().GetMainNode(dataNodeNumber);
	SignedIndex initialMarker = (SignedIndex)dataNode.GetInitialVector()[0]; //length of initial vector already checked in Node-consistency checks

	Index slidingMarkerSize = cObject->GetParameters().slidingMarkerNumbers.NumberOfItems();
	Index slidingMarkerOffsetSize = cObject->GetParameters().slidingMarkerOffsets.NumberOfItems();

	if (slidingMarkerSize != slidingMarkerOffsetSize)
	{
		errorString = "ObjectJointSliding2D: the slidingMarkerNumbers list (size=" + EXUstd::ToString(slidingMarkerSize) + ") must have same size as the slidingMarkerOffsets list (size=" + EXUstd::ToString(slidingMarkerOffsetSize) + ")";
		return false;
	}

	if (initialMarker < 0 || (Index)initialMarker >= slidingMarkerSize)
	{
		errorString = "ObjectJointSliding2D: initial Data variable must be >= 0 and < " + EXUstd::ToString(slidingMarkerSize);
		return false;
	}

	//check for valid node number already done prior to this function
	if (std::strcmp(mainSystem.GetMainSystemData().GetMainNode(dataNodeNumber).GetTypeName(), "GenericData") != 0)
	{
		errorString = "ObjectJointSliding2D: node must be of type 'GenericData'";
		return false;
	}

	Index nc = ((const CNodeGenericData&)(cObject->GetCSystemData()->GetCNode(dataNodeNumber))).GetNumberOfDataCoordinates();
	const Index nodeGenericDataSize = 2; //current cable in markerlist and the global sliding position
	if (nc != nodeGenericDataSize)
	{
		errorString = STDstring("ObjectJointSliding2D: NodeGenericData (Node ") + EXUstd::ToString(dataNodeNumber) + ") must have " + 
			EXUstd::ToString(nodeGenericDataSize) + " coordinates (found: " + EXUstd::ToString(nc) + ")";
		return false;
	}

	//Check indidual types:
	const ArrayIndex& nMarkers = cObject->GetMarkerNumbers();
	if (!(mainSystem.GetCSystem()->GetSystemData().GetCMarker(nMarkers[0]).GetType() & Marker::Position))
	{
		errorString = STDstring("ObjectJointSliding2D: Marker 0 must be of type = 'Position'");
		return false;
	}

	if (mainSystem.GetMainSystemData().GetMainMarkers()[nMarkers[1]]->GetTypeName() != "Cable2DCoordinates")
	{
		errorString = STDstring("ObjectJointSliding2D: Marker 1 must be of type = 'Cable2DCoordinates'");
		return false;
	}

	return true;
}

//! compute the (local) sliding coordinate within the current cable element
Real CObjectJointSliding2D::ComputeLocalSlidingCoordinate() const
{
	const Index slidingCoordinateIndex = 2;
	Real slidingPos = GetCurrentAEcoordinate(slidingCoordinateIndex); //this is only the small changed solved in Newton

	Index slidingMarkerIndex = (Index)GetCNode(0)->GetCurrentCoordinate(0); //this contains the current index in the cable marker list; slidingMarkerIndex will always be in valid range!
	slidingPos += GetCNode(0)->GetCurrentCoordinate(1); //this contains the startOfStep value of the sliding coordinate (or initial value); ranges from 0 to total length of sliding cables

	slidingPos -= parameters.slidingMarkerOffsets[slidingMarkerIndex]; //slidingPos now ranges from 0 to L in current cable element

	return slidingPos;
}

//bool sjnew = true; //new formulation of sliding joint using projection into tangential and normal direction, also suitable for index 2 formulation

//! Computational function: compute algebraic equations and write residual into "algebraicEquations"
void CObjectJointSliding2D::ComputeAlgebraicEquations(Vector& algebraicEquations, const MarkerDataStructure& markerData, Real t, bool velocityLevel) const
{
	//markerData.GetMarkerData(1).vectorValue/_t:cable (refCoordinates+coordinates)/velocities
	//markerData.GetMarkerData(1).value:cable Length (current cable)
	//markerData.GetMarkerData(0).position:   position on other body (e.g. rigid body or mass point)

	//CObjectJointSliding2D: contains NodeGenericData: for current Cable marker number in marker list
	//3 equations: Residuum X,Residuum Y and force*slope = 0
	//3 alg. unknowns: forceX, forceY and sliding coordinate s
	const Index forceXindex = 0;
	const Index forceYindex = 1;

	if (parameters.activeConnector)
	{
		if (parameters.classicalFormulation)
		{
			if (!velocityLevel)
			{
				//compute ANCF position:
				const Index ns = 4;
				LinkedDataVector qNode0(markerData.GetMarkerData(1).vectorValue, 0, ns); //link to position coordinates (refCoords+displacements)
				LinkedDataVector qNode1(markerData.GetMarkerData(1).vectorValue, ns, ns); //link to position coordinates (refCoords+displacements)

				Real L = markerData.GetMarkerData(1).value; //kind of hack ...
				Real slidingCoordinate = ComputeLocalSlidingCoordinate();

				Vector4D SV = CObjectANCFCable2D::ComputeShapeFunctions(slidingCoordinate, L);
				Vector4D SV_x = CObjectANCFCable2D::ComputeShapeFunctions_x(slidingCoordinate, L);

				Vector2D slidingPosition = CObjectANCFCable2D::MapCoordinates(SV, qNode0, qNode1);
				Vector2D slopeVector = CObjectANCFCable2D::MapCoordinates(SV_x, qNode0, qNode1);


				algebraicEquations.SetNumberOfItems(GetAlgebraicEquationsSize());
				Vector2D vPos;
				vPos[0] = (slidingPosition[0] - markerData.GetMarkerData(0).position[0]); //this is the difference between the sliding position and the position of marker0
				vPos[1] = (slidingPosition[1] - markerData.GetMarkerData(0).position[1]);

				//Vector2D normalVector({-markerData.GetMarkerData(1).vectorValue[1], markerData.GetMarkerData(1).vectorValue[0]});

				algebraicEquations[0] = vPos[0];
				algebraicEquations[1] = vPos[1];

				Real forceX = GetCurrentAEcoordinate(forceXindex);
				Real forceY = GetCurrentAEcoordinate(forceYindex);
				algebraicEquations[2] = slopeVector[0] * forceX + slopeVector[1] * forceY; //force in sliding direction must be zero
			}
			else
			{
				CHECKandTHROWstring("CObjectJointSliding2D::ComputeAlgebraicEquations: velocityLevel not implemented");

			}
		}
		else //new formulation
		{
			//compute ANCF position:
			const Index ns = 4;
			LinkedDataVector qNode0(markerData.GetMarkerData(1).vectorValue, 0, ns); //link to position coordinates (refCoords+displacements)
			LinkedDataVector qNode1(markerData.GetMarkerData(1).vectorValue, ns, ns); //link to position coordinates (refCoords+displacements)

			Real L = markerData.GetMarkerData(1).value; //kind of hack ...
			Real slidingCoordinate = ComputeLocalSlidingCoordinate();

			Vector4D SV = CObjectANCFCable2D::ComputeShapeFunctions(slidingCoordinate, L);
			Vector4D SV_x = CObjectANCFCable2D::ComputeShapeFunctions_x(slidingCoordinate, L);

			Vector2D slidingPosition = CObjectANCFCable2D::MapCoordinates(SV, qNode0, qNode1);
			Vector2D slopeVector = CObjectANCFCable2D::MapCoordinates(SV_x, qNode0, qNode1);


			algebraicEquations.SetNumberOfItems(GetAlgebraicEquationsSize());
			Vector2D vPos;
			vPos[0] = (slidingPosition[0] - markerData.GetMarkerData(0).position[0]); //this is the difference between the sliding position and the position of marker0
			vPos[1] = (slidingPosition[1] - markerData.GetMarkerData(0).position[1]);

			LinkedDataVector qNode0_t(markerData.GetMarkerData(1).vectorValue_t, 0, ns); //link to velocity coordinates
			LinkedDataVector qNode1_t(markerData.GetMarkerData(1).vectorValue_t, ns, ns); //link to velocity coordinates

			Vector2D slidingVelocity = CObjectANCFCable2D::MapCoordinates(SV, qNode0_t, qNode1_t);
			Vector2D slopeVector_t = CObjectANCFCable2D::MapCoordinates(SV_x, qNode0_t, qNode1_t);

			Vector2D normalVector({ -slopeVector[1], slopeVector[0] });
			Vector2D normalVector_t({ -slopeVector_t[1], slopeVector_t[0] });

			Vector2D vVel;
			vVel[0] = (slidingVelocity[0] - markerData.GetMarkerData(0).velocity[0]); //this is the difference between the sliding position and the position of marker0
			vVel[1] = (slidingVelocity[1] - markerData.GetMarkerData(0).velocity[1]);


			Real forceX = GetCurrentAEcoordinate(forceXindex); //coordinate forceX is redundant; forceY becomes force in normal direction
			algebraicEquations[0] = forceX; //dummy equation; should be erased in future

			if (!velocityLevel)
			{
				//index3:
				algebraicEquations[1] = vPos * normalVector;
				algebraicEquations[2] = vPos * slopeVector; //index1
			}
			else
			{
				//index2; fully comes without differentiation of s (reason: s could be fully eliminated from constraint equations)
				algebraicEquations[1] = vVel * normalVector + vPos * normalVector_t; //all slidingCoordinate_t values vanish!
				algebraicEquations[2] = vPos * slopeVector; //index1; this equation leads immediately to sliding position if solved
			}
		}
	}
	else
	{
		algebraicEquations.SetNumberOfItems(GetAlgebraicEquationsSize());
		const Index slidingCoordinateIndex = 2;
		Real slidingPos = GetCurrentAEcoordinate(slidingCoordinateIndex); //get 3rd algebraic variable ==> s
		algebraicEquations[0] = markerData.GetLagrangeMultipliers()[0];   //leads to crash; forceX = 0
		algebraicEquations[1] = markerData.GetLagrangeMultipliers()[1];   //leads to crash; forceY = 0
		algebraicEquations[2] = slidingPos;   //s = 0

		//algebraicEquations[0] = GetCurrentAEcoordinate(forceXindex);   //forceX = 0
		//algebraicEquations[1] = GetCurrentAEcoordinate(forceYindex);   //forceY = 0
		//Index s = markerData.GetLagrangeMultipliers().NumberOfItems();
	}

}


void CObjectJointSliding2D::ComputeJacobianAE(ResizableMatrix& jacobian, ResizableMatrix& jacobian_t, ResizableMatrix& jacobian_AE, 
	const MarkerDataStructure& markerData, Real t) const
{
	//CHECKandTHROWstring("CObjectJointSliding2D::ComputeJacobianAE");

	const Index ns = 4; //number of ANCF shape functions
	Index columnsOffset = markerData.GetMarkerData(0).positionJacobian.NumberOfColumns();
	jacobian.SetNumberOfRowsAndColumns(3, columnsOffset + 2 * ns);
	jacobian.SetAll(0.);

	if (parameters.activeConnector)
	{
		if (parameters.classicalFormulation)
		{
			//marker0: contains position jacobian
			const Index forceXindex = 0;
			const Index forceYindex = 1;
			//const Index slidingCoordinateIndex = 2;

			//compute ANCF position:
			LinkedDataVector qNode0(markerData.GetMarkerData(1).vectorValue, 0, ns); //link to position coordinates (refCoords+displacements)
			LinkedDataVector qNode1(markerData.GetMarkerData(1).vectorValue, ns, ns); //link to position coordinates (refCoords+displacements)

			Real L = markerData.GetMarkerData(1).value; //kind of hack ...
			Real slidingCoordinate = ComputeLocalSlidingCoordinate();

			Vector4D SV = CObjectANCFCable2D::ComputeShapeFunctions(slidingCoordinate, L);
			Vector4D SV_x = CObjectANCFCable2D::ComputeShapeFunctions_x(slidingCoordinate, L);
			Vector4D SV_xx = CObjectANCFCable2D::ComputeShapeFunctions_xx(slidingCoordinate, L);

			Vector2D slidingPosition = CObjectANCFCable2D::MapCoordinates(SV, qNode0, qNode1);
			Vector2D slopeVector = CObjectANCFCable2D::MapCoordinates(SV_x, qNode0, qNode1);
			Vector2D slopeVector_x = CObjectANCFCable2D::MapCoordinates(SV_xx, qNode0, qNode1);

			//Vector2D vPos;
			//vPos[0] = (slidingPosition[0] - markerData.GetMarkerData(0).position[0]); //this is the difference between the sliding position and the position of marker0
			//vPos[1] = (slidingPosition[1] - markerData.GetMarkerData(0).position[1]);

			Real forceX = GetCurrentAEcoordinate(forceXindex);
			Real forceY = GetCurrentAEcoordinate(forceYindex);

			jacobian_AE.SetScalarMatrix(3, 0.);
			jacobian_AE(2, 0) = slopeVector[0];
			jacobian_AE(2, 1) = slopeVector[1];

			jacobian_AE(0, 2) = slopeVector[0];
			jacobian_AE(1, 2) = slopeVector[1];

			jacobian_AE(2, 2) = slopeVector_x[0] * forceX + slopeVector_x[1] * forceY;

			for (Index i = 0; i < columnsOffset; i++)
			{
				jacobian(0, i) = -markerData.GetMarkerData(0).positionJacobian(0, i);
				jacobian(1, i) = -markerData.GetMarkerData(0).positionJacobian(1, i);
			}
			for (Index i = 0; i < ns; i++)
			{
				jacobian(0, 2 * i + columnsOffset) = SV[i];
				jacobian(1, 2 * i + 1 + columnsOffset) = SV[i];

				jacobian(2, 2 * i + columnsOffset) = SV_x[i] * forceX;
				jacobian(2, 2 * i + 1 + columnsOffset) = SV_x[i] * forceY;
			}
		}
		else
		{
			//marker0: contains position jacobian
			const Index forceXindex = 0;
			const Index forceYindex = 1;
			//const Index slidingCoordinateIndex = 2;

			//compute ANCF position:
			LinkedDataVector qNode0(markerData.GetMarkerData(1).vectorValue, 0, ns); //link to position coordinates (refCoords+displacements)
			LinkedDataVector qNode1(markerData.GetMarkerData(1).vectorValue, ns, ns); //link to position coordinates (refCoords+displacements)

			Real L = markerData.GetMarkerData(1).value; //kind of hack ...
			Real slidingCoordinate = ComputeLocalSlidingCoordinate();

			Vector4D SV = CObjectANCFCable2D::ComputeShapeFunctions(slidingCoordinate, L);
			Vector4D SV_x = CObjectANCFCable2D::ComputeShapeFunctions_x(slidingCoordinate, L);
			Vector4D SV_xx = CObjectANCFCable2D::ComputeShapeFunctions_xx(slidingCoordinate, L);

			Vector2D slidingPosition = CObjectANCFCable2D::MapCoordinates(SV, qNode0, qNode1);
			Vector2D slopeVector = CObjectANCFCable2D::MapCoordinates(SV_x, qNode0, qNode1);
			Vector2D slopeVector_x = CObjectANCFCable2D::MapCoordinates(SV_xx, qNode0, qNode1);

			Vector2D normalVector({ -slopeVector[1], slopeVector[0] });
			Vector2D normalVector_x({ -slopeVector_x[1], slopeVector_x[0] });
			//Vector2D normalVector_t({ -slopeVector_t[1], slopeVector_t[0] });

			Vector2D vPos;
			vPos[0] = (slidingPosition[0] - markerData.GetMarkerData(0).position[0]); //this is the difference between the sliding position and the position of marker0
			vPos[1] = (slidingPosition[1] - markerData.GetMarkerData(0).position[1]);


			//algebraicEquations[0] = forceX; //dummy equation; should be erased in future
			//algebraicEquations[1] = vPos * normalVector;
			//algebraicEquations[2] = vPos * slopeVector; //index1

			Real forceX = GetCurrentAEcoordinate(forceXindex);
			Real forceY = GetCurrentAEcoordinate(forceYindex);

			jacobian_AE.SetScalarMatrix(3, 0.);

			//derivatives w.r.t. to s and w.r.t. forceX (dummy)
			jacobian_AE(forceXindex, forceXindex) = 1;
			//OLD: 2020-03-09: jacobian_AE(1, 2) = slidingPosition * normalVector_x;	//d(vPos * normalVector)/ds = r'*n + r*n'
			jacobian_AE(1, 2) = slidingPosition * normalVector_x + slopeVector * normalVector;	//d(vPos * normalVector)/ds = r'*n + r*n'
			jacobian_AE(2, 2) = slopeVector*slopeVector + vPos*slopeVector_x;	//deq2/dq2 = d(vPos * slopeVector)/ds = r'*r' + vPos*r'' ; vPos = r(s) - p0

			//jacobian(1, part1) = -posJac*n
			//jacobian(2, part1) = -posJac*r
			for (Index i = 0; i < columnsOffset; i++)
			{
				Vector2D posJaci({ -markerData.GetMarkerData(0).positionJacobian(0, i), -markerData.GetMarkerData(0).positionJacobian(1, i) });
				jacobian(1, i) = posJaci * normalVector;
				jacobian(2, i) = posJaci * slopeVector;
			}

			for (Index i = 0; i < ns; i++)
			{
				//jacobian(1, part2) = d(r(s) - p0)*n / dq1 = SV*n + (r(s) - p0) * [-SV'[1],SV'[0]] 
				jacobian(1, 2 * i + columnsOffset) = SV[i] * normalVector[0] + vPos[1] * SV_x[i]; //special sign, follows from normal!
				jacobian(1, 2 * i + 1 + columnsOffset) = SV[i] * normalVector[1] - vPos[0] * SV_x[i];

				//jacobian(2, part2) = d(r(s) - p0)*r'/ dq1 = SV*r' + (r(s) - p0) * SV' = SV*r' + SV' * (r(s) - p0)
				jacobian(2, 2 * i + columnsOffset)     = SV[i] * slopeVector[0] + SV_x[i] * vPos[0];
				jacobian(2, 2 * i + 1 + columnsOffset) = SV[i] * slopeVector[1] + SV_x[i] * vPos[1];
			}

		}
	}
	else
	{
		jacobian_AE.SetScalarMatrix(3, 1.);
	}


}

//now generated automatically in python:
////! Flags to determine, which output variables are available (displacment, velocity, stress, ...)
//OutputVariableType CObjectJointSliding2D::GetOutputVariableTypes() const
//{
//	return (OutputVariableType)((Index)OutputVariableType::ScalarPosition + 
//		(Index)OutputVariableType::Position +
//		(Index)OutputVariableType::Velocity +
//		(Index)OutputVariableType::Force);
//}

//! provide according output variable in "value"
void CObjectJointSliding2D::GetOutputVariableConnector(OutputVariableType variableType, const MarkerDataStructure& markerData, Vector& value) const
{
	switch (variableType)
	{
	case OutputVariableType::Position: value.CopyFrom(markerData.GetMarkerData(0).position); break;
	case OutputVariableType::Velocity: value.CopyFrom(markerData.GetMarkerData(0).velocity); break;
	case OutputVariableType::Force: 
	{
		const Index forceXindex = 0;
		const Index forceYindex = 1;
		Real forceX = GetCurrentAEcoordinate(forceXindex);
		Real forceY = GetCurrentAEcoordinate(forceYindex);

		value = Vector({forceX, forceY, 0.}); //as all output quantities, they are provided as 3D vectors for 2D objects
		break; 
	}
	case OutputVariableType::SlidingCoordinate: 
	{
		const Index slidingCoordinateIndex = 2;
		Real slidingPos = GetCurrentAEcoordinate(slidingCoordinateIndex); //this is only the small increment in a solution step; zero when evaluated in python function?
		slidingPos += GetCNode(0)->GetCurrentCoordinate(1); //this contains the startOfStep value of the sliding coordinate (or initial value); ranges from 0 to total length of sliding cables
		value = Vector({ slidingPos });
		break;
	}
	default:
		SysError("CObjectJointSliding2D::GetOutputVariable failed"); //error should not occur, because types are checked!
	}

}

bool slidingJoint2Dwarned = false;
//! function called after Newton method; returns a residual error (force); 
//! done for two different computation states in order to estimate the correct time of contact
Real CObjectJointSliding2D::PostNewtonStep(const MarkerDataStructure& markerDataCurrent, PostNewtonFlags::Type& flags)
{
	//return force-type error in case of contact: in case that the assumed contact state has been wrong, 
	//  the contact force (also negative) is returned as measure of the error

	//THIS IS VERY SPECIFIC AND MIGHT BE A POTENTIAL PROBLEM IN CASE OF CHANGES OF THE ASSEMBLE/SOLVER STRUCTURE
	//after change of slidingMarkerIndex, we must also change LTG list, if marker is changed ...; 
	//also use second dataCoordinate for current (+initial) position!!!

	Real discontinuousError = 0;
	flags = PostNewtonFlags::_None;

	Real L = markerDataCurrent.GetMarkerData(1).value; //kind of hack ...
	const Index slidingCoordinateIndex = 2;

	LinkedDataVector currentState = ((CNodeData*)GetCNode(0))->GetCoordinateVector(ConfigurationType::Current);	//copy, but might change values ...

	Real slidingCoordinate = ComputeLocalSlidingCoordinate();
	//Real slidingPos = GetCurrentAEcoordinate(slidingCoordinateIndex); //this is only the small changed solved in Newton
	//slidingPos += currentState[1]; //this contains the startOfStep value of the sliding coordinate (or initial value); ranges from 0 to total length of sliding cables
	//slidingPos -= parameters.slidingMarkerOffsets[(Index)currentState[0]]; //slidingPos now ranges from 0 to L in current cable element

	//pout << "  L=" << L << "\n";
	//pout << "  currentCoordinate=" << GetCurrentAEcoordinate(slidingCoordinateIndex) << "\n";
	//pout << "  offset=" << parameters.slidingMarkerOffsets[(Index)currentState[0]] << "\n";

	//pout << "  slidingCoordinate=" << slidingCoordinate << "\n";
	//pout << "  currentState[0]=" << currentState[0] << "\n";
	//pout << "  currentState[1]=" << currentState[1] << "\n";

	if (slidingCoordinate < 0) //reduce cableMarkerIndex
	{
		discontinuousError = fabs(-slidingCoordinate);
		if (currentState[0] > 0) { currentState[0] -= 1; } //move to previous cable element if possible
		else
		{
			if (!slidingJoint2Dwarned) { PyWarning("WARNING: SlidingJoint2D: sliding coordinate < 0; further warnings suppressed!\n"); }
			slidingJoint2Dwarned = true;
		}
		parameters.markerNumbers[1] = parameters.slidingMarkerNumbers[(Index)currentState[0]]; //now use the new cableMarker
		flags = PostNewtonFlags::UpdateLTGLists; //this signals the system that the LTG lists need to be updated due to major system change
		//pout << "new cable marker: " << parameters.markerNumbers[1] << "\n";

	}
	if (slidingCoordinate > L) //increase cableMarkerIndex
	{
		discontinuousError = fabs(slidingCoordinate - L);
		if (currentState[0] < parameters.slidingMarkerNumbers.NumberOfItems()-1) 
		{ currentState[0] += 1; } //move to next cable element if possible
		else
		{
			if (!slidingJoint2Dwarned) { PyWarning("WARNING: SlidingJoint2D: sliding coordinate > beam length; further warnings suppressed!\n"); }
			slidingJoint2Dwarned = true;
		}
		parameters.markerNumbers[1] = parameters.slidingMarkerNumbers[(Index)currentState[0]]; //now use the new cableMarker
		flags = PostNewtonFlags::UpdateLTGLists; //this signals the system that the LTG lists need to be updated do to major system change
		//pout << "new cable marker: " << parameters.markerNumbers[1] << "\n";
	}

	//now update the sliding coordinate in data variable (next newton iteration starts with 0 in algebraic slidingCoordinate variable)
	currentState[1] += GetCurrentAEcoordinate(slidingCoordinateIndex);

	//put data variable in valid range; out of range does not lead to discontinuousError (cannot be corrected)!
	if (currentState[1] < 0) { currentState[1] = 0; }
	Real maxLength = parameters.slidingMarkerOffsets[parameters.slidingMarkerOffsets.NumberOfItems() - 1] + L;
	if (currentState[1] > maxLength) { currentState[1] = maxLength; }

	//pout << "  ==>currentState[0]=" << currentState[0] << "\n";

	return discontinuousError;
}

//! function called after discontinuous iterations have been completed for one step (e.g. to finalize history variables and set initial values for next step)
void CObjectJointSliding2D::PostDiscontinuousIterationStep()
{
}


