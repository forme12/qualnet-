// Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive
//                          Suite 1250
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

/// \file terrain.h
/// \brief QualNet Terrain API
/// 
/// This file describes data structures and functions used by physical terrain models.

#ifndef TERRAIN_H
#define TERRAIN_H

#include <list>
#include <string>
#include <vector>

#include "coordinates.h"
#include "fileio.h"
#include "geometry.h"

#define MAX_NUM_ELEVATION_SAMPLES 16384
#define MIN_BUILDING_SEPARATION   1.0 // this must be > 0
#define THIN_WALL_THICKNESS       0.06

// pre-declare for inclusion in other classes.
class PathSegment;
class TerrainRegion;
class TerrainData;

using namespace std;

/// \enum TerrainRegionType
/// \brief Pre-processed region and path segment types.
enum TerrainRegionType {
    OPEN_REGION,
    RURAL_REGION,
    URBAN_REGION,
    SUBURBAN_REGION,
    INDOOR_REGION
};

namespace TERRAIN
{

/// Construction materials for walls and floors for detailed urban formats.
enum ConstructionMaterials {
    CONCRETE = 0,
    LIGHTWEIGHTCONCRETE,
    SYNTHETICRESIN,
    PLASTERBOARD,
    ROCKWOOL,
    GLASS,
    FIBERGLASS,
    WOOD,
    METAL,
    NA
};

}

/// Foliage states
enum FoliatedState {
    IN_LEAF,
    OUT_OF_LEAF
};

/// Location states
enum TerrainLocationType {
    INSIDE_LOCATION, 
    OUTSIDE_LOCATION
};

/// Location states
enum TerrainIndoorType {
    ROOM_INDOORS, 
    CORRIDOR_INDOORS,
    OPEN_AREA_INDOORS
};

/// PathSegment is used to break a path into segments that cross several
//  regions.  The region type and region* are stored in the segment for
//  efficient access.
class PathSegment {
private:
    Coordinates       m_source;
    Coordinates       m_dest;
    TerrainRegionType m_pathType;
    TerrainRegion*    m_region;
public:
    PathSegment() {
        m_region = NULL;
    }
    ~PathSegment() {}

    void setSource(const Coordinates* c)    { m_source = *c; }
    void setDest(const Coordinates* c)      { m_dest = *c; }
    void setType(const TerrainRegionType t) { m_pathType = t; }
    void setRegion(TerrainRegion* r)        { m_region = r; }

    Coordinates       getSource() { return m_source; }
    Coordinates       getDest()   { return m_dest; }
    TerrainRegionType getType()   { return m_pathType; }
    TerrainRegion*    getRegion() { return m_region; }
};

/// TerrainRegion contains pre-computed values for a sub-region of the overall
//  terrain.  Used for abstract, high-efficiency versions of urban propagation
//  functions.
class TerrainRegion {
private:
    int  m_coordinateSystemType;

    Coordinates m_sw;
    Coordinates m_ne;
    double      m_area; // meters squared

    double m_maxElevation; // altitude in meters
    double m_minElevation; // altitude in meters

    // outdoor urban properties
    double m_maxRoofHeight; // meters
    double m_minRoofHeight; // meters
    double m_avgRoofHeight; // meters
    int    m_numBuildings;
    double m_buildingCoverage; // percentage of region covered
    double m_avgStreetWidth;   // meters

    // indoor urban properties
    double m_avgCeilingHeight; // meters
    double m_avgWallSpacing;   // meters
    double m_avgWallThickness; // meters (maybe centimeters?)
    double m_percentageThickWalls;

    TERRAIN::ConstructionMaterials m_thickWallMaterial;
    TERRAIN::ConstructionMaterials m_thinWallMaterial;
    
    // foliage properties
    int    m_numFoliage;
    double m_foliageCoverage;   // percentage of region covered
    double m_avgFoliageHeight;  // meters
    double m_avgFoliageDensity; // some measure of thickness
    FoliatedState m_typicalFoliatedState;

    // derived properties
    TerrainRegionType m_regionType;

