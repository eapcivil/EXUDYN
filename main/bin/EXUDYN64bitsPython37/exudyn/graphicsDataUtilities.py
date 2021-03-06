# Utility functions and structures for Exudyn
"""
Created on Fri Jul 26 10:53:30 2019

@author: Johannes Gerstmayr

goal: support functions, which simplify the generation of models
"""

from exudyn.basicUtilities import *
from exudyn.rigidBodyUtilities import ComputeOrthonormalBasis

#constants and fixed structures:
import numpy as np #LoadSolutionFile
import copy as copy #to be able to copy e.g. lists
#import time        #AnimateSolution

# color definitions
color4red = [1,0,0,1.]
color4green = [0,1,0,1]
color4blue = [0,0,1,1]

color4cyan = [0,1,1,1]
color4magenta = [1,0,1,1]
color4yellow = [1,1,0,1]

color4lightred = [0.9,0.4,0.4,1.]
color4lightgreen = [0.3,0.9,0.3,1.]
color4steelblue = [0.4,0.4,0.9,1.]

color4grey = [0.5,0.5,0.5,1.]

color4darkgrey = [0.25,0.25,0.25,1.]
color4lightgrey = [0.75,0.75,0.75,1.]
color4white = [1.,1.,1.,1.]

#define a list of 4 colors for numbered colors
color4list = [color4red, color4green, color4blue, 
              color4cyan, color4magenta, color4yellow,
              color4lightred, color4lightgreen, color4steelblue, 
              color4grey]

#************************************************
#**function: generate graphics data for 2D rectangle
#**input: minimal and maximal cartesian coordinates in (x/y) plane; color provided as list of 4 RGBA values
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataRectangle(xMin, yMin, xMax, yMax, color=[0.,0.,0.,1.]): 

    rect = [xMin, yMin,xMax,yMax]
    dataRect = {'type':'Line', 'color': color, 'data':[rect[0],rect[1],0, rect[2],rect[1],0, rect[2],rect[3],0, rect[0],rect[3],0, rect[0],rect[1],0]}

    return dataRect

#************************************************
#**function: generate graphics data for orthogonal cube drawn with lines
#**input: minimal and maximal cartesian coordinates for orthogonal cube; color provided as list of 4 RGBA values
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataOrthoCubeLines(xMin, yMin, zMin, xMax, yMax, zMax, color=[0.,0.,0.,1.]): 

    dataRect = {'type':'Line', 'color': color, 'data':[xMin,yMin,zMin, xMin,yMax,zMin, xMin,yMin,zMin, xMax,yMin,zMin, xMax,yMax,zMin, xMax,yMin,zMin, 
                                                       xMax,yMin,zMax, xMax,yMax,zMax, xMax,yMin,zMax, xMin,yMin,zMax, xMin,yMax,zMax, xMin,yMin,zMax, 
                                                       xMin,yMin,zMin, xMin,yMax,zMin, xMax,yMax,zMin, xMax,yMax,zMax, xMin,yMax,zMax, xMin,yMax,zMin]}

    return dataRect

#**function: generate graphics data for orthogonal 3D cube with min and max dimensions
#**input: minimal and maximal cartesian coordinates for orthogonal cube; color provided as list of 4 RGBA values
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataOrthoCube(xMin, yMin, zMin, xMax, yMax, zMax, color=[0.,0.,0.,1.]): 
    
    pList = [[xMin,yMin,zMin], [xMax,yMin,zMin], [xMax,yMax,zMin], [xMin,yMax,zMin],
             [xMin,yMin,zMax], [xMax,yMin,zMax], [xMax,yMax,zMax], [xMin,yMax,zMax]]
    return GraphicsDataCube(pList, color)

#**function: generate graphics data forfor orthogonal 3D cube with center point and size
#**input: center point and size of cube (as 3D list or np.array); color provided as list of 4 RGBA values
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataOrthoCubePoint(centerPoint, size, color=[0.,0.,0.,1.]): 
    
    xMin = centerPoint[0] - 0.5*size[0]
    yMin = centerPoint[1] - 0.5*size[1]
    zMin = centerPoint[2] - 0.5*size[2]
    xMax = centerPoint[0] + 0.5*size[0]
    yMax = centerPoint[1] + 0.5*size[1]
    zMax = centerPoint[2] + 0.5*size[2]

    return GraphicsDataOrthoCube(xMin, yMin, zMin, xMax, yMax, zMax, color)

