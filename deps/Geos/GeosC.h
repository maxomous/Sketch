
#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <optional>
#include <functional>
#include <algorithm>
	
#include <geos_c.h>  
#include <MaxLib.h>	

// delete this, for debugging only
//static void printMaxLib::Geom::Vec2(const std::string& text, const MaxLib::Geom::MaxLib::Geom::Vec2& v) {
//    std::cout << text << ": (" << v.x << ", " << v.y << ")" << std::endl;
//}


// original is here: geos::operation::buffer::BufferParameters
// \see GEOSBufferParams_create()
// \see GEOSBufferParams_destroy()

//struct GeosBufferParams {
//    int QuadrantSegments    = 30; // # lines segments in 90 degree arc
//    int CapStyle            = GEOSBufCapStyles::GEOSBUF_CAP_ROUND;
//    int JoinStyle           = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND;
//    double MitreLimit       = 1.0;
//};

//typedef struct GEOSBufParams_t GEOSBufferParams;
const double MIN_GEOMETRY_TOLERANCE = 0.01;



class GeosC {
public:   
    
    struct BufferParameters {
        double arcTolerance     = MIN_GEOMETRY_TOLERANCE; // # lines segments in 90 degree arc
        int capStyle            = GEOSBufCapStyles::GEOSBUF_CAP_ROUND;
        int joinStyle           = GEOSBufJoinStyles::GEOSBUF_JOIN_ROUND;
        double mitreLimit       = 1.0;
    };
    
    GeosC();
    ~GeosC();
    // returns centroid of a polygon
    std::optional<MaxLib::Geom::Vec2> Centroid(const MaxLib::Geom::LineString& points);
    
    // converts a vector<MaxLib::Geom::Vec2> to a Geos Point, MaxLib::Geom::LineString or Polygon, defined by the number of points it contains
    GEOSGeometry* GeosPointLineStringOrPolygon(const MaxLib::Geom::LineString& points) {
        // err if no points
        if(points.size() == 0) { return {}; }
        // input is point
        if(points.size() == 1) { return GEOSGeom_createPointFromXY(points[0].x, points[0].y); }
        // return polygon if loop
        if(points.front() == points.back()) { return GeosPolygon(points); }
        // return linestring
        return GeosLineString(points);
    }
     
 
    // For spatial operator definitions, see: (http://docs.safe.com/fme/html/FME_Desktop_Documentation/FME_Transformers/Transformers/spatialrelations.htm#DE9IM_Matrix)
    