    void reset() {
        m_maxElevation         = 0.0;
        m_minElevation         = 0.0;
        m_maxRoofHeight        = 0.0;
        m_minRoofHeight        = 0.0;
        m_avgRoofHeight        = 0.0;
        m_numBuildings         = 0;
        m_buildingCoverage     = 0.0;
        m_avgStreetWidth       = 0.0;
        m_avgCeilingHeight     = 0.0;
        m_avgWallSpacing       = 0.0;
        m_avgWallThickness     = 0.0;
        m_percentageThickWalls = 0.5;
        m_thickWallMaterial    = TERRAIN::LIGHTWEIGHTCONCRETE;
        m_thinWallMaterial     = TERRAIN::PLASTERBOARD;
        m_numFoliage           = 0;
        m_foliageCoverage      = 0.0;
        m_avgFoliageHeight     = 0.0;
        m_avgFoliageDensity    = 0.0;
        m_typicalFoliatedState = IN_LEAF;
    }

public:
    enum Mode {
        FULL,     // populate with all data
        SIMPLE,   // only count buildings and avgHeight
        BUILDINGS // all building values, but no foliage
    };

    TerrainRegion(const int coordinateSystem,
                  const Coordinates* sw,
                  const Coordinates* ne) {
        m_coordinateSystemType = coordinateSystem;
        m_sw = *sw;
        m_ne = *ne;
        reset();
        calculateArea();
    }
    TerrainRegion() { reset(); }
    ~TerrainRegion() { }

    void setCoordinateSystem(const int type) { m_coordinateSystemType = type; }
    void setSW(const Coordinates* sw) { m_sw = *sw; }
    void setNE(const Coordinates* ne) { m_ne = *ne; }
    void calculateArea() {
        Box box(m_sw, m_ne);
        m_area = box.getArea(m_coordinateSystemType);
    }

    Coordinates getSW() const { return m_sw; }
    Coordinates getNE() const { return m_ne; }
    double getArea() const    { return m_area; }

    void calculateRegionType();
    void setRegionType(const TerrainRegionType rType) { m_regionType = rType; }
    TerrainRegionType getRegionType() const { return m_regionType; }

    bool contains(const Coordinates* point) const {
        return COORD_PointWithinRange(m_coordinateSystemType,
                                      &m_sw, &m_ne, point);
    }
    bool contains(const Coordinates point) const {
        return COORD_PointWithinRange(m_coordinateSystemType,
                                      &m_sw, &m_ne, &point);
    }
    /*! checks whether the given line intersects the region */
    bool intersects(const Coordinates* c1,
                    const Coordinates* c2) const;
    bool intersects(const Box* box) const;

    /*! returns a path segment containing the points where the line segment
     *  intersects the region */
    PathSegment getIntersect(const Coordinates* c1,
                             const Coordinates* c2);

    void setMaxElevation(const double val)      { m_maxElevation      = val; }
    void setMinElevation(const double val)      { m_minElevation      = val; }
    void setMaxRoofHeight(const double val)     { m_maxRoofHeight     = val; }
    void setMinRoofHeight(const double val)     { m_minRoofHeight     = val; }
    void setAvgRoofHeight(const double val)     { m_avgRoofHeight     = val; }
    void setNumBuildings(const int val)         { m_numBuildings      = val; }
    void setBuildingCoverage(const double val)  { m_buildingCoverage  = val; }
    void setAvgStreetWidth(const double val)    { m_avgStreetWidth    = val; }
    void setAvgCeilingHeight(const double val)  { m_avgCeilingHeight  = val; }
    void setAvgWallSpacing(const double val)    { m_avgWallSpacing    = val; }
    void setAvgWallThickness(const double val)  { m_avgWallThickness  = val; }
    void setNumFoliage(const int val)           { m_numFoliage        = val; }
    void setFoliageCoverage(const double val)   { m_foliageCoverage   = val; }
    void setAvgFoliageHeight(const double val)  { m_avgFoliageHeight  = val; }
    void setAvgFoliageDensity(const double val) { m_avgFoliageDensity = val; }
    void setFoliatedState(const FoliatedState val) {
        m_typicalFoliatedState = val;
    }
    void setPercentageThickWalls(const double val) {
        m_percentageThickWalls = val;
    }
    void setThickWallMaterial(const TERRAIN::ConstructionMaterials val) {
        m_thickWallMaterial = val;
    }
    void setThinWallMaterial(const TERRAIN::ConstructionMaterials val) {
        m_thinWallMaterial = val;
    }

