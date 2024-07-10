#pragma once

// MaxLib
#include <MaxLib.h>
using namespace MaxLib; 

// Geos
#define USE_UNSTABLE_GEOS_CPP_API
// Geom
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/Location.h>
#include <geos/geom/util/LinearComponentExtracter.h>
// Operation
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/linemerge/LineMerger.h>
#include <geos/operation/polygonize/Polygonizer.h>
// Noding
#include <geos/noding/snapround/SnapRoundingNoder.h>
#include <geos/noding/snap/SnappingNoder.h>
#include <geos/noding/ValidatingNoder.h>
#include <geos/noding/NodedSegmentString.h>
// Triangulate
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>


// https://github.com/libgeos/geos
// https://github.com/vmx/geos/blob/master/doc/example.cpp (older cpp example)

 
class Geos
{ 
public:
    //using namespace geos::geom;

    // TODO: Replace any new or delete with unique ptrs
    // TODO: Select GeometryUsage should be: if inside polygon and closest to centroid, select that shape
    // TODO: TEST MULTILINNESTRING / MULTIPOLYGON 
        
    // Constructor
    Geos();
    
// ***********************************************************************
// ************************ RETRIEVE VECTOR<VEC2> ************************
 
    class Output
    {
    public:
        //  LineString / Vec2   GEOS_POINT              // a point
        //  LineString          GEOS_LINESTRING         // a linestring
        //  LineString          GEOS_LINEARRING         // a linear ring (linestring with 1st point == last point)
        //  Polygon             GEOS_POLYGON            // a polygon
        //  vector<LineString>  GEOS_MULTIPOINT         // a collection of points
        //  vector<LineString>  GEOS_MULTILINESTRING    // a collection of linestrings
        //  vector<Polygon>     GEOS_MULTIPOLYGON       // a collection of polygons
        //  GeometryCollection  GEOS_GEOMETRYCOLLECTION // a collection of heterogeneus geometries
    
        // Create LineString from geos coordinateSequence
        Geom::LineString CoordSeq(const geos::geom::CoordinateSequence* coordSeq);
        // Create LineString from geos point, lineString or linearRing
        Geom::LineString LineString(const geos::geom::Geometry* geometry);
        // Create Polygon from geos polygon
        Geom::Polygon Polygon(const geos::geom::Polygon* geometry);
        // Create LineStrings from geos multiLineString
        std::vector<Geom::LineString> LineStrings(const geos::geom::MultiLineString* multiLineString);
        // Create Polygons from geos multiPolygon
        std::vector<Geom::Polygon> Polygons(const geos::geom::MultiPolygon* multiPolygon);
        // Create LineStrings from container of geos LineStrings
        std::vector<Geom::LineString> LineStrings(const std::vector<const geos::geom::LineString*>& lineStrings);
        // Create Polygons from container of geos Polygons
        std::vector<Geom::Polygon> Polygons(const std::vector<const geos::geom::Polygon*>& polygons);
        // Returns every geometry inside geometry
        Geom::GeometryCollection Collection(geos::geom::Geometry* geometry);
        
    private:
        // Create a geometry type from a geos geometry type
        template<class T1, class T2, class T3>
        std::vector<T1> MultiGeometry(T2 multiGeometry, std::function<T1(T3)> cb)
        {
            // make a vector of linestrings to return 
            std::vector<T1> returnGeometry;
            // add each geometry to vector 
            for(size_t n = 0; n < multiGeometry->getNumGeometries(); n++){
                T3 item = multiGeometry->getGeometryN(n);
                if(!item->isEmpty()) { returnGeometry.emplace_back(std::move(cb(item)));}
            }
            // return the line strings
            return returnGeometry;
        }
        // Returns every geometry inside geometry
        void Collection(const geos::geom::Geometry* geometry, Geom::GeometryCollection& returnGeometry);
    };
    
    
    
    
// ***********************************************************************
// ************************* INPUT VECTOR<VEC2> **************************
 
    
    
    
    class Create
    { 
    public:
        // Creates a Geos Geometry from a Vector<Vec2>
        //     Point if size = 1, or
        //     Polygon if size > 3 && p0 == pLast, or
        //     Linestring if size > 1
        std::unique_ptr<geos::geom::Geometry> GeosGeometry(const Geom::LineString& points, const std::vector<Geom::LineString>& holes = {});
        // Creates a Geos Geometry from a Polygon
        std::unique_ptr<geos::geom::Geometry> GeosGeometry(const Geom::Polygon& polygon);
        // Create a geos point
        std::unique_ptr<geos::geom::Point> GeosPoint(const Geom::Vec2& point);
        // Create a geos coordSequence from a linestring
        std::unique_ptr<geos::geom::CoordinateSequence> GeosCoordSequence(const Geom::LineString& lineString);
        // Create a geos coordSequence from a polygon
        std::unique_ptr<geos::geom::CoordinateSequence> GeosCoordSequence(const Geom::Polygon& polygon);
        // Create a geos linestring
        std::unique_ptr<geos::geom::LineString> GeosLineString(const Geom::LineString& points);
        // Create a geos polygon
        std::unique_ptr<geos::geom::Polygon>  GeosPolygon(const Geom::LineString& shell, const std::vector<Geom::LineString>& holes = {});
        // Create a geos polygon
        std::unique_ptr<geos::geom::Polygon>  GeosPolygon(const Geom::Polygon& polygon);

