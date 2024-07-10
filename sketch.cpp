
#include <iostream>
#include "deps/GLViewer/glcore/deps/imgui_modules/imgui_modules.h" // also ibncludes imgui
#include "sketch.h"
 



        
namespace Sketch {

using namespace MaxLib::String;
using namespace MaxLib::Geom;



PolygonizedGeometry::PolygonizedGeometry(Sketcher* parent, const std::vector<Geometry>& geometries) 
    : m_Parent(parent)
{
    // TODO: Temporary waiting for Geos update
    Geos geos;
    // Polygonize the line data
    Geos::Operation::PolygonizedResult polygonized = geos.operation.Polygonize(geometries);
    // move data into 
    // TODO: handle holes, m_Geometry should be a Polygon
    for(auto& geometry : polygonized.polygons) { 
        m_Geometry.emplace_back(std::move(geometry));
    }
    for(auto& geometry : polygonized.dangles)   { m_Geometry.emplace_back(std::move(geometry)); }
}

            //    std::vector<LineString> valid;
// Clears all of the geometries' hovered flags
void PolygonizedGeometry::ClearHovered() {
    for(auto& geometry : m_Geometry) {
        geometry.m_IsHovered = false;
    }
}
// Clears all of the geometries' seleceted flags
void PolygonizedGeometry::ClearSelected() {
    for(auto& geometry : m_Geometry) {
        geometry.m_IsSelected = false;
    }
}
// Finds geometry within a tolerance to position p and sets their Hovered flag to true
bool PolygonizedGeometry::SetHoveredByPosition(const Vec2& p) {
    return FindIntersects(p, [](SelectablePolygon& geometry) {
        geometry.m_IsHovered = true;
    });
}   
// Finds geometry within a tolerance to position p and sets their Selected flag to true
bool PolygonizedGeometry::SetSelectedByPosition(const Vec2& p) {
    return FindIntersects(p, [](SelectablePolygon& geometry) {
        geometry.m_IsSelected = !geometry.m_IsSelected;
    });
}
 
// Calls callback function on each SelectablePolygon item  
void PolygonizedGeometry::ForEachGeometry(std::function<void(SelectablePolygon&)> cb)  {
    for(auto& geometry : m_Geometry) {
        cb(geometry);
    }
}

// Finds geometry within a tolerance to position p and sets their hovered flag to true
// l is a polygon which 
bool PolygonizedGeometry::FindIntersects(const Vec2& p, std::function<void(SelectablePolygon&)> cb) 
{
    // draw a tolerence ring around point, and check if intersect
    float selectionDistance = m_Parent->Factory().parameters.selectionDistance.ElementScaled();
    LineString p_with_tol = RenderCircle(p, selectionDistance, m_Parent->Factory().parameters.arcTolerance);
    // create instance of geos for calculating intersect 
    Geos geos;
    
    bool itemFound = false;
    ForEachGeometry([&](SelectablePolygon& geometry) 
    {
        if(itemFound) { return; } // skip to end after first item found
        
        // Check whether geometry intersects with p
        if(geos.operation.Intersects(p_with_tol, geometry.Polygon())) {
                cb(geometry);
                // TODO: this just returns first found
                itemFound = true;
        } 
    });
    return itemFound;
}



SketchRenderer::SketchRenderer(Sketcher* parent) 
    : m_Parent(parent) {}
    
    

const RenderData& SketchRenderer::GetRenderData() const { 
    return m_RenderData; 
}
 
 

bool SketchRenderer::UpdateRenderData()
{


    auto RenderElements = [&](RenderData::Data& data, std::function<bool(const Item&)> cb_Condition = [](const Item& item){ (void)item; return true; }) { 
                
        // Clear the old data
        data.Clear();
        // Draw Element / Points
        m_Parent->Factory().ForEachElement([&](Sketch::Element* element) {
            // Point buffer
            Points points;
            // Go through each point
            element->ForEachItemPoint([&](Sketch::Item_Point& item) {
                // If condition met, add point to buffer
                if(cb_Condition(item)) {
                    points.push_back(item.p);                
                }
            });
            // Add Point(s) to render data
            if(!points.empty()) { data.points.push_back(std::move(points)); }
            
            // Return early if condtion not met for element
            if(!cb_Condition(element->Item_Elem())) { return; }
            
            // Line Data
            LineString l = m_Parent->Renderer().RenderElement(element);
            assert(!l.empty() && "No render data in linestring");
            
            // Add Element to renderdata
            data.linestrings.push_back(std::move(l));
        }, true); // include origin point
        
    };
    
    // Render polygon + holes as linestrings
    auto RenderPolygonizedElements = [&](vector<Geometry>& linestrings, std::function<bool(const SelectablePolygon&)> cb_Condition = [](const SelectablePolygon& item){ (void)item; return true; }) { 
                
        // draw polygonized geometry
        m_Parent->Events().m_PolygonizedGeometry.ForEachGeometry([&](SelectablePolygon& geometry) {
                // If condition met, add point to buffer
            if(cb_Condition(geometry)) { 
                // Shell
                linestrings.push_back(geometry.Polygon().shell);
                // Holes
                for(auto& hole : geometry.Polygon().holes) {
                    linestrings.push_back(hole); 
                }
            }
        });
    };
    
    // Render Polygon as Polygon
    auto RenderPolygonizedPolygons = [&](vector<Polygon>& linestrings, std::function<bool(const SelectablePolygon&)> cb_Condition = [](const SelectablePolygon& item){ (void)item; return true; }) { 
                
        // draw polygonized geometry
        m_Parent->Events().m_PolygonizedGeometry.ForEachGeometry([&](SelectablePolygon& geometry) {
                // If condition met, add point to buffer
            if(cb_Condition(geometry)) { 
                linestrings.push_back(geometry.Polygon()); 
            }
        });
    };
 
    auto RenderConstraints = [&](RenderData::Data& data, std::function<bool(const Constraint*)> cb_Condition = [](const Constraint* constraint){ (void)constraint; return true; }) { 
        // Clear the old data
        data.Clear();
        // Draw Element / Points
        m_Parent->Factory().ForEachConstraint([&](Sketch::Constraint* constraint) {
            
            // Return early if condtion not met for constraint
            if(!cb_Condition(constraint)) { return; }
                
            // Points / lines buffer
            Points points;
            LineString l;



 //~ ConstraintType::Coincident        DEFAULT
 //~ ConstraintType::Midpoint          DEFAULT
 //~ ConstraintType::Vertical          { -1.0f, 0.0f }
 //~ ConstraintType::Horizontal        { 0.0f, 1.0f }
 //~ ConstraintType::Parallel          DEFAULT 
 //~ ConstraintType::Perpendicular     DEFAULT will want from points
 //~ ConstraintType::Tangent           DEFAULT     
 //~ ConstraintType::Equal             DEFAULT
 //~ ConstraintType::Distance          DEFAULT
 //~ ConstraintType::Radius            DEFAULT
 //~ ConstraintType::Angle             DEFAULT
 
 
                                   
            
                   
            //~ auto imageAtSketchItem = [&](ConstraintType type, SketchItem item) {
                //~ const Vec2& p = m_Parent->Factory().GetPositionBySketchItem(item);
                //~ data.images.push_back({ type, p, GetOffsetDirection(type, p) });
            //~ };
            //~ auto imageAtSketchItems = [&](ConstraintType type, SketchItem item1, SketchItem item2) {
                //~ const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item1);
                //~ const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(item2);
                //~ Vec2 direction = GetOffsetDirection(type, p0, p1);
                //~ data.images.push_back({ type, p0, direction });
                //~ data.images.push_back({ type, p1, direction });
            //~ };
             
            // Draws a distance & value
            auto DistanceArrow = [&](const Vec2& p0, const Vec2& p1, double distance, const Vec2& pStart) {
                std::string s = MaxLib::String::va_str("%.1f", distance);
                Vec2 pMid = (p0 + p1) / 2.0 + pStart ; // centre point with offset
                data.texts.push_back({ s, pMid });
                
                Vec2 p0A = p0 + pStart;
                Vec2 p1A = p1 + pStart;
                // endpoints of line
                float offset = m_Parent->Factory().parameters.selectionDistance.DimensionArrowOffsetScaled(); 
                Vec2 pA = Geom::PointOnLine(p0A, p1A, offset, p0A);
                Vec2 pB = Geom::PointOnLine(p0A, p1A, -offset, p1A);
                
                // points on line where inner part of arrow starts
                offset = m_Parent->Factory().parameters.selectionDistance.DimensionArrowSizeScaled(); 
                Vec2 pAin = Geom::PointOnLine(pA, pB, offset, pA);
                Vec2 pBin = Geom::PointOnLine(pA, pB, -offset, pB);
                // corner of arrow
                Vec2 offsetPos = (offset / hypot(p0.y-p1.y, p1.x-p0.x)) * Vec2(p0.y-p1.y, p1.x-p0.x) / 2.0;
                 
                 // Cut back lines so we dont cover text. "100.0" == w27 x h17 (half text width + padding)
                float trim_distance = (14.0f + 5.0f) * m_Parent->Factory().parameters.selectionDistance.scaleFactor;
                
                Vec2 pA_endpoint = Geom::PointOnLine(pA, pMid, -trim_distance, pMid);
                Vec2 pB_endpoint = Geom::PointOnLine(pMid, pB, trim_distance, pMid);
                
                // Parallel Dimension Lines
                data.linestrings.push_back({ pA, pA_endpoint });
                data.linestrings.push_back({ pB_endpoint, pB });
                
                // Arrow heads
                data.linestrings.push_back({ pAin+offsetPos, pA, pAin-offsetPos });
                data.linestrings.push_back({ pBin+offsetPos, pB, pBin-offsetPos });
                
                // Perpendicular lines
                data.linestrings.push_back({ p0, p0A });
                data.linestrings.push_back({ p1, p1A });
                
                
            };
                
            auto RadiusConstraint = [&](SketchItem item, double radius, const Vec2& pStart) {  
                // Add a coincident constraint image on each point           
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item);
                const Vec2& p1 = p0 + Vec2(0.0, radius);         //TODO: p1 should actually be chosen point
                DistanceArrow(p0, p1, radius, pStart);
            };
                
                   
            auto DistanceConstraint = [&](SketchItem item1, SketchItem item2, double distance, const Vec2& pStart) {  
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item1);
                const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(item2); 
                DistanceArrow(p0, p1, distance, pStart);
            };     
            auto DistanceConstraintPointToLine = [&](Distance_PointToLine* c) { 
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_1());
                const Vec2& p1 = c->GetEndpoint_DistanceToLine();
                DistanceArrow(p0, p1, c->distance, c->GetScaledOffset()); 
            };  
            auto CentrePosition = [&](SketchItem item1, SketchItem item2) {  
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item1);
                const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(item2);  
                return (p0 + p1) / 2.0;
            };  
            
            // Add a distance constraint
            if(auto* c = dynamic_cast<Sketch::Distance_PointToPoint*>(constraint))      { DistanceConstraint(c->Ref_1(), c->Ref_2(), c->distance, c->GetScaledOffset()); } 
            else if(auto* c = dynamic_cast<Sketch::Distance_PointToLine*>(constraint))  { DistanceConstraintPointToLine(c); }
            // Add radius
            else if(auto* c = dynamic_cast<Sketch::AddRadius_Circle*>(constraint))      { RadiusConstraint(c->Ref(), c->radius, c->GetScaledOffset()); } 
            else if(auto* c = dynamic_cast<Sketch::AddRadius_Arc*>(constraint))         { RadiusConstraint(c->Ref(), c->radius, c->GetScaledOffset()); }   
            // TODO: Draw Angle
            else if(auto* c = dynamic_cast<Sketch::Angle_LineToLine*>(constraint))      { (void)c; }
            // default
            else {
                // Get constraint 2D position(s) - Constraints have 1 or 2 positions
                for(const Vec2& position : constraint->GetImagePositions()) {
                    // TODO: Remove type from data.images
                    ConstraintType type = constraint->GetType();
                    data.images.push_back({ type, position + constraint->GetScaledOffset() });
                }
            }
            
            //~ // Add a coincident constraint image on point of constraint
            //~ if(auto* c = dynamic_cast<Sketch::Coincident_PointToPoint*>(constraint))        { data.images.push_back({ (ConstraintType::Coincident, c->Ref_1()); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Coincident_PointToLine*>(constraint))    { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Coincident_PointToArc*>(constraint))     { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Coincident_PointToCircle*>(constraint))  { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            
            // Add a distance constraint
            //~ else if(auto* c = dynamic_cast<Sketch::Distance_PointToPoint*>(constraint))     { DistanceConstraint(c->Ref_1(), c->Ref_2(), c->distance); } 
            //~ else if(auto* c = dynamic_cast<Sketch::Distance_PointToLine*>(constraint))      { DistanceConstraint(c->Ref_1(), c->Ref_2(), c->distance); } 
            