    double getMaxElevation() const         { return m_maxElevation; }
    double getMinElevation() const         { return m_minElevation; }
    double getMaxRoofHeight() const        { return m_maxRoofHeight; }
    double getMinRoofHeight() const        { return m_minRoofHeight; }
    double getAvgRoofHeight()const         { return m_avgRoofHeight; }
    int    getNumBuildings() const         { return m_numBuildings; }
    double getBuildingCoverage() const     { return m_buildingCoverage; }
    double getAvgStreetWidth() const       { return m_avgStreetWidth; }
    double getAvgCeilingHeight() const     { return m_avgCeilingHeight; }
    double getAvgWallSpacing() const       { return m_avgWallSpacing; }
    double getAvgWallThickness() const     { return m_avgWallThickness; }
    double getPercentageThickWalls() const { return m_percentageThickWalls; }
    int    getNumFoliage() const           { return m_numFoliage; }
    double getFoliageCoverage() const      { return m_foliageCoverage; }
    double getAvgFoliageHeight() const     { return m_avgFoliageHeight; }
    double getAvgFoliageDensity() const    { return m_avgFoliageDensity; }
    FoliatedState getFoliatedState() const { return m_typicalFoliatedState; }
    TERRAIN::ConstructionMaterials getThickWallMaterial() const {
        return m_thickWallMaterial;
    }
    TERRAIN::ConstructionMaterials getThinWallMaterial() const {
        return m_thinWallMaterial;
    }
    void print() const;
};


/// UrbanIndoorPathProperties is a fully implemented base class that provides the
//  methods required for indoor propagation models.
class UrbanIndoorPathProperties {
private:
    int m_numFloors;
    int m_numWalls;
    int m_numThinWalls;
    int m_numThickWalls;
    double m_distance;
public:
    std::vector<TERRAIN::ConstructionMaterials> m_floorMaterials;
    std::vector<TERRAIN::ConstructionMaterials> m_wallMaterials;
    std::vector<double> m_floorThicknesses;
    std::vector<double> m_wallThicknesses;

    UrbanIndoorPathProperties() { }

    virtual void countWalls() { // iterates through walls to count thick and thin
        std::vector<double>::const_iterator iter;
        m_numThinWalls = 0;
        m_numThickWalls = 0;
        for (iter = m_wallThicknesses.begin();
             iter != m_wallThicknesses.end();
             iter++) {
            if (*iter <= THIN_WALL_THICKNESS) m_numThinWalls++;
            else m_numThickWalls++;
        }
        assert((m_numThinWalls + m_numThickWalls) == m_numWalls);
    }

    void setDistance(const double val) { m_distance = val; }
    void setNumFloors(const int val) { m_numFloors = val; }
    void setNumWalls(const int val)  { m_numWalls = val; }
    void setNumThinWalls(const int val) { m_numThinWalls = val; }
    void setNumThickWalls(const int val) { m_numThickWalls = val; }

    int  getNumFloors() const     { return m_numFloors; }
    int  getNumWalls() const      { return m_numWalls; }
    int  getNumThickWalls() const { return m_numThickWalls; }
    int  getNumThinWalls() const  { return m_numThinWalls; }
    double getDistance() const    { return m_distance; }

    // returns most common material
    virtual TERRAIN::ConstructionMaterials getMostCommonFloorMaterial();
    virtual TERRAIN::ConstructionMaterials getMostCommonThickWallMaterial();
    virtual TERRAIN::ConstructionMaterials getMostCommonThinWallMaterial();

    virtual double getAvgFloorThickness();
    virtual double getAvgWallThickness();
    virtual double getAvgThickWallThickness();
    virtual double getAvgThinWallThickness();

    virtual void print() = 0;
};


/// UrbanPathProperties is an abstract super-class that includes all the function
//  calls that are required for the urban propagation code.  Each urban terrain
//  interface must create a subclass that calculates the values.
class UrbanPathProperties {
protected:
    Coordinates m_source;
    Coordinates m_dest;

