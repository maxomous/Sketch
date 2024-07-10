#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include "libslvs/include/slvs.h"

using namespace std;


/*  README:
 *      https://github.com/solvespace/solvespace/blob/0e0b0252e23dd5bd4ae82ababcc54c44aee036d6/exposed/DOC.txt
 * 
 *  Things from read me:
 *      - Each sketch should be in its own group
 *      - More complicated sketches could be split into sepearate groups? may be faster?
 *          "Consider point A in group 1, and point B in group 2. We have a constraint
             in group 2 that makes the points coincident. When we solve group 2, the
             solver is allowed to move point B to place it on top of point A. It is
             not allowed to move point A to put it on top of point B, because point
             A is outside the group being solved."
 * 
 */ 



namespace Solver {

// Equiv. to Slvs_hConstraint
typedef uint32_t Constraint; 

// Group Type (e.g. Axis is fixed, Elements are Free)
enum Group { Fixed = 1, Free = 2 };

// Solver success values
enum Success { Okay, Inconsistent, DidntConverge, TooManyUnknowns };

// Solver result structure
struct SolverResult {
    // Indicates whether the solver succeeded
    Success success;
    // Indicates the number of unconstrained degrees of freedom.
    int dof = -1;
    // Indicated the failed constraints
    std::vector<Constraint> failed;
};




// Forward declare  
class ConstraintSolver;
  
  
class Point
{
public:
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Point() = default;
    
    Slvs_hEntity entity;
private:
    // Only allow Point2D and Point3D to construct
    Point() = default;
    friend class Point2D;
    friend class Point3D;
};
  
class Point3D : public Point
{
public:
    Point3D() {}           // Blank Constructor
    Slvs_hParam paramX ;
    Slvs_hParam paramY;
    Slvs_hParam paramZ;
    
private:
    // Only Constraints can construct this
    Point3D(ConstraintSolver* parent, Group group, double x, double y, double z);
    friend class ConstraintSolver;
    friend class Axis;
};
   
class Normal
{
public:
    Normal() {}           // Blank Constructor
    Slvs_hParam parameters[4];
    Slvs_hEntity entity;
    
private:
    // Only Constraints can construct this
    // default has basis vectors (1 0 0) and (0 1 0)
    Normal(ConstraintSolver* parent, Group group, double ux = 1, double uy = 0, double uz = 0, double vx = 0, double vy = 1, double vz = 0);
    friend class ConstraintSolver;
    friend class Axis;
};

class Workplane 
{
public:
    Workplane() {}           // Blank Constructor
    Slvs_hEntity entity;
    
private:
    // Only Constraints can construct this
    Workplane(ConstraintSolver* parent, Group group, Point3D& origin, Normal& normal);
    friend class ConstraintSolver;
    friend class Axis;
};
    
class Axis
{
public:
    Axis() {}           // Blank Constructor
    Point3D origin;
    Normal normal;
    Workplane plane;
private:
    // Only Constraints can construct this
    Axis(ConstraintSolver* parent, Group group, double x = 0, double y = 0, double z = 0);
    friend class ConstraintSolver;
};

class Distance
{
public:
    Slvs_hParam parameter;
    Slvs_hEntity entity;
    
private:
    // Only Constraints can construct this
    Distance(ConstraintSolver* parent, Group group, Axis& axis, double distance);
    friend class ConstraintSolver;
    friend class Circle;
};

         
// ********************* ELEMENTS ************************ //
class Element
{
public:
    Element() {}
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Element() = default;
};

class Point2D : public Point, public Element
{
public:
    Slvs_hParam paramX;
    Slvs_hParam paramY;
    
private:
    // Only Constraints can construct this
    Point2D(ConstraintSolver* parent, Group group, Axis& axis, double x, double y);
    friend class ConstraintSolver;
    friend class Line;
    friend class Arc;
    friend class Circle;
};

class Line : public Element
{
public:
    Slvs_hEntity entity;
    Point2D p0;
    Point2D p1;

private:
    // Only Constraints can construct this
    Line(ConstraintSolver* parent, Group group, Axis& axis, double x0, double y0, double x1, double y1);
    friend class ConstraintSolver;
};

class Arc : public Element
{
public:
    Slvs_hEntity entity;
    Point2D pC;
    Point2D p0;
    Point2D p1;
    bool isCW;
private:
    // Only Constraints can construct this
    // The arc runs counter-clockwise from its beginning to its end (with
    // the workplane's normal pointing towards the viewer)
    Arc(ConstraintSolver* parent, Group group, Axis& axis, bool isClockwise, double xC, double yC, double x0, double y0, double x1, double y1);
    friend class ConstraintSolver;
};
    
    
class Circle : public Element
{
public:
    Slvs_hEntity entity;
    Point2D pC;
    Distance radius;
    
private:
    // Only Constraints can construct this
    Circle(ConstraintSolver* parent, Group group, Axis& axis, double xC, double yC, double r);
    friend class ConstraintSolver;
};








    
class ConstraintSolver
{
public:
 