#**function: generate graphics data for general cube with endpoints, according to given vertex definition
#**input: 
#  pList: is a list of points [[x0,y0,z0],[x1,y11,z1],...]
#  color: provided as list of 4 RGBA values
#  faces: includes the list of six binary values (0/1), denoting active faces (value=1); set index to zero to hide face
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataCube(pList, color=[0.,0.,0.,1.], faces=[1,1,1,1,1,1]): 
# bottom: (z goes upwards from node 1 to node 5)
# ^y
# |
# 3---2
# |   |
# |   |
# 0---1-->x
#
# top:
# ^y
# |
# 7---6
# |   |
# |   |
# 4---5-->x
#
# faces: bottom, top, sideface0, sideface1, sideface2, sideface3 (sideface0 has nodes 0,1,4,5)

    colors=[]
    for i in range(8):
        colors=colors+color

    points = []
    for p in pList:
        points += p
#    points = [xMin,yMin,zMin, xMax,yMin,zMin, xMax,yMax,zMin, xMin,yMax,zMin,
#              xMin,yMin,zMax, xMax,yMin,zMax, xMax,yMax,zMax, xMin,yMax,zMax]

    #1-based ... triangles = [1,3,2, 1,4,3, 5,6,7, 5,7,8, 1,2,5, 2,6,5, 2,3,6, 3,7,6, 3,4,7, 4,8,7, 4,1,8, 1,5,8 ]
    #triangles = [0,2,1, 0,3,2, 6,4,5, 6,7,4, 0,1,4, 1,5,4, 1,2,5, 2,6,5, 2,3,6, 3,7,6, 3,0,7, 0,4,7]

#    triangles = [0,1,2, 0,2,3, 6,5,4, 6,4,7, 0,4,1, 1,4,5, 1,5,2, 2,5,6, 2,6,3, 3,6,7, 3,7,0, 0,7,4]
    trigList = [[0,1,2, 0,2,3], [6,5,4, 6,4,7], [0,4,1, 1,4,5], [1,5,2, 2,5,6], [2,6,3, 3,6,7], [3,7,0, 0,7,4]]
    triangles = []
    for i in range(6):
        if faces[i]:
            triangles += trigList[i]
    
    data = {'type':'TriangleList', 'colors': colors, 'points':points, 'triangles':triangles}

    return data

#**function: switch order of three items in a list; mostly used for reverting normals in triangles
#**input: 3D vector as list or as np.array
#**output: interchanged 2nd and 3rd component of list
def SwitchTripletOrder(vector):
    v=copy.deepcopy(vector) #copy, such that vector is not changed
    a = v[2]
    v[2] = v[1]
    v[1] = a
    return v

#**function: generate graphics data for a sphere with point p and radius
#**input:
#  point: center of sphere (3D list or np.array)
#  radius: positive value
#  color: provided as list of 4 RGBA values
#  nTiles: used to determine resolution of sphere >=3; use larger values for finer resolution
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataSphere(point, radius, color=[0.,0.,0.,1.], nTiles = 8):
    if nTiles < 3: print("WARNING: GraphicsDataSphere: nTiles < 3: set nTiles=3")
    
    p = copy.deepcopy(point)
    r = radius
    #orthonormal basis:
    e0=np.array([1,0,0])
    e1=np.array([0,1,0])
    e2=np.array([0,0,1])
    
    points = []
    normals = []
    colors = []
    triangles = []
    
    #create points for circles around z-axis with tiling
    for i0 in range(nTiles+1):
        for iphi in range(nTiles):
            z = -r*np.cos(np.pi*i0/nTiles)    #runs from -r .. r (this is the coordinate of the axis of circles)
            phi = 2*np.pi*iphi/nTiles #angle
            
            fact = np.sin(np.pi*i0/nTiles)
            x = fact*r*np.sin(phi)
            y = fact*r*np.cos(phi)

            vv = x*e0 + y*e1 + z*e2
            points += list(p + vv)
            
            n = Normalize(list(-vv))
            normals += n
            
            colors += color

    
    for i0 in range(nTiles):
        for iphi in range(nTiles):
            p0 = i0*nTiles+iphi
            p1 = (i0+1)*nTiles+iphi
            iphi1 = iphi + 1
            if iphi1 >= nTiles: 
                iphi1 = 0
            p2 = i0*nTiles+iphi1
            p3 = (i0+1)*nTiles+iphi1

            triangles += [p0,p3,p1, p0,p2,p3]
            
    data = {'type':'TriangleList', 'colors':colors, 'normals':normals, 'points':points, 'triangles':triangles}
    return data
            