    int     m_coordinateSystemType;
    double  m_distance;
    int     m_numBuildings;
    double  m_avgBuildingSeparation;
    double  m_avgBuildingHeight;
    double  m_avgStreetWidth;
    double  m_srcDistanceToBuilding;  // used by street microcell nlos
    double  m_destDistanceToBuilding; // used by street microcell nlos
    double  m_srcDistanceToIntersection;
    double  m_destDistanceToIntersection;
    double  m_intersectionAngle;
    double  m_relativeOrientation;
    double  m_maxRoofHeight;
    double  m_minRoofHeight;
    int     m_numFoliage;
    double  m_avgFoliageHeight;
    std::vector<double> m_distancesThroughBuildings;
    std::vector<double> m_distancesThroughFoliage;
    std::vector<double> m_foliageDensities;
    std::vector<FoliatedState> m_foliatedStates;
public:
    UrbanPathProperties(const Coordinates* c1,
                        const Coordinates* c2,
                        const int coordinateSystem) {
        m_coordinateSystemType = coordinateSystem;
        m_source                     = *c1;
        m_dest                       = *c2;
        COORD_CalcDistance(coordinateSystem, c1, c2, &m_distance);
        m_numBuildings               = 0;
        m_avgBuildingSeparation      = MIN_BUILDING_SEPARATION;
        m_avgBuildingHeight          = 0.0;
        m_avgStreetWidth             = 0.0;
        m_srcDistanceToBuilding      = 0.0;
        m_destDistanceToBuilding     = 0.0;
        m_srcDistanceToIntersection  = 0.0;
        m_destDistanceToIntersection = 0.0;
        m_intersectionAngle          = 0.0;
        m_relativeOrientation        = 90.0;
        m_maxRoofHeight              = 0.0;
        m_minRoofHeight              = 0.0;
        m_numFoliage                 = 0;
        m_avgFoliageHeight           = 0.0;
        m_distancesThroughBuildings.clear();
        m_distancesThroughFoliage.clear();
        m_foliageDensities.clear();
        m_foliatedStates.clear();
    }
    virtual ~UrbanPathProperties() {
        m_distancesThroughBuildings.clear();
        m_distancesThroughFoliage.clear();
        m_foliageDensities.clear();
        m_foliatedStates.clear();
    }

    virtual double getDistance()                   { return m_distance; }
    virtual int    getNumBuildings()               { return m_numBuildings; }
    virtual double getAvgBuildingSeparation()      { return m_avgBuildingSeparation; }
    virtual double getAvgBuildingHeight()          { return m_avgBuildingHeight; }          
    virtual double getAvgStreetWidth()             { return m_avgStreetWidth; }
    virtual double getSrcDistanceToBuilding()      { return m_srcDistanceToBuilding; }
    virtual double getDestDistanceToBuilding()     { return m_destDistanceToBuilding; }
    virtual double getSrcDistanceToIntersection()  { return m_srcDistanceToIntersection; }
    virtual double getDestDistanceToIntersection() { return m_destDistanceToIntersection; }
    virtual double getIntersectionAngle()          { return m_intersectionAngle; }
    virtual double getRelativeOrientation()        { return m_relativeOrientation; }
    virtual double getMaxRoofHeight()              { return m_maxRoofHeight; }
    virtual double getMinRoofHeight()              { return m_minRoofHeight; }
    virtual int    getNumFoliage()                 { return m_numFoliage; }
    virtual double getAvgFoliageHeight()           { return m_avgFoliageHeight; }

    virtual bool getDoStreetsCross(const Coordinates* c1,
                                   const Coordinates* c2) { return false; }

    virtual double getDistanceThroughBuilding(const int b) = 0;
    virtual double getDistanceThroughFoliage(const int f)  = 0;
    virtual double getFoliageDensity(const int f)          = 0;
    virtual FoliatedState getFoliatedState(const int f)    = 0;

    void setDistance(const double val)                   { m_distance = val; }
    void setNumBuildings(const int val)                  { m_numBuildings = val; }
    void setAvgBuildingSeparation(const double val)      { m_avgBuildingSeparation = val; }
    void setAvgBuildingHeight(const double val)          { m_avgBuildingHeight = val; }
    void setAvgStreetWidth(const double val)             { m_avgStreetWidth = val; }
    void setSrcDistanceToBuilding(const double val)      { m_srcDistanceToBuilding = val; }
    void setDestDistanceToBuilding(const double val)     { m_destDistanceToBuilding = val; }
    void setSrcDistanceToIntersection(const double val)  { m_srcDistanceToIntersection = val; }
    void setDestDistanceToIntersection(const double val) { m_destDistanceToIntersection = val; }
    void setIntersectionAngle(const double val)          { m_intersectionAngle = val; }
    void setRelativeOrientation(const double val)        { m_relativeOrientation = val; }
    void setMaxRoofHeight(const double val)              { m_maxRoofHeight = val; }
    void setMinRoofHeight(const double val)              { m_minRoofHeight = val; }
    void setNumFoliage(const int val)                    { m_numFoliage = val; }
    void setAvgFoliageHeight(const double val)           { m_avgFoliageHeight = val; }

