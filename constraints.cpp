
#include <initializer_list>
#include "constraints.h"

#include "elementfactory.h"

namespace Sketch {

// Global counter for elements
static ConstraintID m_ConstraintCounter = 0; 


// Constructor
Constraint::Constraint(ElementFactory* parent, ConstraintType type) 
    : m_ID(++m_ConstraintCounter), m_Parent(parent), m_Type(type) {
    // Default value
    offsetDistance = m_Parent->parameters.offset_Constraint;
    std::cout << "Adding Constraint: " << m_ID << std::endl;
}
    

// Calculates scaled & offseted position
MaxLib::Geom::Vec2 Constraint::GetScaledOffset() { 
    return offsetDistance * GetOffsetDirection() * m_Parent->parameters.selectionDistance.scaleFactor; 
}


// Returns direction for distance constraint
MaxLib::Geom::Vec2 Constraint::GetDistanceDirection(SketchItem item1, SketchItem item2) 
{ 
    MaxLib::Geom::Vec2 p0 = m_Parent->GetPositionBySketchItem(item1);
    MaxLib::Geom::Vec2 p1 = m_Parent->GetPositionBySketchItem(item2);
    return PointPerpendicularToLine(p0, p1, 1.0, { 0.0f, 0.0f }); 
}


// Returns direction for distance constraint
MaxLib::Geom::Vec2 Distance_PointToLine::GetOffsetDirection() 
{ 
    MaxLib::Geom::Vec2 p0 = m_Parent->GetPositionBySketchItem(Ref_1());
    MaxLib::Geom::Vec2 p1 = GetEndpoint_DistanceToLine();
    return PointPerpendicularToLine(p0, p1, 1.0, { 0.0f, 0.0f }); 
}

            
            
// Returns direction for radius constraint
MaxLib::Geom::Vec2 Constraint::GetRadiusDirection(SketchItem item, float radius) { 
    MaxLib::Geom::Vec2 p0 = m_Parent->GetPositionBySketchItem(item);
    MaxLib::Geom::Vec2 p1 = p0 + MaxLib::Geom::Vec2(0.0f, radius);
    return PointPerpendicularToLine(p0, p1, 1.0, { 0.0f, 0.0f }); 
}
// Returns direction for angle constraint
MaxLib::Geom::Vec2 Constraint::GetAngleDirection(SketchItem item1, SketchItem item2, float angle) { 

    MaxLib::Geom::Vec2 p0 = m_Parent->GetPositionBySketchItem(item1.P0());
    MaxLib::Geom::Vec2 p1 = m_Parent->GetPositionBySketchItem(item1.P1());
    MaxLib::Geom::Vec2 p2 = m_Parent->GetPositionBySketchItem(item2.P0());
    MaxLib::Geom::Vec2 p3 = m_Parent->GetPositionBySketchItem(item2.P1());
    
    std::optional<MaxLib::Geom::Vec2> intersect = IntersectLines(p0, p1, p2, p3);
    if(intersect) {
        MaxLib::Geom::Vec2 pC = *intersect;
        // TODO: We are assuming here that p1 & p3 are endpoints
        MaxLib::Geom::Vec2 midPoint = ((p1 + p3) / 2) - pC;
        return midPoint / Hypot(midPoint); // Normalise
    }
    return { 0.0f, 1.0f }; // should never reach, maybe lines are parallel?
}

    
// Constraint

void Constraint::ClearSolverData() { 
    m_SolverConstraint = 0; 
}
                 
Solver::Constraint Constraint::SolverConstraint() const { 
    return m_SolverConstraint; 
}
/*
// Passes each of these elements to Callback Function
void Constraint::ForTheseElements(std::function<void(SketchItem&)> cb, std::vector<SketchItem&> elements) {
    for(auto element : elements) { 
        cb(element); 
    }
}
*/

MaxLib::Geom::Vec2 Constraint_Template_TwoItems::GetMidpoint() 
{
    const MaxLib::Geom::Vec2& p0 = m_Parent->GetPositionBySketchItem(m_Ref_1);
    const MaxLib::Geom::Vec2& p1 = m_Parent->GetPositionBySketchItem(m_Ref_2);
    return (p0 + p1) / 2.0;
}


MaxLib::Geom::Vec2 Constraint_Template_TwoItems::GetEndpoint_DistanceToLine() 
{
    const MaxLib::Geom::Vec2& p0 = m_Parent->GetPositionBySketchItem(m_Ref_1);
    const MaxLib::Geom::Vec2& p1a = m_Parent->GetPositionBySketchItem(m_Ref_2.P0());
    const MaxLib::Geom::Vec2& p1b = m_Parent->GetPositionBySketchItem(m_Ref_2.P1());
    // Redefine p0 as the projected point p1 falls on line (p0a->p0b)
    return IntersectLinePoint(p1a, p1b, p0);
}

MaxLib::Geom::Vec2 Constraint_Template_TwoItems::GetMidpoint_DistanceToLine() 
{
    const MaxLib::Geom::Vec2& p0 = m_Parent->GetPositionBySketchItem(m_Ref_1);
    const MaxLib::Geom::Vec2& p1 = GetEndpoint_DistanceToLine();
    return (p0 + p1) / 2.0; // return midpoint
}


std::vector<MaxLib::Geom::Vec2> Constraint_Template_OneItem::GetImagePositions() { return { m_Parent->GetPositionBySketchItem(m_Ref) }; }
std::vector<MaxLib::Geom::Vec2> Constraint_Template_TwoItems::GetImagePositions() { return { m_Parent->GetPositionBySketchItem(m_Ref_1), m_Parent->GetPositionBySketchItem(m_Ref_2) }; }

std::vector<MaxLib::Geom::Vec2> Coincident_PointToPoint::GetImagePositions()     { return { m_Parent->GetPositionBySketchItem(m_Ref_1) }; }
std::vector<MaxLib::Geom::Vec2> Coincident_PointToLine::GetImagePositions()      { return { m_Parent->GetPositionBySketchItem(m_Ref_1) }; }
std::vector<MaxLib::Geom::Vec2> Coincident_PointToArc::GetImagePositions()       { return { m_Parent->GetPositionBySketchItem(m_Ref_1) }; }
std::vector<MaxLib::Geom::Vec2> Coincident_PointToCircle::GetImagePositions()    { return { m_Parent->GetPositionBySketchItem(m_Ref_1) }; }

std::vector<MaxLib::Geom::Vec2> Distance_PointToPoint::GetImagePositions()       { return { GetMidpoint() }; }
std::vector<MaxLib::Geom::Vec2> Distance_PointToLine::GetImagePositions()        { return { GetMidpoint_DistanceToLine() }; }
    
std::vector<MaxLib::Geom::Vec2> AddMidPoint_PointToLine::GetImagePositions()     { return { m_Parent->GetPositionBySketchItem(m_Ref_1) }; }
std::vector<MaxLib::Geom::Vec2> AddRadius_Circle::GetImagePositions()            { return { m_Parent->GetPositionBySketchItem(m_Ref) + Vec2(0.0f, 0.5f * radius) }; }
std::vector<MaxLib::Geom::Vec2> AddRadius_Arc::GetImagePositions()               { return { m_Parent->GetPositionBySketchItem(m_Ref) + Vec2(0.0f, 0.5f * radius) }; }
std::vector<MaxLib::Geom::Vec2> Angle_LineToLine::GetImagePositions()            { return { GetMidpoint() }; }
std::vector<MaxLib::Geom::Vec2> Vertical::GetImagePositions()                   { return { GetMidpoint() }; }
std::vector<MaxLib::Geom::Vec2> Horizontal::GetImagePositions()                 { return { GetMidpoint() }; } 
std::vector<MaxLib::Geom::Vec2> Tangent_Arc_Line::GetImagePositions()            { return { m_Parent->GetPositionBySketchItem((tangentPoint) ? m_Ref_1.P1() : m_Ref_1.P0()) }; }  

void Coincident_PointToPoint::AddToSolver(Solver::ConstraintSolver& solver)     { m_SolverConstraint = solver.Add_Coincident_PointToPoint(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2)); }
void Coincident_PointToLine::AddToSolver(Solver::ConstraintSolver& solver)      { m_SolverConstraint = solver.Add_Coincident_PointToLine(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }
void Coincident_PointToArc::AddToSolver(Solver::ConstraintSolver& solver)       { m_SolverConstraint = solver.Add_Coincident_PointToArc(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverArc(m_Ref_2)); }
void Coincident_PointToCircle::AddToSolver(Solver::ConstraintSolver& solver)    { m_SolverConstraint = solver.Add_Coincident_PointToCircle(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverCircle(m_Ref_2)); }
void Distance_PointToPoint::AddToSolver(Solver::ConstraintSolver& solver)       { m_SolverConstraint = solver.Add_Distance_PointToPoint(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2), distance); }
void Distance_PointToLine::AddToSolver(Solver::ConstraintSolver& solver)        { m_SolverConstraint = solver.Add_Distance_PointToLine(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2), distance); }
void AddMidPoint_PointToLine::AddToSolver(Solver::ConstraintSolver& solver)     { m_SolverConstraint = solver.Add_MidPoint(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }
void AddRadius_Circle::AddToSolver(Solver::ConstraintSolver& solver)            { m_SolverConstraint = solver.Add_Radius(m_Parent->GetSolverCircle(m_Ref), radius); }
void AddRadius_Arc::AddToSolver(Solver::ConstraintSolver& solver)               { m_SolverConstraint = solver.Add_Radius(m_Parent->GetSolverArc(m_Ref), radius); }
void Angle_LineToLine::AddToSolver(Solver::ConstraintSolver& solver)            { m_SolverConstraint = solver.Add_Angle(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2), angle); }
void Vertical::AddToSolver(Solver::ConstraintSolver& solver)                    { m_SolverConstraint = solver.Add_Vertical(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2)); }
void Horizontal::AddToSolver(Solver::ConstraintSolver& solver)                  { m_SolverConstraint = solver.Add_Horizontal(m_Parent->GetSolverPoint(m_Ref_1), m_Parent->GetSolverPoint(m_Ref_2)); }
void Parallel::AddToSolver(Solver::ConstraintSolver& solver)                    { m_SolverConstraint = solver.Add_Parallel(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }        
void Perpendicular::AddToSolver(Solver::ConstraintSolver& solver)               { m_SolverConstraint = solver.Add_Perpendicular(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }          
void Tangent_Arc_Line::AddToSolver(Solver::ConstraintSolver& solver)            { 
    const Solver::Arc& arc = m_Parent->GetSolverArc(m_Ref_1);
    // flip tangent point if one or the other
    bool flipTangentPoint = (arc.isCW ^ tangentPoint);
    m_SolverConstraint = solver.Add_Tangent(arc, m_Parent->GetSolverLine(m_Ref_2), flipTangentPoint); 
}
void Tangent_Arc_Arc::AddToSolver(Solver::ConstraintSolver& solver)             { m_SolverConstraint = solver.Add_Tangent(m_Parent->GetSolverArc(m_Ref_1), m_Parent->GetSolverArc(m_Ref_2)); }
void EqualLength::AddToSolver(Solver::ConstraintSolver& solver)                 { m_SolverConstraint = solver.Add_EqualLength(m_Parent->GetSolverLine(m_Ref_1), m_Parent->GetSolverLine(m_Ref_2)); }         
void EqualRadius_Arc_Arc::AddToSolver(Solver::ConstraintSolver& solver)         { m_SolverConstraint = solver.Add_EqualRadius(m_Parent->GetSolverArc(m_Ref_1), m_Parent->GetSolverArc(m_Ref_2)); }          
void EqualRadius_Arc_Circle::AddToSolver(Solver::ConstraintSolver& solver)      { m_SolverConstraint = solver.Add_EqualRadius(m_Parent->GetSolverArc(m_Ref_1), m_Parent->GetSolverCircle(m_Ref_2)); }
void EqualRadius_Circle_Circle::AddToSolver(Solver::ConstraintSolver& solver)   { m_SolverConstraint = solver.Add_EqualRadius(m_Parent->GetSolverCircle(m_Ref_1), m_Parent->GetSolverCircle(m_Ref_2)); }

 
} // end namespace Sketch
