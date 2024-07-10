#include "Geos.h"
  
typedef Geos::Output                 Output;
typedef Geos::Create                 Create;
typedef Geos::Operation              Operation;
typedef Geos::Operation::GeosNoder   GeosNoder;

//    #include <geos/io/WKTReader.h> 
//
//    geos::io::WKTReader wktreader;
//
//    std::vector<std::string> inputWKT {
//        "LINESTRING (100 180, 20 20, 160 20, 100 180)",
//        "LINESTRING (100 180, 80 60, 120 60, 100 180)",
//    };
//
//    std::vector<std::unique_ptr<geos::geom::Geometry>> inputGeoms;
//
//    for (const auto& wkt : inputWKT) {
//        inputGeoms.push_back(wktreader.read(wkt));
//    }
//
 

//  ****************************************************************************
//  *********************************** GEOS ***********************************

// constructor
Geos::Geos()
    : create(this), operation(this), factory(geos::geom::GeometryFactory::create())
{}
 

//  ****************************************************************************
//  ********************************** OUTPUT **********************************

Geom::LineString Output::CoordSeq(const geos::geom::CoordinateSequence* coordSeq)
{
    // Array to return
    Geom::LineString returnCoords;
    // Copy coords to return array
    for(size_t i = 0; i < coordSeq->getSize(); i++) {
        const Coordinate& coord = (*coordSeq)[i];
        returnCoords.emplace_back(coord.x, coord.y);
    }
    // Return the array
    return returnCoords;
}

// May be either geos::geom::Point, LineString or LinearRing
Geom::LineString Output::LineString(const geos::geom::Geometry* geometry)
{
    // Check there is some geometry
    if(geometry->isEmpty()) { return {}; }
    // Ensure Geometry is a point, linestring or linear ring as we are passing a generic Geometry type
    geos::geom::GeometryTypeId type = geometry->getGeometryTypeId();
    if(type != geos::geom::GEOS_POINT && type != geos::geom::GEOS_LINESTRING && type != geos::geom::GEOS_LINEARRING) { return {}; }
    // Linestring to return
    return CoordSeq(geometry->getCoordinates().get());
}
    
Geom::Polygon Output::Polygon(const geos::geom::Polygon* geometry)
{
    // Check there is some geometry
    if(geometry->isEmpty()) { return {}; }
    // Polygon to return
    Geom::Polygon polygon;
    
    // Get the exterior ring (shell)
    const geos::geom::LinearRing* shell = geometry->getExteriorRing();
    polygon.shell = CoordSeq(shell->getCoordinates().get());
    
    // Get number of interior rings (hole)
    std::size_t nRings = geometry->getNumInteriorRing();
    // For each interior ring
    for(size_t i = 0; i < nRings; i++) {
        // Get interior ring
        const geos::geom::LinearRing* hole = geometry->getInteriorRingN(i);
        polygon.holes.emplace_back(CoordSeq(hole->getCoordinates().get()));
    }
    // Return the polygon
    return polygon;
}

std::vector<Geom::LineString> Output::LineStrings(const geos::geom::MultiLineString* multiLineString)
{ 
    return MultiGeometry<Geom::LineString, const geos::geom::MultiLineString*, const geos::geom::LineString*>(multiLineString, [&](const geos::geom::LineString* item) {
        return std::move(LineString(item));
    });
}

std::vector<Geom::Polygon> Output::Polygons(const geos::geom::MultiPolygon* multiPolygon)
{
    return MultiGeometry<Geom::Polygon, const geos::geom::MultiPolygon*, const geos::geom::Polygon*>(multiPolygon, [&](const geos::geom::Polygon* item) {
        return Polygon(item);
    });
}

std::vector<Geom::LineString> Output::LineStrings(const std::vector<const geos::geom::LineString*>& lineStrings)
{
    // make a vector of linestrings / polygons to return 
    std::vector<Geom::LineString> returnGeometry;
    // add each geometry to vector 
    for(auto item : lineStrings) { 
        if(!item->isEmpty()) { returnGeometry.emplace_back(std::move(LineString(item))); } 
    }
    // return the line strings / polygon
    return returnGeometry;
}