    virtual int getNumObstructions() { return (m_numBuildings + m_numFoliage); }

    virtual void print() = 0;
};

/// UrbanRegionPathProperties is a subclass of UrbanPathProperties for
//  retrieving values from the local pre-processed region
class UrbanRegionPathProperties : public UrbanPathProperties {
private:
    TerrainRegion* m_region;
    double m_avgBuildingWidth;
    double m_avgFoliageWidth;

    /// called after the constructor to analyze the region to make a guess at
    //  the number of number of buildings and foliage that cross the path.
    void initialize();
public:
    UrbanRegionPathProperties(TerrainRegion* r,
                              const Coordinates* c1,
                              const Coordinates* c2,
                              const int coordinateSystem) :
        UrbanPathProperties(c1, c2, coordinateSystem)
    {
        m_region = r;
        initialize();
    }
    ~UrbanRegionPathProperties() {}

    // Many of these functions can't be supported by the pre-computed values,
    // so we have to estimate.
    int    getNumBuildings() const                { return m_numBuildings; }
    double getAvgBuildingSeparation() const       { return (m_region->getAvgStreetWidth() / 2.0); }
    double getAvgBuildingHeight() const           { return m_region->getAvgRoofHeight(); }
    double getAvgStreetWidth() const              { return m_region->getAvgStreetWidth(); }
    double getSrcDistanceToBuilding() const       { return getAvgBuildingSeparation(); }
    double getDestDistanceToBuilding() const      { return getAvgBuildingSeparation(); }
    double getSrcDistanceToIntersection() const   { return (m_distance / 2.0); }
    double getDestDistanceToIntersection() const  { return (m_distance / 2.0); }
    double getIntersectionAngle() const           { return 90.0; }
    double getRelativeOrientation() const         { return 90.0; }
    double getMaxRoofHeight() const               { return m_region->getMaxRoofHeight(); }
    double getMinRoofHeight() const               { return m_region->getMinRoofHeight(); }
    int    getNumFoliage() const                  { return m_numFoliage; }
    double getAvgFoliageHeight() const            { return m_region->getAvgFoliageHeight(); }

    // The region based path calculation can't support these directly, so returns
    // averages.
    double getDistanceThroughBuilding(const int b) const { return m_avgBuildingWidth; }
    double getDistanceThroughFoliage(const int f) const  { return m_avgFoliageWidth; }
    double getFoliageDensity(const int f) const          { return m_region->getAvgFoliageDensity(); }
    FoliatedState getFoliatedState(const int f) const    { return m_region->getFoliatedState(); }

    void print();
};

/// ElevationTerrainData is a base class for elevation data formats such
//  as DTED or DEM.
class ElevationTerrainData {
protected:
    TerrainData* m_terrainData;
    Coordinates  m_sw;
    Coordinates  m_ne;
    std::string  m_modelName;
public:
    ElevationTerrainData(TerrainData* td) {
        m_terrainData = td;
        m_modelName = "NONE";
    }
    virtual ~ElevationTerrainData() {}

    virtual std::string getModelName() { return m_modelName; }

    void setSWCorner(const Coordinates* c) {m_sw = *c;}
    void setNECorner(const Coordinates* c) {m_ne = *c;}
    bool pointInRange(const int          coordinateType,
                      const Coordinates* point) {
        return COORD_PointWithinRange(coordinateType,
                                      &m_sw,
                                      &m_ne,
                                      point);
    }

    virtual void initialize(NodeInput* nodeInput) {}
    virtual void finalize() {}

    virtual bool hasData() { return false; } // subclasses should return true.

    virtual double getElevationAt(const Coordinates* c) { return 0.0; }
    virtual int getElevationArray(const Coordinates* c1,
                                  const Coordinates* c2,
                                  const double       distance,
                                  const double       samplingDistance,
                                  double             elevationArray[],
                                  const int          maxSamples = MAX_NUM_ELEVATION_SAMPLES);

    virtual void getHighestAndLowestElevation(const Coordinates* sw,
                                              const Coordinates* ne,
                                              double* highest,
                                              double* lowest) {
        *highest = 0.0;
        *lowest = 0.0;
    }
};