    ConstraintSolver()
    {
        // to access, you can use [ID-1] (Currently...)
        // initialise the arrays and set default values
        sys.param = new Slvs_Param[m_Capacity_Param];
        sys.entity = new Slvs_Entity[m_Capacity_Entity];
        sys.constraint = new Slvs_Constraint[m_Capacity_Constraint];
        sys.failed = new Slvs_hConstraint[m_Capacity_Constraint];
        sys.faileds = m_Capacity_Constraint;
        sys.params = sys.constraints = sys.entities = 0;
        // Create axis
        defaultAxis = CreateAxis(); // Create our workplane with origin (x y z) = (0 0 0) and normal to XY plane
    }

    ~ConstraintSolver() {
        delete[] sys.param;
        delete[] sys.entity;
        delete[] sys.constraint;
        delete[] sys.failed;
    }
    


    // The base group for an axis is kept constant and we'll solve for group_free
    Axis    CreateAxis(double x = 0, double y = 0, double z = 0)                                    { return { this, Group::Fixed, x, y, z }; }
    
    Point2D CreatePoint2D(Axis& axis, double x = 0, double y = 0)                                   { return { this, Group::Free, axis, x, y }; }
    Point2D CreatePoint2D(double x = 0, double y = 0)                                               { return CreatePoint2D(defaultAxis, x, y); }

    Point3D CreatePoint3D(double x = 0, double y = 0, double z = 0)                                 { return { this, Group::Free, x, y, z }; }

    Line    CreateLine(Axis& axis, double x0, double y0, double x1, double y1)                      { return { this, Group::Free, axis, x0, y0, x1, y1 }; }
    Line    CreateLine(double x0, double y0, double x1, double y1)                                  { return CreateLine(defaultAxis, x0, y0, x1, y1); }

    Arc     CreateArc(Axis& axis, bool isClockwise, double xC, double yC, double x0, double y0, double x1, double y1) { return { this, Group::Free, axis, isClockwise, xC, yC, x0, y0, x1, y1 }; }
    Arc     CreateArc(bool isClockwise, double xC, double yC, double x0, double y0, double x1, double y1)             { return CreateArc(defaultAxis, isClockwise, xC, yC, x0, y0, x1, y1); }

    Circle  CreateCircle(Axis& axis, double xC, double yC, double r)                                { return { this, Group::Free, axis, xC, yC, r }; }
    Circle  CreateCircle(double xC, double yC, double r)                                            { return CreateCircle(defaultAxis, xC, yC, r); }


