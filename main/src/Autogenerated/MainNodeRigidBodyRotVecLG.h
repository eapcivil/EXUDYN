/** ***********************************************************************************************
* @class        MainNodeRigidBodyRotVecLGParameters
* @brief        Parameter class for MainNodeRigidBodyRotVecLG
*
* @author       Gerstmayr Johannes
* @date         2019-07-01 (generated)
* @date         2020-02-05  00:03:34 (last modfied)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See "LICENSE.txt" for more details.
* @note         Bug reports, support and further information:
                - email: johannes.gerstmayr@uibk.ac.at
                - weblink: missing
                
************************************************************************************************ */
#pragma once

#include <ostream>

#include "Utilities/ReleaseAssert.h"
#include "Utilities/BasicDefinitions.h"

#include <pybind11/pybind11.h>      //! AUTO: include pybind for dictionary access
#include <pybind11/stl.h>           //! AUTO: needed for stl-casts; otherwise py::cast with std::vector<Real> crashes!!!
namespace py = pybind11;            //! AUTO: "py" used throughout in code
#include "Autogenerated/CNodeRigidBodyRotVecLG.h"

#include "Autogenerated/VisuNodeRigidBodyRotVecLG.h"

//! AUTO: Parameters for class MainNodeRigidBodyRotVecLGParameters
class MainNodeRigidBodyRotVecLGParameters // AUTO: 
{
public: // AUTO: 
    Vector6D initialCoordinates;                  //!< AUTO: initial displacement coordinates: ux,uy,uz and rotation vector relative to reference coordinates
    Vector6D initialCoordinates_t;                //!< AUTO: initial velocity coordinate: time derivatives of ux,uy,uz and angular velocity vector
    //! AUTO: default constructor with parameter initialization
    MainNodeRigidBodyRotVecLGParameters()
    {
        initialCoordinates = Vector6D({0.,0.,0., 0.,0.,0.});
        initialCoordinates_t = Vector6D({0.,0.,0., 0.,0.,0.});
    };
};


/** ***********************************************************************************************
* @class        MainNodeRigidBodyRotVecLG
* @brief        A 3D rigid body node based on rotation vector and Lie group methods for rigid bodies or beams; the node has 3 displacement coordinates (displacements of center of mass - COM: \f$[u_x,u_y,u_z]\f$) and three rotation coordinates (rotation vector \f$\mathbf{v} = [v_x,v_y,v_z]^T = \varphi \cdot \mathbf{n}\f$, defining the rotation axis \f$\mathbf{n}\f$ and the angle \f$\varphi\f$ for rotations around x,y, and z-axis); the velocity coordinates are based on the translational (global) velocity and the (local/body-fixed) angular velocity vector; this node can only be integrated using special Lie group integrators; NOTE that this node has a singularity if the rotation is zero or multiple of \f$2\pi\f$.
*
* @author       Gerstmayr Johannes
* @date         2019-07-01 (generated)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See "LICENSE.txt" for more details.
* @note         Bug reports, support and further information:
                - email: johannes.gerstmayr@uibk.ac.at
                - weblink: missing
                
************************************************************************************************ */
#pragma once

#include <ostream>

#include "Utilities/ReleaseAssert.h"
#include "Utilities/BasicDefinitions.h"

//! AUTO: MainNodeRigidBodyRotVecLG
class MainNodeRigidBodyRotVecLG: public MainNode // AUTO: 
{
protected: // AUTO: 
    CNodeRigidBodyRotVecLG* cNodeRigidBodyRotVecLG; //pointer to computational object (initialized in object factory) AUTO:
    VisualizationNodeRigidBodyRotVecLG* visualizationNodeRigidBodyRotVecLG; //pointer to computational object (initialized in object factory) AUTO:
    MainNodeRigidBodyRotVecLGParameters parameters; //! AUTO: contains all parameters for MainNodeRigidBodyRotVecLG

public: // AUTO: 
    //! AUTO: default constructor with parameter initialization
    MainNodeRigidBodyRotVecLG()
    {
        name = "";
    };

    // AUTO: access functions
    //! AUTO: Get pointer to computational class
    CNodeRigidBodyRotVecLG* GetCNodeRigidBodyRotVecLG() { return cNodeRigidBodyRotVecLG; }
    //! AUTO: Get const pointer to computational class
    const CNodeRigidBodyRotVecLG* GetCNodeRigidBodyRotVecLG() const { return cNodeRigidBodyRotVecLG; }
    //! AUTO: Set pointer to computational class (do this only in object factory!!!)
    void SetCNodeRigidBodyRotVecLG(CNodeRigidBodyRotVecLG* pCNodeRigidBodyRotVecLG) { cNodeRigidBodyRotVecLG = pCNodeRigidBodyRotVecLG; }

