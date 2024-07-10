#include <iostream>
#include <string>
#include <functional>
#include <MaxLib.h>
#include "deps/GLViewer/glviewer.h"
#include "sketch.h"
#include "sketch_params.h"

using namespace std;
using namespace MaxLib;
using namespace MaxLib::Geom;
using namespace GLViewer;
using namespace ImGuiModules;
using namespace Sketch;



#define GUI_WINDOW_NAME     "Sketch"
#define GUI_WINDOW_W        1280
#define GUI_WINDOW_H        720

#define GUI_CONFIG_FILE     "uiconfig.ini"
#define GUI_IMG_ICON        "img/icon.png"



//~ square
//~ slot
//~ polygon
//~ text



int main()
{    
    // GLFW Config General
    auto cb_GLFW_Config = [&](GLFWwindow* window) {
        // set minimum window size
        glfwSetWindowSizeLimits(window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE); 
    };

    // ImGui Config
    auto cb_imgui_Config = [](GLFWwindow* window) {
        // Style
        ImGui::StyleColorsDark();
        // Get IO
        ImGuiIO& io = ImGui::GetIO();
        
        // ImGui ini File
        static string iniFile = File::ThisDir(GUI_CONFIG_FILE);
        io.IniFilename = iniFile.c_str();
        
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    
        io.ConfigDockingAlwaysTabBar = true;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowMenuButtonPosition  = ImGuiDir_Right;   // icon in menu of window
        style.ScrollbarRounding         = 3.0f;             // scroll bars
        style.FrameRounding             = 2.0f;             // frames i.e. buttons, textboxes etc.
        
        // Load icon
        static string iconLocation = File::ThisDir(GUI_IMG_ICON);
	std::cout << "icon location: " << iconLocation << std::endl;
        if(!LoadIconFromFile(window, iconLocation.c_str()))
            Log::Error(string("Could not find icon: ") + iconLocation);
    };
    
    // Fonts & Images
    auto cb_imgui_Assets = [&]() { 
	//~ // Get IO
	//~ ImGuiIO& io = ImGui::GetIO();
	   
	//~ // Load Fonts (primary first)
	//~ font_medium = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 17.0f);
	//~ if(!font_medium)
	    //~ cout << "Error: Could not find font: Geomanist 17" << endl;
	//~ font_small = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 13.0f);
	//~ if(!font_small)
	    //~ cout << "Error: Could not find font: Geomanist 13" << endl;
	    
	// Images
	//~ img_Add.Init(File::ThisDir("img/img_add.png").c_str());
    };

    
    // glsl version  / glfw version major & minor 	TODO: "#version 300 es"
    GLSystem glsys(GUI_WINDOW_W, GUI_WINDOW_H, GUI_WINDOW_NAME, "#version 130", 3, 0, cb_GLFW_Config, cb_imgui_Config, cb_imgui_Assets);  
    // Create Viewer instance
    Viewer viewer;
    
    Sketch::Sketcher sketcher;
    SketchParameters parameters;
        
    // Events (Mouse & Keyboard etc)
    
    auto IsSketchActive = [&]() {
        // ignore if a ImGui window is hovered over
        if(ImGui::GetIO().WantCaptureMouse) { return false; }
	// ignore if not in 2d mode
	if(viewer.camera.GetViewMode() != Camera::ViewMode::View2D) { return false; }
	// ignore if sketch tool not active
	if(sketcher.Events().GetCommandType() == SketchEvents::CommandType::None) { return false; }
	// Sketcher is active
	return true;
    };
    
    // Update selection distance on mouse scroll
    Event<Event_MouseScroll>::RegisterPersistantHandler([&](Event_MouseScroll& data) {
	(void)data; // Ignore data
	if (!IsSketchActive()) { return; }
	// scale the selection distance (for zoom) 
	sketcher.Factory().parameters.selectionDistance.scaleFactor = viewer.ScaleToPx(1.0f); 
        sketcher.Renderer().SetUpdateFlag(UpdateFlag::Constraints);
    });
    
    // Pass keyboard event to sketcher
    Event<Event_KeyInput>::RegisterPersistantHandler([&](Event_KeyInput& data) {
	if (!IsSketchActive()) { return; }
	// Sketch Event (Keyboard)
        sketcher.Events().Event_Keyboard(glfwGetTime(), data.Key, (Sketch::SketchEvents::KeyAction)data.Action, (Sketch::SketchEvents::KeyModifier)data.Modifier);
    });
    
    
    Event<Event_MouseButton>::RegisterPersistantHandler([&](Event_MouseButton& data) {
	if (!IsSketchActive()) { 
	    sketcher.Events().Mouse_Button_Reset();
	    return; 
	}
        // ignore if not a mouse button
        if((data.Button != GLFW_MOUSE_BUTTON_LEFT) && (data.Button != GLFW_MOUSE_BUTTON_RIGHT) && (data.Button != GLFW_MOUSE_BUTTON_MIDDLE)) { return; } 
	// Sketch Event (Mouse Button)
	parameters.update.isRequired |= sketcher.Events().Mouse_Button(glfwGetTime(), (Sketch::SketchEvents::MouseButton)data.Button, (Sketch::SketchEvents::MouseAction)data.Action, (Sketch::SketchEvents::KeyModifier)data.Modifier);
    });
            
    Event<Event_MouseMove>::RegisterPersistantHandler([&](Event_MouseMove& data) {
	
	if (!IsSketchActive()) { return; }
        // Convert mouse position to world coordinates
	glm::vec3 cursorPosition = viewer.ScreenToWorldCoords({ data.PosX, data.PosY });
	// TODO snap cursor in sketcher and store the position so we can read from it later
	// Send mouse position to sketcher
	parameters.update.isRequired |= sketcher.Events().Mouse_Move(glfwGetTime(), Vec2(cursorPosition.x, cursorPosition.y));
    });
    
    
    
    
    
    
    
    // Custom Keyboard Event
    Event<Event_KeyInput>::RegisterPersistantHandler([&](Event_KeyInput& data) {
        if(data.Action == GLFW_PRESS) {
            switch (data.Key) {
                case GLFW_KEY_T:
		
		    if (viewer.camera.GetViewMode() == Camera::ViewMode::View2D) {
			// Set 3D View mode
			viewer.camera.SetViewMode(Camera::ViewMode::View3D);
			sketcher.Events().SetCommandType(SketchEvents::CommandType::None);
		    } else {
			// Set 2D View mode
			viewer.camera.SetViewMode(Camera::ViewMode::View2D);
			sketcher.Events().SetCommandType(SketchEvents::CommandType::Select);
		    }
		
                    break;
            }
        } 
    });
    
    
    // Create Buffers
    Buffers& buffers = viewer.buffers;
    
    
    // Add Static Buffer
    buffers.CreateStaticBuffer([&](StaticBuffer& buffer) {
	// Add fine & course Grid
	buffer.lineWidth = 1.0f;
	buffer.AddGrid({ 1000.0f, 1000.0f }, 10, parameters.colours.grid_fine.value, Transform({0, 0, 0}));
	buffer.AddGrid({ 1000.0f, 1000.0f }, 100, parameters.colours.grid_coarse.value, Transform({0, 0, 0}));
	
	// Set Parameters
	buffer.pointSize = parameters.pointSize;
	buffer.lineWidth = parameters.lineWidth;
	buffer.SetDepthFunctionByIndex(parameters.depthFunction);
    });
	
    
    // Create dynamic sketch buffers // TODO move to sketch viewer
    TextBufferID textBuffer_Sketch = buffers.CreateTextBuffer();
    ImageBufferID imageBuffer_Sketch = buffers.CreateImageBuffer();
    GeometryBufferID dynamicBuffer_Sketch = buffers.CreateDynamicBuffer();
    
    // Create users dyanamic buffers
    TextBufferID textBuffer = buffers.CreateTextBuffer();
    ImageBufferID imageBuffer = buffers.CreateImageBuffer();
    GeometryBufferID dynamicBuffer = buffers.CreateDynamicBuffer();
        
	
    //~ TODO: add mouse position to cursor's text buffer
    
    // Add sketch's texts to viewer's text buffer
    auto SketchViewer_SetTextBuffer = [&](TextBuffer& buffer) 
    {                        
            
        auto AddToBuffer = [&](const vector<RenderData::Text>& texts, const glm::vec3& colour, const glm::vec2& offset = { 0.0f, 0.0f }, const glm::vec2& align = { 0.5f, 0.5f }) {
		  // Send each text to viewer
            for(const RenderData::Text& text : texts) {
		glm::vec3 position =  glm::vec3{ text.position.x + offset.x, text.position.y + offset.y, 0.0f };
		buffer.Add3DText(text.value, position, { colour.r, colour.g, colour.b, 1.0f }, align);
            }
	};
	
	auto& r = sketcher.Renderer().GetRenderData();
	SketchParameters::Colours& c = parameters.colours;
	
	AddToBuffer(r.elements.unselected.texts, 		c.sketch_elements_texts_unselected.value); // Unused
	AddToBuffer(r.elements.selected.texts, 			c.sketch_elements_texts_selected.value); // Unused
	AddToBuffer(r.elements.hovered.texts, 			c.sketch_elements_texts_hovered.value);	// Unused
	AddToBuffer(r.elements.failed.texts, 			c.sketch_elements_texts_failed.value); // Unused
	
	AddToBuffer(r.constraints.unselected.texts, 		c.sketch_constraints_texts_unselected.value); 	// dimensions
	AddToBuffer(r.constraints.selected.texts, 		c.sketch_constraints_texts_selected.value); 	// dimensions
	AddToBuffer(r.constraints.hovered.texts, 		c.sketch_constraints_texts_hovered.value); 	// dimensions
	AddToBuffer(r.constraints.failed.texts, 		c.sketch_constraints_texts_failed.value); 	// dimensions
	
	AddToBuffer(r.preview.texts, 				c.sketch_preview_texts.value); // Unused
	AddToBuffer(r.cursor.texts, 				c.sketch_cursor_texts.value, { parameters.cursor.textOffset.x, parameters.cursor.textOffset.y }, { 0.0f, 0.0f }); // current position (bottom right text align)
            
	//~ // For each data item in render data
        //~ sketcher.Renderer().GetRenderData().ForEachItem([&](const Sketch::RenderData::Data& data) {
	     //~ // Send each text to viewer
            //~ for(const Sketch::RenderData::Text& text : data.texts) {
		//~ glm::vec3 position =  glm::vec3{ text.position.x, text.position.y, 0.0f };
		//~ buffer.Add3DText(text.value, position);
            //~ }
	//~ });
    };



	    
    // Add sketch's images to viewer's image buffer
    auto SketchViewer_SetImageBuffer = [&](ImageBuffer& buffer) 
    {    	
        auto AddToBuffer = [&](const vector<RenderData::Image>& images, const glm::vec3& colour) {
	    // Send each image to viewer
            for(const RenderData::Image& image : images) {
		// Get image texture
		ImageTexture texture = parameters.images.constraints.GetImageTextureByType(image.type); // TODO: Type shouldnt be in here, OR make enum ImageType to include others images insted of constrainttype
		
		glm::vec3 position =  glm::vec3{ image.position.x, image.position.y, 0.0f };
		buffer.Add3DImage(texture, position, parameters.image.size, { colour.r, colour.g, colour.b, 1.0f });
            }
	};
	
	auto& r = sketcher.Renderer().GetRenderData();
	SketchParameters::Colours& c = parameters.colours;
	
	AddToBuffer(r.elements.unselected.images, 		c.sketch_elements_images_unselected.value);
	AddToBuffer(r.elements.selected.images, 		c.sketch_elements_images_selected.value);
	AddToBuffer(r.elements.hovered.images, 			c.sketch_elements_images_hovered.value);
	AddToBuffer(r.elements.failed.images, 			c.sketch_elements_images_failed.value);
	
	AddToBuffer(r.constraints.unselected.images, 		c.sketch_constraints_images_unselected.value);
	AddToBuffer(r.constraints.selected.images, 		c.sketch_constraints_images_selected.value);
	AddToBuffer(r.constraints.hovered.images, 		c.sketch_constraints_images_hovered.value);
	AddToBuffer(r.constraints.failed.images, 		c.sketch_constraints_images_failed.value);
	
	AddToBuffer(r.preview.images, 				c.sketch_preview_images.value);
	AddToBuffer(r.cursor.images, 				c.sketch_cursor_images.value);
	
	
	//~ // For each data item in render data
        //~ sketcher.Renderer().GetRenderData().ForEachItem([&](const Sketch::RenderData::Data& data) {
	     //~ // Send each image to viewer
            //~ for(const Sketch::RenderData::Image& image : data.images) {
		//~ // Get image texture
		//~ ImageTexture texture = parameters.images.constraints.GetImageTextureByType(image.type); // TODO: Type shouldnt be in here, OR make enum ImageType to include others images insted of constrainttype
		
		//~ glm::vec3 position =  glm::vec3{ image.position.x, image.position.y, 0.0f };
		//~ buffer.Add3DImage(texture, position, parameters.image.size);
            //~ }
	//~ });
    };



    // This put all values directly on viewer
    auto SketchViewer_SetDynamicBuffer = [&](DynamicBuffer& buffer) 
    {
	// Set Parameters
	buffer.pointSize = parameters.pointSize;
	buffer.lineWidth = parameters.lineWidth;
	buffer.SetDepthFunctionByIndex(parameters.depthFunction);
	// Points
	// For each data item in render data
        auto AddToBuffer = [&](const vector<Geometry>& geometries, const glm::vec3& colour, uint primitive)
	{
	     // Send each image to viewer
            for(const Geometry& geometry : geometries) {
		// create list of points
		Vertices vertices(colour);
		for(const Vec2& p : geometry) {
		    vertices.position.emplace_back(p.x, p.y, 0.0f);
		}
		buffer.AddVertices(primitive, vertices); // TODO: Consider vertex list
            }
	};
	 
	 //TODO
	   //~ - selecting vertical constraint doesnt work (should be middle of line), actual positoin is end of line,  
	   //~ - dimension from line to point should go perpendicular to line, currently from line midpoint
	   //~ - points dont drag
	   //~ - should be able to drag point onto geometry to make constraint
	
	auto& r = sketcher.Renderer().GetRenderData();
	SketchParameters::Colours& c = parameters.colours;

	AddToBuffer(r.constraints.unselected.linestrings,	c.sketch_constraints_lines_unselected.value,		GL_LINE_STRIP);	// Dimenions
	AddToBuffer(r.constraints.hovered.linestrings, 		c.sketch_constraints_lines_hovered.value,		GL_LINE_STRIP);
	AddToBuffer(r.constraints.selected.linestrings, 	c.sketch_constraints_lines_selected.value,		GL_LINE_STRIP);
	AddToBuffer(r.constraints.failed.linestrings, 		c.sketch_constraints_lines_failed.value,		GL_LINE_STRIP);
	    
	AddToBuffer(r.constraints.unselected.points, 		c.sketch_constraints_points_unselected.value,		GL_POINTS);	// Unused
	AddToBuffer(r.constraints.hovered.points, 		c.sketch_constraints_points_hovered.value,		GL_POINTS);
	AddToBuffer(r.constraints.selected.points, 		c.sketch_constraints_points_selected.value,		GL_POINTS);
	AddToBuffer(r.constraints.failed.points, 		c.sketch_constraints_points_failed.value,		GL_POINTS);
	   
	   
	// Add sketch geometry and colours to Viewer 
	AddToBuffer(r.elements.unselected.linestrings, 		c.sketch_elements_lines_unselected.value,		GL_LINE_STRIP);	// Elements
	AddToBuffer(r.elements.hovered.linestrings, 		c.sketch_elements_lines_hovered.value,			GL_LINE_STRIP);
	AddToBuffer(r.elements.selected.linestrings, 		c.sketch_elements_lines_selected.value,			GL_LINE_STRIP);
	AddToBuffer(r.elements.failed.linestrings, 		c.sketch_elements_lines_failed.value,			GL_LINE_STRIP);
	    
	AddToBuffer(r.elements.unselected.points, 		c.sketch_elements_points_unselected.value,		GL_POINTS);	// Element Endpoints
	AddToBuffer(r.elements.hovered.points, 			c.sketch_elements_points_hovered.value,			GL_POINTS);
	AddToBuffer(r.elements.selected.points, 		c.sketch_elements_points_selected.value,		GL_POINTS);
	AddToBuffer(r.elements.failed.points, 			c.sketch_elements_points_failed.value,			GL_POINTS);
		
	   
	AddToBuffer(r.preview.linestrings, 			c.sketch_preview_lines.value,				GL_LINE_STRIP);	// Preview Elements
	AddToBuffer(r.preview.points, 				c.sketch_preview_points.value,				GL_POINTS);	// Preview Element Endpoints
			
																	// Selection Box
	AddToBuffer(r.cursor.linestrings, 			(r.selectionBoxDirection > 0) ? c.sketch_cursor_lines_right.value : c.sketch_cursor_lines_left.value,	GL_LINE_STRIP);
	AddToBuffer(r.cursor.points, 				c.sketch_cursor_points.value,				GL_POINTS);	// Cursor

	
	//~ const Sketch::RenderData::Data& cursor = renderData.cursor;
        //~ // Change colour of selection box if left of click position
        //~ if(cursor.points.size() == 1) {
            //~ if(cursor.points[0].size() == 4) {
                //~ // change colour depending on whether selection box is to left or right
                //~ const glm::vec3& selectionBoxColour = ((cursor.points[0][2].x - cursor.points[0][0].x) < 0) ? glm::vec3(0.1f, 0.3f, 0.8f) : glm::vec3(0.306f, 0.959f, 0.109f);
                //~ CopyVertices(m_ViewerLineLists, cursor.linestrings, selectionBoxColour);
            //~ }
        //~ }
	
	
	
	//~ // TODO: Remove ForEachItem()
	
	//~ // Points
	//~ // For each data item in render data
        //~ sketcher.Renderer().GetRenderData().ForEachItem([&](const Sketch::RenderData::Data& data) 
	//~ {
	     //~ // Send each image to viewer
            //~ for(const Geometry& points : data.points) {
		//~ glm::vec3 colour = Colour::Black; // TODO: should be inside
		//~ // create list of points
		//~ Vertices vertices(colour);
		//~ for(const Vec2& p : points) {
		    //~ vertices.position.emplace_back(p.x, p.y, 0.0f);
		//~ }
		//~ buffer.AddVertices(GL_POINTS, vertices); // TODO: Consider vertex list
            //~ }
	//~ });
	
	//~ // Linestrings
	//~ // For each data item in render data
        //~ sketcher.Renderer().GetRenderData().ForEachItem([&](const Sketch::RenderData::Data& data) {
	    
	     //~ // Send each image to viewer
            //~ for(const Geometry& linestring : data.linestrings) {
		    
		//~ glm::vec3 colour = Colour::Black; // TODO: should be inside
		//~ Vertices vertices(colour);
		//~ for(const Vec2& p : linestring) {
		    //~ vertices.position.emplace_back(p.x, p.y, 0.0f);
		//~ }
		//~ buffer.AddVertices(GL_LINE_STRIP, vertices);
            //~ }
	//~ });
	
	
	
	
	
	
        
        //~ const Sketch::RenderData& renderData = sketcher.Renderer().GetRenderData();
        
        //~ CopyData(renderData.elements.unselected,        settings.p.sketch.point.colour,     settings.p.sketch.line.colour);
        //~ CopyData(renderData.elements.failed,            { 1.0f, 0.0f, 0.0f },               { 1.0f, 0.0f, 0.0f });
        //~ CopyData(renderData.elements.hovered,           { 0.568f, 0.019f, 0.940f },         { 0.6f, 0.8f, 0.8f });
        //~ CopyData(renderData.elements.selected,          { 1.0f, 1.0f, 1.0f },               { 1.0f, 1.0f, 1.0f });
        
        //~ CopyData(renderData.constraints.unselected,     settings.p.sketch.point.colour,     settings.p.sketch.line.colour);
        //~ CopyData(renderData.constraints.failed,         { 1.0f, 0.0f, 0.0f },               { 1.0f, 0.0f, 0.0f });
        //~ CopyData(renderData.constraints.hovered,        { 0.568f, 0.019f, 0.940f },         { 0.6f, 0.8f, 0.8f });
        //~ CopyData(renderData.constraints.selected,       { 1.0f, 1.0f, 1.0f },               { 1.0f, 1.0f, 1.0f });
        
        //~ CopyData(renderData.preview,                    settings.p.sketch.point.colour,     settings.p.sketch.line.colour);
        //~ const Sketch::RenderData::Data& cursor = renderData.cursor;
        //~ // Change colour of selection box if left of click position
        //~ if(cursor.points.size() == 1) {
            //~ if(cursor.points[0].size() == 4) {
                //~ // change colour depending on whether selection box is to left or right
                //~ const glm::vec3& selectionBoxColour = ((cursor.points[0][2].x - cursor.points[0][0].x) < 0) ? glm::vec3(0.1f, 0.3f, 0.8f) : glm::vec3(0.306f, 0.959f, 0.109f);
                //~ CopyVertices(m_ViewerLineLists, cursor.linestrings, selectionBoxColour);
            //~ }
        //~ }
        
    };







        
    ElementFactory& f = sketcher.Factory();
    
    SketchItem l1 = f.AddLine({ 100.0, 100.0 }, { 200.0, 523.0 });
    SketchItem l2 = f.AddLine({ 200.0, 523.0 }, { 500.0, 500.0 });
    SketchItem l3 = f.AddLine({ 500.0, 500.0 }, { 100.0, 100.0 });
    
    //Circle* circle = m_Factory.AddCircle({ -6.0, -7.0 }, 200.0);
    
    //m_Factory.AddConstraint<Coincident_PointToPoint>(l1.p1, p1);
        
    f.AddConstraint<Coincident_PointToPoint>(l1.P1(), l2.P0());
    f.AddConstraint<Coincident_PointToPoint>(l2.P1(), l3.P0());
    f.AddConstraint<Coincident_PointToPoint>(l3.P1(), l1.P0());
    
    f.AddConstraint<Distance_PointToPoint>(l1, 300.0);
    f.AddConstraint<Distance_PointToPoint>(l2, 400.0);
    f.AddConstraint<Distance_PointToPoint>(l3, 550.0);        
     
     
     
    
    

    
//~ class SketchViewer
//~ {
//~ public:    
    //~ class GUI
    //~ {
    //~ public:
	
    //~ private:
	
    //~ } gui;

//~ private:
    
//~ };    

    
// Sketch LEVEL  
        // SketchTools
	// Draw ImGui Widgets
        auto DrawImGui_Tools = [&]() {
	    // Centre widget about toolbarItemHeight
	    auto ImGui_SameLineCentred = [&]() { 
		ImGui::SameLine();
		ImGuiModules::CentreItemVerticallyAboutItem(parameters.gui.imageButton_SketchPrimary.buttonSize.y, parameters.gui.imageButton_Sketch.buttonSize.y); 
	    };
	    
	    // Get currently active command
	    using CommandType = SketchEvents::CommandType;
	    CommandType type = sketcher.Events().GetCommandType();
	    
            //~ // Select Button
	    ImGui::BeginGroup();
	    
		// Add Select Button
		//~ if (ImGuiCustomModules::ImageButtonWithText_CentredVertically("Select##ToolbarButton", parameters.images.select, parameters.gui.imageButton_SketchPrimary, isActive, parameters.gui.frameHeight)) { 
		if(ImGuiModules::ImageButtonWithText("Select##ToolbarButton", parameters.images.select, &parameters.gui.imageButton_SketchPrimary, type == CommandType::Select)) {
		    sketcher.Events().SetCommandType(CommandType::Select);
		}
	    
		ImGui::SameLine();
		
		// Add Select Loop Button
		if(ImGuiModules::ImageButtonWithText("Select Loop##ToolbarButton", parameters.images.selectLoop, &parameters.gui.imageButton_SketchPrimary, type == CommandType::SelectLoop)) {
		    sketcher.Events().SetCommandType(CommandType::SelectLoop);
		}
	    
		ImGui_SameLineCentred();
		
		// Add Point Button
		if (ImGuiModules::ImageButtonWithText("Point##ToolbarButton", parameters.images.point, &parameters.gui.imageButton_Sketch, (type == CommandType::Add_Point))) {  
		     sketcher.Events().SetCommandType(CommandType::Add_Point);
		}
		
		ImGui_SameLineCentred();
		
		// Add Line Button
		if (ImGuiModules::ImageButtonWithText("Line##ToolbarButton", parameters.images.line, &parameters.gui.imageButton_Sketch, (type == CommandType::Add_Line))) { 
		     sketcher.Events().SetCommandType(CommandType::Add_Line);
		}
		ImGui_SameLineCentred();
		
		// Add Arc Button
		if (ImGuiModules::ImageButtonWithText("Arc##ToolbarButton", parameters.images.arc, &parameters.gui.imageButton_Sketch, (type == CommandType::Add_Arc))) { 
		     sketcher.Events().SetCommandType(CommandType::Add_Arc);
		}
		
		ImGui_SameLineCentred();
		// Add Circle Button
		
		if (ImGuiModules::ImageButtonWithText("Circle##ToolbarButton", parameters.images.circle, &parameters.gui.imageButton_Sketch, (type == CommandType::Add_Circle))) {  
		     sketcher.Events().SetCommandType(CommandType::Add_Circle);
		}
		ImGui_SameLineCentred();
		// Add Circle Button
		
		if (ImGuiModules::ImageButtonWithText("Square##ToolbarButton", parameters.images.square, &parameters.gui.imageButton_Sketch, (type == CommandType::Add_Square))) {  
		     sketcher.Events().SetCommandType(CommandType::Add_Square);
		}
	    ImGui::EndGroup();
        };
        
        auto DrawImGui_Constraints = [&]() {
            
            // Add Constraint Buttons
            ImGui::BeginGroup();
                // Centre contraint buttons about main toolbar buttons
                ImGuiModules::CentreItemVerticallyAboutItem(parameters.gui.frameHeight, parameters.gui.imageButton_Constraint.buttonSize.y);
                // TODO: Order of selection needs to be preserved - if circle then arc, circle should jump onto arc... atm it's just default order
                sketcher.Draw_ConstraintButtons(
		
		    // Draw ImGui Image Button
		    [&](const std::string& name, ConstraintType imageType) {
			// Get image texture
			ImageTexture texture = parameters.images.constraints.GetImageTextureByType(imageType);
			// Draw Imgui image buttons
			return ImGuiModules::ImageButtonWithText(name, texture, &parameters.gui.imageButton_Constraint);
		    }, 
		    
		    // Draw ImGui input values
		    [&](double* value) {	
			// Set inputbox width
			ImGui::SetNextItemWidth(parameters.gui.itemWidth);
			// Centre align to the constraint buttons
			ImGuiModules::CentreItemVerticallyAboutItem(parameters.gui.frameHeight);
			// Draw inputbox
			ImGui::InputDouble(std::string("##" + std::to_string((int)value)).c_str(), value, 0.0, 0.0, "%g"); // use value ptr as ID
		    }
		);
            ImGui::EndGroup();
        };
        
	
    



	auto DrawImGui_Colours = [&]() 
	{
	    if(ImGui::TreeNode("Colours")) {
		for(size_t i = 0; i < parameters.colours.data.Size(); i++) {
		    ImGui::ColorEdit3(parameters.colours.data[i].name.c_str(), &parameters.colours.data[i].value[0], ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoInputs);
		}
		ImGui::TreePop();
	    }
	};
	
	auto DrawImGui_Parameters = [&]() 
	{
	    ImGui::TextUnformatted("General"); 
	    ImGui::Indent(); 
		ImGui::SliderFloat("Point Size", &parameters.pointSize, 1.0f, 100.0f);
		ImGui::SliderFloat("Line Width", &parameters.lineWidth, 1.0f, 100.0f);
		ImGui::SliderFloat2("Image Size", &parameters.image.size[0], 1.0f, 100.0f);
		ImGui::SliderFloat("Image offset", &parameters.image.offset, 1.0f, 100.0f);
		ImGui::SliderFloat("Dimension Input Width", &parameters.gui.dimensionInputWidth, 1.0f, 100.0f);

        
		// Depth Function
		//~ ImGui::Combo("Depth Function", &parameters.depthFunction, "Never\0<\0=\0<=\0>\0!=\0>=\0Always\0\0");
		//~ static int imgui_wireframe = 0;
		//~ if(ImGui::Combo("Edges", &imgui_wireframe, "Show Edges\0Show Hidden Edges\0Hide Edges\0\0")) { // <=  Always
		    //~ if(imgui_wireframe == 0) { parameters.depthFunction = 3; } // <=
		    //~ if(imgui_wireframe == 1) { parameters.depthFunction = 7; } // Always
		    //~ if(imgui_wireframe == 2) { parameters.depthFunction = 0; } // Never
		//~ }
		
		ImGui::SameLine();
		ImGui::Text("%g Cursor Size", parameters.cursor.size);
		ImGui::SliderFloat2("Current Position Text Offset", &parameters.cursor.textOffset[0], -100.0f, 100.0f);
		
		//~ ImGui::SliderFloat("Cursor Snap Distance", &settings.p.sketch.cursor.SnapDistance, 0.1f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
		//~ ImGui::SameLine();
		//~ ImGui::Text("%g Scaled", settings.p.sketch.cursor.SnapDistance_Scaled);
	    ImGui::Unindent();  
	    
	    ImGui::Separator();
		 
	    ImGui::TextUnformatted("Axis"); ImGui::Indent();  
		ImGui::SliderFloat("Size", &parameters.axis.size, 1.0f, 500.0f);
		ImGui::SliderFloat("Width", &parameters.axis.width, 1.0f, 100.0f);
	    ImGui::Unindent();  ImGui::Separator();
	};
	    
	
	
    
    
    
    

    auto cb_Configure = [&]() {
	// set background colour
	glm::vec3& colour = parameters.colours.background.value;
	GLCall(glClearColor(colour.r, colour.g, colour.b, 1.0f));
	// Update Sketcher
	parameters.update.isRequired |= sketcher.Update();
    };
    
    auto cb_Draw_ImGui = [&]() {
	// Draw ImGui windows
	ImGuiModules::Dockspace::Begin();
	// show ImGui Demo 
	ImGui::ShowDemoWindow(NULL);
	
	
	static ImGuiModules::ImGuiWindow window_sketch("sketch_settings");
	if(window_sketch.Begin()) {
	
	    ImGui::Text("Sketch Parameters");
	    
	    sketcher.DrawImGui(); 
	    DrawImGui_Tools();	
	    DrawImGui_Constraints();
	    
	    DrawImGui_Parameters();
	    DrawImGui_Colours();
	    
	    window_sketch.End();
	}
	
    
	// End dockspace
	ImGuiModules::Dockspace::End();
    };
    
    // Set Dynamic / Stream / Text / Image buffers
    // Static buffers should only be set once, before the render loop
    auto cb_Set_DynamicBuffers = [&]() {
	
	// Return if no update required
	if(!parameters.update.isRequired) { return; }
	// update_required = false; update every frame
	
	
	
	
	// Sketch Buffers // TODO move these to sketchviewer.SetDynamicBuffer
	
	// Set Text Buffer
	buffers.SetBufferData(textBuffer_Sketch, SketchViewer_SetTextBuffer);
	// Set Image Buffer
	buffers.SetBufferData(imageBuffer_Sketch, SketchViewer_SetImageBuffer);
	// Set Dynamic Buffer
	buffers.SetBufferData(dynamicBuffer_Sketch, SketchViewer_SetDynamicBuffer);
	
	
	
	
	// Set Buffers
	
	float axisSize = viewer.ScaleToPx(parameters.axis.size);
	
	// Set Text Buffer
	buffers.SetBufferData(textBuffer, [&](TextBuffer& buffer) {
	    buffer.Add3DAxisLabels(Transform({0,0,0}, glm::vec3(axisSize, axisSize, axisSize)));
	});
	
	// Set Image Buffer
	buffers.SetBufferData(imageBuffer, [&](ImageBuffer& buffer) {
	    (void)buffer;
	});
	
	// Set Dynamic Buffer
	buffers.SetBufferData(dynamicBuffer, [&](DynamicBuffer& buffer) {
	
	    // Draw axis
	    buffer.lineWidth = parameters.axis.width;
	    buffer.AddAxes(Transform({0,0,0}, glm::vec3(axisSize, axisSize, axisSize)));
	    //~ buffer.AddShape(Shapes::Body_Cube(),      Colour::Green, Transform(MPos, scaleTool, {45.0f, angle }));
	});
    };
    
    // Run the render loop
    viewer.RenderLoop(glsys, cb_Configure, cb_Draw_ImGui, cb_Set_DynamicBuffers);
    
    
    return 0;
}

//~ } // end namespace Viewer