    // ********************* CONSTRAINTS ************************ //



/*
    Constraints that may be used in 3d or projected into a workplane are
    marked with a single star (*). Constraints that must always be used with
    a workplane are marked with a double star (**). Constraints that ignore
    the wrkpl member are marked with no star.

TODO:   *** ADD THESE CONSTRAINTS: ***
 
    SLVS_C_LENGTH_RATIO*

        The length of line entityA divided by the length of line entityB is
        equal to valA.

    SLVS_C_LENGTH_DIFFERENCE*

        The lengths of line entityA and line entityB differ by valA.

    SLVS_C_EQUAL_ANGLE*

        The angle between lines entityA and entityB is equal to the angle
        between lines entityC and entityD.

        If other is true, then the angles are supplementary (i.e., theta1 =
        180 - theta2) instead of equal.

    SLVS_C_SYMMETRIC*

        The points ptA and ptB are symmetric about the plane entityA. This
        means that they are on opposite sides of the plane and at equal
        distances from the plane, and that the line connecting ptA and ptB
        is normal to the plane.


    SLVS_C_SYMMETRIC_HORIZ
    SLVS_C_SYMMETRIC_VERT**

        The points ptA and ptB are symmetric about the horizontal or vertical
        axis of the specified workplane.

    SLVS_C_SYMMETRIC_LINE**

        The points ptA and ptB are symmetric about the line entityA.

    SLVS_C_WHERE_DRAGGED*

        The point ptA is locked at its initial numerical guess, and cannot
        be moved. This constrains two degrees of freedom in a workplane,
        and three in free space. It's therefore possible for this constraint
        to overconstrain the sketch, for example if it's applied to a point
        with one remaining degree of freedom.



IGNORE:

    SLVS_C_PROJ_PT_DISTANCE

        The distance between points ptA and ptB, as projected along the line
        or normal entityA, is equal to valA. This is a signed distance.

    SLVS_C_PT_PLANE_DISTANCE

        The distance from point ptA to workplane entityA is equal to
        valA. This is a signed distance; positive versus negative valA
        correspond to a point that is above vs. below the plane.

    SLVS_C_PT_IN_PLANE

        The point ptA lies in plane entityA.

    SLVS_C_EQ_LEN_PT_LINE_D*

        The length of the line entityA is equal to the distance from point
        ptA to line entityB.
        
    SLVS_C_EQ_PT_LN_DISTANCES*

        The distance from the line entityA to the point ptA is equal to the
        distance from the line entityB to the point ptB.

    SLVS_C_EQUAL_LINE_ARC_LEN*

        The length of the line entityA is equal to the length of the circular
        arc entityB.
        
    SLVS_C_SAME_ORIENTATION

        The normals entityA and entityB describe identical rotations. This
        constraint therefore restricts three degrees of freedom.
        
    SLVS_C_CUBIC_LINE_TANGENT*

        The cubic entityA is tangent to the line entityB. The variable
        other indicates:

            if false: the cubic is tangent at its beginning
            if true:  the cubic is tangent at its end

        The beginning of the cubic is point[0], and the end is point[3].
 
*/


    // Add dragged point
    Constraint Add_Dragged(Axis& axis, const Point& point)  { return MakeConstraint(axis, SLVS_C_WHERE_DRAGGED, 0.0, point.entity, 0, 0, 0); }
    // default axis
    Constraint Add_Dragged(const Point& point)              { return Add_Dragged(defaultAxis, point); }
    

    // Coincident Point to Point*
    //    Points ptA and ptB are coincident (i.e., exactly on top of each other).
    Constraint Add_Coincident_PointToPoint(Axis& axis, const Point& pointA, const Point& pointB)    { return MakeConstraint(axis, SLVS_C_POINTS_COINCIDENT, 0.0, pointA.entity, pointB.entity, 0, 0); }
    
    // Coincident Point to Line*
    //    The point ptA lies on the line entityA.
    //
    //    Note that this constraint removes one degree of freedom when projected
    //    in to the plane, but two degrees of freedom in 3d.
    Constraint Add_Coincident_PointToLine(Axis& axis, const Point& point, const Line& line)         { return MakeConstraint(axis, SLVS_C_PT_ON_LINE, 0.0, point.entity, 0, line.entity, 0); }
    