    //! AUTO: Get pointer to visualization class
    VisualizationNodeRigidBodyRotVecLG* GetVisualizationNodeRigidBodyRotVecLG() { return visualizationNodeRigidBodyRotVecLG; }
    //! AUTO: Get const pointer to visualization class
    const VisualizationNodeRigidBodyRotVecLG* GetVisualizationNodeRigidBodyRotVecLG() const { return visualizationNodeRigidBodyRotVecLG; }
    //! AUTO: Set pointer to visualization class (do this only in object factory!!!)
    void SetVisualizationNodeRigidBodyRotVecLG(VisualizationNodeRigidBodyRotVecLG* pVisualizationNodeRigidBodyRotVecLG) { visualizationNodeRigidBodyRotVecLG = pVisualizationNodeRigidBodyRotVecLG; }

    //! AUTO: Get const pointer to computational base class object
    virtual CNode* GetCNode() const { return cNodeRigidBodyRotVecLG; }
    //! AUTO: Set pointer to computational base class object (do this only in object factory; type is NOT CHECKED!!!)
    virtual void SetCNode(CNode* pCNode) { cNodeRigidBodyRotVecLG = (CNodeRigidBodyRotVecLG*)pCNode; }

    //! AUTO: Get const pointer to visualization base class object
    virtual VisualizationNode* GetVisualizationNode() const { return visualizationNodeRigidBodyRotVecLG; }
    //! AUTO: Set pointer to visualization base class object (do this only in object factory; type is NOT CHECKED!!!)
    virtual void SetVisualizationNode(VisualizationNode* pVisualizationNode) { visualizationNodeRigidBodyRotVecLG = (VisualizationNodeRigidBodyRotVecLG*)pVisualizationNode; }

    //! AUTO: Write (Reference) access to parameters
    virtual MainNodeRigidBodyRotVecLGParameters& GetParameters() { return parameters; }
    //! AUTO: Read access to parameters
    virtual const MainNodeRigidBodyRotVecLGParameters& GetParameters() const { return parameters; }

    //! AUTO:  Get type name of node (without keyword "Node"...!); could also be realized via a string -> type conversion?
    virtual const char* GetTypeName() const override
    {
        return "RigidBodyRotVecLG";
    }

    //! AUTO:  Call a specific node function ==> automatically generated in future
    virtual py::object CallFunction(STDstring functionName, py::dict args) const override;

    //! AUTO:  return internally stored initial coordinates (displacements) of node
    virtual LinkedDataVector GetInitialVector() const override
    {
        return parameters.initialCoordinates;
    }

    //! AUTO:  return internally stored initial coordinates (velocities) of node
    virtual LinkedDataVector GetInitialVector_t() const override
    {
        return parameters.initialCoordinates_t;
    }


    //! AUTO:  dictionary write access
    virtual void SetWithDictionary(const py::dict& d) override
    {
        EPyUtils::SetVector6DSafely(d, "referenceCoordinates", cNodeRigidBodyRotVecLG->GetParameters().referenceCoordinates); /*! AUTO:  safely cast to C++ type*/
        if (EPyUtils::DictItemExists(d, "initialDisplacements")) { EPyUtils::SetVector6DSafely(d, "initialDisplacements", GetParameters().initialCoordinates); /*! AUTO:  safely cast to C++ type*/} 
        if (EPyUtils::DictItemExists(d, "initialVelocities")) { EPyUtils::SetVector6DSafely(d, "initialVelocities", GetParameters().initialCoordinates_t); /*! AUTO:  safely cast to C++ type*/} 
        EPyUtils::SetStringSafely(d, "name", name); /*! AUTO:  safely cast to C++ type*/
        if (EPyUtils::DictItemExists(d, "Vshow")) { visualizationNodeRigidBodyRotVecLG->GetShow() = py::cast<bool>(d["Vshow"]); /* AUTO:  read out dictionary and cast to C++ type*/} 
        if (EPyUtils::DictItemExists(d, "VdrawSize")) { visualizationNodeRigidBodyRotVecLG->GetDrawSize() = py::cast<float>(d["VdrawSize"]); /* AUTO:  read out dictionary and cast to C++ type*/} 
        if (EPyUtils::DictItemExists(d, "Vcolor")) { visualizationNodeRigidBodyRotVecLG->GetColor() = py::cast<std::vector<float>>(d["Vcolor"]); /* AUTO:  read out dictionary and cast to C++ type*/} 
    }

