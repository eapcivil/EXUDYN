/** ***********************************************************************************************
* @brief		Implentation for CSolverImplicitSecondOrderTimeInt
*
* @author		Gerstmayr Johannes
* @date			2019-12-11 (generated)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See 'LICENSE.txt' for more details.
* @note			Bug reports, support and further information:
* 				- email: johannes.gerstmayr@uibk.ac.at
* 				- weblink: missing
* 				
*
************************************************************************************************ */
#pragma once

#include <pybind11/pybind11.h> //for integrated python connectivity (==>put functionality into separate file ...!!!)
#include <pybind11/eval.h>
#include <fstream>

#include "Linalg/BasicLinalg.h" //for Resizable Vector
#include "Main/CSystem.h"
#include "Autogenerated/CMarkerBodyPosition.h"
//#include "Solver/CSolverBase.h" 
#include "Solver/CSolverImplicitSecondOrder.h" 


namespace py = pybind11;	//for py::object


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++   IMPLICIT SECOND ORDER SOLVER   ++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//! reduce step size (1..normal, 2..severe problems); return true, if reduction was successful
bool CSolverImplicitSecondOrderTimeInt::ReduceStepSize(CSystem& computationalSystem, const SimulationSettings& simulationSettings, Index severity)
{
	//it.currentTime is the only important value to be updated in order to reset the step time:
	it.currentTime = computationalSystem.GetSystemData().GetCData().currentState.time;

	if (it.currentStepSize > it.minStepSize)
	{
		it.currentStepSize *= 0.5;

		it.currentStepSize = EXUstd::Maximum(it.minStepSize, it.currentStepSize);
		return true;
	}

	return false;
}