        // Create a geos geometry collection 
        // T can be either LineString or Polygon
        template<typename T>
        std::vector<std::unique_ptr<geos::geom::Geometry>>  GeosGeometries(const std::vector<T>& geometries)
        {
            // Make a container of geos geometries
            std::vector<std::unique_ptr<geos::geom::Geometry>> geosGeometries;
            for(auto& g : geometries) {
                // Add to container if valid
                if(auto geom = GeosGeometry(g)) { 
                    geosGeometries.emplace_back(std::move(geom));
                }
            }
            // Return the geometries
            return geosGeometries;
        }
        // Create a geos geometry collection 
        // T can be either LineString or Polygon
        template<typename T>
        std::unique_ptr<geos::geom::Geometry>  GeosCollection(const std::vector<T>& geometries)
        {
            if(geometries.empty()) { return {}; }
            // we hold the geos geometry unique_ptr in a vector as we need them later
            std::vector<std::unique_ptr<geos::geom::Geometry>> geosLineStrings = GeosGeometries(geometries);
            // make geometry collection
            return m_Geos->factory->createGeometryCollection(std::move(geosLineStrings));
        }
        
    private:
        Geos* m_Geos; 
        // Constructor
        Create(Geos* geos);
        // A container of geos coordinates to a geos geometry collection 
        std::vector<geos::geom::Coordinate> GeosCoordinates(const Geom::LineString& points);
        // Create a geos linear ring
        std::unique_ptr<geos::geom::LinearRing> GeosLinearRing(const Geom::LineString& points);
        // Allow parent to construct
        friend class Geos;
    };



// ***********************************************************************
// ***************************** OPERATION *******************************
 
    class Operation
    {
    public:
        
        // Creates noded geometry
        class GeosNoder
        {
        public:    
            // Returns Nodes Geometries
            std::unique_ptr<geos::geom::Geometry> NodeValidated(std::vector<std::unique_ptr<geos::geom::Geometry>>& geometries, double snapDist = 1.0);
                
        private:
            // Pointer to parent
            Geos* m_Geos; 
            // Constructor
            GeosNoder(Geos* geos);
            // Creates SegmentStrings from LineStrings
            std::vector<geos::noding::SegmentString*> ToSegmentStrings(std::vector<const geos::geom::LineString*>& lines);
            // Creates a Linestring or MultiLineString from SegmentStrings
            std::unique_ptr<geos::geom::Geometry> ToLines(const std::vector<geos::noding::SegmentString*>* nodedList);
            // Allow parent to construct
            friend class Geos;
        };
      
        // Return geometry from polygonize
        struct PolygonizedResult
        {
            std::vector<Geom::Polygon> polygons;           
            std::vector<Geom::LineString> dangles;         
            std::vector<Geom::LineString> cutEdges;        
            std::vector<Geom::LineString> invalidRingLines;
        };
         
        // Parameters for offset
        struct OffsetParameters
        {
            typedef geos::operation::buffer::BufferParameters::EndCapStyle GeosEndCapStyle;
            GeosEndCapStyle endCapStyle = GeosEndCapStyle::CAP_ROUND;
            double arcTolerance = 0.01;
        };
        