            // Add midpoint constraint
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
            
          
          // Add Point(s) to render data
          if(!points.empty()) { data.points.push_back(std::move(points)); }
          if(!l.empty()) { data.linestrings.push_back(std::move(l)); }
          
        }); 

    };
    
    // This holds a list of the selected items as sketch_items
    auto UpdateSelectionList = [&]() {
        // Clear old data
        m_Parent->Events().m_SelectedPoints.clear();
        m_Parent->Events().m_SelectedElements.clear();
        m_Parent->Events().m_SelectedPolygons.clear();
        
        // Make array of all the selected points
        m_Parent->Factory().ForEachItemPoint([&](Item_Point& item) {
            if(item.IsSelected()) { m_Parent->Events().m_SelectedPoints.push_back(item.Reference()); }
        }, true); // include origin point
        
        // Make array of all the selected elements
        m_Parent->Factory().ForEachItemElement([&](Item_Element& item) {
            // ignore points as elements (they are handled as points)
            if(item.Type() == SketchItem::Type::Point) { return; }
            if(item.IsSelected()) { m_Parent->Events().m_SelectedElements.push_back(item.Reference()); }
        }, true);
        
        // Make array of all the selected polygonized polygons
        RenderPolygonizedPolygons(m_Parent->Events().m_SelectedPolygons, [](const SelectablePolygon& item) { return item.IsSelected(); });
    };
    
    
    // return if no update required
    if(m_Update == UpdateFlag::None) { return false; }
    
    // A full update includes this, clears input data
    if(m_Update & UpdateFlag::ClearInputData) {
        std::cout << "updating Clear Input Data" << std::endl;          
        // reset preview data
        m_Parent->Events().m_InputData.clear();
        
        SketchItem& previousElement = m_Parent->Events().m_PreviousElement;
        // setting input data so that it can join to previous element
        if(m_Update & UpdateFlag::DontSetInputDataToLastElement) {
            previousElement = {};            
        } 
        // Continue element from end of last element (used for lines and arcs)
        else { 
            if(previousElement.type == SketchItem::Type::Line || previousElement.type == SketchItem::Type::Arc) {
                Vec2 p1 = m_Parent->Factory().GetPositionBySketchItem(previousElement.P1());
                m_Parent->Events().m_InputData.push_back(p1); 
            }
        }

    } 
        
    // A full update includes this, clears selection box, selection / hovered items
    if(m_Update & UpdateFlag::ClearSelection) {
        std::cout << "updating Clear Selection" << std::endl;
        // reset dragged selection box
        m_Parent->Events().m_IsSelectionBox = false;    
        m_Parent->Events().m_IsDimensionClicked = false;    
        // clear the selected / hovered flags on elements (note: flags on polygonized geometry are reset with UpdateFlag::Elements)
        m_Parent->Factory().ClearHovered();
        m_Parent->Factory().ClearSelected(); 
        m_Parent->Factory().ClearHoveredConstraints();
        m_Parent->Factory().ClearSelectedConstraints();
        m_Parent->Events().m_PolygonizedGeometry.ClearSelected();
        m_Parent->Events().m_PolygonizedGeometry.ClearHovered();
    } 
        
                
    RenderData::Data_Selectable& elements = m_RenderData.elements;
        
    // Update Elements
    if(m_Update & UpdateFlag::Elements) {
        std::cout << "updating Elements" << std::endl;
        // Render all items to unselected (no condition)
        RenderElements(elements.unselected);
        // Render all failed to solve items
        RenderElements(elements.failed, [](const Item& item) { return item.IsFailed(); });
        
        // Update the polygonized geometry (Selectable polygons / dangles geometry from all of the element's linestrings)
        m_Parent->Events().m_PolygonizedGeometry = { m_Parent, elements.unselected.linestrings };
    }
    
    RenderData::Data_Selectable& constraints = m_RenderData.constraints;
    
    // Update Constraints
    if(m_Update & UpdateFlag::Constraints) { 
        std::cout << "updating Constraints" << std::endl;
        // Render all items
        RenderConstraints(constraints.unselected);
        // Render all failed to solve constraints
        RenderConstraints(constraints.failed, [](const Constraint* constraint) { return constraint->IsFailed(); });
    }
    
    // Update Selected / Hovered
    if(m_Update & UpdateFlag::Selection) {
        std::cout << "updating Selection" << std::endl;
        
        // Render all hovered items
        RenderElements(elements.hovered,  [](const Item& item) { return item.IsHovered();  });
        // Render all selected items
        RenderElements(elements.selected, [](const Item& item) { return item.IsSelected(); });

        // Render all selected constraints
        RenderConstraints(constraints.selected, [](const Constraint* constraint) { return constraint->IsSelected(); });
        // Render all hovered constraints
        RenderConstraints(constraints.hovered,  [](const Constraint* constraint) { return constraint->IsHovered(); });
        
        // Render all hovered polygonized items
        RenderPolygonizedElements(elements.hovered.linestrings,  [](const SelectablePolygon& item) { return item.IsHovered(); });
        // Render all selected polygonized items
        RenderPolygonizedElements(elements.selected.linestrings, [](const SelectablePolygon& item) { return item.IsSelected(); });
        
        // this holds a list of the selected items as sketch items
        UpdateSelectionList();
    }
    
    
    
    
    
    
    
    
    
  /*
Coincident_PointToPoint
Coincident_PointToLine
Coincident_PointToArc
Coincident_PointToCircle
Distance_PointToPoint(ElementFactory* parent, SketchItem p0, SketchItem p1, double distance)       : Constraint_Template_TwoItems(parent, p0, p1), m_Distance(distance) {}
Distance_PointToLine
AddMidPoint_PointToLine
AddRadius_Circle
AddRadius_Arc
Angle_LineToLine
Vertical(ElementFactory* parent, SketchItem p0, SketchItem p1)       : Constraint_Template_TwoItems(parent, p0, p1) {}
Horizontal(ElementFactory* parent, SketchItem p0, SketchItem p1)       : Constraint_Template_TwoItems(parent, p0, p1) {}
Parallel
Perpendicular
Tangent_Arc_Line
Tangent_Arc_Arc
EqualLength
EqualRadius_Arc_Circle
EqualRadius_Arc_Arc
EqualRadius_Circle_Circle


    C               coincident
    arrow + value   distance
    ==              midpoint
    arc + value     angle
    V               vertical
    H               horizontal
    //              parallel
    |_              perpendicular wants image...
    (_              tangent
    =               equal
  
*/
    
    
  /*
  // Go through each point
  constraint->ForEachItem([&](Sketch::SketchItem& sketchItem) {
      const Vec2& p = GetPositionBySketchItem(sketchItem);
  });*/
    
     
 
      
     
    // Update Preview
    if(m_Update & UpdateFlag::Preview) {
        std::cout << "updating Preview" << std::endl;
        UpdatePreview();
    }
    
    if(m_Update & UpdateFlag::Cursor) { 
        std::cout << "updating Cursor" << std::endl; 

        RenderData::Data& cursor = m_RenderData.cursor;
        // Clear old selection box data
        cursor.Clear();
        
        // Render dragged selection box
        if(m_Parent->Events().m_IsSelectionBox) {
        
            Vec2& p0 = m_Parent->Events().m_CursorClickedPos;
            Vec2& p1 = m_Parent->Events().m_CursorPos;
                
            cursor.points.push_back({ p0, { p0.x, p1.y }, p1, { p1.x, p0.y } });
            cursor.linestrings.push_back({ p0, { p0.x, p1.y }, p1, { p1.x, p0.y }, p0 });
            m_RenderData.selectionBoxDirection = (p1.x > p0.x);
        } 
        //~ else if(m_IsDimensionClicked) {
          //  
        //~ }
        // if not dragging, render cursor as point
        else { 
            cursor.points.push_back({ m_Parent->Events().m_CursorPos });
        }
        // Add position as text
        std::string position = va_str("%.1f,  %.1f", m_Parent->Events().m_CursorPos.x, m_Parent->Events().m_CursorPos.y );
        cursor.texts.push_back({ position, m_Parent->Events().m_CursorPos });
        
    }
    
    // Clear the update flag
    m_Update = UpdateFlag::None;
    std::cout << "Resetting UpdateFlag" << std::endl;
    return true;
}



void SketchRenderer::SetUpdateFlag(UpdateFlag flag) 
{ 
    m_Update = m_Update | flag; 
}
 
LineString SketchRenderer::RenderElement(Sketch::Element* element) 
{
    // Line Data
    LineString l;
    // skip point, as it is added to pointdata 
    if(auto* point = dynamic_cast<const Sketch::Point*>(element))           { l.emplace_back(point->P()); }
    // other element are addded with line buffer 
    else if(auto* line = dynamic_cast<const Sketch::Line*>(element))        { return Geom::RenderLine(line->P0(), line->P1()); }
    else if(auto* arc = dynamic_cast<const Sketch::Arc*>(element))          { return Geom::RenderArc(arc->P0(), arc->P1(), arc->PC(), arc->Direction(), m_Parent->Factory().parameters.arcTolerance); }
    else if(auto* circle = dynamic_cast<const Sketch::Circle*>(element))    { return Geom::RenderCircle(circle->PC(), circle->Radius(), m_Parent->Factory().parameters.arcTolerance); }
    else { assert(0 && "Cannot render element, type unknown"); }            // Should never reach
    
    return std::move(l);
}

LineString SketchRenderer::RenderElementBySketchItem(SketchItem item) 
{
    return RenderElement(m_Parent->Factory().GetElementByID<Sketch::Element>(item.element));
}