    // True if no point of either geometry touchess or is within the other.
    std::optional<bool> Disjoint(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSDisjoint(geom_A, geom_B); });
    }

    // True if geometries share boundaries at one or more points, but do not have interior overlaps.
    std::optional<bool> Touches(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSTouches(geom_A, geom_B); });
    }

    // True if geometries are not disjoint.
    std::optional<bool> Intersects(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSIntersects(geom_A, geom_B); });
    }

    // True if geometries interiors interact but their boundares do not. Most useful for finding line crosses cases.
    std::optional<bool> Crosses(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSCrosses(geom_A, geom_B); });
    }

    // True if geometry g1 is completely within g2, and not touching the boundary of g2.
    std::optional<bool> Within(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSWithin(geom_A, geom_B); });
    }

    // True if geometry g2 is completely within g1.
    std::optional<bool> Contains(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSContains(geom_A, geom_B); });
    }

    // True if geometries share interiors but are neither within nor contained   
    std::optional<bool> Overlaps(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSOverlaps(geom_A, geom_B); });
    }

    // True if geometries cover the same space on the place.
    std::optional<bool> Equals(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSEquals(geom_A, geom_B); });
    }

    // True if geometry g1 is completely within g2, including possibly touching the boundary of g2.
    std::optional<bool> Covers(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B){ return GEOSCovers(geom_A, geom_B); });
    }
    
    // True if geometry g2 is completely within g1, including possibly touching the boundary of g1.
    std::optional<bool> CoveredBy(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB) 
    {
        return BooleanOperation(geomA, geomB, [](GEOSGeometry* geom_A, GEOSGeometry* geom_B) {return GEOSCoveredBy(geom_A, geom_B);
        });
    }
    
          
    // Combines and simplifies all the geometries in geom
    std::vector<MaxLib::Geom::Geometry> CombinePolygons(const std::vector<MaxLib::Geom::Geometry>& geom)
    {
        if(geom.empty()) { return {}; }
        
        GEOSGeometry* geom_A = GeosPointLineStringOrPolygon(geom[0]);
        if(!geom_A) return {};
        
        for(size_t i = 1; i < geom.size(); i++)
        {
            GEOSGeometry* geom_B = GeosPointLineStringOrPolygon(geom[i]);
            if(!geom_B) return {};
            
            GEOSGeometry* result = GEOSUnion(geom_A, geom_B);
            
            GEOSGeom_destroy(geom_A);
            GEOSGeom_destroy(geom_B);
            geom_A = result;
        }
        GEOSGeometry* simplified = GEOSSimplify(geom_A, MIN_GEOMETRY_TOLERANCE);
        std::vector<MaxLib::Geom::Geometry> output = GeosGetGeometry(simplified);
        GEOSGeom_destroy(simplified);
        GEOSGeom_destroy(geom_A);
        return std::move(output);
    }
    
    
          
    // Combines and simplifies all the linestrings in geom
    std::vector<MaxLib::Geom::Geometry> CombineLineStrings(const std::vector<MaxLib::Geom::Geometry>& geom)
    {
        if(geom.empty()) { return {}; }
    
        std::vector<GEOSGeometry*> lineStrings;
    
        // Define geos linestrings
        for(size_t i = 0; i < geom.size(); i++) { 
            lineStrings.push_back(GeosC::GeosLineString(geom[i]));
            if(!lineStrings.back()) return {};
        }
        // make a geos geometry collection
        GEOSGeometry* collection = GEOSGeom_createCollection(GEOS_MULTILINESTRING, lineStrings.data(), lineStrings.size());
        // merge the linestrings combining share points
        GEOSGeometry* merged = GEOSLineMerge(collection);
        // convert geos geometry to vector<linestring>
        std::vector<MaxLib::Geom::Geometry> output = GeosGetGeometry(merged);
        
        // Free old geos geometry (linestrings are all passed over to collection)
        GEOSGeom_destroy(collection);
        GEOSGeom_destroy(merged);
        // return output
        return std::move(output);
    }
    
  // 
  // std::vector<Geometry> CombinePolygons(const std::vector<Geometry>& geom)
  // {
  //     if(geom.empty()) { return {}; }
  //     
  //     GEOSGeometry* geom_A = GeosPolygon(geom[0]);
  //     if(!geom_A) return {};
  //     
  //     
  //     GEOSDifference();
  //     
  //     
  //     GEOSConstrainedDelaunayTriangulation;
  //     
  // }   
  // 
    
    struct PolygonizedResult
    {
        std::vector<MaxLib::Geom::LineString> valid;
        std::vector<MaxLib::Geom::LineString> cuts;
        std::vector<MaxLib::Geom::LineString> dangles;
        std::vector<MaxLib::Geom::LineString> invalid;
    };
    
    
    // Converts a vector of linestrings into polygons 
    // (calculates nodes at intersections between lines and generates new polygons using these nodes)
    // usage should be:
    //  if inside polygon and closest to centroid, select that shape
    // Geometry can be vector of Linestring or Polyons
    PolygonizedResult Polygonize(const std::vector<MaxLib::Geom::Geometry>& input)
    {   
        std::vector<GEOSGeometry*> lineStrings;
        // Make geos line string for each input linestring
        for(const MaxLib::Geom::LineString& linestring : input) {
            if(linestring.size() >= 2) {                
                lineStrings.push_back(GeosLineString(linestring));
            }
        }
        /* takes ownership of the points in the array */
        /* but not the array itself */
        GEOSGeometry* collection = GEOSGeom_createCollection(GEOS_MULTILINESTRING, lineStrings.data(), lineStrings.size());

        // Create nodes at intersections
        GEOSGeometry* nodedLines = GEOSNode(collection);
        if(!nodedLines) return {};   
     
        // Type: Geometry Collection
        GEOSGeometry *cuts, *dangles, *invalid; 
    
        // Polygonize (convert the noded linestring into polygons)
        GEOSGeometry* polygons = GEOSPolygonize_full(nodedLines, &cuts, &dangles, &invalid);
        if(!polygons) return {}; 
        
        
        PolygonizedResult output;
        output.valid = GeosGetPolygons(polygons);
        output.cuts = GeosGetLineStrings(cuts);
        output.dangles = GeosGetLineStrings(dangles);
        output.invalid = GeosGetPolygons(invalid);
        
        // Free memory        

        /* frees collection and contained points */
        GEOSGeom_destroy(collection);
        
 //       GEOSGeom_destroy(nodedLines);
        GEOSGeom_destroy(cuts);
        GEOSGeom_destroy(dangles);
        GEOSGeom_destroy(invalid);
        GEOSGeom_destroy(polygons);
       
        // return polygons
        return std::move(output);        
    }

    // returns offset of a line or polygon. result may be multiple linestrings
    // offset is negative for right side offset / positive for left side
    // determines whether points are a line or polygon by if the first and last point is the same 
    std::vector<MaxLib::Geom::LineString> Offset(const MaxLib::Geom::LineString& points, float offset, BufferParameters& params);
    std::vector<MaxLib::Geom::LineString> OffsetLine(const MaxLib::Geom::LineString& points, float offset, BufferParameters& params);
    std::vector<MaxLib::Geom::LineString> OffsetPolygon(const MaxLib::Geom::LineString& points, float offset, BufferParameters& params);
    
    struct RecursiveOffset {
        std::vector<MaxLib::Geom::LineString> path;
        std::vector<MaxLib::Geom::LineString> enclosingPath;        
    };
    
    RecursiveOffset OffsetPolygon_Recursive(const std::vector<MaxLib::Geom::LineString>& lineStrings, float pathOffset, bool isReversed, BufferParameters& params)
    {
        RecursiveOffset returnVals;
        MaxLib::Geom::LineString buffer;
        // dynamic start point (finds closest point in offset)
        std::vector<size_t> startIndex(lineStrings.size(), 0);
        OffsetPolygon_Recursive(returnVals, lineStrings, pathOffset, buffer, startIndex, true, params);
        // reverse
        if(isReversed) {
            for(size_t i = 0; i < returnVals.path.size(); i++) {
                std::reverse(returnVals.path[i].begin(), returnVals.path[i].end());
            }
            for(size_t i = 0; i < returnVals.enclosingPath.size(); i++) {
                std::reverse(returnVals.enclosingPath[i].begin(), returnVals.enclosingPath[i].end());
            }
            std::reverse(returnVals.path.begin(), returnVals.path.end());
            std::reverse(returnVals.enclosingPath.begin(), returnVals.enclosingPath.end());
        }
        return returnVals;
    }
    