//! set/compute initial conditions (solver-specific!); called from InitializeSolver()
void CSolverImplicitSecondOrderTimeInt::InitializeSolverInitialConditions(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	//call base class for general tasks
	CSolverBase::InitializeSolverInitialConditions(computationalSystem, simulationSettings); //set currentState = initialState


	Vector& solutionODE2_tt = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_tt;
	Vector& solutionAE = computationalSystem.GetSystemData().GetCData().currentState.AECoords;

	bool computeInitialAccelerations = simulationSettings.timeIntegration.generalizedAlpha.computeInitialAccelerations;
	//+++++++++++++++++++++++++++++++++++++++++
	//compute initial values for accelerations:
	//to be fully consistent the initial accelerations must be computed with Lagrange multipliers

	if (computeInitialAccelerations)
	{
		//initial accelerations can be computed, if the system is written in acceleration form
		//[ M    C_q^T][q_tt  ]   [             -ODE2RHS                 ]   [0]
		//|           ||      | + |                                      | = | |
		//[C_q   0    ][lambda]   [ C_tt + 2(C_q)_t*q_t + (C_q*q_t)_q*q_t]   [0]
		//==> constraints need to be transformed to acceleration level
		//for now, the following terms are neglegted: C_tt = 0, 2(C_q)_t*q_t = 0
		//   ==> no explicit dependence of constraints w.r.t. time, or at least requiring this dependence to be zero at initialization
		//[ M    C_q^T][q_tt  ]   [   -ODE2RHS     ]   [0]
		//|           ||      | + |                | = | |
		//[C_q   0    ][lambda]   [ (C_q*q_t)_q*q_t]   [0]
		//the term (C_q*q_t)_q*q_t is computed numerically and only exists, if initial velocities are != 0

		//size of systemJacobian already set!
		data.systemJacobian->SetAllZero(); //entries are not set to zero inside jacobian computation!
		Real factorAE_ODE2 = 1.;			//for position level constraints: depends, if index reduction is used
		Real factorAE_ODE2_t = 1.;			//for velocity constraints ==> C_qt*q_tt term from velocity level, if C=C(q_t)
		bool fillIntoSystemMatrix = true;
		bool velocityLevel = false;

		//+++++++++++++++++++++++++++++
		//Jacobian of algebraic euqations
		//Real factorAE = EXUstd::Square(stepSize) * newmarkBeta; //Index3
		computationalSystem.JacobianAE(data.tempCompData, newton, *(data.systemJacobian), factorAE_ODE2, factorAE_ODE2_t, velocityLevel, fillIntoSystemMatrix);

		//Mass matrix - may also be directly filled into data.systemJacobian?
		data.systemMassMatrix->SetAllZero();
		computationalSystem.ComputeMassMatrix(data.tempCompData, *(data.systemMassMatrix));
		data.systemJacobian->AddSubmatrix(*(data.systemMassMatrix));

		//compute RHS
		Vector systemRHS(data.nSys);
		systemRHS.SetAll(0.);
		LinkedDataVector ode2RHS(systemRHS, 0, data.nODE2);
		LinkedDataVector aeRHS(systemRHS, data.startAE, data.nAE);

		//compute system RHS for initial conditions:
		computationalSystem.ComputeODE2RHS(data.tempCompData, ode2RHS);
		aeRHS.SetAll(0);

		if (IsVerbose(3)) { Verbose(3, "    initial accelerations update Jacobian: Jac    = " + EXUstd::ToString(*(data.systemJacobian)) + "\n"); }

		if (computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_t.GetL2Norm() > 1e-10)
		{
			if (simulationSettings.linearSolverType != LinearSolverType::EXUdense)
			{
				PyWarning("Generalized alpha: initial accelerations due to initial velocities can only be computed in dense matrix mode!");
			}
			else
			{

				Index rowOffset = 0;
				Index columnOffset = 0;
				Real factor = -1.; //(C_q*q_t)_q*q_t put on RHS
				Vector& vInitial = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_t; //=initialState! for consistency here, only currentState is used
				data.jacobianAE->SetNumberOfRowsAndColumns(data.nAE, data.nODE2);
				computationalSystem.ComputeConstraintJacobianDerivative(data.tempCompData, newton.numericalDifferentiation, data.tempODE2F0, data.tempODE2F1, vInitial, *(data.jacobianAE), factor, rowOffset, columnOffset);

				Vector Cqv2(data.nAE);
				data.jacobianAE->MultMatrixVector(vInitial, Cqv2);
				aeRHS += Cqv2;

				if (IsVerbose(3)) { Verbose(3, STDstring("vInitial = ") + EXUstd::ToString(vInitial) + "\n"); }
				if (IsVerbose(3)) { Verbose(3, STDstring("Cqv2     = ") + EXUstd::ToString(Cqv2) + "\n"); }
			}
		}

		data.systemJacobian->FinalizeMatrix();
		if (data.systemJacobian->Factorize() != 0)
		{
			PyWarning("CSolverImplicitSecondOrder::InitializeSolverInitialConditions: System Jacobian not invertible!\nWARNING: using zero initial accelerations\n");
			solutionODE2_tt.SetAll(0.);
		}
		else
		{
			Vector systemInitialValues(data.nSys);
			systemInitialValues.SetAll(0.);
			LinkedDataVector ode2InitialValues(systemInitialValues, 0, data.nODE2);
			data.systemJacobian->Solve(systemRHS, systemInitialValues);
			solutionODE2_tt.CopyFrom(ode2InitialValues); //initial lagrange multipliers are not considered! Should we?
		}
		//Vector& solutionODE2_tt = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_tt;
		//Vector& solutionAE = computationalSystem.GetSystemData().GetCData().currentState.AECoords;

		////these vectors are used in time stepping from previous step
		//solutionODE2_tt = data.u_tt0;
		//solutionAE.SetAll(0.); 
		//data.aAlgorithmic.CopyFrom(solutionODE2_tt); //this may be improved for spectralRadius != 0, see Martin Arnold's paper

	}
	else
	{
		solutionODE2_tt.SetAll(0.);
	}

	//these vectors are used in time stepping from previous step
	solutionAE.SetAll(0.);
	data.aAlgorithmic.CopyFrom(solutionODE2_tt);

	if (IsVerbose(3)) { Verbose(3, STDstring("initial accelerations = ") + EXUstd::ToString(solutionODE2_tt) + "\n"); }

}

//! initialize static step / time step: do some outputs, checks, etc.
void CSolverImplicitSecondOrderTimeInt::UpdateCurrentTime(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (!it.adaptiveStep)
	{
		it.currentTime = it.currentStepIndex * it.currentStepSize + it.startTime; //use this to avoid round-off errors in time 
	}
	else
	{
		if (it.currentTime + it.currentStepSize > it.endTime)
		{
			it.currentStepSize = it.endTime - it.currentTime;
		}
		it.currentTime += it.currentStepSize;
	}

}