// Render preview element 
void SketchRenderer::UpdatePreview() 
{

    //~ auto SnapCursor = []() {
        
        //~ Elements
        //~ Nearest Rounded Value 
        //~ Vertical / Horizontal / 45 deg
    //~ };
    //~ unless presseing ctrl

    
    RenderData::Data& preview = m_RenderData.preview;
    // Clear old preview data
    preview.Clear();
        
    Points points;
    LineString linestring;
    
    SketchEvents::CommandType command   = m_Parent->Events().m_CommandType;
    //~ const Vec2& p                       = SnapCursor(m_Parent->Events().m_CursorPos); 
    const Vec2& p                       = m_Parent->Events().m_CursorPos; 
    std::vector<Vec2>& inputData        = m_Parent->Events().m_InputData;
    Direction& inputDirection            = m_Parent->Events().m_InputDirection;
    
    // Render Point     
    if(command == SketchEvents::CommandType::Add_Point) {
        points.push_back(p);
            
    }
    // Render Line
    else if(command == SketchEvents::CommandType::Add_Line) {
        
        
        if(inputData.size() == 0) {
            points.push_back(p);        
        }
        
        if(inputData.size() == 1) {
        
            Vec2 p1 = p;
            SketchItem previousElement = m_Parent->Events().m_PreviousElement;
            if(previousElement.type == SketchItem::Type::Arc) {
                
                const Vec2& arc_pC = m_Parent->Factory().GetPositionBySketchItem(previousElement.PC());
                const Vec2& arc_p1 = m_Parent->Factory().GetPositionBySketchItem(previousElement.P1());
                // calculate the perpendicular distance from cursor to line pC->p1, this will be the length of the tangent line
                double d = Geom::DistanceBetween(arc_pC, arc_p1, p);
                // determine which way line should go
                Direction direction = Geom::LeftOfLine(arc_pC, arc_p1, p) ? Direction::CCW : Direction::CW;
                // calculate the end point of the line tangent to arc used previously
                p1 = Geom::ArcTangentLine(arc_pC, arc_p1, direction, d);
            } else {             
                auto [c, pNew] = m_Parent->Events().ConstrainPoint_HorizontalVertical(inputData[0], p1);
                //~ constraintType = c;
                p1 = pNew;
            }
            // add points to points buffer            
            points.push_back(inputData[0]);
            points.push_back(p1);        
            // add line to line buffer            
            linestring = RenderLine(inputData[0], p1);
        }
    }
    // Render Arc
    else if(command == SketchEvents::CommandType::Add_Arc) {
        if(inputData.size() == 0) {
            points.push_back(p); 
        }
        else {
             
            if(m_Parent->Events().m_PreviousElement.type == SketchItem::Type::Line) 
            {
                // 1 point has already been added
                if(inputData.size() == 1) {
                    // p0 of last line
                    Vec2 l0 = m_Parent->Factory().GetPositionBySketchItem(m_Parent->Events().m_PreviousElement.P0());
                    // calculate centre point such that arc is tangent to previous line
                    std::optional<Vec2> pC = Geom::ArcCentreFromTangentLine(l0, inputData[0], p);
                    // if centre point found, add the render data
                    if(pC) {                        
                        points.push_back(inputData[0]); // p0
                        points.push_back(p);            // p1
                        points.push_back(*pC);          // pC
                        inputDirection = Geom::LeftOfLine(l0, inputData[0], p) ? Direction::CCW : Direction::CW;
                        linestring = RenderArc(inputData[0], p, *pC, inputDirection, m_Parent->Factory().parameters.arcTolerance);    
                    }
                } 
            }
            else 
            {
                // 1 point has already been added
                if(inputData.size() == 1) {
                    // calculate centre point as mid point between p0 and p1
                    Vec2 pC = (p + inputData[0]) / 2;
                    // add the render data
                    points.push_back(inputData[0]); // P0
                    points.push_back(p);            // p1
                    points.push_back(pC);           // pC
                    linestring = RenderArc(inputData[0], p, pC, inputDirection, m_Parent->Factory().parameters.arcTolerance);    
                }
                // 2 points have already been added
                else if(inputData.size() == 2) {
                    // Calculate centre based on p
                    Vec2 pC = Geom::ArcCentre(inputData[0], inputData[1], p);
                    // add the render data
                    points.push_back(inputData[0]); // p0
                    points.push_back(inputData[1]); // p1
                    points.push_back(pC);           // pC
                    linestring = RenderArc(inputData[0], inputData[1], pC, inputDirection, m_Parent->Factory().parameters.arcTolerance);
                }                
            }
            
        }
    }
    // Render Circle
    else if(command == SketchEvents::CommandType::Add_Circle) {
        if(inputData.size() == 0) {
            points.push_back(p); 
        }
        else if(inputData.size() == 1) {
            points.push_back(inputData[0]); 
            // calculate circle from radius (p to pC)
            linestring = RenderCircle(inputData[0], Hypot(p-inputData[0]), m_Parent->Factory().parameters.arcTolerance);
        }
    }
    // Render square
    else if(command == SketchEvents::CommandType::Add_Square) {
        if(inputData.size() == 0) {
            points.push_back(p); 
        }
        else if(inputData.size() == 1) {
            points.push_back(inputData[0]); 
            points.push_back(p); 
            // calculate circle from radius (p to pC)
            linestring = RenderSquare(inputData[0], p);
        }
    }
    
    if(!points.empty())       { preview.points.push_back(std::move(points)); }
    if(!linestring.empty())   { preview.linestrings.push_back(std::move(linestring)); }
}
    
    





SketchEvents::CommandType SketchEvents::GetCommandType() const { return m_CommandType; }

void SketchEvents::SetCommandType(CommandType commandType) 
{
    // Set command
    m_CommandType = commandType;
    
    // Reset the selection filter    
    m_SelectionFilter = SelectionFilter::All;
    // Update render data (for arc to line or line to arc, use the previous element's data as a start point)
    if(m_PreviousElement.type == SketchItem::Type::Line && m_CommandType == CommandType::Add_Arc) {
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);        
    }
    else if(m_PreviousElement.type == SketchItem::Type::Arc && m_CommandType == CommandType::Add_Line) {
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);        
    }
    else if(m_CommandType == CommandType::None) {
        m_Parent->Renderer().SetUpdateFlag((UpdateFlag)((int)UpdateFlag::Full & ~(int)(UpdateFlag::Constraints))); // full but not constraints
    } else { // Full update
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
    }  
}




void SketchEvents::Event_Keyboard(double time, int key, KeyAction action, KeyModifier modifier) 
{   
    (void)time; (void)modifier;
    // GLFW_KEY_DELETE
    if (key == 261 && action == KeyAction::Press) {
        m_Parent->Factory().DeleteSelection();
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
    }
    // GLFW_KEY_ESCAPE 
    if (key == 256 && action == KeyAction::Press) {
        SetCommandType(SketchEvents::CommandType::Select);
        // update handled in SetCommandType()
    }
}




void SketchEvents::Mouse_Button_Reset() 
{    
    m_MouseButton = MouseButton::None; 
    m_MouseAction = MouseAction::None;
    m_CursorPos = {};
    m_CursorClickedPos = {};
    m_CursorClickedTime = 0.0;
    m_IsDoubleClicked = false;
    m_IsSelectionBox = false; 
    m_IsDimensionClicked = false;
    
    //~ CommandType m_CommandType = CommandType::None;
    //~ Direction m_InputDirection = Direction::CW;
    //~ SelectionFilter m_SelectionFilter = SelectionFilter::All;   // Filters the input selection to points / lines / polygons 
}






// Mouse Button Event
//
//   On Left Click                 
//       -  select item
//       -  if(there is no item), deselect items
//   Left Click w/ Ctrl or Shift
//       -  add to selection
//   On Release
//       -  if(selection box) select these items

