#pragma once

#include <algorithm>
#include <MaxLib.h>


#include "sketch_common.h"
#include "elementfactory.h"


namespace Sketch {

using namespace MaxLib;
using namespace MaxLib::Vector;
using namespace MaxLib::Geom;

// forward declare
class Sketcher;




/*
    LATEST:
    //  TODO: for each point which has moved, only update any constraints / items on those constraints
        dont allow duplicate constraints
        ADD FIX CONSTRAINT
        * Tangent is causes excess lag and not working anyway

    TODO: Cant select line element if both points stuck at same point
    TODO: Tangent_Arc_Arc
    TODO: m_Factory->AddConstraint<Coincident_PointToPoint>(p0, p1); when creating points


    // TODO:    - SketchItem to become ItemRef
    //          - Should SketchItem just be a ref to Item_Element / Item_Point?
    //          - remove comments (old code)





    use std::variant for class with 2 return types
    // TODO: Check if conincident point on arc works?
    TODO: Make SketchItems work for elements...
    TODO: Check if point on arc works on solver?
    TODO: May have an error where it gets index - 1 but if 1 gets deleted, thats not true, so search with id
    TODO: Dont pass ptrs around top level
    TODO: remove unnessesary dynamic_cast, change to static_cast
    TODO: Are we actually using ID?
    TODO: Dynamically allocate memory (or at leasst add up what we need), currently we have hardcoded a number#
    TODO: Change name of Point2D, to Point somehow
    TODO: ensure all radius' are double
    * Rename pointType to more general type
    * 
    * Perhaps Constraints_ could be replaced by Constraints<> and move specific to solver
    * Would SketchItem<...>() work?
    * 
    TODO: make sure circle elememts are also updating radius and distances 
    * remove 
        using namespace MaxLib::;
    * from header files
 
    TODO: Perhaps SelectablePolygon should replace how we select all stuff?...
      this is slightly different as other finds closest point for points, but we can handle this inside the class, not easily as requires p + tolerance
   
    TODO: Check if point on arc works?
    
TODO: RenderLine, Arc etc. should be on renderer, not elementfactory

  */

/*
    The following pattern allows us to be in control of all of the members within the factory, the user receives ID's for the members and uses these IDs to call functions.

        class Member
        {
        public:
            Member(Factory* parent) : parent(m_Parent), m_ID(#)
        private:
            Factory* m_Parent
            ID m_ID;
        };
         
        class Factory
        {
        public:
            CreateMember() { m_Members.push_back(...); return m_Members.back().ID(); }
            DoSomething(ID id)  { member = FindMember(id); do something... }
        private:
            vector<Member> m_Members;
            FindMember(ID id) { for each ... }
        };
        
        
        ** User **
        
        Factory factory;
        ID memberID = factory.CreateMember();
        factory.DoSomething(memberID) 

 */
 
        
        
// A container of all the renderable data in sketcher
struct RenderData
{
    struct Image
    {
        ConstraintType type;
        Vec2 position;
    };
    struct Text
    {
        std::string value;
        Vec2 position;
    };
    struct Data
    {
        vector<Geometry> points;
        vector<Geometry> linestrings;
        vector<Image> images;
        vector<Text> texts;
        
        void Clear() { 
            points.clear(); 
            linestrings.clear(); 
            images.clear(); 
            texts.clear(); 
        } 
    };
        
    struct Data_Selectable
    {
        Data unselected;
        Data selected;
        Data hovered;
        Data failed;
        
        // call a function on each item
        void ForEachItem(std::function<void(const Data&)> cb) const {
            cb(unselected); 
            cb(selected); 
            cb(hovered); 
            cb(failed); 
        }
        // Clear each item
        void Clear() { 
            unselected.Clear(); 
            selected.Clear(); 
            hovered.Clear(); 
            failed.Clear(); 
        } 
    };
    
    Data_Selectable     elements;      // all the elements 
    Data_Selectable     constraints;   // the contraints
    Data                preview;       // preview elements before adding them
    Data                cursor;        // dragged box etc.
    bool                selectionBoxDirection = 1; // 0 left  1 right
    
    // call a function on each data item
    void ForEachItem(std::function<void(const Data&)> cb) const {
        elements.ForEachItem([&](const Data& data) { cb(data); });
        constraints.ForEachItem([&](const Data& data) { cb(data); });
        cb(preview);
        cb(cursor);
    }
};



// A piece of geometry which can be either selected or hovered
// This will take ownership of input geometry
class SelectablePolygon
{
public:     
    // Geometry is equiv. to std::vector<Vec2>, LineString, Polygon, Points
    // Will take ownership of input geometry
    SelectablePolygon(Geom::Polygon&& geometry) : m_Geometry(std::move(geometry)) {}
    