private:    

    // TODO: Rewrite OffsetPolygon_Recursive()
    //  We need to offset n*offset from the original path, otherwise the number of lines in an arc increase n^2
    //      OffsetLines = OffsetPolygon(ORIGINAL PATH, n * pathOffset, params);
    // This will also allow us to do adaptive arc segments

    // adds linestring and subsequent offsets into returnPoints
    // if offset > 0 we recursively make offsets until offset fails
    void OffsetPolygon_Recursive(RecursiveOffset& returnVals, const std::vector<MaxLib::Geom::LineString>& lineStrings, float pathOffset, MaxLib::Geom::LineString& buffer, std::vector<size_t>& startIndex, bool isEnclosingPath, BufferParameters& params)
    {    
        // We recursively offset each linestring in linestrings 
        // We take a set of linestrings because when we do an offset, a single linestring can become multiple linestrings
        // sanity check
        assert(lineStrings.size() == startIndex.size() && "Start index size isnt equal to linestring size");
        
        auto endOffsetGroup = [&]() {
            returnVals.path.push_back(buffer);
            buffer.clear();
        };
        

        for(size_t n = 0; n < lineStrings.size(); n++) {
            const MaxLib::Geom::LineString& l = lineStrings[n];
            // check there is 2 or more points
            if(l.size() < 2) { continue; }
            
            // sanity checks
            assert(l.front() == l.back() && "Linestring start and end points do not match");
            assert(startIndex[n] < l.size() && "Start index isnt within linestring");
            
            // add input linestring to buffer
            for(size_t i = 0; i < l.size()-1; i++) { // -1 because first and last point are the same
                size_t index = (int)(i + startIndex[n]) % (l.size()-1);
                assert(index < l.size() && "Index out of range");
                buffer.push_back(l[index]);
            }
            // make first and last point the same
            buffer.push_back(l[startIndex[n]]);
            // is top level of item
            if(isEnclosingPath) {
                returnVals.enclosingPath.push_back(buffer);
            }
            
            // break if this is just a single lineloop
            if(pathOffset == 0.0f) { 
                endOffsetGroup();
                continue; 
            } 
            // recursively offset the lineString to bore out
            std::vector<MaxLib::Geom::LineString> OffsetLines = OffsetPolygon(l, pathOffset, params);
            // if no more offsets, add centroid point as last position
            if(!OffsetLines.size()) {
                if(auto centroid = Centroid(l)) { 
                    buffer.push_back(*centroid); 
                }
                endOffsetGroup();
                continue;
            }
            bool isNewEnclosingPath = false;
            // we have finished a set of offsets, add buffer to return vector
            if(OffsetLines.size() > lineStrings.size()) {
                isNewEnclosingPath = true;
                endOffsetGroup();
            }
            // check for the closest point and make this the new start point
            std::vector<size_t> nextStartIndex = DetermineStartPoints(OffsetLines, l, startIndex[n]);
            // repeat function
            OffsetPolygon_Recursive(returnVals, OffsetLines, pathOffset, buffer, nextStartIndex, isNewEnclosingPath, params);
        }
    } 
    
    std::vector<size_t> DetermineStartPoints(std::vector<MaxLib::Geom::LineString>& OffsetLines, const MaxLib::Geom::LineString& l, size_t startIndex)
    {
        assert(OffsetLines.size());
        assert(startIndex < l.size());
        
        const MaxLib::Geom::Vec2& p0 = l[startIndex];
        
        std::vector<size_t> startIndexNew;
        
        for(size_t n = 0; n < OffsetLines.size(); n++)
        {   
            size_t newIndex = 0;
            // ensure the line falls within the current offset polygon (to prevent cutting material we wouldn't want to)
            for(size_t i = 0; i < OffsetLines[n].size()-1; i++) // -1 because first and last point are the same
            {
                MaxLib::Geom::Vec2& p1 = OffsetLines[n][i];
                if(Within({ p0, p1 }, l)) {
                    newIndex = i;
                    break;
                }
            }
            
            /* FIND THE CLOSEST POINT (VERY SLOW...)
            // reset the start position & minimum length
            float LMin = -1.0f;
            size_t newIndex = 0;
            
            for(size_t i = 0; i < OffsetLines[n].size()-1; i++) // -1 because first and last point are the same
            {   // get next point in offset and compare its length to prev
                MaxLib::Geom::Vec2& p1 = OffsetLines[n][i];                
                MaxLib::Geom::Vec2 dif = p1 - p0;
                // compare length
                float L = hypotf(dif.x, dif.y);
                if(L < LMin || LMin == -1.0f) {
                    if(LineIsInsidePolygon(p0, p1, l)) {
                        newIndex = i;
                        LMin = L;
                    }
                }
            }
            * */
            startIndexNew.push_back(newIndex);
        }
        return move(startIndexNew);
    }

    