bool SketchEvents::Mouse_Button(double time, MouseButton button, MouseAction action, KeyModifier modifier) 
{    
    m_MouseButton = button; 
    m_MouseAction = action;
    // Update cursor clicked pos
    if(button == MouseButton::Left && action == MouseAction::Press) { 
        // Calculate time since last click
        m_IsDoubleClicked = ((time - m_CursorClickedTime) < m_Parent->Factory().parameters.doubleClickTime);
        if(m_IsDoubleClicked) {
            std::cout << "time - m_CursorClickedTime: " << time - m_CursorClickedTime << std::endl;
            
        }
        // Set time of click
        m_CursorClickedTime = time; 
        // Set position of click
        m_CursorClickedPos = m_CursorPos; 
    }

    // TODO: Work out where point should be (i.e. if snapped etc) 
    
    auto Command_ClearSelected = [&]() {
        // Clear selected if mouse is clicked or mouse is released during selection box
        bool isClicked                  = (button == MouseButton::Left && action == MouseAction::Press);
        bool isReleaseOnSelectionBox    = (button == MouseButton::Left) && (action == MouseAction::Release) && m_IsSelectionBox && (m_CursorPos != m_CursorClickedPos); // m_IsSelectionBox gets activated as we click so we need to check it's actually moved else it may not really be a drag
        bool isCtrlOrShift              = (modifier == KeyModifier::Ctrl || modifier == KeyModifier::Shift);
        // Reset selected item if ctrl or shift is not pressed
        if((isClicked || isReleaseOnSelectionBox) && !isCtrlOrShift) {
            m_Parent->Factory().ClearSelected();
            m_Parent->Factory().ClearSelectedConstraints();
            m_PolygonizedGeometry.ClearSelected();
        }
    };
    
    /*
    
    
    auto RenderConstraints = [&](RenderData::Data& data, std::function<bool(const Constraint*)> cb_Condition = [](const Constraint* constraint){ (void)constraint; return true; }) { 
        // Clear the old data
        data.Clear();
        // Draw Element / Points
        m_Parent->Factory().ForEachConstraint([&](Sketch::Constraint* constraint) {
            
            // Return early if condtion not met for constraint
            if(!cb_Condition(constraint)) { return; }
                
            // Points / lines buffer
            Points points;
            LineString l;



 //~ ConstraintType::Coincident        DEFAULT
 //~ ConstraintType::Midpoint          DEFAULT
 //~ ConstraintType::Vertical          { -1.0f, 0.0f }
 //~ ConstraintType::Horizontal        { 0.0f, 1.0f }
 //~ ConstraintType::Parallel          DEFAULT 
 //~ ConstraintType::Perpendicular     DEFAULT will want from points
 //~ ConstraintType::Tangent           DEFAULT     
 //~ ConstraintType::Equal             DEFAULT
 //~ ConstraintType::Distance          DEFAULT
 //~ ConstraintType::Radius            DEFAULT
 //~ ConstraintType::Angle             DEFAULT
 

            auto PerpendicularOffset = [&](const Vec2& p0, const Vec2& p1) {
                return PointPerpendicularToLine(p0, p1, 1.0, { 0.0f, 0.0f });
            };
 
            auto GetOffsetDirection = [&](ConstraintType type, const Vec2& p0, const Vec2 p1 = {}) {
                // TODO: Will need point information
                // Vertical
                if(type == ConstraintType::Vertical)           { return Vec2{ -1.0f, 0.0f }; }
                // Horizontal
                else if(type == ConstraintType::Horizontal)    { return Vec2{ 0.0f, 1.0f }; }
                // Distance
                else if(type == ConstraintType::Distance)      { return PerpendicularOffset(p0, p1); }
                // Default direction
                else                                                    { return Vec2{ -0.707f, 0.707f }; }
            };                                             
            
                   
                   
                   
            auto imageAtSketchItem = [&](ConstraintType type, SketchItem item) {
                const Vec2& p = m_Parent->Factory().GetPositionBySketchItem(item);
                data.images.push_back({ type, p, GetOffsetDirection(type, p) });
            };
            auto imageAtSketchItems = [&](ConstraintType type, SketchItem item1, SketchItem item2) {
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item1);
                const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(item2);
                Vec2 direction = GetOffsetDirection(type, p0, p1);
                data.images.push_back({ type, p0, direction });
                data.images.push_back({ type, p1, direction });
            };
            
            // Draws a distance & value
            auto DistanceArrow = [&](const Vec2& p0, const Vec2& p1, double distance) {
                std::string s = MaxLib::String::va_str("%.1f", distance);
                Vec2 midPoint = (p0 + p1) / 2.0;
                Vec2 direction = GetOffsetDirection(ConstraintType::Distance, p1 - p0);
                data.texts.push_back({ s, midPoint, direction });
                // TODO: AND ADD LINESTRINGS of double ended arrow
            };
                   
            auto DistanceConstraint = [&](SketchItem item1, SketchItem item2, double distance) {  
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item1);
                const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(item2);         //TODO: p1 should actually be perpendicular point  
                DistanceArrow(p0, p1, distance);
            };  
            auto RadiusConstraint = [&](SketchItem item, double radius) {  
                // Add a coincident constraint image on each point           
                const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(item);
                const Vec2& p1 = p0 + Vec2(0.0, radius);         //TODO: p1 should actually be chosen point
                DistanceArrow(p0, p1, radius);
            };
                
            
            // Add a coincident constraint image on point of constraint
            if(auto* c = dynamic_cast<Sketch::Coincident_PointToPoint*>(constraint))        { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            else if(auto* c = dynamic_cast<Sketch::Coincident_PointToLine*>(constraint))    { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            else if(auto* c = dynamic_cast<Sketch::Coincident_PointToArc*>(constraint))     { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            else if(auto* c = dynamic_cast<Sketch::Coincident_PointToCircle*>(constraint))  { imageAtSketchItem(ConstraintType::Coincident, c->Ref_1()); } 
            
            // Add a distance constraint
            else if(auto* c = dynamic_cast<Sketch::Distance_PointToPoint*>(constraint))     { DistanceConstraint(c->Ref_1(), c->Ref_2(), c->distance); } 
            else if(auto* c = dynamic_cast<Sketch::Distance_PointToLine*>(constraint))      { DistanceConstraint(c->Ref_1(), c->Ref_2(), c->distance); } 
            
            // Add midpoint constraint
            else if(auto* c = dynamic_cast<Sketch::AddMidPoint_PointToLine*>(constraint))   { imageAtSketchItem(ConstraintType::Midpoint, c->Ref_1()); }
            
            // Add radius
            else if(auto* c = dynamic_cast<Sketch::AddRadius_Circle*>(constraint))          { RadiusConstraint(c->Ref(), c->radius); } 
            else if(auto* c = dynamic_cast<Sketch::AddRadius_Arc*>(constraint))             { RadiusConstraint(c->Ref(), c->radius); }     

            else if(auto* c = dynamic_cast<Sketch::Angle_LineToLine*>(constraint)) {   
                (void)c;  // TODO: Draw Angle
            } 
            
            // Add Vertical constraint
            else if(auto* c = dynamic_cast<Sketch::Vertical*>(constraint))                  { imageAtSketchItems(ConstraintType::Vertical, c->Ref_1(), c->Ref_2()); }
            // Add Horizontal constraint
            else if(auto* c = dynamic_cast<Sketch::Horizontal*>(constraint))                { imageAtSketchItems(ConstraintType::Horizontal, c->Ref_1(), c->Ref_2()); }
            // Add Parallel constraint
            else if(auto* c = dynamic_cast<Sketch::Parallel*>(constraint))                  { imageAtSketchItems(ConstraintType::Parallel, c->Ref_1(), c->Ref_2()); }
            // Add Perpendicular constraint
            else if(auto* c = dynamic_cast<Sketch::Perpendicular*>(constraint))             { imageAtSketchItems(ConstraintType::Perpendicular, c->Ref_1(), c->Ref_2()); }
            // Add Tangent constraint
            else if(auto* c = dynamic_cast<Sketch::Tangent_Arc_Line*>(constraint))          { imageAtSketchItem(ConstraintType::Tangent, (!c->tangentPoint) ? c->Ref_1() : c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::Tangent_Arc_Arc*>(constraint))           { imageAtSketchItems(ConstraintType::Tangent, c->Ref_1(), c->Ref_2()); }
            // Add Equal constraint
            else if(auto* c = dynamic_cast<Sketch::EqualLength*>(constraint))               { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::EqualRadius_Arc_Circle*>(constraint))    { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::EqualRadius_Arc_Arc*>(constraint))       { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
            else if(auto* c = dynamic_cast<Sketch::EqualRadius_Circle_Circle*>(constraint)) { imageAtSketchItems(ConstraintType::Equal, c->Ref_1(), c->Ref_2()); }
        
        }); 

    };
    
    
    */
    
    
    
    
    
    
    
    
    
    
    // Select Tool
    auto Command_Select = [&]() 
    {
        if(button == MouseButton::Left && action == MouseAction::Press) {
            // get closest constrsint
            ConstraintType type = m_Parent->Factory().SetSelectedConstraintByPosition(m_CursorPos);
            // if dimension / radius etc
            m_IsDimensionClicked = ConstaintHasInput(type);
            
            if(type != ConstraintType::None) {
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Constraints);
            }
            
            bool success = (bool)type;
            // if couldnt select constraint, set selected geometry instead
            if(!success) { success |= m_Parent->Factory().SetSelectedByPosition(m_CursorPos, m_SelectionFilter); }
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
            return success;
        }
        else if(button == MouseButton::Left && action == MouseAction::Release) {
            
            // cursor clicked = cursor release (no drag) bounding box has no size
            if(m_CursorPos == m_CursorClickedPos)  { m_IsSelectionBox = false; }
            
            // If we have dragged the cursor whilst button was pressed
            if(m_IsSelectionBox) {
                // include geometry partially inside selection box of fully contained within
                bool includePartiallyInside = (m_CursorPos.x - m_CursorClickedPos.x) < 0;
                // Find selection inside selection box
                bool success = m_Parent->Factory().AddSelectionBetween(m_CursorPos, m_CursorClickedPos, m_SelectionFilter, includePartiallyInside);
                m_IsSelectionBox = false;                
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection | UpdateFlag::Cursor);
                return success;
            }
            
            // Drag dimension
            if(m_IsDimensionClicked) {
                // cursor clicked = cursor release (no drag)
                //~ if(m_CursorPos == m_CursorClickedPos)  {
                if(m_IsDoubleClicked)  {
                    // Open dimension popup on if mouse click and mouse release are the same
                    m_Parent->popup_ConstraintDimension.Open();
                    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Constraints);
                } 
                // mouse release
                m_IsDimensionClicked = false;
            }
        }  
        return false;
    };
    
    auto Command_SelectPolygonize = [&]() {
         
        if(button == MouseButton::Left && action == MouseAction::Press) {
            // selected item at cursorpos
             bool success = m_PolygonizedGeometry.SetSelectedByPosition(m_CursorPos);
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
            return success;
        }  
        return false;
    };
    
    // Return if no command set
    if(m_CommandType == CommandType::None) { return false; } // do nothing - this assumes we dont need to update if only the cursor position has changed
    // Handle Select Command
    else if(m_CommandType == CommandType::Select) {
        // Clear the selection data for points, elements and polygons
        Command_ClearSelected();
        // Try to select something with the select tool
        // We start on SelectionFilter::All, the filter changes to the same type as the type selected 
        bool success = Command_Select();
        
        // start dragging selection box if no item under cursor
        if(!success && (button == MouseButton::Left) && (action == MouseAction::Press)) {
            m_IsSelectionBox = true;
        }
    }
    
    // Handle Select loop Command
    else if(m_CommandType == CommandType::SelectLoop) {
        // Clear the selection data for points, elements and polygons
        Command_ClearSelected();
        // TODO: combine mouse button press
        
        // We start on SelectionFilter::All, the filter changes to the same type as the type selected 
        // Try to select an element with the select tool
        bool success = Command_Select();
        // Set the selction filter to the type of the first item clicked
        if((button == MouseButton::Left && action == MouseAction::Press) || (button == MouseButton::Left && action == MouseAction::Release)) {
            if(success && (m_SelectionFilter == SelectionFilter::All)) { m_SelectionFilter = SelectionFilter::Basic; }
        }
        // If no elements found, look for polygons 
        if(!success && (m_SelectionFilter & SelectionFilter::Polygons)) { 
            success |= Command_SelectPolygonize();
            // Set the selction filter to the type of the first item clicked
            if(button == MouseButton::Left && action == MouseAction::Press) {
                if(success && (m_SelectionFilter == SelectionFilter::All)) { m_SelectionFilter = SelectionFilter::Polygons; }
            }
        }
        // if neither elements or polgons were found, 
        if(!success && (button == MouseButton::Left) && (action == MouseAction::Press)) {
            // Start dragging selection box
            m_IsSelectionBox = true;
            // And we're not selecting multiple items, reset the selection filter
            if((modifier != KeyModifier::Ctrl) && (modifier != KeyModifier::Shift)) { 
                m_SelectionFilter = SelectionFilter::All; 
            }
        }
        return success;
    }
    // Handle add element commands
    else if(m_CommandType == CommandType::Add_Point || m_CommandType == CommandType::Add_Line || m_CommandType == CommandType::Add_Arc || m_CommandType == CommandType::Add_Circle || m_CommandType == CommandType::Add_Square) 
    {
        ConstraintType constraintType = ConstraintType::None;
        bool needsSolve = false; // flag if solve constraints is required
        ElementFactory& f = m_Parent->Factory();
        
        if(button == MouseButton::Left && action == MouseAction::Press) {
             
        /*            
                       inputdata.Size     Last Point
            point   -       N/A
            line    -   1               if(m_CursorClickedPos == m_InputData[0]) // dont allow p1 on p0
            arc     -   2               if(m_CursorClickedPos == m_InputData[0] || m_CursorClickedPos == m_InputData[1]) // dont allow pC on p0 or p1
            circle  -   1               if(m_CursorClickedPos == m_InputData[0]) // dont allow p on pC (0 radius)
        */
            
            m_InputData.push_back(m_CursorClickedPos); 
            
            
            // Handle Add Point     
            if(m_CommandType == CommandType::Add_Point) 
            {
                // check that input data is the corrent size
                if(m_InputData.size() != 1) { return false; }
                // Add point
                SketchItem point = f.AddPoint(m_InputData[0]);
                // Constrain Point to closest element / point
                if(f.ConstrainPointToClosest(point, m_InputData[0])) { needsSolve = true; }
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
            }
            // Handle Add Line
            else if(m_CommandType == CommandType::Add_Line) {
                 
                // check that input data is the corrent size
                if(m_InputData.size() != 2) { return false; }
                // make sure points aren't the same
                if(m_InputData[1] == m_InputData[0]) { m_InputData.pop_back(); return false; }
                 
                Vec2 p1 = m_InputData[1];
                
                // Update point to be tangent to arc
                if(m_PreviousElement.type == SketchItem::Type::Arc) {
                    const Vec2& arc_pC = f.GetPositionBySketchItem(m_PreviousElement.PC());
                    const Vec2& arc_p1 = f.GetPositionBySketchItem(m_PreviousElement.P1());
                    // calculate the perpendicular distance from cursor to line pC->p1, this will be the length of the tangent line
                    double d = Geom::DistanceBetween(arc_pC, arc_p1, m_InputData[1]);
                    // determine which way line should go
                    Direction direction = Geom::LeftOfLine(arc_pC, arc_p1, m_InputData[1]) ? Direction::CCW : Direction::CW;
                    // calculate the end point of the line tangent to arc used previously
                    p1 = Geom::ArcTangentLine(arc_pC, arc_p1, direction, d);
                } 
                // Check if Vertical / Horixontal
                else {  
                    auto [c, p] = m_Parent->Events().ConstrainPoint_HorizontalVertical(m_InputData[0], m_InputData[1]);
                    constraintType = c;
                    p1 = p;
                }
            
                // create new Line
                SketchItem line = f.AddLine(m_InputData[0], p1);
                 
                // constraint to beggining of our new line
                if(m_PreviousElement.type == SketchItem::Type::Line) {
                    // add constraint between end of last line to beginning of this line
                    f.AddConstraint<Coincident_PointToPoint>(m_PreviousElement.P1(), line.P0());
                }  
                // tangent constraint to beggining of our new line
                else if(m_PreviousElement.type == SketchItem::Type::Arc) {
                    // Constrain the end point of previous line to the start point of the new arc
                    f.AddConstraint<Coincident_PointToPoint>(m_PreviousElement.P1(), line.P0());
                    // add tangent constraint between the previous line and the beginning of the new arc
                    f.AddConstraint<Tangent_Arc_Line>(m_PreviousElement, line, 1); // to p1
                } 
                else { // Constrain p0 of line to nearby existing geometry (this is covered by the above constraints ) 
                    if(f.ConstrainPointToClosest(line.P0(), m_InputData[0])) { needsSolve = true; }
                }
                // Constrain p1 of line to nearby existing geometry
                if(f.ConstrainPointToClosest(line.P1(), p1)) { needsSolve = true; } 
                
                // Add vertical constraint
                if(constraintType == ConstraintType::Vertical) { 
                    f.AddConstraint<Vertical>(line.P0(), line.P1()); 
                } // Add horizontal constraint
                else if(constraintType == ConstraintType::Horizontal) {
                    f.AddConstraint<Horizontal>(line.P0(), line.P1()); 
                }
                
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);
                // Set last item for next time
                m_PreviousElement = line;
                 
            }
            // Handle Add Arc
            else if(m_CommandType == CommandType::Add_Arc) {
                // continue tangent to previous line
                if(m_PreviousElement.type == SketchItem::Type::Line) {
                    
                    // check that input data is the corrent size
                    if(m_InputData.size() != 2) { return false; }
                    // make sure points aren't the same
                    if(m_InputData[1] == m_InputData[0]) { m_InputData.pop_back(); return false; }
                    // Calculate centre from point
                    std::optional<Vec2> newCentre = Geom::ArcCentreFromTangentLine(f.GetPositionBySketchItem(m_PreviousElement.P0()), m_InputData[0], m_InputData[1]);
                    // make sure points aren't the same
                    if(!newCentre) { m_InputData.pop_back(); return false; }                       
                    // Create new Arc
                    SketchItem arc = f.AddArc(m_InputData[0], m_InputData[1], *newCentre, m_Parent->Events().m_InputDirection); 
                    // Constrain the end point of previous line to the start point of the new arc
                    f.AddConstraint<Coincident_PointToPoint>(m_PreviousElement.P1(), arc.P0());
                    // add tangent constraint between the previous line and the beginning of the new arc
                    f.AddConstraint<Tangent_Arc_Line>(arc, m_PreviousElement, 0);
                    
                    // Constrain p1 & pC of arc to nearby existing geometry (p0 is covered by the previous click) 
                    if(f.ConstrainPointToClosest(arc.P1(), m_InputData[1])) { needsSolve = true; }
                    if(f.ConstrainPointToClosest(arc.PC(), m_InputData[2])) { needsSolve = true; } 
                    
                    // Update render data
                    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_SetInputDataToLastElement);
                    // Set last item for next time
                    m_PreviousElement = arc;
                }  
                // start new arc
                else {
                    // check that input data is the corrent size
                    if(m_InputData.size() != 3) { return false; }
                    // make sure points aren't the same
                    if(m_InputData[2] == m_InputData[0] || m_InputData[2] == m_InputData[1]) { m_InputData.pop_back(); return false; }
                    // Calculate closest possible centre point from input centre point
                    Vec2 newCentre = Geom::ArcCentre(m_InputData[0], m_InputData[1], m_InputData[2]);
                    // Create new Arc
                    SketchItem arc = f.AddArc(m_InputData[0], m_InputData[1], newCentre, m_Parent->Events().m_InputDirection);
                    // Constrain p0 of line to nearby existing geometry (this is covered by the above constraints ) 
                    if(f.ConstrainPointToClosest(arc.P0(), m_InputData[0])) { needsSolve = true; }
                    if(f.ConstrainPointToClosest(arc.P1(), m_InputData[1])) { needsSolve = true; }
                    if(f.ConstrainPointToClosest(arc.PC(), m_InputData[2])) { needsSolve = true; } 
                    // Update render data
                    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
                    // Set last item for next time
                    m_PreviousElement = arc;
                }
                
            
            }
            // Handle Add Circle
            else if(m_CommandType == CommandType::Add_Circle) {
                // check that input data is the corrent size
                if(m_InputData.size() != 2) { return false; }
                
                // make sure points aren't the same
                if(m_InputData[1] == m_InputData[0]) { m_InputData.pop_back(); return false; }
                
                double radius = Hypot(m_InputData[1] - m_InputData[0]);
                SketchItem circle = f.AddCircle(m_InputData[0], radius);
                
                // Constrain pCentre of circle to nearby existing geometry 
                if(f.ConstrainPointToClosest(circle.PC(), m_InputData[0])) { needsSolve = true; }
                
                // TODO: Constrain circle to nearby existing geometry
                //if(f.ConstrainCircleToClosest(circle, m_InputData[1])) { needsSolve = true; }
                
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
            }
            // Handle Add Square
            else if(m_CommandType == CommandType::Add_Square) {
                // check that input data is the corrent size
                if(m_InputData.size() != 2) { return false; }
                
                // make sure points aren't the same
                if(m_InputData[1] == m_InputData[0]) { m_InputData.pop_back(); return false; }
                                
                Vec2 pA = { m_InputData[0].x, m_InputData[1].y };
                Vec2 pB = { m_InputData[1].x, m_InputData[0].y };
                
                SketchItem line1 = f.AddLine(m_InputData[0], pA);
                SketchItem line2 = f.AddLine(pA, m_InputData[1]);
                SketchItem line3 = f.AddLine(m_InputData[1], pB);
                SketchItem line4 = f.AddLine(pB, m_InputData[0]);
                
                // Constrain the end point of previous line to the start point of the new arc
                f.AddConstraint<Coincident_PointToPoint>(line1.P1(), line2.P0());
                f.AddConstraint<Coincident_PointToPoint>(line2.P1(), line3.P0());
                f.AddConstraint<Coincident_PointToPoint>(line3.P1(), line4.P0());
                f.AddConstraint<Coincident_PointToPoint>(line4.P1(), line1.P0());
                
                f.AddConstraint<Vertical>(line1);
                f.AddConstraint<Horizontal>(line2);
                f.AddConstraint<Vertical>(line3);
                f.AddConstraint<Horizontal>(line4);
                
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full);
            }
            else {
                assert(0 && "Command doesn't exist");
            }
        }
        // Solve since we added constraints
        if(needsSolve) { 
            m_Parent->SolveConstraints(); 
        }
    }
    return true;
}