std::vector<Geom::Polygon> Output::Polygons(const std::vector<const geos::geom::Polygon*>& polygons)
{
    // make a vector of linestrings / polygons to return 
    std::vector<Geom::Polygon> returnGeometry;
    // add each geometry to vector 
    for(auto item : polygons) { 
        if(!item->isEmpty()) { 
            returnGeometry.emplace_back(std::move(Polygon(item)));
        } 
    }
    // return the line strings / polygon
    return returnGeometry;
}
// Returns every geometry inside geometry
Geom::GeometryCollection Output::Collection(geos::geom::Geometry* geometry)
{
    Geom::GeometryCollection returnGeometry;
    // will recursively run for each container in collection
    Collection(geometry, returnGeometry);
    return returnGeometry;
}

void Output::Collection(const geos::geom::Geometry* geometry, Geom::GeometryCollection& returnGeometry)
{
    // Get geometry type
    geos::geom::GeometryTypeId type = geometry->getGeometryTypeId();
    // a point, linestring or linearring can be added to linestrings
    if(type == geos::geom::GEOS_POINT) {
        returnGeometry.points.emplace_back(geometry->getCoordinate()->x, geometry->getCoordinate()->y);
    }
    else if((type == geos::geom::GEOS_LINESTRING) || (type == geos::geom::GEOS_LINEARRING)) {
        returnGeometry.lineStrings.emplace_back(std::move(LineString(geometry)));
    }
    // a polygon can be added to polygons
    else if(type == geos::geom::GEOS_POLYGON) {
        returnGeometry.polygons.emplace_back(std::move(Polygon(static_cast<const geos::geom::Polygon*>(geometry))));
    }
    // a collection calls this function again on each geometry it contains
    else if((type == geos::geom::GEOS_MULTIPOINT) || (type == geos::geom::GEOS_MULTILINESTRING) || (type == geos::geom::GEOS_MULTIPOLYGON) || (type == geos::geom::GEOS_GEOMETRYCOLLECTION)) {
        for(size_t i = 0; i < geometry->getNumGeometries(); i++) {
            Collection(geometry->getGeometryN(i), returnGeometry);
        }
    }
}


//  ****************************************************************************
//  ********************************** CREATE **********************************

Create::Create(Geos* geos) : m_Geos(geos) {}
   
// Creates a Geos Geometry from a Vector<Vec2>
//     Point if size = 1, or
//     Polygon if size > 3 && p0 == pLast, or
//     Linestring if size > 1
std::unique_ptr<geos::geom::Geometry> Create::GeosGeometry(const Geom::LineString& points, const std::vector<Geom::LineString>& holes) 
{
    // Point is Empty
    if(points.empty()) { return {}; }
    // Create Point
    else if(points.size() == 1) { 
        return GeosPoint(points[0]);
    } 
    // Create Polygon
    else if((points.size() >= 3) && (points.front() == points.back())) { 
        return GeosPolygon(points, holes); 
    }
    // Create LineString
    else { // size >= 2
        return GeosLineString(points); 
    }
}

std::unique_ptr<geos::geom::Geometry> Create::GeosGeometry(const Geom::Polygon& polygon) 
{
    return GeosGeometry(polygon.shell, polygon.holes);
}

// Create a geos point unique_ptr
std::unique_ptr<geos::geom::Point> Create::GeosPoint(const Geom::Vec2& point) 
{
    // create coords 
    geos::geom::Coordinate coord = { point.x, point.y };
    // create linear ring
    return m_Geos->factory->createPoint(coord);
}

// Create a geos coordSequence unique_ptr from a linestring
std::unique_ptr<geos::geom::CoordinateSequence> Create::GeosCoordSequence(const Geom::LineString& lineString)
{
    // create coord sequence
    std::unique_ptr<geos::geom::CoordinateSequence> coordSeq = std::make_unique<geos::geom::CoordinateSequence>();
    // set points
    coordSeq->setPoints(GeosCoordinates(lineString));
    return coordSeq;
}

// Create a geos coordSequence unique_ptr from a polygon
std::unique_ptr<geos::geom::CoordinateSequence> Create::GeosCoordSequence(const Geom::Polygon& polygon)
{
    // create coord sequence
    std::unique_ptr<geos::geom::CoordinateSequence> coordSeq = std::make_unique<geos::geom::CoordinateSequence>();
    // Add all point in shell
    for(auto& coord : GeosCoordinates(polygon.shell)) {
        coordSeq->add(coord);
    }
    // Add all points in each hole
    for(auto& hole : polygon.holes) {
        for(auto& coord : GeosCoordinates(hole)) {
            coordSeq->add(coord);
        }
    }
    return coordSeq;
}
    
