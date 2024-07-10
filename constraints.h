#pragma once

#include <vector>

#include "sketch_common.h"

#include "deps/constraintsolver/solver.h"


namespace Sketch {

// TODO: make sure a sketch item is the correct element on construction

// forward declare
class ElementFactory;




class Constraint
{
public:
    // Constructor
    Constraint(ElementFactory* parent, ConstraintType type);
    // Marked virtual so that any derived classes' destructor also gets called 
    virtual ~Constraint() = default;
    
    virtual std::string Label() = 0;
    virtual std::vector<MaxLib::Geom::Vec2> GetImagePositions() = 0;
    ConstraintID ID() const { return m_ID; }
    // Get constraint render type
    ConstraintType GetType() { return m_Type; }
    
    // Adds constraint to solver
    virtual void AddToSolver(Solver::ConstraintSolver& solver) = 0;
    
    // Passes each element to Callback Function
    virtual void ForEachItem(std::function<void(SketchItem&)> cb) = 0;
    
    void ClearSolverData();
    
    bool IsHovered() const { return m_IsHovered; }
    bool IsSelected() const { return m_IsSelected; }
    bool IsFailed() const { return m_IsFailed; }
         
    Solver::Constraint SolverConstraint() const;
        
        
    // Constraint offset distance from actual position 
    float offsetDistance; // initialised in constructor

    // Calculates scaled & offseted position
    MaxLib::Geom::Vec2 GetScaledOffset();
    
protected:
    ConstraintID m_ID;
    ElementFactory* m_Parent;
    ConstraintType m_Type;
    bool m_IsHovered = false;
    bool m_IsSelected = false;
    bool m_IsFailed = false;
    Solver::Constraint m_SolverConstraint = 0;
    
    // Returns direction of offset from true position when displayed on viewer (for example, horiz = {0, 1} (above) )
    virtual MaxLib::Geom::Vec2 GetOffsetDirection() { return { -0.707f, 0.707f }; }
    
    // Returns direction for distance constraint
    MaxLib::Geom::Vec2 GetDistanceDirection(SketchItem item1, SketchItem item2);
    // Returns direction for radius constraint
    MaxLib::Geom::Vec2 GetRadiusDirection(SketchItem item, float radius);
    // Returns direction for angle constraint
    MaxLib::Geom::Vec2 GetAngleDirection(SketchItem item1, SketchItem item2, float angle);
    
    // Passes each of these elements to Callback Function
   // void ForTheseElements(std::function<void(SketchItem&)> cb, std::vector<SketchItem&> elements);
private:
    friend class ElementFactory;
};

          

// Templates

class Constraint_Template_OneItem : public Constraint
{
public:
    Constraint_Template_OneItem(ElementFactory* parent, SketchItem item, ConstraintType type) : Constraint(parent, type), m_Ref(item) {}
    // Passes each element to Callback Function
    void ForEachItem(std::function<void(SketchItem&)> cb)                          { cb(m_Ref); }
    SketchItem Ref() { return m_Ref; };
    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
protected:
    SketchItem m_Ref;
};

class Constraint_Template_TwoItems : public Constraint
{
public:
    Constraint_Template_TwoItems(ElementFactory* parent, SketchItem item1, SketchItem item2, ConstraintType type) : Constraint(parent, type), m_Ref_1(item1), m_Ref_2(item2)  {}
    // Passes each element to Callback Function
    void ForEachItem(std::function<void(SketchItem&)> cb)                          { cb(m_Ref_1); cb(m_Ref_2); }
    SketchItem Ref_1() { return m_Ref_1; };
    SketchItem Ref_2() { return m_Ref_2; }; 
    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
    MaxLib::Geom::Vec2 GetMidpoint();
    MaxLib::Geom::Vec2 GetEndpoint_DistanceToLine();
    MaxLib::Geom::Vec2 GetMidpoint_DistanceToLine();
    
protected:
    SketchItem m_Ref_1;
    SketchItem m_Ref_2;
};

    //~ // Get constraint render type
    //~ ConstraintType GetType() override { return ConstraintType::Coincident; };
    