private:

    GEOSGeometry* GeosLinearRing(const MaxLib::Geom::LineString& points);
    GEOSGeometry* GeosPolygon(const MaxLib::Geom::LineString& points);
    GEOSGeometry* GeosLineString(const MaxLib::Geom::LineString& points);
    

    GEOSCoordSequence*         GeosCoordSequence(const MaxLib::Geom::LineString& points);
    
    // Converts GEOS point or GEOS linestring to Geom::MaxLib::Geom::LineString
    std::optional<MaxLib::Geom::LineString>  GeosGetCoords(const GEOSGeometry* geometry, bool reversePoints);
    
    // Converts linestrings or polygons into std::vector<MaxLib::Geom::LineString>
    std::vector<MaxLib::Geom::LineString>    GeosGetGeometry(const GEOSGeometry* geometry);
    // Converts GEOS geometry collection to std::vector<Geom::MaxLib::Geom::LineString>
    std::vector<MaxLib::Geom::LineString>    GeosGetLineStrings(const GEOSGeometry* geometry);
    // Converts GEOS geometry collection to std::vector<Geom::MaxLib::Geom::LineString>
    std::vector<MaxLib::Geom::LineString>    GeosGetPolygons(const GEOSGeometry* geometry);

    // Generic boolean operation with callback
    std::optional<bool> BooleanOperation(const MaxLib::Geom::LineString& geomA, const MaxLib::Geom::LineString& geomB, std::function<char(GEOSGeometry*, GEOSGeometry*)> cb) {
        
        GEOSGeometry* geom_A = GeosPointLineStringOrPolygon(geomA);
        if(!geom_A) return {};
        GEOSGeometry* geom_B = GeosPointLineStringOrPolygon(geomB);
        if(!geom_B) return {};
        
        char result = cb(geom_A, geom_B);
        if(result == 2) { return {}; } // error
        
        GEOSGeom_destroy(geom_A);
        GEOSGeom_destroy(geom_B);
        return result;
    }
    

    static void   MsgHandler(const char* fmt, ...);
};