std::unique_ptr<geos::geom::LineString> Create::GeosLineString(const Geom::LineString& points) 
{ 
    if(points.size() < 2) { return {}; }
    // create coord sequence
    std::unique_ptr<geos::geom::CoordinateSequence> coordSeq = GeosCoordSequence(points);
    // create linear ring
    return m_Geos->factory->createLineString(std::move(coordSeq));
}

std::unique_ptr<geos::geom::Polygon> Create::GeosPolygon(const Geom::LineString& shell, const std::vector<Geom::LineString>& holes)
{
    if(shell.size() < 3 || (shell.front() != shell.back())) { return {}; }
    // Create Geos Shell
    std::unique_ptr<geos::geom::LinearRing> geosShell = GeosLinearRing(shell);
    // Create Geos Holes
    std::vector<std::unique_ptr<geos::geom::LinearRing>> geosHoles;
    for(const Geom::LineString& hole : holes) {
        if(hole.size() < 3 || (hole.front() != hole.back())) { continue; }
        geosHoles.emplace_back(GeosLinearRing(hole));
    }
    // Create Geos Polygon
    return m_Geos->factory->createPolygon(std::move(geosShell), std::move(geosHoles));
}

std::unique_ptr<geos::geom::Polygon> Create::GeosPolygon(const Geom::Polygon& polygon)
{
    return GeosPolygon(polygon.shell, polygon.holes);
}

std::vector<geos::geom::Coordinate> Create::GeosCoordinates(const Geom::LineString& points) 
{   // make array of coordinates from Vec2
    std::vector<Coordinate> coords;
    for(const Geom::Vec2& p : points) {
        coords.emplace_back(p.x, p.y);
    }
    return coords;
}

std::unique_ptr<geos::geom::LinearRing> Create::GeosLinearRing(const Geom::LineString& points) 
{
    // create coord sequence
    std::unique_ptr<geos::geom::CoordinateSequence> coordSeq = GeosCoordSequence(points);
    // create linear ring
    return m_Geos->factory->createLinearRing(std::move(coordSeq));
}


//  ****************************************************************************
//  ********************************* OPERATION ********************************

// Constructor 
Operation::Operation(Geos* geos) 
    : m_Geos(geos), noder(GeosNoder(m_Geos)) 
{}

// Combines adjacent / overlapping geometries and then simplifies lines within tolerance
std::vector<Geom::Polygon> Operation::Combine(const std::vector<Geom::Polygon>& polygons)
{
    if(polygons.empty()) { return {}; }
    // get the first polgon
    auto geom_A = m_Geos->create.GeosGeometry(polygons[0]);
    if(!geom_A) return {};
    // combine all the geoms together
    for(size_t i = 1; i < polygons.size(); i++) {
        auto geom_B = m_Geos->create.GeosGeometry(polygons[i]);
        if(!geom_B) return {};
        // union geometries
        auto result = geom_A->Union(geom_B.get());
        geom_A = std::move(result);
    }
    
    // Simplify TODO: DO WE WANT THIS???
    //std::unique_ptr<geos::geom::Geometry> simplified =  geos::simplify::DouglasPeuckerSimplifier::simplify(*(geom_A->getCoordinates()), geometryTolerance);
    if (geom_A->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
        return {{ m_Geos->output.Polygon(static_cast<geos::geom::Polygon*>(geom_A.get())) }};
    } else if(geom_A->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON) {
        return m_Geos->output.Polygons(static_cast<geos::geom::MultiPolygon*>(geom_A.get()));
    }
    assert(0 && "Combined geometry is not a polygon or multipolygon");
}

      
// Combines and simplifies all the linestrings in geom
std::vector<Geom::LineString> Operation::Combine(const std::vector<Geom::LineString>& lineString)
{
    if(lineString.empty()) { return {}; }

    // we hold the geos geometry unique_ptr in a vector as we need them until getMergedLineStrings is called
    std::vector<std::unique_ptr<geos::geom::Geometry>> geosLineStrings = m_Geos->create.GeosGeometries(lineString);
    // this merges our geos linestrings
    geos::operation::linemerge::LineMerger lineMerger;
    // make a vector of geos geometry from geom and add them to the line merger
    for(auto& l : geosLineStrings) { lineMerger.add(l.get()); }
    // merge the lines in line merger
    std::vector<std::unique_ptr<geos::geom::LineString>> merged = lineMerger.getMergedLineStrings();
    // make a vector of pointers to the merged geometry
    const std::vector<const geos::geom::LineString*> mergedPtrs = MaxLib::Vector::VectorCopy<std::unique_ptr<geos::geom::LineString>, const geos::geom::LineString*>(merged, [](std::unique_ptr<geos::geom::LineString>& from) {
        return from.get();
    });
    // Simplify  TODO: DO WE WANT THIS???
    //std::unique_ptr<geos::geom::Geometry> simplified =  geos::simplify::DouglasPeuckerSimplifier::simplify(*(geom_A->getCoordinates()), geometryTolerance);
    
    // return the linestrings
    return m_Geos->output.LineStrings(mergedPtrs);
}