// Mouse Move Event
//
//  Input should be (x, y) coords in sketch space
//
//  Hover over item -  Highlights it
//  Drag            -  if (clicked)
//                        -  if(Selected) drag items
//                        -  if(!selected) drag selection box

// return true if update required
bool SketchEvents::Mouse_Move(double time, const Vec2& p)
{ 
    (void)time;
    // TODO: Work out where point should be (i.e. if snapped etc)
    // std::cout << "m_SelectionFilter: " << (int)m_SelectionFilter << std::endl;
    // Reset the mouse button / action
    if(m_MouseAction == MouseAction::Release) { m_MouseButton = MouseButton::None; m_MouseAction = MouseAction::None; }
    
    //~ std::cout << "m_MouseAction: " << (int)m_MouseAction << std::endl;
    
    
    
    // return early if no change to p, to prevent solving constraints unnecessarily 
    if(p == m_CursorPos) { return false; }
    // Update cursor
    Vec2 pDif = p - m_CursorPos;
    m_CursorPos = p;
    // Update render data
    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Cursor);
    
    
    auto Command_ClearSelected = [&]() {
        // Is mouse moving but no buttons pressed, or dragging a selection box?
        bool isNoMouseButton         = m_MouseButton == MouseButton::None;
        bool isDraggingSelectionBox  = (m_MouseButton == MouseButton::Left) && (m_MouseAction == MouseAction::Press) && m_IsSelectionBox && (m_CursorPos != m_CursorClickedPos);
        // Clear the hovered points & elements and polygons
        if(isNoMouseButton || isDraggingSelectionBox) { // TODO: is this meant to be !isDraggingSelectionBox?
            m_Parent->Factory().ClearHovered();
            m_Parent->Factory().ClearHoveredConstraints();
            m_Parent->Events().m_PolygonizedGeometry.ClearHovered();
        }
    };
    
    
    auto Command_Select = [&](bool isDragEnabled = false) {
        bool success = false;
         // Highlight item if hovered over
        if(m_MouseButton == MouseButton::None) {
            // Find hovered constraints 
            success |= m_Parent->Factory().SetHoveredConstraintByPosition(m_CursorPos);
            // Find hovered elements 
            if(!success) { success |= m_Parent->Factory().SetHoveredByPosition(m_CursorPos, m_SelectionFilter); }
            // Update render data
            if(success) { m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection); }
            
        }
        
        if(m_MouseButton == MouseButton::Left && m_MouseAction == MouseAction::Press) {
            // Dragging selection box
            if(m_IsSelectionBox) {
                
                bool includePartiallyInside = (m_CursorPos.x - m_CursorClickedPos.x) < 0;
                // Find selection inside selection box
                success |= m_Parent->Factory().AddHoveredBetween(m_CursorPos, m_CursorClickedPos, m_SelectionFilter, includePartiallyInside);               
                // Update render data
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
            } 
            else if(m_IsDimensionClicked) {
                //~ SetSelectedConstraint();
                Vec2 pDif = m_CursorPos - m_CursorClickedPos;
                
                
                
                // Drag dimension constraint
                m_Parent->Factory().ForEachConstraint([&](Sketch::Constraint* constraint) {
                    // if the constraint is selected
                    if(constraint->IsSelected()) {
                        
                        
                        auto SetDimensionOffset = [&](const Vec2& p0, const Vec2& p1) {  
                            // calculate distance between cursor and line made by constraint points
                            constraint->offsetDistance = Geom::DistanceBetween(p0, p1, m_CursorPos) / m_Parent->Factory().parameters.selectionDistance.scaleFactor; 
                            // Flip if other side of line
                            if(!Geom::LeftOfLine(p0, p1, m_CursorPos)) { constraint->offsetDistance = -constraint->offsetDistance; }
                        };   
                        
                        //~ c->distance, c->radius, c->GetScaledOffset()
                        
                        // Add a distance constraint
                        if(auto* c = dynamic_cast<Sketch::Distance_PointToPoint*>(constraint))      { 
                            const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_1());
                            const Vec2& p1 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_2());
                            SetDimensionOffset(p0, p1); 
                        } 
                        else if(auto* c = dynamic_cast<Sketch::Distance_PointToLine*>(constraint))  { 
                            const Vec2& p0 = m_Parent->Factory().GetPositionBySketchItem(c->Ref_1());
                            const Vec2& p1 = c->GetEndpoint_DistanceToLine(); // project p0 onto line
                            SetDimensionOffset(p0, p1); 
                        } 
                        // Add radius
                        else if(auto* c = dynamic_cast<Sketch::AddRadius_Circle*>(constraint))      { 
                            const Vec2& pC = m_Parent->Factory().GetPositionBySketchItem(c->Ref());
                            const Vec2& p1 = pC + Vec2{ 0.0f, c->radius };
                            SetDimensionOffset(pC, p1); 
                        } 
                        // RadiusConstraint(c->Ref(), c->radius, c->GetScaledOffset()); } 
                        else if(auto* c = dynamic_cast<Sketch::AddRadius_Arc*>(constraint))         { 
                            const Vec2& pC = m_Parent->Factory().GetPositionBySketchItem(c->Ref());
                            const Vec2& p1 = pC + Vec2{ 0.0f, c->radius };
                            SetDimensionOffset(pC, p1); 
                        }   
                        //~ // TODO: Draw Angle
                        //~ else if(auto* c = dynamic_cast<Sketch::Angle_LineToLine*>(constraint)) {   
                            //~ (void)c;
                        //~ }
                        // default
                        else {
                            // Do nothing...
                        }
                        
                        
                    
                        //~ constraint->GetOffsetDirection();
                        //~ Geom::PointOnLine();
                        //~ Geom::IntersectLines(p0, p1, p2, p3);
                        //~ constraint->offsetDistance = MaxLib::Geom::Hypot(pDif); // TODO MAKE PERPENDICULAR DISTANMECV
                    }
                }); 
            
                
                m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Constraints | UpdateFlag::Selection);
            }
            else {  
                // drag a point 
                if(isDragEnabled) {
                    std::cout << "p: " << p << std::endl;
                    m_Parent->SolveConstraints(pDif);
                    // Update render data (selection is required for knowing which is dragged)
                    m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Full_DontClearSelection);
                    return true;
                }
            }
        }
        return success;
    };
    
    auto Command_SelectPolygonize = [&]() {
        
        // Highlight item if hovered over
        if(m_MouseButton == MouseButton::None) { 
            bool success = m_Parent->Events().m_PolygonizedGeometry.SetHoveredByPosition(m_CursorPos);
            // Update render data
            m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Selection);
            return success;
        }
        return false;
    };
    
    // Return if no command set
    if(m_CommandType == CommandType::None) { return false;} // do nothing - this assumes we dont need to update if only the cursor position has changed
    // Hover / drag / 
    else if(m_CommandType == CommandType::Select) {
        // Clear the selection data for points, elements and polygons
        Command_ClearSelected();
        // Highlight item if hovered over
        return Command_Select(true); // drag enabled
    } 
    // Hover / drag / 
    else if(m_CommandType == CommandType::SelectLoop) {
        // Clear the selection data for points, elements and polygons
        Command_ClearSelected();
        // Highlight an element with the select tool if mouse over
        // We start on SelectionFilter::All, the filter changes to the same type as the type selected 
        bool success = Command_Select();
        std::cout << "element found success: " << success << std::endl;
        // If no elements found, look for polygons 
        if(!success && (m_SelectionFilter & SelectionFilter::Polygons)) { 
            success |= Command_SelectPolygonize();
            std::cout << "polgon found success: " << success << std::endl << std::endl;
        } 
        return success;
    } 
    // Preview New Element
    else if(m_CommandType == CommandType::Add_Point || m_CommandType == CommandType::Add_Line || m_CommandType == CommandType::Add_Arc || m_CommandType == CommandType::Add_Circle || m_CommandType == CommandType::Add_Square) {
        // update the preview render data from the cursorPos
        m_Parent->Renderer().SetUpdateFlag(UpdateFlag::Preview);
    }
    
    return false;
} 

