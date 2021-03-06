/** ***********************************************************************************************
* @brief		Implementation for computational sensors (collective implementation file for GetSensorValues functions);
*               This file covers implementation parts of loads, specifically python bindings
*
* @author		Gerstmayr Johannes
* @date			2020-01-24 (generated)
* @pre			...
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See 'LICENSE.txt' for more details.
* @note			Bug reports, support and further information:
* 				- email: johannes.gerstmayr@uibk.ac.at
* 				- weblink: missing
* 				
*
* *** Example code ***
*
************************************************************************************************ */

//#include "Main/CSystemData.h"
#include "Main/MainSystem.h"
#include "Pymodules/PybindUtilities.h"

#include "Autogenerated/CSensorNode.h"
#include "Autogenerated/CSensorObject.h"
#include "Autogenerated/CSensorBody.h"
#include "Autogenerated/CSensorSuperElement.h"
#include "Autogenerated/CSensorLoad.h"

py::object MainSensor::GetSensorValues(const CSystemData& cSystemData, ConfigurationType configuration) const
{
	Vector value;
	GetCSensor()->GetSensorValues(cSystemData, value, configuration);

	//check if it is scalar or a vector-valued:
	if (value.NumberOfItems() == 1) { return py::float_(value[0]); }
	else { return py::array_t<Real>(value.NumberOfItems(), value.GetDataPointer()); }

}


//! main function to generate sensor output values
void CSensorNode::GetSensorValues(const CSystemData& cSystemData, Vector& values, ConfigurationType configuration) const
{
	const CNode& cNode = cSystemData.GetCNode(parameters.nodeNumber);
	cNode.GetOutputVariable(parameters.outputVariableType, configuration, values);
}

//! main function to generate sensor output values
void CSensorObject::GetSensorValues(const CSystemData& cSystemData, Vector& values, ConfigurationType configuration) const
{
	const CObject* cObject = cSystemData.GetCObjects()[parameters.objectNumber];
	if (((Index)cObject->GetType() & (Index)CObjectType::Connector) == 0)
	{
		//must be object ==> may leed to illegal call, if not implemented
		cObject->GetOutputVariable(parameters.outputVariableType, values);
	}
	else
	{
		const CObjectConnector* cConnector = (const CObjectConnector*)cObject;

		MarkerDataStructure markerDataStructure;
		const bool computeJacobian = false; //not needed for OutputVariables
		cSystemData.ComputeMarkerDataStructure(cConnector, computeJacobian, markerDataStructure);

		cConnector->GetOutputVariableConnector(parameters.outputVariableType, markerDataStructure, values);

	}
}

//! main function to generate sensor output values
void CSensorBody::GetSensorValues(const CSystemData& cSystemData, Vector& values, ConfigurationType configuration) const
{
	const CObjectBody& cObject = cSystemData.GetCObjectBody(parameters.bodyNumber);
	cObject.GetOutputVariableBody(parameters.outputVariableType, parameters.localPosition, configuration, values);
}

//! main function to generate sensor output values
void CSensorSuperElement::GetSensorValues(const CSystemData& cSystemData, Vector& values, ConfigurationType configuration) const
{
	const CObjectSuperElement* cObject = (const CObjectSuperElement*)(cSystemData.GetCObjects()[parameters.bodyNumber]);
	cObject->GetOutputVariableSuperElement(parameters.outputVariableType, parameters.meshNodeNumber, configuration, values);
}


//! main function to generate sensor output values
void CSensorLoad::GetSensorValues(const CSystemData& cSystemData, Vector& values, ConfigurationType configuration) const
{
	Real time = cSystemData.GetCData().GetCurrent().GetTime();
	const CLoad& cLoad = *(cSystemData.GetCLoads()[parameters.loadNumber]);

	if (cLoad.IsVector()) //vector
	{
		values.CopyFrom(cLoad.GetLoadVector(time));
	}
	else //scalar
	{
		values.CopyFrom(Vector1D(cLoad.GetLoadValue(time)));
	}
}