        enum class OffsetType { BothSides, OneSide };
        // Offsets geometries of type LineString or Polygon by distance
        template<typename T>
        Geom::GeometryCollection Offset(const std::vector<T>& geometries, double distance, const OffsetParameters& parameters, OffsetType offsetType = OffsetType::OneSide)
        {
            if(geometries.empty()) { return {}; }
            // make geometry collection
            std::unique_ptr<geos::geom::Geometry> collection = m_Geos->create.GeosCollection(geometries);
            // calculate number of segments per 90deg arc from min deviation tolerance
            int nSegments = Geom::ArcSegments(distance, parameters.arcTolerance);
            
            // buffer offset operation
// TODO: std::unique_ptr<geos::operation::buffer::BufferOp> bufferOp(collection.get(), { nSegments, parameters.endCapStyle });
            auto bufferOp = std::make_unique<geos::operation::buffer::BufferOp>(collection.get(), geos::operation::buffer::BufferParameters(nSegments, parameters.endCapStyle));

            // set if offset only 1 side of geometry
            bufferOp->setSingleSided((bool)offsetType);
            // perform offset
            std::unique_ptr<geos::geom::Geometry> offset = bufferOp->getResultGeometry(distance);
            
            return m_Geos->output.Collection(offset.get());
        }
        
        
//      std::vector<MaxLib::Geom::LineString> OffsetPolygon_Recursive(const std::vector<MaxLib::Geom::LineString>& lineStrings, float pathOffset, BufferParameters& params, bool isReversed)
//      {
//          std::vector<MaxLib::Geom::LineString> returnVals;
//          MaxLib::Geom::LineString buffer;
//          // dynamic start point (finds closest point in offset)
//          std::vector<size_t> startIndex(lineStrings.size(), 0);
//          OffsetPolygon_Recursive(returnVals, lineStrings, pathOffset, buffer, startIndex, params);
//          // reverse
//          if(isReversed) {
//              for(size_t i = 0; i < returnVals.path.size(); i++) {
//                  std::reverse(returnVals.path[i].begin(), returnVals.path[i].end());
//              }
//              std::reverse(returnVals.path.begin(), returnVals.path.end());
//          }
//          return returnVals;
//      }
    
private:    
//       std::vector<size_t> DetermineStartPoints(std::vector<LineString>& OffsetLines, const LineString& l, size_t startIndex)
//       {
//           assert(OffsetLines.size());
//           assert(startIndex < l.size());
//           
//           const Vec2& p0 = l[startIndex];
//           
//           std::vector<size_t> startIndexNew;
//           
//           for(size_t n = 0; n < OffsetLines.size(); n++)
//           {   
//               size_t newIndex = 0;
//               // ensure the line falls within the current offset polygon (to prevent cutting material we wouldn't want to)
//               for(size_t i = 0; i < OffsetLines[n].size()-1; i++) // -1 because first and last point are the same
//               {
//                   Vec2& p1 = OffsetLines[n][i];
//                   if(Within({ p0, p1 }, l)) {
//                       newIndex = i;
//                       break;
//                   }
//               }
//               
//               /* FIND THE CLOSEST POINT (VERY SLOW...)
//               // reset the start position & minimum length
//               float LMin = -1.0f;
//               size_t newIndex = 0;
//               
//               for(size_t i = 0; i < OffsetLines[n].size()-1; i++) // -1 because first and last point are the same
//               {   // get next point in offset and compare its length to prev
//                   Vec2& p1 = OffsetLines[n][i];                
//                   Vec2 dif = p1 - p0;
//                   // compare length
//                   float L = hypotf(dif.x, dif.y);
//                   if(L < LMin || LMin == -1.0f) {
//                       if(LineIsInsidePolygon(p0, p1, l)) {
//                           newIndex = i;
//                           LMin = L;
//                       }
//                   }
//               }
//               * */
//               startIndexNew.push_back(newIndex);
//           }
//           return move(startIndexNew);
//       }
//
//   // TODO: Rewrite OffsetPolygon_Recursive()
//   //  We need to offset n*offset from the original path, otherwise the number of lines in an arc increase n^2
//   //      OffsetLines = OffsetPolygon(ORIGINAL PATH, n * pathOffset, params);
//   // This will also allow us to do adaptive arc segments
//
//   // adds linestring and subsequent offsets into returnPoints
//   // if offset > 0 we recursively make offsets until offset fails
//   void OffsetPolygon_Recursive(std::vector<MaxLib::Geom::LineString>& returnVals, const std::vector<MaxLib::Geom::LineString>& lineStrings, float pathOffset, MaxLib::Geom::LineString& buffer, std::vector<size_t>& startIndex, BufferParameters& params)
//   {    
//       // We recursively offset each linestring in linestrings 
//       // We take a set of linestrings because when we do an offset, a single linestring can become multiple linestrings
//       // sanity check
//       assert(lineStrings.size() == startIndex.size() && "Start index size isnt equal to linestring size");
//       
//       auto endOffsetGroup = [&]() {
//           returnVals.emplace_back(std::move(buffer));
//           buffer = MaxLib::Geom::LineString(); // make new buffer
//       };
//       
//
//       for(size_t n = 0; n < lineStrings.size(); n++) {
//           const MaxLib::Geom::LineString& l = lineStrings[n];
//           // check there is 2 or more points
//           if(l.size() < 2) { continue; }
//           
//           // sanity checks
//           assert(l.front() == l.back() && "Linestring start and end points do not match");
//           assert(startIndex[n] < l.size() && "Start index isnt within linestring");
//           
//           // add input linestring to buffer
//           for(size_t i = 0; i < l.size()-1; i++) { // -1 because first and last point are the same
//               size_t index = (int)(i + startIndex[n]) % (l.size()-1);
//               assert(index < l.size() && "Index out of range");
//               buffer.push_back(l[index]);
//           }
//           // make first and last point the same
//           buffer.push_back(l[startIndex[n]]);
//           
//           // break if this is just a single lineloop
//           if(pathOffset == 0.0f) { 
//               endOffsetGroup();
//               continue; 
//           } 
//           // recursively offset the lineString to bore out
//           std::vector<MaxLib::Geom::LineString> OffsetLines = OffsetPolygon(l, pathOffset, params);
//           // if no more offsets, add centroid point as last position
//           if(!OffsetLines.size()) {
//               if(auto centroid = Centroid(l)) { 
//                   buffer.push_back(*centroid); 
//               }
//               endOffsetGroup();
//               continue;
//           }
//           // we have finished a set of offsets, add buffer to return vector
//           if(OffsetLines.size() > lineStrings.size()) {
//               endOffsetGroup();
//           }
//           // check for the closest point and make this the new start point
//           std::vector<size_t> nextStartIndex = DetermineStartPoints(OffsetLines, l, startIndex[n]);
//           // repeat function
//           OffsetPolygon_Recursive(returnVals, OffsetLines, pathOffset, buffer, nextStartIndex, params);
//       }
//   } 
        
public:
        
        
        
        
        