// Points are in world space // TODO: This should move to other constraints? e.g. tangent
std::pair<ConstraintType, Vec2> SketchEvents::ConstrainPoint_HorizontalVertical(const Vec2& p0, const Vec2& p1) {
    
    Vec2 dif = Abs(p1 - p0);
    float selectionDistance = m_Parent->Factory().parameters.selectionDistance.HorizVertScaled(); 
    
    if(dif.x < selectionDistance) { 
        return { ConstraintType::Vertical,  Vec2(p0.x, p1.y) };
    }
    else if(dif.y < selectionDistance) { 
        return { ConstraintType::Horizontal, Vec2(p1.x, p0.y) };
    }
    return { ConstraintType::None, p1 };
} 



 bool ConstraintButtons::DrawImGui(std::function<bool(const std::string&, ConstraintType)> cb_ImageButton, std::function<void(double*)> cb_InputValue) 
{
    const std::vector<SketchItem>& points = m_Parent->Events().GetSelectedPoints();
    const std::vector<SketchItem>& elements = m_Parent->Events().GetSelectedElements();
    
    ElementFactory& factory = m_Parent->Factory();
    
    // return ealry if nothing selected
    if(points.empty() && elements.empty()) { return false; }
                    
    bool needsUpdate = false;
    
    // 0 elements selected
    if(elements.empty()) 
    {
        // 1 points selected
        if(points.size() == 1) {
            // Fix point constraint
        }
        // 2 points selected
        else if(points.size() == 2) {
            // Add Coincident constraint between 2 points
            if(cb_ImageButton("Coincident", ConstraintType::Coincident)) {
                factory.AddConstraint<Coincident_PointToPoint>(points[0], points[1]);
                needsUpdate = true;
            }
                
            ImGui::SameLine();
            // Add Horizontal constraint between 2 points
            if(cb_ImageButton("Horizontal", ConstraintType::Horizontal)) {
                factory.AddConstraint<Horizontal>(points[0], points[1]);
                needsUpdate = true;
            }
                
            ImGui::SameLine();
            // Add Vertical constraint between 2 points
            if(cb_ImageButton("Vertical", ConstraintType::Vertical)) {
                factory.AddConstraint<Vertical>(points[0], points[1]);
                needsUpdate = true;
            }
                
            ImGui::SameLine();
            // Add Distance constraint between 2 points
            if(cb_ImageButton("Distance", ConstraintType::Distance)) {
                factory.AddConstraint<Distance_PointToPoint>(points[0], points[1], m_Distance);
                needsUpdate = true;
            }
            ImGui::SameLine();
            // Draw inputbox
            cb_InputValue(&m_Distance);
        }
    } 
    // 1 element selected
    else if(elements.size() == 1) {
        
        // 0 points and 1 element selected
        if(points.size() == 0) 
        {
            if(elements[0].type == SketchItem::Type::Line) {
                
                // Add Horizontal constraint of Line
                if(cb_ImageButton("Horizontal", ConstraintType::Horizontal)) {
                    factory.AddConstraint<Horizontal>(elements[0]);
                    needsUpdate = true;
                }
                    
                ImGui::SameLine();
                // Add Vertical constraint of Line
                if(cb_ImageButton("Vertical", ConstraintType::Vertical)) {
                    factory.AddConstraint<Vertical>(elements[0]);
                    needsUpdate = true;
                }
                    
                ImGui::SameLine();
                // Add Distance constraint of Line    
                if(cb_ImageButton("Distance", ConstraintType::Distance)) {   
                    factory.AddConstraint<Distance_PointToPoint>(elements[0], m_Distance);
                    needsUpdate = true;
                }
                ImGui::SameLine();
                // Draw inputbox
                cb_InputValue(&m_Distance);
                
            }
            else if(elements[0].type == SketchItem::Type::Arc) {
                // Add Radius constraint of Arc                
                if(cb_ImageButton("Radius", ConstraintType::Radius)) {
                    factory.AddConstraint<AddRadius_Arc>(elements[0], m_Radius);
                    needsUpdate = true;
                }
                ImGui::SameLine();
                // Draw inputbox
                cb_InputValue(&m_Radius);
            }
            else if(elements[0].type == SketchItem::Type::Circle) {
                // Add Radius constraint of Circle                 
                if(cb_ImageButton("Radius", ConstraintType::Radius)) {  
                    factory.AddConstraint<AddRadius_Circle>(elements[0], m_Radius);
                    needsUpdate = true;
                }
                ImGui::SameLine();
                // Draw inputbox
                cb_InputValue(&m_Radius);
            }
        }
        // 1 point and 1 element selected
        else if(points.size() == 1) 
        {
            if(elements[0].type == SketchItem::Type::Line) {
                // Add Coincident constraint between Point and Line 
                if(cb_ImageButton("Coincident", ConstraintType::Coincident)) {                   
                    factory.AddConstraint<Coincident_PointToLine>(points[0], elements[0]);
                    needsUpdate = true;
                }
                
                ImGui::SameLine();
                // Add Midpoint constraint between Point and Line              
                if(cb_ImageButton("Midpoint", ConstraintType::Midpoint)) {      
                    factory.AddConstraint<AddMidPoint_PointToLine>(points[0], elements[0]);
                    needsUpdate = true;
                }
                    
                ImGui::SameLine();
                // Add Distance constraint between Point and Line         
                if(cb_ImageButton("Distance", ConstraintType::Distance)) {
                    factory.AddConstraint<Distance_PointToLine>(points[0], elements[0], m_Distance);
                    needsUpdate = true;
                }
                    
                ImGui::SameLine();
                // Draw inputbox
                cb_InputValue(&m_Distance);
                
            }
            else if(elements[0].type == SketchItem::Type::Arc) {
                // Add Coincident constraint between Point and Arc  
                if(cb_ImageButton("Coincident", ConstraintType::Coincident)) { 
                    factory.AddConstraint<Coincident_PointToArc>(points[0], elements[0]);
                    needsUpdate = true;
                }
            }
            else if(elements[0].type == SketchItem::Type::Circle) {
                // Add Coincident constraint between Point and Circle  
                if(cb_ImageButton("Coincident", ConstraintType::Coincident)) { 
                    factory.AddConstraint<Coincident_PointToCircle>(points[0], elements[0]);
                    needsUpdate = true;
                }
            }
        }
    } 
    // 2 element selected
    else if(elements.size() == 2) {
        
        // 0 points and 2 elements selected
        if(points.size() == 0) 
        {
            // Line and Line selected
            if(elements[0].type == SketchItem::Type::Line && elements[1].type == SketchItem::Type::Line) {
                // Add Midpoint constraint            
                if(cb_ImageButton("Perpendicular", ConstraintType::Perpendicular)) { 
                    factory.AddConstraint<Perpendicular>(elements[0], elements[1]);
                    needsUpdate = true;
                } 
                
                ImGui::SameLine();
                // Add Parallel constraint
                if(cb_ImageButton("Parallel", ConstraintType::Parallel)) { 
                    factory.AddConstraint<Parallel>(elements[0], elements[1]);
                    needsUpdate = true;
                } 
                
                ImGui::SameLine();
                // Add Equal Length constraint  
                if(cb_ImageButton("Equal", ConstraintType::Equal)) {              
                    factory.AddConstraint<EqualLength>(elements[0], elements[1]);
                    needsUpdate = true;
                } 
                    
                ImGui::SameLine();
                // Add Angle constraint       
                if(cb_ImageButton("Angle", ConstraintType::Angle)) {   
                    factory.AddConstraint<Angle_LineToLine>(elements[0], elements[1], m_Angle);
                    needsUpdate = true;
                } 
                    
                ImGui::SameLine();
                // Draw inputbox
                cb_InputValue(&m_Angle);
                
            }
            // Arc and Arc selected
            else if(elements[0].type == SketchItem::Type::Arc && elements[1].type == SketchItem::Type::Arc) {
                // Add Equal radius constraint               
                if(cb_ImageButton("Equal", ConstraintType::Equal)) {   
                    factory.AddConstraint<EqualRadius_Arc_Arc>(elements[0], elements[1]);
                    needsUpdate = true;
                }  
                
                ImGui::SameLine();
                // Add Tangent constraint               
                if(cb_ImageButton("Tangent", ConstraintType::Tangent)) {   
                    factory.AddConstraint<Tangent_Arc_Arc>(elements[0], elements[1]);
                    needsUpdate = true;
                }  
            }
            // Circle and Circle selected
            else if(elements[0].type == SketchItem::Type::Circle && elements[1].type == SketchItem::Type::Circle) {
                // Add Equal radius constraint               
                if(cb_ImageButton("Equal", ConstraintType::Equal)) {   
                    factory.AddConstraint<EqualRadius_Circle_Circle>(elements[0], elements[1]);
                    needsUpdate = true;
                }  
            }
            
            // Arc and Line selected
            else if(elements[0].type == SketchItem::Type::Arc && elements[1].type == SketchItem::Type::Line) {
                // Add Tangent constraint               
                if(cb_ImageButton("Tangent", ConstraintType::Tangent)) {  
                    ConstraintButton_Tangent_ArcLine(elements[0], elements[1]);
                    needsUpdate = true;
                }   
            }
            // Line and Arc selected (reverse order)
            else if(elements[0].type == SketchItem::Type::Line && elements[1].type == SketchItem::Type::Arc) {
                // Add Tangent constraint                  
                if(cb_ImageButton("Tangent", ConstraintType::Tangent)) {  
                    ConstraintButton_Tangent_ArcLine(elements[1], elements[0]);
                    needsUpdate = true;
                }  
            }
            
            // Arc and Circle selected
            else if(elements[0].type == SketchItem::Type::Arc && elements[1].type == SketchItem::Type::Circle) {
                // Add Equal Radius constraint                
                if(cb_ImageButton("Equal", ConstraintType::Equal)) {  
                    factory.AddConstraint<EqualRadius_Arc_Circle>(elements[0], elements[1]);
                    needsUpdate = true;
                }  
            }
            // Circle and Arc selected (reverse order)
            else if(elements[0].type == SketchItem::Type::Circle && elements[1].type == SketchItem::Type::Arc) {
                // Add Equal Radius constraint                  
                if(cb_ImageButton("Equal", ConstraintType::Equal)) {  
                    factory.AddConstraint<EqualRadius_Arc_Circle>(elements[1], elements[0]);
                    needsUpdate = true;
                }   
            }
        }
    }

    
    return needsUpdate;
} 
    // Add constraint between 1 or 2 points, returns true if new constraint was added
void ConstraintButtons::ConstraintButton_Tangent_ArcLine(SketchItem arc, SketchItem line) 
{
    ElementFactory& factory = m_Parent->Factory();
    
    SketchItem item_arc_p0  = { SketchItem::Type::Arc_P0, arc.element };
    SketchItem item_arc_p1  = { SketchItem::Type::Arc_P1, arc.element };
    SketchItem item_line_p0 = { SketchItem::Type::Line_P0, line.element };
    SketchItem item_line_p1 = { SketchItem::Type::Line_P1, line.element };
    
    // get p0 & p1 from arc and line
    const Vec2& arc_p0  = factory.GetItemPointBySketchItem(item_arc_p0).p;
    const Vec2& arc_p1  = factory.GetItemPointBySketchItem(item_arc_p1).p;
    const Vec2& line_p0 = factory.GetItemPointBySketchItem(item_line_p0).p;
    const Vec2& line_p1 = factory.GetItemPointBySketchItem(item_line_p1).p;
    
    // Check which points are closest together (default to arc p0 to line p0)
    double shortestDistance = Hypot(line_p0 - arc_p0);
    // flags determine which point 
    int closestPoints = 0b00;
    
    auto checkIfCloser = [&shortestDistance, &closestPoints](const Vec2& arc_p, const Vec2& line_p, int closestPointsFlag) {
        double distance = Hypot(line_p - arc_p);
        if(distance < shortestDistance) {
            shortestDistance = distance;
            closestPoints = closestPointsFlag;
        } 
    };
    checkIfCloser(arc_p1, line_p0, 0b10);
    checkIfCloser(arc_p1, line_p1, 0b11);
    checkIfCloser(arc_p0, line_p1, 0b01); 
                
    // Constrain these closest points
    factory.AddConstraint<Coincident_PointToPoint>((closestPoints & 0b10) ? item_arc_p1 : item_arc_p0, (closestPoints & 0b01) ? item_line_p1 : item_line_p0);
    // Add Tangent constraint   
    //std::cout << "Adding tangent constraint to: P" << (closestPoints & 0b10) << std::endl;
    factory.AddConstraint<Tangent_Arc_Line>(arc, line, (bool)(closestPoints & 0b10));
}



Sketcher::Sketcher() 
    : m_Events(this), m_Renderer(this), m_ConstraintButtons(this)
{}



bool InputDouble2(std::string label, Vec2* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0) 
{
    bool isModified = false;
        
    // Make unique id for widgets
    std::string id = "##" + std::to_string((int)v);
    float widgetWidth = ImGui::GetWindowWidth() / 3.0f;
    
    // match spacing to inputfloat2 spacing
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { ImGui::GetStyle().ItemInnerSpacing.x, ImGui::GetStyle().ItemSpacing.y });
    
        ImGui::SetNextItemWidth(widgetWidth);
        isModified = ImGui::InputDouble(std::string(id + "x").c_str(), &(v->x), step, step_fast, format, flags);
        ImGui::SameLine();
        
        ImGui::SetNextItemWidth(widgetWidth);
        isModified |= ImGui::InputDouble(std::string(id + "y").c_str(), &(v->y), step, step_fast, format, flags);
        
        ImGui::SameLine();
        
        ImGui::SetNextItemWidth(widgetWidth);
        ImGui::TextUnformatted(label.c_str());
        
    ImGui::PopStyleVar();
                    
    return isModified;
}


