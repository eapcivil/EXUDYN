/** ***********************************************************************************************
* @class        VisualizationSensorBody
* @brief        A sensor attached to a body-object with local position. As a difference to other ObjectSensors, the body sensor has a local position at which the sensor is attached to. The sensor measures OutputVariableBody and outputs values into a file, showing per line [time, sensorValue[0], sensorValue[1], ...]. A user function can be attached to postprocess sensor values accordingly.
*
* @author       Gerstmayr Johannes
* @date         2019-07-01 (generated)
* @date         2020-09-08  18:14:41 (last modfied)
*
* @copyright    This file is part of Exudyn. Exudyn is free software: you can redistribute it and/or modify it under the terms of the Exudyn license. See "LICENSE.txt" for more details.
* @note         Bug reports, support and further information:
                - email: johannes.gerstmayr@uibk.ac.at
                - weblink: https://github.com/jgerstmayr/EXUDYN
                
************************************************************************************************ */

#ifndef VISUALIZATIONSENSORBODY__H
#define VISUALIZATIONSENSORBODY__H

#include <ostream>

#include "Utilities/ReleaseAssert.h"
#include "Utilities/BasicDefinitions.h"
#include "System/ItemIndices.h"

class VisualizationSensorBody: public VisualizationSensor // AUTO: 
{
protected: // AUTO: 

public: // AUTO: 
    //! AUTO: default constructor with parameter initialization
    VisualizationSensorBody()
    {
        show = true;
    };

    // AUTO: access functions
    //! AUTO:  Update visualizationSystem -> graphicsData for item; index shows item Number in CData
    virtual void UpdateGraphics(const VisualizationSettings& visualizationSettings, VisualizationSystem* vSystem, Index itemNumber) override;

};



#endif //#ifdef include once...