    // Coincident Point to Arc
    //    The point ptA lies on the right cylinder obtained by extruding circle
    //    or arc entityA normal to its plane.
    // TODO: Check if point on arc works?
    Constraint Add_Coincident_PointToArc(Axis& axis, const Point& point, const Arc& arc)            { return MakeConstraint(axis, SLVS_C_PT_ON_CIRCLE, 0.0, point.entity, 0, arc.entity, 0); }
    // Coincident Point to Circle
    Constraint Add_Coincident_PointToCircle(Axis& axis, const Point& point, const Circle& circle)   { return MakeConstraint(axis, SLVS_C_PT_ON_CIRCLE, 0.0, point.entity, 0, circle.entity, 0); }
    
    
    // Distance Point to Point*
    //    The distance between points ptA and ptB is equal to valA. This is an
    //    unsigned distance, so valA must always be positive.
    Constraint Add_Distance_PointToPoint(Axis& axis, const Point& pointA, const Point& pointB, double value)    { return MakeConstraint(axis, SLVS_C_PT_PT_DISTANCE, value, pointA.entity, pointB.entity, 0, 0); }
    
    // Distance Point to Line*
    //    The distance from point ptA to line segment entityA is equal to valA.
    //
    //    If the constraint is projected, then valA is a signed distance;
    //    positive versus negative valA correspond to a point that is above
    //    vs. below the line.
    //
    //    If the constraint applies in 3d, then valA must always be positive.
    Constraint Add_Distance_PointToLine(Axis& axis, const Point& point, const Line& line, double value)         { return MakeConstraint(axis, SLVS_C_PT_LINE_DISTANCE, value, point.entity, 0, line.entity, 0); }
    
    // Midpoint of Line*
    //    The point ptA lies at the midpoint of the line entityA.
    Constraint Add_MidPoint(Axis& axis, const Point& point, const Line& line)               { return MakeConstraint(axis, SLVS_C_AT_MIDPOINT, 0.0, point.entity, 0, line.entity, 0); }
    
    // Radius
    //    The diameter of circle or arc entityA is equal to valA.
    Constraint Add_Radius(Axis& axis, const Circle& circle, double radius)                  { return MakeConstraint(axis, SLVS_C_DIAMETER, radius*2, 0, 0, circle.entity, 0); }
    Constraint Add_Radius(Axis& axis, const Arc& arc, double radius)                        { return MakeConstraint(axis, SLVS_C_DIAMETER, radius*2, 0, 0, arc.entity, 0); }
 
    // Angle*
    //    The angle between lines entityA and entityB is equal to valA, where
    //    valA is specified in degrees. This constraint equation is written
    //    in the form
    //
    //        (A dot B)/(|A||B|) = cos(valA)
    //
    //    where A and B are vectors in the directions of lines A and B. This
    //    equation does not specify the angle unambiguously; for example,
    //    note that valA = +/- 90 degrees will produce the same equation.
    //
    //    If other is true, then the constraint is instead that
    //
    //        (A dot B)/(|A||B|) = -cos(valA)
    Constraint Add_Angle(Axis& axis, const Line& line1, const Line& line2, double value)    { return MakeConstraint(axis, SLVS_C_ANGLE, value, 0, 0, line1.entity, line2.entity); }

    // Vertical** / Horizontal
    //    The line connecting points ptA and ptB is horizontal or vertical. Or,
    //    the line segment entityA is horizontal or vertical. If points are
    //    specified then the line segment should be left zero, and if a line
    //    is specified then the points should be left zero.
    Constraint Add_Vertical(Axis& axis, const Point& pointA, const Point& pointB)           { return MakeConstraint(axis, SLVS_C_VERTICAL, 0.0, pointA.entity, pointB.entity, 0, 0); }
    
    Constraint Add_Horizontal(Axis& axis, const Point& pointA, const Point& pointB)         { return MakeConstraint(axis, SLVS_C_HORIZONTAL, 0.0, pointA.entity, pointB.entity, 0, 0); }
    
    // Parallel*
    //    Lines entityA and entityB are parallel.
    //    Note: that this constraint removes one degree of freedom when projected
    //     in to the plane, but two degrees of freedom in 3d.
    Constraint Add_Parallel(Axis& axis, const Line& line1, const Line& line2)               { return MakeConstraint(axis, SLVS_C_PARALLEL, 0.0, 0, 0, line1.entity, line2.entity); }
    