void Sketcher::SolveConstraints(Vec2 pDif) 
{      
    // Update Solver set dragged point and its position
    bool success = m_Factory.UpdateSolver(pDif);
    if(!success) {
        Log::Error("Unable to solve constraints");
    }
}



       
void Sketcher::Draw_ConstraintButtons(std::function<bool(const std::string&, ConstraintType)> cb_ImageButton, std::function<void(double*)> cb_InputValue)
{
   // Draw Constraints Buttons
    if(m_ConstraintButtons.DrawImGui(cb_ImageButton, cb_InputValue)) {
        // We need to clear the selected items, otherwise when we update, it will fix all selected items to their current position
        m_Factory.ClearSelected();
        SolveConstraints();
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);     
    } 
}  
        
        
// TODO: This should be owned by SketchViewer class
        
//TODO:
//    on inputdouble enter, add the point (inputData.push_back(m_Events.m_CursorPos))
//    may be helpful? IsItemDeactivatedAfterEdit

void Sketcher::DrawImGui()
{
    
      /*
    // Cursor Popup
    static ImGuiModules::ImGuiPopup popup_CursorRightClick("popup_Sketch_CursorRightClick");
    // open
    if(m_Drawings.HasItemSelected()) 
    {
        if(auto id =  m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_GetID()) 
        {
            // set to open
            if(!ImGui::GetIO().WantCaptureMouse && IsActive()) {
                if(trigger(settings.p.sketch.cursor.popup.shouldOpen)) { popup_CursorRightClick.Open(); }
                * 
                *  
                    // returns value of input and switches input to false if true 
                    bool trigger(bool& input)
                    {
                        if(!input) {        
                            return false;
                        }
                        input = false;
                        return true;
                    }
                * 
                * 
            }
            // draw cursor popup
            popup_CursorRightClick.Draw([&]() {
                ImGui::Text("Point %u", (uint)*id);
                ImGui::Separator();
                // delete
                if(ImGui::Selectable("Delete")) {
                    if(m_Drawings.CurrentItem().m_ElementFactory.ActivePoint_Delete()) {
                        settings.SetUpdateFlag(SqeakUpdate::Full);
                    }
                }
            });
        }
    }
    
    
    static bool isNewDrawing = true;
    
    // display x, y coord on screen if not over imgui window
    if(!ImGui::GetIO().WantCaptureMouse && IsActive()) {
        DrawPopup_Cursor(settings);
    }
    
    */
    
    // set default size / position
    //ImGui::SetNextWindowSize(m_Size, ImGuiCond_Appearing);
    //ImGui::SetNextWindowPos(m_Pos, ImGuiCond_Appearing);
    
    ImGui::SetNextWindowPos(ImVec2(), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

    // id of element / contraint to be deleted - must only modify at end / beginning of frame
    ElementID deleteElement = 0;
    ConstraintID deleteConstraint = 0;
        
    // Main body of the Demo window starts here.
    if (ImGui::Begin("SketchApp", NULL))
    {
        // Draw User input values for when creating elements
        Sketcher::DrawImGui_ElementInputValues();
        ImGui::Separator();

        // Draw Element List (returns element to delete if needed)
        if(ImGui::CollapsingHeader("Elements")) {
            Sketcher::DrawImGui_Elements(deleteElement);
        }
        ImGui::Separator();
        
        // Draw Constraint List (returns constraint to delete if needed)
        if(ImGui::CollapsingHeader("Constraints")) {
            Sketcher::DrawImGui_Constraints(deleteConstraint);
        }
        ImGui::Separator();

        // Draw Settings
        if(ImGui::CollapsingHeader("Settings")) {
            Sketcher::DrawImGui_UpdateButton();
            Sketcher::DrawImGui_FactorySettings();
        }
    }
    ImGui::End(); 
        
     
        
        
        /*
        if (ImGui::SmallButton("New Drawing")) {
            m_Drawings.Add(A_Drawing("Drawing " + to_string(m_DrawingIDCounter++)));
            isNewDrawing = true;
            settings.SetUpdateFlag(SqeakUpdate::Full);
        } 
        
        for(size_t i = 0; i < m_Drawings.Size(); )
        {
            // set active drawing to be open initially & inactive drawings to be closed
            if(m_Drawings.CurrentIndex() == (int)i) { 
                if(isNewDrawing) {
                    ImGui::SetNextItemOpen(true); 
                    isNewDrawing = false;
                }
            } else { ImGui::SetNextItemOpen(false); }
            // close button flag - set by imgui
            bool closeIsntClicked = true; 
            if (ImGui::CollapsingHeader(m_Drawings[i].Name().c_str(), &closeIsntClicked)) {
                // set the active index to match the open tab
                if(m_Drawings.CurrentIndex() != (int)i) {
                    std::cout << "Setting current drawing index" << std::endl;
                    m_Drawings.SetCurrentIndex(i);
                    settings.SetUpdateFlag(SqeakUpdate::Full);
                }
                // draw the imgui widgets for drawing 
                m_Drawings.CurrentItem().DrawImGui(settings); 
            }
            if(!closeIsntClicked) { // has been closed
                m_Drawings.Remove(i); 
                settings.SetUpdateFlag(SqeakUpdate::Full);
            } else { 
                i++; 
            }                        

        }
        window.End();
    }*/
    
    
    auto SolveAndUpdate = [&]() {
        std::cout << "solving: " << std::endl;
        SolveConstraints();
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);
    };
    
    // Draw Constraint popup
    popup_ConstraintDimension.Draw([&]() {
             
        //~ TODO: ImGui::PushItemWidth(parameters.gui.dimensionInputWidth); // button width
        ImGui::PushItemWidth(50.0f); // button width
            m_Factory.ForEachConstraint([&](Sketch::Constraint* constraint) 
            {
                // Draw constraint(s) which are selected
                if(constraint->IsSelected()) { 
                    if(DrawImGui_ConstraintInputs(constraint, false)) { // don't show text
                        // Update on change
                        SolveAndUpdate();
                    }
                }
            });
        ImGui::PopItemWidth();
    });
    
    
    
    // modify should be and start / end of frame
    if(deleteElement) { 
        m_Factory.RemoveElement(deleteElement);
        SolveAndUpdate();
    }
    if(deleteConstraint) { 
        m_Factory.RemoveConstraint(deleteConstraint); 
        SolveAndUpdate();
    }
    
}

void Sketcher::DrawImGui_Elements(ElementID& deleteElement) 
{        
    // TODO: Move to params
    float deleteButtonPos = 140.0f;
    float inputBoxPos = 200.0f; 
    float inputBoxWidth = 75.0f;
    
    auto drawFlags = [&inputBoxPos](Item& item) 
    {
        if(item.IsSelected())  { ImGui::SameLine(inputBoxPos); ImGui::Text("(Selected)"); }
        if(item.IsHovered())   { ImGui::SameLine(); ImGui::Text("(Hovered)"); }                        
        if(item.IsFailed())   { ImGui::SameLine(); ImGui::Text("(Failed)"); }                        
    };
        
    // Draw ImGui widgets for a point
    auto drawPointStats = [&](const std::string& name, Item_Point& item) {

        if(InputDouble2(name.c_str(), &item.p)) {
            // select item and solve for the position we just changed
            Factory().SetSelected(item.Reference(), true);
            SolveConstraints({ 0.0, 0.0 });
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);  
        }
        drawFlags(item);
    };
    
    // Draw ImGui Treenode widgets for an element
    auto drawElementTreeNode = [&](const std::string& name, Sketch::Element* element) {
        bool isTreeNodeOpen = ImGui::TreeNode(va_str("%s %d", name.c_str(), element->ID()).c_str());
        drawFlags(element->Item_Elem());         
        ImGui::SameLine(deleteButtonPos); 
        if(ImGui::SmallButton(va_str("x##Element%d", element->ID()).c_str())) { deleteElement = element->ID(); }
        return isTreeNodeOpen;                      
    };
    

    Sketch::Point& point = m_Factory.OriginElement();
    
    // draw origin imgui TODO 
    bool isTreeNodeOpen = ImGui::TreeNode("Origin");
    drawFlags(point.Item_Elem());
    if(isTreeNodeOpen) {
        Item_Point& item = point.Item_P();
        ImGui::Text("p: (%.3f, %.3f)", item.p.x, item.p.y);            
        drawFlags(item);
        ImGui::TreePop(); ImGui::Separator();
    }
    
    m_Factory.ForEachElement([&](Sketch::Element* element) {
        
        if(auto* point = dynamic_cast<Sketch::Point*>(element)) {
        
            if(drawElementTreeNode("Point", element)) {
                drawPointStats("P", point->Item_P());
                ImGui::TreePop(); ImGui::Separator();
            }
        }
        else if(auto* line = dynamic_cast<Sketch::Line*>(element)) {
            
            if(drawElementTreeNode("Line", element)) {
                drawPointStats("P0", line->Item_P0());
                drawPointStats("P1", line->Item_P1());
                ImGui::TreePop(); ImGui::Separator();
            }
        }
        else if(auto* arc = dynamic_cast<Sketch::Arc*>(element)) {
            
            if(drawElementTreeNode("Arc", element)) {
                drawPointStats("P0", arc->Item_P0());
                drawPointStats("P1", arc->Item_P1());
                drawPointStats("PC", arc->Item_PC());
                ImGui::TreePop(); ImGui::Separator();
            }
        }
        else if(auto* circle = dynamic_cast<Sketch::Circle*>(element)) {
            
            if(drawElementTreeNode("Circle", element)) {
                drawPointStats("PC", circle->Item_PC());
                ImGui::Text("Radius: %.3f", circle->Radius());
                ImGui::TreePop(); ImGui::Separator();
            }
        }
        else { // Should never reach
            assert(0 && "Cannot draw imgui for element, type unknown");                
        }
    });
    
    ImGui::Separator();
}


bool Sketcher::DrawImGui_ConstraintInputs(Sketch::Constraint* constraint, bool showText)
{
    if(auto* c = dynamic_cast<Distance_PointToPoint*>(constraint)) {
        return ImGui::InputDouble(String::va_str(showText ? "Distance##%d" : "##%d", (int)c).c_str(), &(c->distance), 0.0f, 0.0f, "%.2f");
    }
    else if(auto* c = dynamic_cast<Distance_PointToLine*>(constraint)) {
        return ImGui::InputDouble(String::va_str(showText ? "Distance##%d" : "##%d", (int)c).c_str(), &(c->distance), 0.0f, 0.0f, "%.2f");
    }
    else if(auto* c = dynamic_cast<AddRadius_Circle*>(constraint)) {
        return ImGui::InputDouble(String::va_str(showText ? "Radius##%d" : "##%d", (int)c).c_str(), &(c->radius), 0.0f, 0.0f, "%.2f");
    }
    else if(auto* c = dynamic_cast<AddRadius_Arc*>(constraint)) {
        return ImGui::InputDouble(String::va_str(showText ? "Radius##%d" : "##%d", (int)c).c_str(), &(c->radius), 0.0f, 0.0f, "%.2f");
    }
    else if(auto* c = dynamic_cast<Angle_LineToLine*>(constraint)) {
        return ImGui::InputDouble(String::va_str(showText ? "Angle##%d" : "##%d", (int)c).c_str(), &(c->angle), 0.0f, 0.0f, "%.2f");
    }
    else if(auto* c = dynamic_cast<Tangent_Arc_Line*>(constraint)) {
        return ImGui::Combo(String::va_str(showText ? "Tangent Point##%d" : "##%d", (int)c).c_str(), &(c->tangentPoint), "P0\0P1\0\0");
    }
    return false;
}


    
    