    //~ // Get constraint 2D position
    //~ std::vector<MaxLib::Geom::Vec2> GetImagePositions() override { return { m_Parent->Factory().GetPositionBySketchItem(c->Ref_1()) }; };
            // Add a coincident constraint image on point of constraint
            //~ if(auto* c = dynamic_cast<Sketch::Coincident_PointToPoint*>(constraint))        { data.images.push_back({ (ConstraintType::Coincident, c->Ref_1()); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Coincident_PointToLine*>(constraint))    { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Coincident_PointToArc*>(constraint))     { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Coincident_PointToCircle*>(constraint))  { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            
            // Add a distance constraint
            //~ else if(auto* c = dynamic_cast<Sketch::Distance_PointToPoint*>(constraint))     { DistanceConstraint(c->Ref_1(), c->Ref_2(), c->distance); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Distance_PointToLine*>(constraint))      { DistanceConstraint(c->Ref_1(), c->Ref_2(), c->distance); } 

            
            //~ // Add midpoint constraint
            //~ else if(auto* c = dynamic_cast<Sketch::AddMidPoint_PointToLine*>(constraint))   { imageAtSketchItem(ConstraintType::Midpoint, c->Ref_1()); }
            
            //~ // Add radius
            //~ else if(auto* c = dynamic_cast<Sketch::AddRadius_Circle*>(constraint))          { RadiusConstraint(c->Ref(), c->radius); } 
            //~ else if(auto* c = dynamic_cast<Sketch::AddRadius_Arc*>(constraint))             { RadiusConstraint(c->Ref(), c->radius); }     

            //~ else if(auto* c = dynamic_cast<Sketch::Angle_LineToLine*>(constraint)) {   
                //~ (void)c;  // TODO: Draw Angle
            //~ } 
            
            //~ // Add Vertical constraint
            //~ else if(auto* c = dynamic_cast<Sketch::Vertical*>(constraint))                  { imageAtSketchItems(ConstraintType::Vertical, c->Ref_1(), c->Ref_2()); }
            //~ // Add Horizontal constraint
            //~ else if(auto* c = dynamic_cast<Sketch::Horizontal*>(constraint))                { imageAtSketchItems(ConstraintType::Horizontal, c->Ref_1(), c->Ref_2()); }
            //~ // Add Parallel constraint
            //~ else if(auto* c = dynamic_cast<Sketch::Parallel*>(constraint))                  { imageAtSketchItems(ConstraintType::Parallel, c->Ref_1(), c->Ref_2()); }
            //~ // Add Perpendicular constraint
            //~ else if(auto* c = dynamic_cast<Sketch::Perpendicular*>(constraint))             { imageAtSketchItems(ConstraintType::Perpendicular, c->Ref_1(), c->Ref_2()); }
            //~ // Add Tangent constraint
            //~ else if(auto* c = dynamic_cast<Sketch::Tangent_Arc_Line*>(constraint))          { imageAtSketchItem(ConstraintType::Tangent, (!c->tangentPoint) ? c->Ref_1() : c->Ref_2()); }
            //~ else if(auto* c = dynamic_cast<Sketch::Tangent_Arc_Arc*>(constraint))           { imageAtSketchItems(ConstraintType::Tangent, c->Ref_1(), c->Ref_2()); }
            //~ // Add Equal constraint
            //~ else if(auto* c = dynamic_cast<Sketch::EqualLength*>(constraint))               { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
            //~ else if(auto* c = dynamic_cast<Sketch::EqualRadius_Arc_Circle*>(constraint))    { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
            //~ else if(auto* c = dynamic_cast<Sketch::EqualRadius_Arc_Arc*>(constraint))       { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
            //~ else if(auto* c = dynamic_cast<Sketch::EqualRadius_Circle_Circle*>(constraint)) { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
            
          
// Coincident: Point To Point

class Coincident_PointToPoint : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToPoint(ElementFactory* parent, SketchItem p0, SketchItem p1)         : Constraint_Template_TwoItems(parent, p0, p1, ConstraintType::Coincident) {}
    // Returns constraint label
    std::string Label() override { return "Coincident"; }
    // Get constraint 2D position
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    // Adds constraint to solver    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};
  
  
  
// Coincident: Point To Line

class Coincident_PointToLine : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToLine(ElementFactory* parent, SketchItem point, SketchItem line)         : Constraint_Template_TwoItems(parent, point, line, ConstraintType::Coincident) {}
    // Returns constraint label
    std::string Label() override { return "Coincident"; }
    // Get constraint 2D position
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    // Adds constraint to solver    
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};
 
// Coincident: Point To Arc

