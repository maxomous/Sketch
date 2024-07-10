
#include <iostream>
#include "elements.h"

 

namespace Sketch
{
// Global counter for elements
static ElementID m_ElementCounter = 0;

SketchItem Item_WithReference::Reference() { 
    return SketchItem({ m_Type, m_Parent->ID() }); 
}

void Item_Point::UseSolverValue(Solver::ConstraintSolver& solver) { 
    p = m_Parent->GetResult(solver, cb_SolverValue(m_Parent)); 
}


Element::Element(SketchItem::Type type) : m_ID(++m_ElementCounter), m_Item_Element(this, type) {
    std::cout << "Adding Element: " << m_ID << std::endl;
}

ElementID Element::ID() const { 
    return m_ID; 
}

void Element::ClearSolverData() 
{ 
    m_SolverElement.reset(); 
}


// Point

Point::Point(const Vec2& p) 
    : Element(SketchItem::Type::Point), 
    
    m_Item_P(this, SketchItem::Type::Point, p, [](Element* element) -> Solver::Point2D& { 
        return *(element->SolverElement<Solver::Point2D>()); 
    })
{}


void Point::AddToSolver(Solver::ConstraintSolver& solver) 
{
    m_SolverElement = move(std::make_unique<Solver::Point2D>(solver.CreatePoint2D(P().x, P().y)));
}

// Line

    
Line::Line(const Vec2& p0, const Vec2& p1) 
    : Element(SketchItem::Type::Line), 
    
    m_Item_P0(this, SketchItem::Type::Line_P0, p0, [](Element* element) -> Solver::Point2D&  { 
        return element->SolverElement<Solver::Line>()->p0; 
    }),
    m_Item_P1(this, SketchItem::Type::Line_P1, p1, [](Element* element) -> Solver::Point2D&  {
        return element->SolverElement<Solver::Line>()->p1; 
    })
{}
    

void Line::AddToSolver(Solver::ConstraintSolver& solver) {
    m_SolverElement = std::make_unique<Solver::Line>(solver.CreateLine(P0().x, P0().y, P1().x, P1().y));
}

// Arc

Arc::Arc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction) 
    : Element(SketchItem::Type::Arc), m_Direction(direction),
    
    m_Item_P0(this, SketchItem::Type::Arc_P0, p0, [&](Element* element) -> Solver::Point2D&  { 
        return element->SolverElement<Solver::Arc>()->p0; 
        //return (m_Direction == Direction::CW) ? element->SolverElement<Solver::Arc>()->p0 : element->SolverElement<Solver::Arc>()->p1; 
    }),
    m_Item_P1(this, SketchItem::Type::Arc_P1, p1, [&](Element* element) -> Solver::Point2D&  {
        return element->SolverElement<Solver::Arc>()->p1; 
        //return (m_Direction == Direction::CW) ? element->SolverElement<Solver::Arc>()->p1 : element->SolverElement<Solver::Arc>()->p0; 
    }),
    m_Item_PC(this, SketchItem::Type::Arc_PC, pC, [](Element* element) -> Solver::Point2D&  {
        return element->SolverElement<Solver::Arc>()->pC; 
    })
{}
    

const MaxLib::Geom::Direction& Arc::Direction() const { 
    return m_Direction; 
}

void Arc::AddToSolver(Solver::ConstraintSolver& solver) {

    //const Vec2& p0 = (m_Direction == Direction::CW) ? P0() : P1();
    //const Vec2& p1 = (m_Direction == Direction::CW) ? P1() : P0();
    //m_SolverElement = std::make_unique<Solver::Arc>(solver.CreateArc(PC().x, PC().y, p0.x, p0.y, p1.x, p1.y));
    m_SolverElement = std::make_unique<Solver::Arc>(solver.CreateArc((m_Direction == MaxLib::Geom::Direction::CW), PC().x, PC().y, P0().x, P0().y, P1().x, P1().y));
}






// Circle

Circle::Circle(const Vec2& pC, double radius) 
  : Element(SketchItem::Type::Circle), 
  
    m_Item_Radius(this, radius, [](Element* element) {
        return element->SolverElement<Solver::Circle>()->radius.parameter;
    }), 
    m_Item_PC(this, SketchItem::Type::Circle_PC, pC, [](Element* element) -> Solver::Point2D& {
        return element->SolverElement<Solver::Circle>()->pC; 
    })

{}
   
   
void Circle::AddToSolver(Solver::ConstraintSolver& solver) {
    m_SolverElement = std::make_unique<Solver::Circle>(solver.CreateCircle(PC().x, PC().y, m_Item_Radius.value));
}


} // end namespace Sketch