void Sketcher::DrawImGui_Constraints(ConstraintID& deleteConstraint) 
{
    float deleteButtonPos = 140.0f;
    float inputBoxPos = 200.0f;
    float inputBoxWidth = 75.0f;
    
    auto drawConstraintTreeNode = [&deleteConstraint, &deleteButtonPos](Sketch::Constraint* c) {
        std::string title = va_str("%s %d", c->Label().c_str(), c->ID());
        bool isTreeNodeOpen = ImGui::TreeNode(title.c_str());
        if(c->IsSelected()) { ImGui::SameLine(); ImGui::Text("(Selected)"); }
        if(c->IsHovered())  { ImGui::SameLine(); ImGui::Text("(Hovered)"); }    
        if(c->IsFailed())   { ImGui::SameLine(); ImGui::Text("(Failed)"); }
        ImGui::SameLine(deleteButtonPos); 
        if(ImGui::SmallButton(va_str("x##Constraint%d", c->ID()).c_str())) { deleteConstraint = c->ID(); }
        return isTreeNodeOpen;                      
    };
    
    auto drawConstraintItems = [&](Sketch::Constraint* c) {
        // For each SketchItem in constraint
        c->ForEachItem([&](SketchItem& ref) {
            ImGui::Text("%s", ref.Name().c_str());
        });
    };
    
    
    ImGui::PushItemWidth(inputBoxWidth); // button width
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0.0f));
    
    bool updateRequired = false;
    m_Factory.ForEachConstraint([&](Sketch::Constraint* constraint) 
    {
        // Draw constrain header
        bool success = drawConstraintTreeNode(constraint);
        // Draw input if constraint includes one
        if(ConstaintHasInput(constraint->GetType())) {
            // Draw input boxes on same line as header
             ImGui::SameLine(inputBoxPos);
            // display input in header
            updateRequired |= DrawImGui_ConstraintInputs(constraint);
        }
        
        // Draw items in constraint
        if(success) {
            drawConstraintItems(constraint);
            ImGui::TreePop(); ImGui::Separator();
        }
        
    });
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();
    
    if(updateRequired)  {                        
        SolveConstraints();
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);  
    }
}


void Sketcher::DrawImGui_ElementInputValues() 
{
    // get currently selected points
    std::vector<Vec2>& inputData = m_Events.InputData();
    
    if(m_Events.GetCommandType() == SketchEvents::CommandType::None) {
        ImGui::TextUnformatted("Choose a Command");
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::Select) {
        
        const std::vector<SketchItem>& points = m_Events.GetSelectedPoints();
        const std::vector<SketchItem>& elements = m_Events.GetSelectedElements();
        
        // Display current selection
        ImGui::TextUnformatted("Current Selection");
        ImGui::Indent();
            if(points.empty() && elements.empty())  { ImGui::Text("None"); }
            for(SketchItem item : points)           { ImGui::Text(item.Name().c_str()); }
            for(SketchItem item : elements)         { ImGui::Text(item.Name().c_str()); }
        ImGui::Unindent();
        
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::SelectLoop) {
        ImGui::TextUnformatted("Select Geometry Loop");
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Point) {
        ImGui::TextUnformatted("Set Point Position");
                    
        if(InputDouble2("P (X, Y)", &m_Events.m_CursorPos)) {
            m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
        }
        
        if(ImGui::Button("Add Point")) {
            m_Factory.AddPoint(m_Events.m_CursorPos);
            m_Renderer.SetUpdateFlag(UpdateFlag::Full);
        }
        
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Line) {
        
        if(inputData.size() == 0)       { 
            ImGui::TextUnformatted("Set P0 Position");
            // draw p0
            if(InputDouble2("P0 (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw disabled p1
            ImGui::BeginDisabled();
                static Vec2 p1_disabled = { 0.0, 0.0 };
                InputDouble2("P1 (X, Y)", &p1_disabled);
            ImGui::EndDisabled();
                            
        }
        else if(inputData.size() == 1)  { 
            ImGui::TextUnformatted("Set P1 Position"); 
            // draw p0
            if(InputDouble2("P0 (X, Y)", &inputData[0])) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw p1
            if(InputDouble2("P1 (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
        }        
        // Draw Add Line Button (Only enable button if p0 has a value)
        ImGui::BeginDisabled(inputData.size() == 0);
            if(ImGui::Button("Add Line")) {
                m_Factory.AddLine(inputData[0], m_Events.m_CursorPos);
                m_Renderer.SetUpdateFlag(UpdateFlag::Full);
            }
        ImGui::EndDisabled();
        
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Arc) {
         // get currently selected points
        std::vector<Vec2>& inputData = m_Events.InputData();
        
        if(inputData.size() == 0)       { 
            ImGui::TextUnformatted("Set P0 Position");
            // draw p0
            if(InputDouble2("P0 (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw disabled p1 / pC
            ImGui::BeginDisabled();
                static Vec2 p1_disabled = { 0.0, 0.0 };
                static Vec2 pC_disabled = { 0.0, 0.0 };
                InputDouble2("P1 (X, Y)", &p1_disabled);
                InputDouble2("Centre (X, Y)", &pC_disabled);
            ImGui::EndDisabled();
                            
        }
        else if(inputData.size() == 1)  { 
            ImGui::TextUnformatted("Set P1 Position"); 
            // draw p0
            if(InputDouble2("P0 (X, Y)", &inputData[0])) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw p1
            if(InputDouble2("P1 (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw disabled pC
            ImGui::BeginDisabled();
                static Vec2 pC_disabled = { 0.0, 0.0 };
                InputDouble2("Centre (X, Y)", &pC_disabled);
            ImGui::EndDisabled();
        }   
        else if(inputData.size() == 2)  {  
            ImGui::TextUnformatted("Set PC Position"); 
            // draw p0
            if(InputDouble2("P0 (X, Y)", &inputData[0])) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw p1
            if(InputDouble2("P1 (X, Y)", &inputData[1])) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw p1
            if(InputDouble2("Centre (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
        }        
        
        static int direction = 0;
        static double radius = 0.0;
        
        radius = (inputData.size() < 2) ? 0.0 : Hypot(m_Events.m_CursorPos - inputData[0]);
        direction = (m_Events.m_InputDirection == Direction::CW) ? 0 : 1;
        
        ImGui::BeginDisabled(inputData.size() < 2);
            // Radius
            
            if(ImGui::InputDouble("Radius##Arc", &radius, 0.0, 0.0, "%.6f", ImGuiInputTextFlags_None/*EnterReturnsTrue*/)) {
                
                // update preview cursor based on radius
                m_Events.m_CursorPos = ArcCentre(inputData[0], inputData[1], radius, m_Events.m_InputDirection);
                
                std::cout << "m_Events.m_CursorPos: " << m_Events.m_CursorPos << std::endl;
                std::cout << "inputData[0]: " << inputData[0] << std::endl;
                std::cout << "inputData[1]: " << inputData[1] << std::endl;
                std::cout << "radius: " << radius << std::endl;
                std::cout << "m_Events.m_InputDirection: " << m_Events.m_InputDirection << std::endl;
                
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
        ImGui::EndDisabled();
        
        // Direction
        if(ImGui::Combo("Direction", &direction, "Clockwise\0Anti-Clockwise\0\0")) {
            m_Events.m_InputDirection = (direction == 0) ? Direction::CW : Direction::CCW;
            m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
        }
        
        ImGui::BeginDisabled(inputData.size() < 2);
        // Draw Add Line Button (Only enable button if p0 has a value)
            if(ImGui::Button("Add Arc")) {
                
                Vec2 new_pC = Geom::ArcCentre(inputData[0], inputData[1], m_Events.m_CursorPos);
                m_Factory.AddArc(inputData[0], inputData[1], new_pC, m_Events.m_InputDirection);
                m_Renderer.SetUpdateFlag(UpdateFlag::Full);
            }
        ImGui::EndDisabled();
                
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Circle) {
         // get currently selected points
        std::vector<Vec2>& inputData = m_Events.InputData();
        
        if(inputData.size() == 0)       { 
            ImGui::TextUnformatted("Set Centre Position");
            // draw pC
            if(InputDouble2("Centre (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
                            
        }
        else if(inputData.size() == 1)  { 
            ImGui::TextUnformatted("Set Radius"); 
            // draw pC
            if(InputDouble2("Centre (X, Y)", &inputData[0])) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
        }   
        
        // Only enable if pC has a value
        ImGui::BeginDisabled(inputData.size() == 0);
            // Direction
            static double radius = 0;
            if(ImGui::InputDouble("Radius##Circle", &radius)) {
                // update preview based on radius
                m_Events.m_CursorPos = inputData[0] + Vec2(0.0, radius);
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // Add Line Button
            if(ImGui::Button("Add Circle")) {
                m_Factory.AddCircle(inputData[0], radius);
                m_Renderer.SetUpdateFlag(UpdateFlag::Full);
            }
        ImGui::EndDisabled();
        
    }
    else if(m_Events.GetCommandType() == SketchEvents::CommandType::Add_Square) {
        
        if(inputData.size() == 0)       { 
            ImGui::TextUnformatted("Set P0 Position");
            // draw p0
            if(InputDouble2("P0 (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw disabled p1
            ImGui::BeginDisabled();
                static Vec2 p1_disabled = { 0.0, 0.0 };
                InputDouble2("P1 (X, Y)", &p1_disabled);
            ImGui::EndDisabled();
                            
        }
        else if(inputData.size() == 1)  { 
            ImGui::TextUnformatted("Set P1 Position"); 
            // draw p0
            if(InputDouble2("P0 (X, Y)", &inputData[0])) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
            // draw p1
            if(InputDouble2("P1 (X, Y)", &m_Events.m_CursorPos)) {
                m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
            }
        }     
        else if(inputData.size() == 2) {
            // TODO: ADD ImGui Wudget for square
            //~ // Draw Add Square Button (Only enable button if p0 has a value)
            //~ ImGui::BeginDisabled(inputData.size() == 0);
                //~ if(ImGui::Button("Add Square")) {
                    
                    
                    //~ // make sure points aren't the same
                    //~ if(inputData[1] == inputData[0]) { inputData.pop_back(); return false; }
                                    
                    //~ Vec2 pA = { inputData[0].x, inputData[1].y };
                    //~ Vec2 pB = { inputData[1].x, inputData[0].y };
                    
                    //~ SketchItem line1 = m_Factory.AddLine(inputData[0], pA);
                    //~ SketchItem line2 = m_Factory.AddLine(pA, inputData[1]);
                    //~ SketchItem line3 = m_Factory.AddLine(inputData[1], pB);
                    //~ SketchItem line4 = m_Factory.AddLine(pB, inputData[0]);
                    
                    //~ // Constrain the end point of previous line to the start point of the new arc
                    //~ m_Factory.AddConstraint<Coincident_PointToPoint>(line1.P1(), line2.P0());
                    //~ m_Factory.AddConstraint<Coincident_PointToPoint>(line2.P1(), line3.P0());
                    //~ m_Factory.AddConstraint<Coincident_PointToPoint>(line3.P1(), line4.P0());
                    //~ m_Factory.AddConstraint<Coincident_PointToPoint>(line4.P1(), line1.P0());
                             
                    //~ m_Factory.AddConstraint<Vertical>(line1);
                    //~ m_Factory.AddConstraint<Horizontal>(line2);
                    //~ m_Factory.AddConstraint<Vertical>(line3);
                    //~ m_Factory.AddConstraint<Horizontal>(line4);
                    
                    
                    
                    //~ m_Renderer.SetUpdateFlag(UpdateFlag::Full);
                //~ }
            //~ ImGui::EndDisabled();
        }
    }
    else { // should never reach
        assert(0 && "Type unknown"); 
    } 
}

void Sketcher::DrawImGui_UpdateButton() 
{
    if(ImGui::Button("Update Solver")) {
        SolveConstraints();
        // Update render data
        m_Renderer.SetUpdateFlag(UpdateFlag::Full);
    }   
}

void Sketcher::DrawImGui_FactorySettings() 
{    
    ImGui::SliderFloat("Scale Factor", &m_Factory.parameters.selectionDistance.scaleFactor, 0.0f, 100.0f);
    
    // Selection Tolerance    
    ImGui::SliderFloat("Constraint Offset", &m_Factory.parameters.offset_Constraint, 0.0f, 100.0f);
    ImGui::SliderFloat("Selection Distance - Element", &m_Factory.parameters.selectionDistance.element, 0.0f, 100.0f);
    ImGui::SliderFloat("Selection Distance - Constraint", &m_Factory.parameters.selectionDistance.constraint, 0.0f, 100.0f);
    ImGui::SliderFloat("Selection Distance - Horizontal / Vertical Snap", &m_Factory.parameters.selectionDistance.horizVert, 0.0f, 100.0f);
    if(ImGui::SliderFloat("Dimension Arrow Offset", &m_Factory.parameters.selectionDistance.dimensionArrowOffset, 0.0f, 100.0f)) {
        m_Renderer.SetUpdateFlag(UpdateFlag::Constraints);
    }
    if(ImGui::SliderFloat("Dimension Arrow Size", &m_Factory.parameters.selectionDistance.dimensionArrowSize, 0.0f, 100.0f)) {
        m_Renderer.SetUpdateFlag(UpdateFlag::Constraints);
    }
    
    
    
    ImGui::InputFloat("Arc Tolerance", &m_Factory.parameters.arcTolerance);
		
    m_Renderer.SetUpdateFlag(UpdateFlag::Preview);
}

} // end namespace Sketch