        // Triangulates geometries of type LineString or Polygon
        template<typename T>
        Geom::GeometryCollection Triangulation(std::vector<T>& geometries, double tolerance = 0.0)
        {
            // Create Geos geometry
            std::unique_ptr<geos::geom::Geometry> geosCollection = m_Geos->create.GeosCollection(geometries);
            
            // Setup builder
            geos::triangulate::DelaunayTriangulationBuilder builder;
            builder.setTolerance(tolerance);
            builder.setSites(*geosCollection);
            // Get triangles
            std::unique_ptr<geos::geom::Geometry> results = builder.getTriangles(*m_Geos->factory); // results = builder.getEdges(geomFact);
            results->normalize();
            
            return m_Geos->output.Collection(results.get());
        }
        
        // Type can be either a LineString or Polygon
        template<typename T>
        PolygonizedResult Polygonize(const std::vector<T>& geometries)
        {
            // Create geos geometries
            std::vector<std::unique_ptr<geos::geom::Geometry>> lineStrings = m_Geos->create.GeosGeometries(geometries);
            // Node the geos geometries
            std::unique_ptr<geos::geom::Geometry> nodedLineStrings = m_Geos->operation.noder.NodeValidated(lineStrings); // TODO: Should be able to take any geometry
              
            // Polygonize the geometry
            geos::operation::polygonize::Polygonizer polygonizer;
            polygonizer.add(nodedLineStrings.get());
            // Build the data structure to return
            PolygonizedResult data;                      
            // Polygons needs to stay alive until we copy the points out, the others are references to geometry so dont need to
            std::vector<std::unique_ptr<geos::geom::Polygon>> polygons = polygonizer.getPolygons();  
            // convert geometries to Geom::Polygon
            const std::vector<const geos::geom::Polygon*> polygonsPtrs = MaxLib::Vector::VectorCopy<std::unique_ptr<geos::geom::Polygon>, const geos::geom::Polygon*>(polygons, [](const std::unique_ptr<geos::geom::Polygon>& from) { return from.get(); });
            data.polygons            = m_Geos->output.Polygons(polygonsPtrs);
            data.dangles             = m_Geos->output.LineStrings(polygonizer.getDangles());
            data.cutEdges            = m_Geos->output.LineStrings(polygonizer.getCutEdges());            
            // convert geometries to Geom::LineStrings
            const std::vector<const geos::geom::LineString*> invalidRingLinesPtrs = MaxLib::Vector::VectorCopy<std::unique_ptr<geos::geom::LineString>, const geos::geom::LineString*>(polygonizer.getInvalidRingLines(), [](const std::unique_ptr<geos::geom::LineString>& from) { return from.get(); });
            data.invalidRingLines    = m_Geos->output.LineStrings(invalidRingLinesPtrs);
            return data;
        }
    