    // Perpendicular*
    //    Identical to SLVS_C_ANGLE with valA = 90 degrees.
    Constraint Add_Perpendicular(Axis& axis, const Line& line1, const Line& line2)          { return MakeConstraint(axis, SLVS_C_PERPENDICULAR, 0.0, 0, 0, line1.entity, line2.entity); }
       
       
    // Tangent - Arc to Line**
    //    The arc entityA is tangent to the line entityB. If other is false,
    //    then the arc is tangent at its beginning (point[1]). If other is true,
    //    then the arc is tangent at its end (point[2]).
    
    
    Constraint Add_Tangent(Axis& axis, const Arc& arc, const Line& line, int arc_point) { 
        return MakeConstraint(axis, SLVS_C_ARC_LINE_TANGENT, 0.0, 0, 0, arc.entity, line.entity, arc_point); 
    }
    
    // Tangent - Curve to Curve**
    //    The two entities entityA and entityB are tangent. These entities can
    //    each be either an arc or a cubic, in any combination. The flags
    //    other and other2 indicate which endpoint of the curve is tangent,
    //    for entityA and entityB respectively:
    //
    //        if false: the entity is tangent at its beginning
    //        if true:  the entity is tangent at its end
    // 
    //    For cubics, point[0] is the beginning, and point[3] is the end. For
    //    arcs, point[1] is the beginning, and point[2] is the end.
    
    /* TODO: 
        The flags  other and other2 indicate which endpoint of the curve is tangent,
        for entityA and entityB respectively:

            if false: the entity is tangent at its beginning
            if true:  the entity is tangent at its end
    */
    Constraint Add_Tangent(Axis& axis, const Arc& arc1, const Arc& arc2)                    { return MakeConstraint(axis, SLVS_C_CURVE_CURVE_TANGENT, 0.0, 0, 0, arc1.entity, arc2.entity); }
    // TODO: Check  circle + arc  &  circle + circle
    
           
    // Equal Length*
    //  `The lines entityA and entityB have equal length.
    Constraint Add_EqualLength(Axis& axis, const Line& line1, const Line& line2)            { return MakeConstraint(axis, SLVS_C_EQUAL_LENGTH_LINES, 0.0, 0, 0, line1.entity, line2.entity); }
    
    // Equal Radius
    //    The circles or arcs entityA and entityB have equal radius.
    Constraint Add_EqualRadius(Axis& axis, const Arc& arc, const Circle& circle)            { return MakeConstraint(axis, SLVS_C_EQUAL_RADIUS, 0.0, 0, 0, arc.entity, circle.entity); }
    Constraint Add_EqualRadius(Axis& axis, const Arc& arc1, const Arc& arc2)                { return MakeConstraint(axis, SLVS_C_EQUAL_RADIUS, 0.0, 0, 0, arc1.entity, arc2.entity); }
    Constraint Add_EqualRadius(Axis& axis, const Circle& circle1, const Circle& circle2)    { return MakeConstraint(axis, SLVS_C_EQUAL_RADIUS, 0.0, 0, 0, circle1.entity, circle2.entity); }