//  ****************************************************************************
//  ******************************** GEOS NODER ********************************
    
// Constructor
GeosNoder::GeosNoder(Geos* geos) 
    : m_Geos(geos) 
{}
// Returns Nodes Geometries
std::unique_ptr<geos::geom::Geometry> GeosNoder::NodeValidated(std::vector<std::unique_ptr<geos::geom::Geometry>>& geometries, double snapDist)
{
    // Make a container of Linear Components from each of geometries
    std::vector<const geos::geom::LineString*> lines;
    for(auto& g : geometries ) {
        geos::geom::util::LinearComponentExtracter::getLines(*g.get(), lines);
    }
    // ssList needs to be disposed after noder is done working
    std::vector<geos::noding::SegmentString*> ssList = ToSegmentStrings(lines);
    
    geos::noding::snap::SnappingNoder snapNoder(snapDist);
   // Noder* noder = &snapNoder;
    geos::noding::ValidatingNoder noderValid(snapNoder);
    // computeNotes might alter ssList, but ssList still
    // holds all memory
    noderValid.computeNodes(&ssList);

    // getNodedSubstrings calls NodedSegmentString::getNodedSubStrings()
    // which creates new NodedSegmentString and new pts member, so complete
    // new copy of data. Can be disposed of after geometries are constructed
    // std::vector<SegmentString*>* nodedList = noderValid.getNodedSubstrings();
    std::vector<geos::noding::SegmentString*>* nodedList = noderValid.getNodedSubstrings();

    // Dispose of ssList
    for (auto ss: ssList) {
        delete ss;
    }

    std::unique_ptr<geos::geom::Geometry> lineGeom = ToLines(nodedList);

    // Dispose of nodedList
    for (auto nss: *nodedList) {
        delete nss;
    }
    delete nodedList;

    return lineGeom;
}
    
// Creates SegmentStrings from LineStrings
std::vector<geos::noding::SegmentString*> GeosNoder::ToSegmentStrings(std::vector<const geos::geom::LineString*>& lines)
{
    std::vector<geos::noding::SegmentString*> nssList;
    for (auto line : lines) {
        // line->getCoordinates() clones CoordinateSequence into a unique_ptr<> which we
        // have to release() to the NodedSegmentString constructor, so nss now owns nss->pts
        geos::noding::NodedSegmentString* nss = new geos::noding::NodedSegmentString(line->getCoordinates().release(), line);
        nssList.push_back(nss);
    }
    return nssList;
}

// Creates a Linestring or MultiLineString from SegmentStrings
std::unique_ptr<geos::geom::Geometry> GeosNoder::ToLines(const std::vector<geos::noding::SegmentString*>* nodedList)
{
    std::vector<std::unique_ptr<geos::geom::Geometry>> lines;

    for (auto nss : *nodedList) {
      geos::geom::CoordinateSequence* pts = nss->getCoordinates();
      // pts is owned by nss, so we make a copy to build the line on top of. Lines are 100% 
      // self-contained and own all their parts. Input nodedList can be freed.
      lines.emplace_back(m_Geos->factory->createLineString(pts->clone()));
    }
    if (lines.size() == 1) return std::move(lines[0]);

    // move the lines to pass ownership to the multiLineString
    return m_Geos->factory->createMultiLineString(std::move(lines));
}