#**function: generate graphics data for a cylinder with given axis, radius and color; nFaces gives the number of tiles (minimum=3)
#**input:
#  pAxis: axis point of one face of cylinder (3D list or np.array)
#  vAxis: vector representing the cylinder's axis (3D list or np.array)
#  radius: positive value representing radius of cylinder
#  color: provided as list of 4 RGBA values
#  nTiles: used to determine resolution of cylinder >=3; use larger values for finer resolution
#  angleRange: given in rad, to draw only part of cylinder (halfcylinder, etc.); for full range use [0..2 * pi]
#  lastFace: if angleRange != [0,2*pi], then the faces of the open cylinder are shown with lastFace = True
#  cutPlain: only used for angleRange != [0,2*pi]; if True, a plane is cut through the part of the cylinder; if False, the cylinder becomes a cake shape ...
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataCylinder(pAxis, vAxis, radius, color=[0.,0.,0.,1.], nTiles = 16, 
                         angleRange=[0,2*np.pi], 
                         lastFace = True, 
                         cutPlain = True):  

    if nTiles < 3: print("WARNING: GraphicsDataCylinder: nTiles < 3: set nTiles=3")
    
    #create points at left and right face
    points0=list(copy.deepcopy(pAxis)) #[pAxis[0],pAxis[1],pAxis[2]] #avoid change of pAxis
    pAxis1=[pAxis[0]+vAxis[0],pAxis[1]+vAxis[1],pAxis[2]+vAxis[2]]
    points1=copy.deepcopy(pAxis1) #[pAxis[0]+vAxis[0],pAxis[1]+vAxis[1],pAxis[2]+vAxis[2]] #copy in order to avoid change of pAxis1 for use lateron
    
    p0 = np.array(pAxis)
    p1 = np.array(pAxis) + np.array(vAxis)
    
    basis = ComputeOrthonormalBasis(vAxis)
    #v0 = basis[0]
    n1 = basis[1]
    n2 = basis[2]
    r=radius
    
    #create normals at left and right face (pointing inwards)
    normals0 = Normalize(vAxis)
    normals1 = Normalize([-vAxis[0],-vAxis[1],-vAxis[2]])

    points2 = []
    points3 = []
    
    alpha = angleRange[1]-angleRange[0] #angular range
    alpha0 = angleRange[0]

    fact = nTiles #create correct part of cylinder
    if alpha < 2.*np.pi: 
        fact = nTiles-1
    
    for i in range(nTiles):
        phi = alpha0 + i*alpha/fact
        x = r*np.sin(phi)
        y = r*np.cos(phi)
        vv = x*n1 + y*n2
        pz0 = p0 + vv
        pz1 = p1 + vv
        points0 += list(pz0)
        points1 += list(pz1)
        points2 += list(pz0) #other points for side faces (different normals)
        points3 += list(pz1) #other points for side faces (different normals)
        n = Normalize(list(-vv))
        normals0 = normals0 + n
        normals1 = normals1 + n
    
    points0 += points1+points2+points3
    normals0 += normals1

    for i in range(nTiles):
        normals0 += Normalize(vAxis)
    for i in range(nTiles):
        normals0 += Normalize([-vAxis[0],-vAxis[1],-vAxis[2]])

    n = nTiles+1 #number of points of one ring+midpoint
    colors=[]
    for i in range(2*n+2*nTiles):
        colors += color

    triangles = []
    #circumference:
    for i in range(nTiles):
        if i != nTiles-1:
            triangles += [1+i,n+1+i+1,n+1+i]
            triangles += [1+i,1+i+1,n+1+i+1]
        else:
            if lastFace and cutPlain:
                triangles += [1+i,n+1,n+1+i]
                triangles += [1+i,1,n+1]
            

    #sides faces left and right:
    nn=2*n #offset
    for i in range(nTiles):
        if i != nTiles-1:
            triangles += [0,nn+i+1,nn+i]
            triangles += [n,nn+nTiles+i,nn+nTiles+i+1]
        else:
            if cutPlain:
                triangles += [0,nn,nn+i]
                triangles += [n,nn+nTiles+i,nn+nTiles]

    #if angles are not 2*pi, add closing face
    if lastFace and not(cutPlain):
        s = int(len(points0)/3) #starting index for side triangles
        p2 = points2[0:3]
        p3 = points3[0:3]
        p4 = points2[len(points2)-3:len(points2)]
        p5 = points3[len(points3)-3:len(points3)]
        points0 += pAxis + pAxis1 + p2 + p3 + pAxis + pAxis1 + p4 + p5
        n1=np.cross(VSub(pAxis,pAxis1),VSub(p3,pAxis))
        n1=list(Normalize(n1))
        n2=np.cross(VSub(pAxis1,pAxis),VSub(p4,pAxis))
        n2=list(Normalize(n2))
        normals0 += n1+n1+n1+n1+n2+n2+n2+n2  #8 additional normals
        triangles += [s+0,s+3,s+1, s+0,s+2,s+3, 
                      s+5,s+6,s+4, s+5,s+7,s+6]
        for i in range(8): #8 additional colors
            colors += color

    #triangle normals point inwards to object ...
    data = {'type':'TriangleList', 'colors':colors, 'normals':normals0, 'points':points0, 'triangles':triangles}

    return data