//! initialize things at the very beginning of initialize
void CSolverImplicitSecondOrderTimeInt::PreInitializeSolverSpecific(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	const TimeIntegrationSettings& timeint = simulationSettings.timeIntegration;

	//do solver-specific tasks and initialization:
	newmarkBeta = timeint.generalizedAlpha.newmarkBeta; //0.25 ... trapezoidal rule
	newmarkGamma = timeint.generalizedAlpha.newmarkGamma; //0.5  ... trapezoidal rule
	factJacAlgorithmic = 1.; //factor for jacobian in case of generalized-alpha due to algorithmic accelerations

	if (!timeint.generalizedAlpha.useNewmark) //use generalized-alpha
	{
		spectralRadius = timeint.generalizedAlpha.spectralRadius;
		alphaM = (2 * spectralRadius - 1) / (spectralRadius + 1);
		alphaF = spectralRadius / (spectralRadius + 1);
		newmarkGamma = 0.5 + alphaF - alphaM;
		newmarkBeta = 0.25*EXUstd::Square(newmarkGamma + 0.5);
		factJacAlgorithmic = (1. - alphaF) / (1. - alphaM);
	}

	//useIndex2Constraints = timeint.generalizedAlpha.useIndex2Constraints; //==> now directly linked to simulationSettings;
}

//! post-initialize for solver specific tasks; called at the end of InitializeSolver
void CSolverImplicitSecondOrderTimeInt::PostInitializeSolverSpecific(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (IsVerbose(2))
	{
		if (simulationSettings.timeIntegration.generalizedAlpha.useNewmark)
		{
			Verbose(2, STDstring("  NEWMARK: beta=") + EXUstd::ToString(newmarkBeta) + ", gamma=" + EXUstd::ToString(newmarkGamma) + "\n");
		}
		else
		{
			Verbose(2, STDstring("  Generalized-alpha: spectralRadius=") + EXUstd::ToString(spectralRadius) +
				", alphaM=" + EXUstd::ToString(alphaM) + ", alphaF=" + EXUstd::ToString(alphaF) +
				", beta=" + EXUstd::ToString(newmarkBeta) + ", gamma=" + EXUstd::ToString(newmarkGamma) +
				", factJacA=" + EXUstd::ToString(factJacAlgorithmic) + "\n");
		}
	}
}