/// \brief UrbanTerrainData is a fully implemented super-class that defines
/// the required interface for supporting new urban terrain formats.
class UrbanTerrainData {
protected:
    TerrainData* m_terrainData;
    std::string  m_modelName;

public:
    UrbanTerrainData(TerrainData* td) {
        m_terrainData = td;
        m_modelName = "NONE";
    }
    ~UrbanTerrainData() { }

    virtual void initialize(NodeInput* nodeInput,
                            bool       masterProcess = false) {}
    virtual void finalize() {}

    std::string getModelName() { return m_modelName; }

    virtual bool hasData() { return false; } // subclasses should return true.

    virtual std::string* fileList(NodeInput* nodeInput) {
        return new std::string("");
    }

    virtual void populateRegionData(
        TerrainRegion* region,
        TerrainRegion::Mode mode = TerrainRegion::FULL) {}

    // point functions
    virtual bool isPositionIndoors(const Coordinates* c) { return false; }
    virtual bool arePositionsInSameBuilding(const Coordinates* c1,
                                            const Coordinates* c2) { return false; }

    virtual double getElevationAt(const Coordinates* c) { return 0.0; }

    virtual bool getEdgeOfBuilding(const  Coordinates* c1,
                                   const  Coordinates* c2,
                                   Coordinates* edge) {return false;}

    // path functions
    virtual UrbanIndoorPathProperties* getIndoorPathProperties(
        const  Coordinates* c1,
        const  Coordinates* c2,
        double pathWidth) { return NULL; }
    virtual UrbanPathProperties* getUrbanPathProperties(
        const  Coordinates* c1,
        const  Coordinates* c2,
        double pathWidth,
        bool   includeFoliage = false) { return NULL; }
};

/// \brief TerrainData is the master location for all terrain data, and
/// should generally be a singleton within each process.
class TerrainData {
private:
    Coordinates m_sw;
    Coordinates m_ne;
    Coordinates m_dimensions; // for convenience

    int m_coordinateSystemType;
    
    bool m_boundaryCheck;

    // terrain grid
    bool m_useRegions;
    int  m_gridRows;
    int  m_gridCols;
    TerrainRegion* m_regions;

    void initializeRegions(NodeInput* nodeInput);
    void calculateNE();
    void calculateDimensions();

public:
    // current design only allows for one urban and one elevation interface.
    ElevationTerrainData* m_elevationData;
    UrbanTerrainData*     m_urbanData;

    TerrainData() {
        m_useRegions    = false;
        m_gridRows      = 1;
        m_gridCols      = 1;
        m_elevationData = NULL;
        m_urbanData     = NULL;
        m_regions       = NULL;
    }
    ~TerrainData() {} // cleanup performed in finalize.

    void initialize(NodeInput* nodeInput,
                    bool       masterProcess);
    void finalize();
    void print();

    bool hasElevationData() { return m_elevationData->hasData(); }
    bool hasUrbanData()     { return m_urbanData->hasData(); }
    std::string getElevationModel() { return m_elevationData->getModelName(); }
    std::string getUrbanModel()     { return m_urbanData->getModelName(); }

    // used by terrain databases that provide both urban and elevation data
    // only urban is initialized and it sets this.
    void setElevationData(ElevationTerrainData* etd) {
        if (m_elevationData != NULL)
            delete m_elevationData;
        m_elevationData = etd;
    }

    // point functions
    double getGroundElevation(const Coordinates* c) {
        if(hasElevationData()) {
            return m_elevationData->getElevationAt(c);
        } else if(hasUrbanData()) {
            return m_urbanData->getElevationAt(c);
        } else {
            return 0.0;
        }
    }
    int getElevationArray(const Coordinates* c1,
                          const Coordinates* c2,
                          double             distance,
                          double             samplingDistance,
                          double             elevationArray[]) {
        return m_elevationData->getElevationArray(c1, c2, distance,
                                                  samplingDistance,
                                                  elevationArray);
    }

    bool isPositionIndoors(const Coordinates* c);

    bool arePositionsInSameBuilding(const Coordinates* c1,
                                    const Coordinates* c2) {
        return m_urbanData->arePositionsInSameBuilding(c1, c2);
    }

    void boundCoordinatesToTerrain(Coordinates* c,
                                   bool printWarnings = true);

    bool pointWithinRange(const Coordinates* c) {
        return COORD_PointWithinRange(m_coordinateSystemType,
                                      &m_sw, &m_ne, c);
    }

    // Location functions
    bool getIsCanyon(const Coordinates* c,
                     const double antennaHeight);