    const Geom::Polygon& Polygon()      { return m_Geometry; }
    bool IsSelected() const             { return m_IsSelected; }
    bool IsHovered() const              { return m_IsHovered; }
    
private:
    Geom::Polygon m_Geometry;
    bool m_IsSelected = false;
    bool m_IsHovered = false;
    friend class PolygonizedGeometry;
};
    
    
     
    
    
// a collection of selectable geometry
class PolygonizedGeometry
{
public:
    // Generates the polygonized selection geometry & will consume input geometries
    PolygonizedGeometry(Sketcher* parent = nullptr, const std::vector<Geometry>& geometries = {});
    
    // Clears all of the geometries' hovered flags
    void ClearHovered();
    // Clears all of the geometries' seleceted flags
    void ClearSelected();
    // Finds geometry within a tolerance to position p and sets their Hovered flag to true
    bool SetHoveredByPosition(const Vec2& p);
    // Finds geometry within a tolerance to position p and sets their Selected flag to true
    bool SetSelectedByPosition(const Vec2& p);
    // Calls callback function on each SelectablePolygon item  
    void ForEachGeometry(std::function<void(SelectablePolygon&)> cb);
     
private:
    Sketcher* m_Parent = nullptr;
    // Polygonized linestring data
    std::vector<SelectablePolygon> m_Geometry;

    // Finds geometry within a tolerance to position p and calls callback
    // l is a polygon which 
    bool FindIntersects(const Vec2& p, std::function<void(SelectablePolygon&)> cb);
};

// A flag used when building render data to only render the required item
enum class UpdateFlag { 
    None                               = 0x0,  
    Cursor                             = 0x1, // handles dragged selection box
    Selection                          = 0x2, // handles selected & hovered
    Elements                           = 0x4, // handles change to elements, also polygonizes elements 
    Preview                            = 0x8, // handles element preview whilst creating an element
    Constraints                        = 0x10, // handles constraints
    
    ClearInputData                     = 0x20, // clears input data
    DontSetInputDataToLastElement      = 0x40, // will prevent seting input data so that it includes the previous elements' end point
    ClearSelection                     = 0x80, // clears selection data
    
    
    Full                               = 0xFF, // updates all above
    Full_DontClearSelection            = Full & ~ClearSelection,  // updates all but doesnt clear selection data
    Full_SetInputDataToLastElement     = Full & ~DontSetInputDataToLastElement // updates all but sets input data so that it includes the previous elements' end point
};

inline UpdateFlag operator|(UpdateFlag a, UpdateFlag b) { return static_cast<UpdateFlag>(static_cast<int>(a) | static_cast<int>(b)); }
inline bool operator&(UpdateFlag a, UpdateFlag b) { return static_cast<bool>(static_cast<int>(a) & static_cast<int>(b)); }



class SketchRenderer
{
public:   

    
    SketchRenderer(Sketcher* parent);
    
    //const std::vector<RenderData>&   GetRenderData() const;
    const RenderData&   GetRenderData() const;
    
    // Updates RenderData based on update flag
    bool UpdateRenderData();
    
    void SetUpdateFlag(UpdateFlag flag);
    
    LineString RenderElement(Sketch::Element* element);
    LineString RenderElementBySketchItem(SketchItem item);
     
private: 
    Sketcher* m_Parent = nullptr;   
    // update flag
    UpdateFlag m_Update = UpdateFlag::Full;
    // draw list for viewer
    RenderData m_RenderData;
    
    
    // update the preview render data from the cursorPos
    void UpdatePreview();
    
    friend class SketchEvents;
};
    
    


class SketchEvents
{
public:
    enum class CommandType { None, Select, SelectLoop, Add_Point, Add_Line, Add_Arc, Add_Circle, Add_Square, Add_Constraint_1_Item, Add_Constraint_2_Items };
    
    // Equivelent to:
    //  GLFW_MOUSE_BUTTON_LEFT      0
    //  GLFW_MOUSE_BUTTON_RIGHT     1
    //  GLFW_MOUSE_BUTTON_MIDDLE    2
    enum class MouseButton { None = -1, Left = 0, Right = 1, Middle = 2 };

    // Equivelent to:
    // GLFW_RELEASE     0
    // GLFW_PRESS       1
    enum class MouseAction { None = -1, Release = 0, Press = 1 };

    // GLFW_REPEAT      2
    enum class KeyAction   { None = -1, Release = 0, Press = 1, Repeat = 2 };

    // Equivelent to:
    // GLFW_MOD_SHIFT      0x0001
    // GLFW_MOD_CONTROL    0x0002
    // GLFW_MOD_ALT        0x0004
    // GLFW_MOD_SUPER      0x0008
    // GLFW_MOD_CAPS_LOCK  0x0010
    // GLFW_MOD_NUM_LOCK   0x0020
    enum class KeyModifier { None = 0x00, Shift = 0x01, Ctrl = 0x02, Alt = 0x04, Super = 0x08, CapsLock = 0x10, NumLock = 0x20 };

 


    SketchEvents(Sketcher* parent) : m_Parent(parent) {}
    
    CommandType GetCommandType() const;
    void SetCommandType(CommandType command);