//! compute residual for Newton method (e.g. static or time step)
//! INPUT: 
//!       - end of last step: [u0, u_t0, u_tt0, aAlgorithmic0 [, lambda0]]; (lambda0 not used in integration scheme)
//!       - end of this step: [solutionODE2_tt]
//! INTERMEDIATE:
//!       - aAlgorithmic is the acceleration in the integration scheme (==solutionODE2_tt for pure Newmark)
//!       - solutionODE2 and solutionODE2_t are computed from integration formulas, based on [u0, u_t0, u_tt0, aAlgorithmic0] and aAlgorithmic
//! OUTPUT: data.systemResidual is updated
void CSolverImplicitSecondOrderTimeInt::ComputeNewtonResidual(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	const TimeIntegrationSettings& timeint = simulationSettings.timeIntegration;

	LinkedDataVector ode2Residual(data.systemResidual, 0, data.nODE2); //link ODE2 coordinates
	LinkedDataVector aeResidual(data.systemResidual, data.startAE, data.nAE); //link ae coordinates

	////link current system vectors for ODE2
	//Vector& solutionODE2 = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords;
	//Vector& solutionODE2_t = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_t;
	Vector& solutionODE2_tt = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_tt;
	//Vector& solutionAE = computationalSystem.GetSystemData().GetCData().currentState.AECoords;

	//STARTTIMER(timer.integrationFormula);
	//data.aAlgorithmic.CopyFrom(solutionODE2_tt);

	//if (!timeint.generalizedAlpha.useNewmark)
	//{//compute algorithmic accelerations aAlgorithmic for generalized alpha method (otherwise aAlgorithmic == solutionODE2_tt)
	//	data.aAlgorithmic *= factJacAlgorithmic;

	//	data.aAlgorithmic.MultAdd(alphaF / (1. - alphaM), computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_tt);

	//	data.aAlgorithmic.MultAdd(-alphaM / (1. - alphaM), data.startOfStepStateAAlgorithmic);
	//}

	//Real fact1 = EXUstd::Square(it.currentStepSize)*0.5*(1. - 2.*newmarkBeta);
	//Real fact2 = EXUstd::Square(it.currentStepSize)*newmarkBeta;
	//Real fact3 = it.currentStepSize * (1. - newmarkGamma);
	//Real fact4 = it.currentStepSize * newmarkGamma;


	////now use Newmark formulas to update solutionODE2 and solutionODE2_t
	////uT = u0 + h*u_t0 + h^2/2*(1-2*beta)*u_tt0 + h^2*beta*aT
	//solutionODE2 = computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords;
	//solutionODE2.MultAdd(it.currentStepSize, computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_t);
	//solutionODE2.MultAdd(fact1, computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_tt);
	//solutionODE2.MultAdd(fact2, data.aAlgorithmic);

	////vT = u_t0 + h*(1-gamma)*u_tt0 + h*gamma*aT
	//solutionODE2_t = computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_t;
	//solutionODE2_t.MultAdd(fact3, computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_tt);
	//solutionODE2_t.MultAdd(fact4, data.aAlgorithmic);

	//STOPTIMER(timer.integrationFormula);

	//now compute the new residual with updated system vectors:
	STARTTIMER(timer.massMatrix);
	data.systemMassMatrix->SetAllZero();
	computationalSystem.ComputeMassMatrix(data.tempCompData, *(data.systemMassMatrix));
	STOPTIMER(timer.massMatrix);

	STARTTIMER(timer.ODE2RHS);
	computationalSystem.ComputeODE2RHS(data.tempCompData, data.tempODE2); //tempODE2 contains RHS (linear case: tempODE2 = F_applied - K*u - D*v)
	STOPTIMER(timer.ODE2RHS);
	STARTTIMER(timer.AERHS);
	computationalSystem.ComputeAlgebraicEquations(data.tempCompData, aeResidual, simulationSettings.timeIntegration.generalizedAlpha.useIndex2Constraints);
	STOPTIMER(timer.AERHS);

	//systemMassMatrix.FinalizeMatrix(); //MultMatrixVector is faster? if directly applied to triplets ...
	data.systemMassMatrix->MultMatrixVector(solutionODE2_tt, ode2Residual);
	//EXUmath::MultMatrixVector(systemMassMatrix, solutionODE2_tt, ode2Residual);
	ode2Residual -= data.tempODE2; //systemResidual contains residual (linear: residual = M*a + K*u+D*v-F

	Vector& solutionAE = computationalSystem.GetSystemData().GetCData().currentState.AECoords;
	//compute CqT*lambda:
	STARTTIMER(timer.reactionForces);
	computationalSystem.ComputeODE2ProjectedReactionForces(data.tempCompData, solutionAE, ode2Residual); //add the forces directly!
	STOPTIMER(timer.reactionForces);
}

