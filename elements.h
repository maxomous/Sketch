#pragma once
#include <memory>
#include <cmath>
#include <MaxLib.h>

#include "sketch_common.h"
 
#include "deps/constraintsolver/solver.h"

namespace Sketch {

using namespace MaxLib::Geom;

// Forward Declare
class Element;



// Contains functions and variables specific to the item
class Item
{
public:     
    Item(Element* parent) : m_Parent(parent) {}
    
    bool IsHovered() const { return m_IsHovered; }
    bool IsSelected() const { return m_IsSelected; }
    bool IsFailed() const { return m_IsFailed; }
    
protected:
    Element* m_Parent;

private:    
    bool m_IsHovered = false;
    bool m_IsSelected = false;
    bool m_IsFailed = false;
    
    friend class ElementFactory;    
};




// Item_Parameter is a parameter inside an element
class Item_Parameter : public Item
{
public:     
    Item_Parameter(Element* parent, double val, std::function<Slvs_hParam(Element*)> solverValue) 
        : Item(parent), value(val), cb_SolverValue(std::move(solverValue)) {}

    double value;

private:    
    // Set Parameter from Solver
    std::function<Slvs_hParam(Element*)> cb_SolverValue;
    void UseSolverValue(Solver::ConstraintSolver& solver) { value = solver.GetResult(cb_SolverValue(m_Parent)); }
    friend class Element;
};




// Contains functions and variables specific to the item
class Item_WithReference : public Item
{
public:     
    Item_WithReference(Element* parent, SketchItem::Type type) 
        : Item(parent), m_Type(type) {}
        
    SketchItem Reference();
    SketchItem::Type Type() { return m_Type; }

private:
    SketchItem::Type m_Type;
};



// Item_Element is the element itself
class Item_Element : public Item_WithReference
{
public:     
    Item_Element(Element* parent, SketchItem::Type type) 
        : Item_WithReference(parent, type) {}
        
};


// Item_Point is a point inside an element
class Item_Point : public Item_WithReference
{
public:
    Item_Point(Element* parent, SketchItem::Type type, const Vec2& position, std::function<Solver::Point2D&(Element* element)> solverValue) 
        : Item_WithReference(parent, type), p(position), cb_SolverValue(std::move(solverValue)) {}
    
    Vec2 p;

private:    
    // Set Position from Solver
    std::function<Solver::Point2D&(Element* element)> cb_SolverValue;
    void UseSolverValue(Solver::ConstraintSolver& solver);
    friend class Element;
};
 




// Contains functions specific to element
class Element
{
public:
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Element() = default;
    
    ElementID ID() const;
    
    Item_Element& Item_Elem() { return m_Item_Element; }
    
    virtual void ForEachItemPoint(std::function<void(Item_Point&)> cb) = 0;
    virtual void ForEachItemParameter(std::function<void(Item_Parameter&)> cb) { (void)cb; } // unused
    
    void UseSolverValues(Solver::ConstraintSolver& solver) 
    {    
        ForEachItemPoint([&solver](Item_Point& item) {
            item.UseSolverValue(solver);
        });
        ForEachItemParameter([&solver](Item_Parameter& item) {
            item.UseSolverValue(solver);
        });
    }
        
    virtual void AddToSolver(Solver::ConstraintSolver& solver) = 0;
    
    void         ClearSolverData();
    
    template<typename T>
    T* SolverElement() 
    { 
        auto element = dynamic_cast<T*>(m_SolverElement.get());
        
        assert(element && "Casting to element failed!");
        
        return element;
    }
    
    Vec2 GetResult(Solver::ConstraintSolver& solver, Solver::Point2D point)
    {
        std::array<double, 2> p = solver.GetResult(point);
        return { p[0], p[1] };
    }
    
protected:
    Element(SketchItem::Type type);
    
    ElementID m_ID = 0;
    
    Item_Element m_Item_Element;
    
    std::unique_ptr<Solver::Element> m_SolverElement;
        
};



    
class Point : public Element
{
public:
    Point(const Vec2& p);
    
    const Vec2& P() const { return m_Item_P.p; }
    Item_Point& Item_P() { return m_Item_P; }

    void ForEachItemPoint(std::function<void(Item_Point&)> cb) override {
        cb(m_Item_P);
    }
    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    
private:
    // Note: The solver treats a point the same as a point element
    Item_Point m_Item_P;
};


class Line : public Element
{
public:
    Line(const Vec2& p0, const Vec2& p1);
    
    const Vec2& P0() const { return m_Item_P0.p; }
    const Vec2& P1() const { return m_Item_P1.p; }
    Item_Point& Item_P0() { return m_Item_P0; }
    Item_Point& Item_P1() { return m_Item_P1; }

    void ForEachItemPoint(std::function<void(Item_Point&)> cb) override {
        cb(m_Item_P0);
        cb(m_Item_P1);
    }
        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    
private:
    Item_Point m_Item_P0;
    Item_Point m_Item_P1;
};


class Arc : public Element
{
public:
    Arc(const Vec2& p0, const Vec2& p1, const Vec2& pC, MaxLib::Geom::Direction direction);
    
    const MaxLib::Geom::Direction& Direction() const;
    
    const Vec2& P0() const { return m_Item_P0.p; }
    const Vec2& P1() const { return m_Item_P1.p; }
    const Vec2& PC() const { return m_Item_PC.p; }
    Item_Point& Item_P0() { return m_Item_P0; }
    Item_Point& Item_P1() { return m_Item_P1; }
    Item_Point& Item_PC() { return m_Item_PC; }

    void ForEachItemPoint(std::function<void(Item_Point&)> cb) override {
        cb(m_Item_P0);
        cb(m_Item_P1);
        cb(m_Item_PC);
    }
            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    
private:
    MaxLib::Geom::Direction m_Direction;
    
    Item_Point m_Item_P0;
    Item_Point m_Item_P1;
    Item_Point m_Item_PC;
};

 
class Circle : public Element
{
public:
    Circle(const Vec2& pC, double radius);
    
    double Radius() const { return m_Item_Radius.value; }
    Item_Parameter& Item_Radius() { return m_Item_Radius; }

    const Vec2& PC() const { return m_Item_PC.p; }
    Item_Point& Item_PC() { return m_Item_PC; }
    

    void ForEachItemPoint(std::function<void(Item_Point&)> cb) override {
        cb(m_Item_PC);
    }
    void ForEachItemParameter(std::function<void(Item_Parameter&)> cb) override {
        cb(m_Item_Radius);
    }
            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    
private:
    Item_Parameter m_Item_Radius;
    Item_Point m_Item_PC;
};

} // end namespace Sketch