    // Default Axis 
    Constraint Add_Coincident_PointToPoint(const Point& pointA, const Point& pointB)                { return Add_Coincident_PointToPoint(defaultAxis, pointA, pointB); }
    Constraint Add_Coincident_PointToLine(const Point& point, const Line& line)                     { return Add_Coincident_PointToLine(defaultAxis, point, line); }
    Constraint Add_Coincident_PointToArc(const Point& point, const Arc& arc)                        { return Add_Coincident_PointToArc(defaultAxis, point, arc); }
    Constraint Add_Coincident_PointToCircle(const Point& point, const Circle& circle)               { return Add_Coincident_PointToCircle(defaultAxis, point, circle); }
    Constraint Add_Distance_PointToPoint(const Point& pointA, const Point& pointB, double value)    { return Add_Distance_PointToPoint(defaultAxis, pointA, pointB, value); }
    Constraint Add_Distance_PointToLine(const Point& point, const Line& line, double value)         { return Add_Distance_PointToLine(defaultAxis, point, line, value); }
    Constraint Add_MidPoint(const Point& point, const Line& line)                                   { return Add_MidPoint(defaultAxis, point, line); }
    Constraint Add_Radius(const Circle& circle, double radius)                                      { return Add_Radius(defaultAxis, circle, radius); }
    Constraint Add_Radius(const Arc& arc, double radius)                                            { return Add_Radius(defaultAxis, arc, radius); }
    Constraint Add_Angle(const Line& line1, const Line& line2, double value)                        { return Add_Angle(defaultAxis, line1, line2, value); }
    Constraint Add_Vertical(const Point& pointA, const Point& pointB)                               { return Add_Vertical(defaultAxis, pointA, pointB); }
    Constraint Add_Horizontal(const Point& pointA, const Point& pointB)                             { return Add_Horizontal(defaultAxis, pointA, pointB); }
    Constraint Add_Parallel(const Line& line1, const Line& line2)                                   { return Add_Parallel(defaultAxis, line1, line2); }    
    Constraint Add_Perpendicular(const Line& line1, const Line& line2)                              { return Add_Perpendicular(defaultAxis, line1, line2); }
    Constraint Add_Tangent(const Arc& arc, const Line& line, int arc_point)                         { return Add_Tangent(defaultAxis, arc, line, arc_point); }
    Constraint Add_Tangent(const Arc& arc1, const Arc& arc2)                                        { return Add_Tangent(defaultAxis, arc1, arc2); }
    Constraint Add_EqualLength(const Line& line1, const Line& line2)                                { return Add_EqualLength(defaultAxis, line1, line2); }
    Constraint Add_EqualRadius(const Arc& arc, const Circle& circle)                                { return Add_EqualRadius(defaultAxis, arc, circle); }
    Constraint Add_EqualRadius(const Arc& arc1, const Arc& arc2)                                    { return Add_EqualRadius(defaultAxis, arc1, arc2); }
    Constraint Add_EqualRadius(const Circle& circle1, const Circle& circle2)                        { return Add_EqualRadius(defaultAxis, circle1, circle2); }
    
    
    // Attempts to not move given point when solving
    void SetDraggedPoint(Point2D& point)
    {
        sys.dragged[0] = point.paramX; // x
        sys.dragged[1] = point.paramY; // y
    }
    
    //~ // TODO: add distance constraint parameters?
    
     //~ If a parameter corresponds to a point (distance, normal, etc.) being
     //~ * dragged, then specify it here. This will cause the solver to favor
     //~ * that parameter, and attempt to change it as little as possible even
     //~ * if that requires it to change other parameters more.
     //~ *
     //~ * Unused members of this array should be set to zero. 
    //~ Slvs_hParam         dragged[4];

    
    
    // returns true on success
    SolverResult Solve(bool calculateFailedConstraints = true)
    {
        // If the solver fails, report which constraints caused the problem
        sys.calculateFaileds = calculateFailedConstraints;

        // Solve for free group (other group is fixed)
        Slvs_Solve(&sys, (Slvs_hGroup)Group::Free);
        
        // Get result
        SolverResult result;
        result.success = (Success)sys.result;
        
        // if success, get DOFs
        if(result.success == Success::Okay) {
            result.dof = sys.dof;
        }
        else { // else get failed constraints
            for(int i = 0; i < sys.faileds; i++) {
                result.failed.push_back(sys.failed[i]);
            }
        }
        
        return result;
    }
    
    
    double GetResult(Slvs_hParam parameter) {
        assert(parameter > 0);
        size_t index = parameter - 1;
        assert(index < m_Capacity_Param);
        return sys.param[index].val;
    }
    
    // Helper function to get x & y result from solver
    std::array<double, 2> GetResult(const Solver::Point2D& point) {
        return { GetResult(point.paramX), GetResult(point.paramY) };
    }
    