void CSolverImplicitSecondOrderTimeInt::ComputeNewtonUpdate(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	STARTTIMER(timer.integrationFormula);
	Vector& solutionODE2_tt = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_tt;
	Vector& solutionAE = computationalSystem.GetSystemData().GetCData().currentState.AECoords;
	LinkedDataVector newtonSolutionODE2(data.newtonSolution, 0, data.nODE2); //temporary subvector for ODE2 components
	LinkedDataVector newtonSolutionAE(data.newtonSolution, data.startAE, data.nAE); //temporary subvector for ODE2 components

	solutionODE2_tt -= newtonSolutionODE2;  //compute new accelerations; newtonSolution contains the Newton correction
	solutionAE -= newtonSolutionAE;			//compute new Lagrange multipliers; newtonSolution contains the Newton correction

	//new code moved here from computeResidual:
	//link current system vectors for ODE2
	Vector& solutionODE2 = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords;
	Vector& solutionODE2_t = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_t;
	//Vector& solutionODE2_tt = computationalSystem.GetSystemData().GetCData().currentState.ODE2Coords_tt;
	//Vector& solutionAE = computationalSystem.GetSystemData().GetCData().currentState.AECoords;

	data.aAlgorithmic.CopyFrom(solutionODE2_tt);

	if (!simulationSettings.timeIntegration.generalizedAlpha.useNewmark)
	{//compute algorithmic accelerations aAlgorithmic for generalized alpha method (otherwise aAlgorithmic == solutionODE2_tt)
		data.aAlgorithmic *= factJacAlgorithmic;

		data.aAlgorithmic.MultAdd(alphaF / (1. - alphaM), computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_tt);

		data.aAlgorithmic.MultAdd(-alphaM / (1. - alphaM), data.startOfStepStateAAlgorithmic);
	}

	Real fact1 = EXUstd::Square(it.currentStepSize)*0.5*(1. - 2.*newmarkBeta);
	Real fact2 = EXUstd::Square(it.currentStepSize)*newmarkBeta;
	Real fact3 = it.currentStepSize * (1. - newmarkGamma);
	Real fact4 = it.currentStepSize * newmarkGamma;


	//now use Newmark formulas to update solutionODE2 and solutionODE2_t
	//uT = u0 + h*u_t0 + h^2/2*(1-2*beta)*u_tt0 + h^2*beta*aT
	solutionODE2 = computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords;
	solutionODE2.MultAdd(it.currentStepSize, computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_t);
	solutionODE2.MultAdd(fact1, computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_tt);
	solutionODE2.MultAdd(fact2, data.aAlgorithmic);

	//vT = u_t0 + h*(1-gamma)*u_tt0 + h*gamma*aT
	solutionODE2_t = computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_t;
	solutionODE2_t.MultAdd(fact3, computationalSystem.GetSystemData().GetCData().startOfStepState.ODE2Coords_tt);
	solutionODE2_t.MultAdd(fact4, data.aAlgorithmic);

	STOPTIMER(timer.integrationFormula);
}
//! compute jacobian for newton method of given solver method
void CSolverImplicitSecondOrderTimeInt::ComputeNewtonJacobian(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	STARTTIMER(timer.totalJacobian);
	data.systemJacobian->SetAllZero(); //entries are not set to zero inside jacobian computation!
	//+++++++++++++++++++++++++++++
	//Tangent stiffness
	//compute jacobian (w.r.t. U ==> also add V); jacobianAE used as temporary matrix
	STARTTIMER(timer.jacobianODE2);
	data.jacobianAE->SetNumberOfRowsAndColumns(data.nODE2, data.nODE2);
	data.jacobianAE->SetAllZero(); //entries are not set to zero inside jacobian computation!
	computationalSystem.NumericalJacobianODE2RHS(data.tempCompData, newton.numericalDifferentiation, data.tempODE2F0, data.tempODE2F1, *(data.jacobianAE)); //fills in part of jacobian

	data.jacobianAE->MultiplyWithFactor(-EXUstd::Square(it.currentStepSize) * newmarkBeta * factJacAlgorithmic); //only ODE2 part; displacements (including those in contraints?) related to unknown accelerations by h^2*beta

	data.systemJacobian->AddSubmatrix(*(data.jacobianAE), 0, 0);
	STOPTIMER(timer.jacobianODE2);

	//+++++++++++++++++++++++++++++
	//'Damping' and gyroscopic terms; jacobianAE used as temporary matrix
	STARTTIMER(timer.jacobianODE2_t);
	data.jacobianAE->SetAllZero(); //entries are not set to zero inside jacobian computation!
	computationalSystem.NumericalJacobianODE2RHS_t(data.tempCompData, newton.numericalDifferentiation, data.tempODE2F0, data.tempODE2F1, *(data.jacobianAE)); //d(ODE2)/dq_t for damping terms
	data.jacobianAE->MultiplyWithFactor(-it.currentStepSize * newmarkGamma * factJacAlgorithmic);
	//jacobianAE *= -stepSize * newmarkGamma * factJacAlgorithmic;
	data.systemJacobian->AddSubmatrix(*(data.jacobianAE), 0, 0);
	STOPTIMER(timer.jacobianODE2_t);

	//+++++++++++++++++++++++++++++
	//Jacobian of algebraic euqations
	//Real factorAE = EXUstd::Square(stepSize) * newmarkBeta; //Index3
	Real factorAE_ODE2 = 1;		//for position level constraints: depends, if index reduction is used
	Real factorAE_ODE2_t = it.currentStepSize * newmarkGamma * factJacAlgorithmic;  //for velocity constraints ==> same for index 2 and index 3

	if (!simulationSettings.timeIntegration.generalizedAlpha.useIndex2Constraints) { factorAE_ODE2 = EXUstd::Square(it.currentStepSize) * newmarkBeta * factJacAlgorithmic; } //Index3:
	else { factorAE_ODE2 = it.currentStepSize * newmarkGamma * factJacAlgorithmic; } //Index2

	STARTTIMER(timer.jacobianAE);
	//add jacobian algebraic equations part to system jacobian:
	computationalSystem.JacobianAE(data.tempCompData, newton, *(data.systemJacobian), factorAE_ODE2, factorAE_ODE2_t, false, true);
	STOPTIMER(timer.jacobianAE);

	STARTTIMER(timer.massMatrix);
	//mass matrix is not updated for jacobian ...! //add a flag?
	data.systemJacobian->AddSubmatrix(*(data.systemMassMatrix)); //systemMassMatrix used from initial step or from previous step; not scaled, because this is linear in unknown accelerations
	STOPTIMER(timer.massMatrix);

	computationalSystem.GetSolverData().signalJacobianUpdate = false; //as jacobian has been computed, no further update is necessary

	if (IsVerbose(3)) { Verbose(3, "    update Jacobian: Jac    = " + EXUstd::ToString(*(data.systemJacobian)) + "\n"); }
	else if (IsVerbose(2)) { Verbose(2, "    update Jacobian\n"); }

	STOPTIMER(timer.totalJacobian);

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//USER FUNCTIONS override:

void CSolverImplicitSecondOrderTimeIntUserFunction::UpdateCurrentTime(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionUpdateCurrentTime == 0) { CSolverImplicitSecondOrderTimeInt::UpdateCurrentTime(computationalSystem, simulationSettings); }
	else { userFunctionUpdateCurrentTime(*mainSolver, *mainSystem, simulationSettings); }
}

