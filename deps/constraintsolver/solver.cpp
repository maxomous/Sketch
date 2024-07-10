
#include <iostream>
#include <functional>

#include "solver.h"

using namespace std;


namespace Solver {

Point3D::Point3D(ConstraintSolver* parent, Group group, double x, double y, double z) 
{
    paramX = parent->MakeParam(group, x);  // x
    paramY = parent->MakeParam(group, y);  // y
    paramZ = parent->MakeParam(group, z);  // z
    
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        return Slvs_MakePoint3d(id, group, paramX, paramY, paramZ);
    });
}

Normal::Normal(ConstraintSolver* parent, Group group, double ux, double uy, double uz, double vx, double vy, double vz) 
{
    double qw, qx, qy, qz;
    Slvs_MakeQuaternion(ux, uy, uz, vx, vy, vz, &qw, &qx, &qy, &qz);
    
    parameters[0] = parent->MakeParam(group, qw);
    parameters[1] = parent->MakeParam(group, qx);
    parameters[2] = parent->MakeParam(group, qy);
    parameters[3] = parent->MakeParam(group, qz);
    
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        return Slvs_MakeNormal3d(id, group, parameters[0], parameters[1], parameters[2], parameters[3]);
    });
}

Workplane::Workplane(ConstraintSolver* parent, Group group, Point3D& origin, Normal& normal)
{
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        return Slvs_MakeWorkplane(id, group, origin.entity, normal.entity);
    });
}

Axis::Axis(ConstraintSolver* parent, Group group, double x, double y, double z) 
    :  origin(parent, group, x, y, z), normal(parent, group), plane(parent, group, origin, normal)
{}

Point2D::Point2D(ConstraintSolver* parent, Group group, Axis& axis, double x, double y) 
{
    paramX = parent->MakeParam(group, x);  // x
    paramY = parent->MakeParam(group, y);  // y
    
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        return Slvs_MakePoint2d(id, group, axis.plane.entity, paramX, paramY);
    });
}

Distance::Distance(ConstraintSolver* parent, Group group, Axis& axis, double distance) 
{
    parameter = parent->MakeParam(group, distance);  
          
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        return Slvs_MakeDistance(id, (Group)group, axis.plane.entity, parameter);
    });
}

Line::Line(ConstraintSolver* parent, Group group, Axis& axis, double x0, double y0, double x1, double y1)
    : p0(parent, group, axis, x0, y0), p1(parent, group, axis, x1, y1)
{            
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        return Slvs_MakeLineSegment(id, group, axis.plane.entity, p0.entity, p1.entity);
    });
}

Arc::Arc(ConstraintSolver* parent, Group group, Axis& axis, bool isClockwise, double xC, double yC, double x0, double y0, double x1, double y1)
    : pC(parent, group, axis, xC, yC), p0(parent, group, axis, x0, y0), p1(parent, group, axis, x1, y1), isCW(isClockwise)
{            
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        if(isCW) { return Slvs_MakeArcOfCircle(id, group, axis.plane.entity, axis.normal.entity, pC.entity, p1.entity, p0.entity); }
        else     { return Slvs_MakeArcOfCircle(id, group, axis.plane.entity, axis.normal.entity, pC.entity, p0.entity, p1.entity); }
    });
}

Circle::Circle(ConstraintSolver* parent, Group group, Axis& axis, double xC, double yC, double r)
    : pC(parent, group, axis, xC, yC), radius(parent, group, axis, r)
{            
    entity = parent->MakeEntity([&](Slvs_hEntity id) {
        return Slvs_MakeCircle(id, group, axis.plane.entity, pC.entity, axis.normal.entity, radius.entity);
    });
}

} // end Solver namespace