        // Combines adjacent / overlapping geometries and then simplifies lines within tolerance
        std::vector<Geom::Polygon> Combine(const std::vector<Geom::Polygon>& polygons);
              
        // Combines and simplifies all the linestrings in geom
        std::vector<Geom::LineString> Combine(const std::vector<Geom::LineString>& lineString);
        
        // Operation Functions
        //   T can be either a LineString or Polygon, but order is critical
        //   Example: bool a = geos.operation.Within(linestring, polygon); // is linestring within polygon?
        
        // Centroid
        template<typename T>
        Geom::Vec2 Centroid(const T& geom) {
            return GeneralOp<Geom::Vec2, T>(geom, [](geos::geom::Geometry* g) { auto c = g->getCentroid(); return Geom::Vec2(c->getX(), c->getY()); });
        }
        // Disjoint
        template<typename T1, typename T2>
        bool Disjoint(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->disjoint(g2); });
        }
        // Touches
        template<typename T1, typename T2>
        bool Touches(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->touches(g2); });
        }
        // Intersects
        template<typename T1, typename T2>
        bool Intersects(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->intersects(g2); });
        }
        // Crosses
        template<typename T1, typename T2>
        bool Crosses(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->crosses(g2); });
        }
        // Within
        template<typename T1, typename T2>
        bool Within(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->within(g2); });
        }
        // Contains
        template<typename T1, typename T2>
        bool Contains(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->contains(g2); });
        }
        // Overlaps
        template<typename T1, typename T2> 
        bool Overlaps(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->overlaps(g2); });
        }
        // Equals
        template<typename T1, typename T2>
        bool Equals(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->equals(g2); });
        }
        // Covers
        template<typename T1, typename T2>
        bool Covers(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->covers(g2); });
        }
        // CoveredBy
        template<typename T1, typename T2>
        bool CoveredBy(const T1& geom1, const T2& geom2) {
            return GeneralOp<bool , T1, T2>(geom1, geom2, [](geos::geom::Geometry* g1, geos::geom::Geometry* g2) { return g1->coveredBy(g2); });
        }
        
        // General purpose Operation functions which convert the input geometry into geos geometry 
        // and passes them into a callback, allowing the user to use them with a geos function
        // TOutput is the type returned from callback
        // TGeom can be either LineString or Polygon
        
        template<typename TOutput, typename TGeom>
        TOutput GeneralOp(const TGeom& geom, std::function<TOutput(geos::geom::Geometry*)> cb) 
        {
            auto g = m_Geos->create.GeosGeometry(geom);
            assert(g && "Geos Geometry couldn't be created");
            return cb(g.get());
        }
        
        template<typename TOutput, typename TGeom1, typename TGeom2>
        TOutput GeneralOp(const TGeom1& geom1, const TGeom2& geom2, std::function<TOutput(geos::geom::Geometry*, geos::geom::Geometry*)> cb) 
        {   
            auto g1 = m_Geos->create.GeosGeometry(geom1);
            auto g2 = m_Geos->create.GeosGeometry(geom2);
            assert((g1 && g2) && "Geos Geometry couldn't be created");
            return cb(g1.get(), g2.get());
        }
        
    private:
        // Constructor 
        Operation(Geos* geos);
        // Pointer to parent
        Geos* m_Geos; 
        // Create Noder Instance
        GeosNoder noder;
        // Allow parent to construct
        friend class Geos;
    };
    
    Output output;
    Create create;
    Operation operation;

    // Used when simplifying geometry
    double geometryTolerance = 0.001;
    
private:
    geos::geom::GeometryFactory::Ptr factory;
    friend class Create;
    friend class Operation;
}; 