    // Getters
    double GetParamValue(Slvs_hParam parameter) {
        assert(parameter > 0 && parameter < m_Capacity_Param);
        return sys.param[parameter - 1].val;
    }
    
    Slvs_hGroup GetParamGroup(Slvs_hParam parameter) {
        if(parameter < 1 || (size_t)parameter > m_Capacity_Param) { assert(0 && "Parameter value invalid"); }
        return sys.param[parameter - 1].group;
    }
    
    Slvs_hGroup GetEntityGroup(Slvs_hEntity entity) {
        if(entity < 1 || (size_t)entity > m_Capacity_Entity) { assert(0 && "Entity value invalid"); }
        return sys.entity[entity - 1].group;
    }
    
    void ModifyParamValue(Slvs_hParam parameter, double newValue) {
        assert(parameter > 0 && parameter < m_Capacity_Param);
        sys.param[parameter - 1].val = newValue;
    }
    
    void ModifyParamGroup(Slvs_hParam parameter, Group group) {
        if(parameter < 1 || (size_t)parameter > m_Capacity_Param) { assert(0 && "Parameter value invalid"); }
        sys.param[parameter - 1].group = (Slvs_hGroup)group;
    }
    
    void ModifyEntityGroup(Slvs_hEntity entity, Group group) {
        if(entity < 1 || (size_t)entity > m_Capacity_Entity) { assert(0 && "Entity value invalid"); }
        sys.entity[entity - 1].group = (Slvs_hGroup)group;
    }
    
    // Returns true if parameter's group is free
    bool IsParamGroupFree(Slvs_hParam parameter) {
        if(parameter < 1 || (size_t)parameter > m_Capacity_Param) { assert(0 && "Parameter value invalid"); }
        return (sys.param[parameter < 1].group == (Slvs_hGroup)Group::Free);
    }
    // Returns true if entity's group is free
    bool IsEntityGroupFree(Slvs_hEntity entity) {
        if(entity < 1 || (size_t)entity > m_Capacity_Entity) { assert(0 && "Entity value invalid"); }
        return (sys.entity[entity - 1].group == (Slvs_hGroup)Group::Free);
    }
    
private:
    Slvs_System sys;
    Axis defaultAxis;
    
    size_t m_Capacity_Param      = 256;
    size_t m_Capacity_Entity     = 64;
    size_t m_Capacity_Constraint = 64;

    template<typename T>
    void Resize(T** array, size_t currentSize, size_t newSize)
    {
        // make new array
        T* newArr = new T[newSize];
        // copy data across
        for(size_t i = 0; i < currentSize; i++) {
            memcpy(&(newArr[i]), &((*array)[i]), sizeof(T));
        }
        // delete old array
        delete[] *array;
        // assign new array
        *array = newArr;
    }
                
            
    Slvs_hParam MakeParam(Group group, double value) 
    {
        // Check array capacity
        while(sys.params >= (int)m_Capacity_Param) {
            // If it's not big enough, double its capacity
            size_t newCapacity = m_Capacity_Param * 2;
            std::cout << "resizing params" << (int)sys.param << " from " << m_Capacity_Param << " to " << newCapacity << std::endl;
            Resize<Slvs_Param>(&sys.param, m_Capacity_Param, newCapacity);
            m_Capacity_Param = newCapacity;
        }

        Slvs_hParam id = sys.params + 1;
        sys.param[sys.params++] = Slvs_MakeParam(id, (Slvs_hGroup)group, value);
        return id;
    }
       
    Slvs_hEntity MakeEntity(std::function<Slvs_Entity(Slvs_hEntity id)> cb) 
    {
        // Check array capacity
        while(sys.entities >= (int)m_Capacity_Entity) {
            // If it's not big enough, double its capacity
            size_t newCapacity = m_Capacity_Entity * 2;
            std::cout << "resizing entity" << (int)sys.entity << " from " << m_Capacity_Entity << " to " << newCapacity << std::endl;
            Resize<Slvs_Entity>(&sys.entity, m_Capacity_Entity, newCapacity); 
            m_Capacity_Entity = newCapacity;
        }
        Slvs_hEntity id = sys.entities + 1;
        sys.entity[sys.entities++] = cb(id);
        return id;
    }
    