class Coincident_PointToArc : public Constraint_Template_TwoItems
{
public:
    Coincident_PointToArc(ElementFactory* parent, SketchItem point, SketchItem arc)            : Constraint_Template_TwoItems(parent, point, arc, ConstraintType::Coincident) {}
    // Returns constraint label
    std::string Label() override { return "Coincident"; }
    // Get constraint 2D position
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};  
    
// Coincident: Point To Circle  
    
class Coincident_PointToCircle : public Constraint_Template_TwoItems    
{   
public: 
    Coincident_PointToCircle(ElementFactory* parent, SketchItem point, SketchItem circle)   : Constraint_Template_TwoItems(parent, point, circle, ConstraintType::Coincident) {}
    // Returns constraint label
    std::string Label() override { return "Coincident"; }
    // Get constraint 2D position
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};


    
// Distance Point to Point

class Distance_PointToPoint : public Constraint_Template_TwoItems
{
public:                                                                                                                             
    Distance_PointToPoint(ElementFactory* parent, SketchItem p0, SketchItem p1, double d)       : Constraint_Template_TwoItems(parent, p0, p1, ConstraintType::Distance), distance(d) {}
    Distance_PointToPoint(ElementFactory* parent, SketchItem line, double d)                    : Distance_PointToPoint(parent, { SketchItem::Type::Line_P0, line.element }, { SketchItem::Type::Line_P1, line.element }, d) {}
    // Returns constraint label
    std::string Label() override { return "Distance"; }
    // Get constraint 2D position
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    
    MaxLib::Geom::Vec2 GetOffsetDirection() override { return GetDistanceDirection(m_Ref_1, m_Ref_2); }
    
    double distance;
};

// Distance Point to Line

class Distance_PointToLine : public Constraint_Template_TwoItems
{
public:
    Distance_PointToLine(ElementFactory* parent, SketchItem point, SketchItem line, double d)       : Constraint_Template_TwoItems(parent, point, line, ConstraintType::Distance), distance(d) {}
    // Returns constraint label
    std::string Label() override { return "Distance"; }
    // Get constraint 2D position
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    // Adds constraint to solver        
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    
    MaxLib::Geom::Vec2 GetOffsetDirection() override;
    
    double distance;
};


           
           
// Midpoint

class AddMidPoint_PointToLine : public Constraint_Template_TwoItems         
{           
public:         
    AddMidPoint_PointToLine(ElementFactory* parent, SketchItem point, SketchItem line)                    : Constraint_Template_TwoItems(parent, point, line, ConstraintType::Midpoint) {}
    // Returns constraint label
    std::string Label() override { return "Midpoint"; }
    // Get constraint 2D position
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    // Adds constraint to solver                
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};
            
      
// Radius
            
class AddRadius_Circle : public Constraint_Template_OneItem         
{           
public:         
    AddRadius_Circle(ElementFactory* parent, SketchItem circle, double r)                    : Constraint_Template_OneItem(parent, circle, ConstraintType::Radius), radius(r) {}
    // Returns constraint label
    std::string Label() override { return "Radius"; }
    // Adds constraint to solver                
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
    MaxLib::Geom::Vec2 GetOffsetDirection() override { return GetRadiusDirection(m_Ref, radius); }
    
    double radius;
};
    
class AddRadius_Arc : public Constraint_Template_OneItem
{
public:
    AddRadius_Arc(ElementFactory* parent, SketchItem arc, double r)                             : Constraint_Template_OneItem(parent, arc, ConstraintType::Radius), radius(r) {}
    // Returns constraint label
    std::string Label() override { return "Radius"; }
    // Adds constraint to solver                
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
    MaxLib::Geom::Vec2 GetOffsetDirection() override { return GetRadiusDirection(m_Ref, radius); }
    
    double radius;
};          

// Angle