    //! AUTO:  dictionary read access
    virtual py::dict GetDictionary() const override
    {
        auto d = py::dict();
        d["nodeType"] = (std::string)GetTypeName();
        d["referenceCoordinates"] = (std::vector<Real>)cNodeRigidBodyRotVecLG->GetParameters().referenceCoordinates; //! AUTO: cast variables into python (not needed for standard types) 
        d["initialDisplacements"] = (std::vector<Real>)GetParameters().initialCoordinates; //! AUTO: cast variables into python (not needed for standard types) 
        d["initialVelocities"] = (std::vector<Real>)GetParameters().initialCoordinates_t; //! AUTO: cast variables into python (not needed for standard types) 
        d["name"] = (std::string)name; //! AUTO: cast variables into python (not needed for standard types) 
        d["Vshow"] = (bool)visualizationNodeRigidBodyRotVecLG->GetShow(); //! AUTO: cast variables into python (not needed for standard types) 
        d["VdrawSize"] = (float)visualizationNodeRigidBodyRotVecLG->GetDrawSize(); //! AUTO: cast variables into python (not needed for standard types) 
        d["Vcolor"] = (std::vector<float>)visualizationNodeRigidBodyRotVecLG->GetColor(); //! AUTO: cast variables into python (not needed for standard types) 
        return d; 
    }

    //! AUTO:  parameter read access
    virtual py::object GetParameter(const STDstring& parameterName) const override 
    {
        if (parameterName.compare("name") == 0) { return py::cast((std::string)name);} //! AUTO: get parameter
        else if (parameterName.compare("referenceCoordinates") == 0) { return py::cast((std::vector<Real>)cNodeRigidBodyRotVecLG->GetParameters().referenceCoordinates);} //! AUTO: get parameter
        else if (parameterName.compare("initialDisplacements") == 0) { return py::cast((std::vector<Real>)GetParameters().initialCoordinates);} //! AUTO: get parameter
        else if (parameterName.compare("initialVelocities") == 0) { return py::cast((std::vector<Real>)GetParameters().initialCoordinates_t);} //! AUTO: get parameter
        else if (parameterName.compare("Vshow") == 0) { return py::cast((bool)visualizationNodeRigidBodyRotVecLG->GetShow());} //! AUTO: get parameter
        else if (parameterName.compare("VdrawSize") == 0) { return py::cast((float)visualizationNodeRigidBodyRotVecLG->GetDrawSize());} //! AUTO: get parameter
        else if (parameterName.compare("Vcolor") == 0) { return py::cast((std::vector<float>)visualizationNodeRigidBodyRotVecLG->GetColor());} //! AUTO: get parameter
        else  {PyError(STDstring("NodeRigidBodyRotVecLG::GetParameter(...): illegal parameter name ")+parameterName+" cannot be read");} // AUTO: add warning for user
        return py::object();
    }


    //! AUTO:  parameter write access
    virtual void SetParameter(const STDstring& parameterName, const py::object& value) override 
    {
        if (parameterName.compare("name") == 0) { EPyUtils::SetStringSafely(value, name); /*! AUTO:  safely cast to C++ type*/; } //! AUTO: get parameter
        else if (parameterName.compare("referenceCoordinates") == 0) { EPyUtils::SetVector6DSafely(value, cNodeRigidBodyRotVecLG->GetParameters().referenceCoordinates); /*! AUTO:  safely cast to C++ type*/; } //! AUTO: get parameter
        else if (parameterName.compare("initialDisplacements") == 0) { EPyUtils::SetVector6DSafely(value, GetParameters().initialCoordinates); /*! AUTO:  safely cast to C++ type*/; } //! AUTO: get parameter
        else if (parameterName.compare("initialVelocities") == 0) { EPyUtils::SetVector6DSafely(value, GetParameters().initialCoordinates_t); /*! AUTO:  safely cast to C++ type*/; } //! AUTO: get parameter
        else if (parameterName.compare("Vshow") == 0) { visualizationNodeRigidBodyRotVecLG->GetShow() = py::cast<bool>(value); /* AUTO:  read out dictionary and cast to C++ type*/; } //! AUTO: get parameter
        else if (parameterName.compare("VdrawSize") == 0) { visualizationNodeRigidBodyRotVecLG->GetDrawSize() = py::cast<float>(value); /* AUTO:  read out dictionary and cast to C++ type*/; } //! AUTO: get parameter
        else if (parameterName.compare("Vcolor") == 0) { visualizationNodeRigidBodyRotVecLG->GetColor() = py::cast<std::vector<float>>(value); /* AUTO:  read out dictionary and cast to C++ type*/; } //! AUTO: get parameter
        else  {PyError(STDstring("NodeRigidBodyRotVecLG::SetParameter(...): illegal parameter name ")+parameterName+" cannot be modified");} // AUTO: add warning for user
    }

};