    Slvs_hConstraint MakeConstraint(Axis& axis, int type, double value, Slvs_hEntity pointA, Slvs_hEntity pointB, Slvs_hEntity entityA, Slvs_hEntity entityB, int other1 = 0, int other2 = 0) 
    {
        // Check array capacity
        while(sys.constraints >= (int)m_Capacity_Constraint) {
            // If it's not big enough, double its capacity
            size_t newCapacity = m_Capacity_Constraint * 2;
            std::cout << "resizing constraints" << (int)sys.constraint << " from " << m_Capacity_Constraint << " to " << newCapacity << std::endl;
            Resize<Slvs_Constraint>(&sys.constraint, m_Capacity_Constraint, newCapacity);
            std::cout << "resizing faileds" << (int)sys.failed << " from " << m_Capacity_Constraint << " to " << newCapacity << std::endl;
            Resize<Slvs_hConstraint>(&sys.failed, m_Capacity_Constraint, newCapacity);
            m_Capacity_Constraint = newCapacity;
            sys.faileds = newCapacity;
        }
        Slvs_hConstraint id = sys.constraints + 1;
        sys.constraint[sys.constraints++] = Slvs_MakeConstraint(id, (Slvs_hGroup)Group::Free, type, axis.plane.entity, value, pointA, pointB, entityA, entityB, other1, other2);
        return id;
    }


        
    friend class Point3D;
    friend class Normal;
    friend class Workplane;
    friend class Axis;
    friend class Point2D;
    friend class Distance;
    friend class Line;
    friend class Arc;
    friend class Circle;

}; // End class Constraints

} // End namespace Solver



//#define SLVS_C_POINTS_COINCIDENT        100000
//#define SLVS_C_PT_PT_DISTANCE           100001
    //#define SLVS_C_PT_PLANE_DISTANCE        100002
//#define SLVS_C_PT_LINE_DISTANCE         100003
    //#define SLVS_C_PT_FACE_DISTANCE         100004
    //#define SLVS_C_PT_IN_PLANE              100005
//#define SLVS_C_PT_ON_LINE               100006
    //#define SLVS_C_PT_ON_FACE               100007
//#define SLVS_C_EQUAL_LENGTH_LINES       100008
    //#define SLVS_C_LENGTH_RATIO             100009
    //#define SLVS_C_EQ_LEN_PT_LINE_D         100010
    //#define SLVS_C_EQ_PT_LN_DISTANCES       100011
    //#define SLVS_C_EQUAL_ANGLE              100012
    //#define SLVS_C_EQUAL_LINE_ARC_LEN       100013
    //#define SLVS_C_SYMMETRIC                100014
    //#define SLVS_C_SYMMETRIC_HORIZ          100015
    //#define SLVS_C_SYMMETRIC_VERT           100016
    //#define SLVS_C_SYMMETRIC_LINE           100017
    //#define SLVS_C_AT_MIDPOINT              100018
//#define SLVS_C_HORIZONTAL               100019
//#define SLVS_C_VERTICAL                 100020
//#define SLVS_C_DIAMETER                 100021
//#define SLVS_C_PT_ON_CIRCLE             100022
    //#define SLVS_C_SAME_ORIENTATION         100023
//#define SLVS_C_ANGLE                    100024
//#define SLVS_C_PARALLEL                 100025
//#define SLVS_C_PERPENDICULAR            100026
//#define SLVS_C_ARC_LINE_TANGENT         100027
    //#define SLVS_C_CUBIC_LINE_TANGENT       100028
//#define SLVS_C_EQUAL_RADIUS             100029
    //#define SLVS_C_PROJ_PT_DISTANCE         100030
    // ? #define SLVS_C_WHERE_DRAGGED            100031
//#define SLVS_C_CURVE_CURVE_TANGENT      100032
    //#define SLVS_C_LENGTH_DIFFERENCE        100033