#**function: generate graphics data for a planar Link between the two joint positions, having two axes
#**input:
#  p0: joint0 center position
#  p1: joint1 center position
#  axis0: direction of rotation axis at p0, if drawn as a cylinder; [0,0,0] otherwise
#  axis1: direction of rotation axis of p1, if drawn as a cylinder; [0,0,0] otherwise
#  radius: list of two radii [radius0, radius1], being the two radii of the joints drawn by a cylinder or sphere    
#  width: list of two widths [width0, width1], being the two widths of the joints drawn by a cylinder; ignored for sphere    
#  thickness: the thickness of the link (shaft) between the two joint positions; thickness in z-direction or diameter (cylinder)
#  color: provided as list of 4 RGBA values
#  nTiles: used to determine resolution of cylinder >=3; use larger values for finer resolution
#**output: graphicsData dictionary, to be used in visualization of EXUDYN objects
def GraphicsDataRigidLink(p0,p1,axis0=[0,0,0], axis1=[0,0,0], radius=[0.1,0.1], 
                          thickness=0.05, width=[0.05,0.05], color=[0.,0.,0.,1.], nTiles = 16):
    linkAxis = VSub(p1,p0)
    linkAxis0 = Normalize(linkAxis)
    a0=copy.deepcopy(axis0)
    a1=copy.deepcopy(axis1)
    
    data0 = GraphicsDataCylinder(p0, linkAxis, 0.5*thickness, color, nTiles)
    data1 = {}
    data2 = {}

    if NormL2(axis0) == 0:
        data1 = GraphicsDataSphere(p0, radius[0], color, nTiles)
    else:
        a0=Normalize(a0)
        data1 = GraphicsDataCylinder(list(np.array(p0)-0.5*width[0]*np.array(a0)), 
                                     list(width[0]*np.array(a0)), 
                                     radius[0], color, nTiles)
        
    if NormL2(axis1) == 0:
        data2 = GraphicsDataSphere(p1, radius[1], color, nTiles)
    else:
        a1=Normalize(a1)
        data2 = GraphicsDataCylinder(list(np.array(p1)-0.5*width[1]*np.array(a1)), 
                                     list(width[1]*np.array(a1)), radius[1], color, nTiles)

    #now merge lists, including appropriate indices of triangle points!
    np0 = int(len(data0['points'])/3) #number of points of first point list ==> this is the offset for next list
    np1 = np0 + int(len(data1['points'])/3) #number of points of first point list ==> this is the offset for next list

    triangles = data0['triangles']
    trigs1 = np.array(data1['triangles'])
    trigs1 += np0
    triangles += list(trigs1)
    
    trigs2 = np.array(data2['triangles'])
    trigs2 += np1
    triangles += list(trigs2)
    
    points = data0['points'] + data1['points'] + data2['points']
    normals = data0['normals'] + data1['normals'] + data2['normals']
    colors = data0['colors'] + data1['colors'] + data2['colors']
    
    data = {'type':'TriangleList', 'colors':colors, 'normals':normals, 'points':points, 'triangles':triangles}
    return data