class Angle_LineToLine : public Constraint_Template_TwoItems
{
public:
    Angle_LineToLine(ElementFactory* parent, SketchItem line1, SketchItem line2, double th)    : Constraint_Template_TwoItems(parent, line1, line2, ConstraintType::Angle), angle(th) {}
    // Returns constraint label
    std::string Label() override { return "Angle"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
    MaxLib::Geom::Vec2 GetOffsetDirection() override { return GetAngleDirection(m_Ref_1, m_Ref_2, angle); }
    
    double angle;
};

// Vertical
       
class Vertical : public Constraint_Template_TwoItems     
{       
public:     
    Vertical(ElementFactory* parent, SketchItem p0, SketchItem p1)       : Constraint_Template_TwoItems(parent, p0, p1, ConstraintType::Vertical) {}
    Vertical(ElementFactory* parent, SketchItem line)                    : Vertical(parent, { SketchItem::Type::Line_P0, line.element }, { SketchItem::Type::Line_P1, line.element }) {}
    // Returns constraint label
    std::string Label() override { return "Vertical"; }

    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    
    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
    MaxLib::Geom::Vec2 GetOffsetDirection() override { return { -1.0f, 0.0f }; }
    
    
};

// Horizontal

class Horizontal : public Constraint_Template_TwoItems
{       
public:     
    Horizontal(ElementFactory* parent, SketchItem p0, SketchItem p1)       : Constraint_Template_TwoItems(parent, p0, p1, ConstraintType::Horizontal) {}
    Horizontal(ElementFactory* parent, SketchItem line)                    : Horizontal(parent, { SketchItem::Type::Line_P0, line.element }, { SketchItem::Type::Line_P1, line.element }) {}
    // Returns constraint label
    std::string Label() override { return "Horizontal"; }
    
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;

    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
    MaxLib::Geom::Vec2 GetOffsetDirection() override { return { 0.0f, 1.0f }; }
};


// Parallel

class Parallel : public Constraint_Template_TwoItems
{
public:
    Parallel(ElementFactory* parent, SketchItem line1, SketchItem line2)                          : Constraint_Template_TwoItems(parent, line1, line2, ConstraintType::Parallel) {}
    // Returns constraint label
    std::string Label() override { return "Parellel"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

// Perpendicular

class Perpendicular : public Constraint_Template_TwoItems
{
public:
    Perpendicular(ElementFactory* parent, SketchItem line1, SketchItem line2)                     : Constraint_Template_TwoItems(parent, line1, line2, ConstraintType::Perpendicular) {}
    // Returns constraint label
    std::string Label() override { return "Perpendicular"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

    
// Tangent

class Tangent_Arc_Line : public Constraint_Template_TwoItems
{
public:
    Tangent_Arc_Line(ElementFactory* parent, SketchItem arc, SketchItem line, int tangentPt)        : Constraint_Template_TwoItems(parent, arc, line, ConstraintType::Tangent), tangentPoint(tangentPt) {}
    // Returns constraint label
    std::string Label() override { return "Tangent"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
    // Returns constraint's 2D positon(s)
    std::vector<MaxLib::Geom::Vec2> GetImagePositions() override;
    
    int tangentPoint; // p0 (0) or p1 (1)
};

class Tangent_Arc_Arc : public Constraint_Template_TwoItems
{
public:
    Tangent_Arc_Arc(ElementFactory* parent, SketchItem arc1, SketchItem arc2)                       : Constraint_Template_TwoItems(parent, arc1, arc2, ConstraintType::Tangent) { }
    // Returns constraint label
    std::string Label() override { return "Tangent"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};
// TODO: Check  circle + arc  &  circle + circle


// Equal Length

class EqualLength : public Constraint_Template_TwoItems
{
public:
    EqualLength(ElementFactory* parent, SketchItem line1, SketchItem line2)                       : Constraint_Template_TwoItems(parent, line1, line2, ConstraintType::Equal) {}
    // Returns constraint label
    std::string Label() override { return "Equal"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

// Equal Radius

class EqualRadius_Arc_Circle : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Arc_Circle(ElementFactory* parent, SketchItem arc,SketchItem circle)             : Constraint_Template_TwoItems(parent, arc, circle, ConstraintType::Equal) {}
    // Returns constraint label
    std::string Label() override { return "Equal"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

class EqualRadius_Arc_Arc : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Arc_Arc(ElementFactory* parent, SketchItem arc1, SketchItem arc2)                   : Constraint_Template_TwoItems(parent, arc1, arc2, ConstraintType::Equal) {}
    // Returns constraint label
    std::string Label() override { return "Equal"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};

class EqualRadius_Circle_Circle : public Constraint_Template_TwoItems
{
public:
    EqualRadius_Circle_Circle(ElementFactory* parent, SketchItem circle1, SketchItem circle2) : Constraint_Template_TwoItems(parent, circle1, circle2, ConstraintType::Equal) {}
    // Returns constraint label
    std::string Label() override { return "Equal"; }
    // Adds constraint to solver            
    void AddToSolver(Solver::ConstraintSolver& solver) override;
};



 
} // end namespace Sketch