void CSolverImplicitSecondOrderTimeIntUserFunction::InitializeStep(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionInitializeStep == 0) { CSolverImplicitSecondOrderTimeInt::InitializeStep(computationalSystem, simulationSettings); }
	else { userFunctionInitializeStep(*mainSolver, *mainSystem, simulationSettings); }
}

void CSolverImplicitSecondOrderTimeIntUserFunction::FinishStep(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionFinishStep == 0) { CSolverImplicitSecondOrderTimeInt::FinishStep(computationalSystem, simulationSettings); }
	else { userFunctionFinishStep(*mainSolver, *mainSystem, simulationSettings); }
}

bool CSolverImplicitSecondOrderTimeIntUserFunction::DiscontinuousIteration(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionDiscontinuousIteration == 0) { return CSolverImplicitSecondOrderTimeInt::DiscontinuousIteration(computationalSystem, simulationSettings); }
	else { return userFunctionDiscontinuousIteration(*mainSolver, *mainSystem, simulationSettings); }
}

bool CSolverImplicitSecondOrderTimeIntUserFunction::Newton(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionNewton == 0) { return CSolverImplicitSecondOrderTimeInt::Newton(computationalSystem, simulationSettings); }
	else { return userFunctionNewton(*mainSolver, *mainSystem, simulationSettings); }
}

void CSolverImplicitSecondOrderTimeIntUserFunction::ComputeNewtonResidual(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionComputeNewtonResidual == 0) { CSolverImplicitSecondOrderTimeInt::ComputeNewtonResidual(computationalSystem, simulationSettings); }
	else { userFunctionComputeNewtonResidual(*mainSolver, *mainSystem, simulationSettings); }
}

void CSolverImplicitSecondOrderTimeIntUserFunction::ComputeNewtonUpdate(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionComputeNewtonUpdate == 0) { CSolverImplicitSecondOrderTimeInt::ComputeNewtonUpdate(computationalSystem, simulationSettings); }
	else { userFunctionComputeNewtonUpdate(*mainSolver, *mainSystem, simulationSettings); }
}

void CSolverImplicitSecondOrderTimeIntUserFunction::ComputeNewtonJacobian(CSystem& computationalSystem, const SimulationSettings& simulationSettings)
{
	if (userFunctionComputeNewtonJacobian == 0) { CSolverImplicitSecondOrderTimeInt::ComputeNewtonJacobian(computationalSystem, simulationSettings); }
	else { userFunctionComputeNewtonJacobian(*mainSolver, *mainSystem, simulationSettings); }
}