#**function: generate graphics data from STL file (text format!) and use color for visualization
#**input:
#  fileName: string containing directory and filename of STL-file (in text / SCII format) to load
#  color: provided as list of 4 RGBA values
#  verbose: if True, useful information is provided during reading
#**output: interchanged 2nd and 3rd component of list
def GraphicsDataFromSTLfileTxt(fileName, color=[0.,0.,0.,1.], verbose=False): 
#file format, just one triangle, using GOMinspect:
#solid solidName
#facet normal -0.979434 0.000138 -0.201766
# outer loop
#    vertex 9.237351 7.700452 -9.816338
#    vertex 9.237478 10.187849 -9.815249
#    vertex 9.706021 10.170116 -12.089709
# endloop
#endfacet
#...
#endsolid solidName
    if verbose: print("read STL file: "+fileName)

    fileLines = []
    try: #still close file if crashes
        file=open(fileName,'r') 
        fileLines = file.readlines()
    finally:
        file.close()    

    colors=[]
    points = []
    normals = []
    triangles = []

    nLines = len(fileLines)
    lineCnt = 0
    if fileLines[lineCnt][0:5] != 'solid':
        raise ValueError("GraphicsDataFromSTLfileTxt: expected 'solid ...' in first line, but received: " + fileLines[lineCnt])
    lineCnt+=1

    while lineCnt < nLines and fileLines[lineCnt].strip().split()[0] != 'endsolid':
        if lineCnt%100000 == 0 and lineCnt !=0: 
            if verbose: print("  read line",lineCnt," / ", len(fileLines))

        normalLine = fileLines[lineCnt].split()
        if normalLine[0] != 'facet' or normalLine[1] != 'normal':
            raise ValueError("GraphicsDataFromSTLfileTxt: expected 'facet normal ...' in line "+str(lineCnt)+", but received: " + fileLines[lineCnt])
        if len(normalLine) != 5:
            raise ValueError("GraphicsDataFromSTLfileTxt: expected 'facet normal n0 n1 n2' in line "+str(lineCnt)+", but received: " + fileLines[lineCnt])
        
        normal = [-float(normalLine[2]),-float(normalLine[3]),-float(normalLine[4])]

        lineCnt+=1
        loopLine = fileLines[lineCnt].strip()
        if loopLine != 'outer loop':
            raise ValueError("GraphicsDataFromSTLfileTxt: expected 'outer loop' in line "+str(lineCnt)+", but received: " + fileLines[lineCnt])

        ind = int(len(points)/3) #index for points of this triangle
        #get 3 vertices:
        lineCnt+=1
        for i in range(3):
            readLine = fileLines[lineCnt].strip().split()
            if readLine[0] != 'vertex':
                raise ValueError("GraphicsDataFromSTLfileTxt: expected 'vertex ...' in line "+str(lineCnt)+", but received: " + fileLines[lineCnt])
            if len(readLine) != 4:
                raise ValueError("GraphicsDataFromSTLfileTxt: expected 'vertex v0 v1 v2' in line "+str(lineCnt)+", but received: " + fileLines[lineCnt])
            
            points+=[float(readLine[1]),float(readLine[2]),float(readLine[3])]
            normals+=normal
            colors+=color
            lineCnt+=1
            
        triangles+=[ind,ind+2,ind+1] #indices of points; flip indices to match definition in EXUDYN

        loopLine = fileLines[lineCnt].strip()
        if loopLine != 'endloop':
            raise ValueError("GraphicsDataFromSTLfileTxt: expected 'endloop' in line "+str(lineCnt)+", but received: " + fileLines[lineCnt])
        lineCnt+=1
        loopLine = fileLines[lineCnt].strip()
        if loopLine != 'endfacet':
            raise ValueError("GraphicsDataFromSTLfileTxt: expected 'endfacet' in line "+str(lineCnt)+", but received: " + fileLines[lineCnt])
        lineCnt+=1
    
    data = {'type':'TriangleList', 'colors':colors, 'normals':normals, 'points':points, 'triangles':triangles}
    return data