    // Path functions
    bool getFreeSpace(const Coordinates* c1,
                      const Coordinates* c2,
                      const double pathWidth);
    bool getLineOfSight(const Coordinates* c1,
                        const Coordinates* c2) {
        return getFreeSpace(c1, c2, 0.0);
    }

    UrbanIndoorPathProperties* getIndoorPathProperties(const Coordinates* c1,
                                                  const Coordinates* c2);
    UrbanPathProperties* getUrbanPathProperties(const  Coordinates* c1,
                                                const  Coordinates* c2,
                                                double pathWidth = 0.0,
                                                bool   includeFoliage = false);
    UrbanPathProperties* getUrbanPathProperties(PathSegment* ps,
                                                bool includeFoliage = false);
    void findEdgeOfBuilding(const Coordinates* indoorPoint,
                            const Coordinates* outdoorPoint,
                            Coordinates*       edgePoint);
    void getMinMaxBuildings(const Coordinates* c1,
                            const Coordinates* c2,
                            double* minHeight,
                            double* maxHeight);
    std::list<PathSegment> getPathSegments(const Coordinates* c1,
                                           const Coordinates* c2);

    std::string* terrainFileList(NodeInput* nodeInput);

    void setCoordinateSystem(int coordinateSystem) {
        m_coordinateSystemType = coordinateSystem;
    }
    int  getCoordinateSystem() { return m_coordinateSystemType; }

    void setSW(const Coordinates* c)         { m_sw = *c; }
    void setNE(const Coordinates* c)         { m_ne = *c; calculateDimensions(); }
    void setOrigin(const Coordinates* c)     { m_sw = *c; }
    void setDimensions(const Coordinates* c) { m_dimensions = *c; calculateNE(); }
    Coordinates getSW()         { return m_sw; }
    Coordinates getNE()         { return m_ne; }
    Coordinates getOrigin()     { return m_sw; }
    Coordinates getDimensions() { return m_dimensions; }

    bool usingRegions() { return m_useRegions; }
    bool checkBoundaries() { return m_boundaryCheck; }
};

/// \brief Sets the Z coordinate of the location to ground level.
/// \param terrainData the complete terrain data
/// \param position the position being set.
static
void TERRAIN_SetToGroundLevel(
    TerrainData* terrainData,
    Coordinates* position) {

    position->common.c3 = terrainData->getGroundElevation(position);
}

/// \brief Bound coordinates to terrain dimensions if necessary
/// \param terrainData   : const TerrainData* : Pointer to the terrain data
/// \param point         : Coordinates* : Coordinates
/// \param printWarnings : bool : Print warnings if out of range
static
void TERRAIN_BoundCoordinatesToTerrain(
    TerrainData* terrainData,
    Coordinates* point,
    bool printWarnings = true) {

    terrainData->boundCoordinatesToTerrain(point, printWarnings);
}


/// \brief Checks whether the coordinates are in the terrain area.
/// \param terrainData Pointer to the terrain data
/// \param point       Coordinates.
/// \return     true if within the area
static bool TERRAIN_PointWithinRange(
    TerrainData* terrainData,
    const Coordinates* point) {

    if (terrainData->getCoordinateSystem() == CARTESIAN) {
        Coordinates origin     = terrainData->getOrigin();
        Coordinates dimensions = terrainData->getDimensions();

        return COORD_PointWithinRange(terrainData->getCoordinateSystem(),
                                      &origin,
                                      &dimensions,
                                      point);
    }
    else
    {
        Coordinates sw = terrainData->getSW();
        Coordinates ne = terrainData->getNE();

        return COORD_PointWithinRange(terrainData->getCoordinateSystem(),
                                      &sw,
                                      &ne,
                                      point);
    }
}


/// \brief Get the array containing a set of elevations
/// \param terrainData      Terrain data.
/// \param c1 first endpoint
/// \param c2 last endpoint
/// \param distance distance between the endpoints
/// \param samplingDistance sampling distance
/// \param elevationArray   array to be filled
/// \return number of elements
static
int TERRAIN_GetElevationArray(
    TerrainData *terrainData,
    const  Coordinates* c1,
    const  Coordinates* c2,
    double distance,
    double samplingDistance,
    double elevationArray[]) {

    return terrainData->getElevationArray(c1, c2, distance, samplingDistance, elevationArray);
};


#endif /*TERRAIN_H*/