    void Event_Keyboard(double time, int key, KeyAction action, KeyModifier modifier);
    
    // Resets mouse button
    void Mouse_Button_Reset();
    
    // Mouse Button Event
    //
    //   On Left Click                 
    //       -  select item
    //       -  if(there is no item), deselect items
    //   Left Click w/ Ctrl or Shift
    //       -  add to selection
    //   On Release
    //       -  if(selection box) select these items
    //   Returns true if update required
    bool Mouse_Button(double time, MouseButton button, MouseAction action, KeyModifier modifier);
    
    // Mouse Move Event
    //
    //  Input should be (x, y) coords in sketch space
    //
    //  Hover over item -  Highlights it
    //  Drag            -  if (clicked)
    //                        -  if(Selected) drag items
    //                        -  if(!selected) drag selection box
    //   Returns true if update required
    bool Mouse_Move(double time, const Vec2& p);


    const std::vector<SketchItem>&  GetSelectedPoints()   { return m_SelectedPoints; }
    const std::vector<SketchItem>&  GetSelectedElements() { return m_SelectedElements; }
    const std::vector<Polygon>&     GetSelectedPolygons() { return m_SelectedPolygons; }
    

    // Returns p1 adjusted such that p0 and p1 are horizontal / vertical (if within tolerance)
    std::pair<ConstraintType, Vec2> ConstrainPoint_HorizontalVertical(const Vec2& p0, const Vec2& p1);
    
private:
    
    Sketcher* m_Parent = nullptr;
    CommandType m_CommandType = CommandType::None;
    
    std::vector<Vec2> m_InputData; // holds a list of points which the user has selected ready for creating a new element
    Direction m_InputDirection = Direction::CW;
    SketchItem m_PreviousElement;
    
                    
    Vec2 m_CursorPos;
    Vec2 m_CursorClickedPos;
    double m_CursorClickedTime;
    bool m_IsDoubleClicked = false;
    bool m_IsSelectionBox = false; 
    bool m_IsDimensionClicked = false;
    
    MouseButton m_MouseButton = MouseButton::None;
    MouseAction m_MouseAction = MouseAction::None;
    //KeyModifier m_Modifier;

    // Filters the input selection to points / lines / polygons 
    SelectionFilter m_SelectionFilter = SelectionFilter::All;
    // Easy to access arrays of selected item
    std::vector<SketchItem> m_SelectedPoints;   // Points as SketchItems
    std::vector<SketchItem> m_SelectedElements; // Lines as SketchItems
    std::vector<Polygon> m_SelectedPolygons;   // Polygons (polygonized) as vector<Vec2>
    
    // polygonizes all of the geometry so we can select loops
    PolygonizedGeometry m_PolygonizedGeometry;


    std::vector<Vec2>& InputData() { return m_InputData; }
    
    
    friend class Sketcher;
    friend class SketchRenderer;
};


class ConstraintButtons
{
public:
    ConstraintButtons(Sketcher* parent) : m_Parent(parent) {}
    // Draw ImGui buttons for constraints
    bool DrawImGui(std::function<bool(const std::string&, ConstraintType)> cb_ImageButton, std::function<void(double*)> cb_InputValue);
private:
    Sketcher* m_Parent;
    double m_Distance = 100.0;
    double m_Radius = 100.0;
    double m_Angle = 90.0;
    
        // Add constraint between 1 or 2 points, returns true if new constraint was added
    void ConstraintButton_Tangent_ArcLine(SketchItem arc, SketchItem line);
};



class Sketcher
{
public:
    Sketcher();
    
    ElementFactory& Factory()   { return m_Factory; }
    SketchEvents& Events()  { return m_Events; }
    SketchRenderer& Renderer()  { return m_Renderer; }
    
    bool Update() 
    {
        // Update render data if flag is set
        bool updateRequired = m_Renderer.UpdateRenderData();
        return updateRequired;
    }
            
    // Attempts to solve constraints. 
    void SolveConstraints(Vec2 pDif = Vec2()); 
    
    void Draw_ConstraintButtons(std::function<bool(const std::string&, ConstraintType)> cb_ImageButton, std::function<void(double*)> cb_InputValue);
    void DrawImGui();
    void DrawImGui_Elements(ElementID& deleteElement);
    void DrawImGui_Constraints(ConstraintID& deleteConstraint);
    void DrawImGui_ElementInputValues();
    void DrawImGui_UpdateButton();
    void DrawImGui_FactorySettings();
    
    ImGuiModules::ImGuiPopup popup_ConstraintDimension = {"popup_Constraint_Dimension"};
    
private:
    bool m_IsActive = false;
    
    ElementFactory m_Factory;
    SketchEvents m_Events;  
    SketchRenderer m_Renderer;
    ConstraintButtons m_ConstraintButtons;
    
    
    bool DrawImGui_ConstraintInputs(Sketch::Constraint* constraint, bool showText = true);
};
     

} // end namespace Sketch
