#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <MaxLib.h>
//~ #include "deps/Geos/Geos.h"
#include "deps/GLViewer/glviewer.h"
#include "sketch.h"



namespace Sketch {



    // Should be constructed after GLSystem
    struct SketchParameters
    {
        // Note: See ElementFactory::Parameters for more
        double time = 0;
        float pointSize = 6.0f;
        float lineWidth = 2.0f;
        int  depthFunction   = GL_ALWAYS; // GL_LEQUAL
    
        struct Image
        {
            glm::vec2 size = { 20.0f, 20.0f };
            float offset = 10.0f;
        } image;
        
        struct Cursor {
            //~ struct Popup {
                //~ bool shouldOpen = false;
            //~ } popup;
            
            //~ std::optional<glm::vec2> Position_Snapped;      // snapped 2d mouse position in current coord sys
            //~ std::optional<glm::vec2> Position_Clicked;      // snapped 2d mouse click position in current coord sys
            //~ std::optional<glm::vec2> Position_Raw;          // raw 2d mouse position in current coord sys
            //~ std::optional<glm::vec2> Position_WorldCoords;  // snapped 2d mouse position in world space
    
            float size = 14.0f; // mm
            //~ float sizeScaled;          // gets updated with change in zoom
            
            
            //~ float snapDistance          = 5.0f; // mm
            //~ float snapDistanceScaled;  // gets updated with change in zoom
            //~ glm::vec2 SnapCursor(const glm::vec2& cursorPos) {
                //~ return roundVec2(snapDistanceScaled, cursorPos);
            //~ }
            glm::vec2 textOffset = { 24.0f, -10.0f };
        } cursor;
        
        struct Axis {
            float size = 100.0f;
            float width = 2.0f;
        } axis;                          
            
        
        struct GUI
        {
            
            float frameHeight 	=  	88.0f; // was m_Settings->guiSettings.toolbarItemHeight
            float itemWidth	    =  	80.0f; // was m_Settings->guiSettings.toolbarWidgetWidth

            float dimensionInputWidth = 50.0f; // TODO: Not currently used
            
            ImVec2 imageSize	= 	{ 24.0f, 24.0f };

            const float CENTRED = 	0.0f;
            // Image Button text / image offsets                            Text Offset             Image Offset
            ImGuiModules::ImageButtonStyle::Offsets imageButtonOffset_Vertical        = { { CENTRED,  17.0f },    { CENTRED,  -10.0f } };
            ImGuiModules::ImageButtonStyle::Offsets imageButtonOffset_Constraint      = { { CENTRED,  12.0f },    { CENTRED,  -7.0f } };

            ImFont* font_medium;
            ImFont* font_small;
            
            ImGuiModules::ImageButtonStyle imageButton_SketchPrimary;
            ImGuiModules::ImageButtonStyle imageButton_Sketch;
            ImGuiModules::ImageButtonStyle imageButton_Constraint;

            // Constructor
            GUI();
                  
        } gui;
        

        struct Colours
        {
            struct Colour { 
                std::string name; glm::vec3 value; 
                Colour(const std::string& _name, const glm::vec3& _value) : name(_name), value(_value) {}
            };
            MaxLib::Vector::Vector_Ptrs<Colour> data;
            Colour& Add(std::string name, glm::vec3 value);
            
            // TODO: Add alpha
            
            // Sketch colours
            Colour& sketch_elements_points_unselected	    = Add("Points - Unselected",		        { 0.857f, 0.911f, 0.062f } );
            Colour& sketch_elements_points_selected	        = Add("Points - Selected",			        { 0.294f, 0.960f, 0.000f } );
            Colour& sketch_elements_points_hovered		    = Add("Points - Hovered",			        { 0.620f, 0.895f, 0.567f } );
            Colour& sketch_elements_points_failed		    = Add("Points - Failed",			        { 1.0f, 0.0f, 0.0f } );
                                                
            Colour& sketch_elements_lines_unselected	    = Add("Lines - Unselected",			        { 0.246f, 0.246f, 0.246f } );
            Colour& sketch_elements_lines_selected		    = Add("Lines - Selected",			        { 1.0f, 1.0f, 1.0f } );
            Colour& sketch_elements_lines_hovered		    = Add("Lines - Hovered",			        { 0.6f, 0.8f, 0.8f } );
            Colour& sketch_elements_lines_failed		    = Add("Lines - Failed",				        { 1.0f, 0.0f, 0.0f } );
            
            Colour& sketch_elements_images_unselected	    = Add("Images - Unselected",		        { 0.885f, 0.915f, 0.173f } );
            Colour& sketch_elements_images_selected	        = Add("Images - Selected",			        { 0.815f, 0.500f, 0.419f } );
            Colour& sketch_elements_images_hovered		    = Add("Images - Hovered",			        { 0.568f, 0.019f, 0.940f } );
            Colour& sketch_elements_images_failed		    = Add("Images - Failed",			        { 1.0f, 0.0f, 0.0f } );
                                                
            Colour& sketch_elements_texts_unselected	    = Add("Texts - Unselected",			        { 0.246f, 0.246f, 0.246f } );
            Colour& sketch_elements_texts_selected		    = Add("Texts - Selected",			        { 1.0f, 1.0f, 1.0f } );
            Colour& sketch_elements_texts_hovered		    = Add("Texts - Hovered",			        { 0.6f, 0.8f, 0.8f } );
            Colour& sketch_elements_texts_failed		    = Add("Texts - Failed",				        { 1.0f, 0.0f, 0.0f } );
            
            
            
            Colour& sketch_constraints_points_unselected	= Add("Constraints - Points - Unselected",	{ 1.0f, 1.0f, 1.0f } );
            Colour& sketch_constraints_points_selected	    = Add("Constraints - Points - Selected",	{ 1.0f, 1.0f, 1.0f } );
            Colour& sketch_constraints_points_hovered	    = Add("Constraints - Points - Hovered",		{ 0.568f, 0.019f, 0.940f } );
            Colour& sketch_constraints_points_failed	    = Add("Constraints - Points - Failed",		{ 1.0f, 0.0f, 0.0f } );
                                    
            Colour& sketch_constraints_lines_unselected	    = Add("Constraints - Lines - Unselected",	{ 0.234f, 0.710f, 0.806f } );
            Colour& sketch_constraints_lines_selected	    = Add("Constraints - Lines - Selected",		{ 1.0f, 1.0f, 1.0f } );
            Colour& sketch_constraints_lines_hovered	    = Add("Constraints - Lines - Hovered",		{ 0.6f, 0.8f, 0.8f } );
            Colour& sketch_constraints_lines_failed	        = Add("Constraints - Lines - Failed",		{ 1.0f, 0.0f, 0.0f } );
            
            Colour& sketch_constraints_images_unselected	= Add("Constraints - Images - Unselected",	{ 1.0f, 1.0f, 1.0f } );
            Colour& sketch_constraints_images_selected	    = Add("Constraints - Images - Selected",    { 0.019f, 0.454f, 0.476f });
            Colour& sketch_constraints_images_hovered	    = Add("Constraints - Images - Hovered",		{ 0.581f, 0.863f, 0.863f } );
            Colour& sketch_constraints_images_failed	    = Add("Constraints - Images - Failed",		{ 1.0f, 0.0f, 0.0f } );
            
            Colour& sketch_constraints_texts_unselected	    = Add("Constraints - Texts - Unselected",	{ 1.0f, 1.0f, 1.0f } );
            Colour& sketch_constraints_texts_selected	    = Add("Constraints - Texts - Selected",     { 1.0f, 1.0f, 1.0f } );
            Colour& sketch_constraints_texts_hovered	    = Add("Constraints - Texts - Hovered",		{ 0.6f, 0.8f, 0.8f } );
            Colour& sketch_constraints_texts_failed	        = Add("Constraints - Texts - Failed",		{ 1.0f, 0.0f, 0.0f } );
                               
                               
                               
            Colour& sketch_preview_points			        = Add("Preview - Points", 			        { 1.0f, 1.0f, 1.0f } );
            Colour& sketch_preview_lines			        = Add("Preview - Lines",			        { 1.0f, 1.0f, 1.0f } );
            Colour& sketch_preview_images			        = Add("Preview - Images", 			        { 1.0f, 1.0f, 1.0f } );
            Colour& sketch_preview_texts			        = Add("Preview - Texts",			        { 1.0f, 1.0f, 1.0f } );
                                     
                    
            Colour& sketch_cursor_points			        = Add("Cursor - Points",			        { 1.0f, 1.0f, 1.0f } );
            //~ Colour& sketch_cursor_lines			            = Add("Cursor - Lines",				        { 0.0f, 0.0f, 1.0f } );
            Colour& sketch_cursor_lines_left			    = Add("Selection Box (Left)",				{ 0.482f, 0.641f, 0.403f } );
            Colour& sketch_cursor_lines_right			    = Add("Selection Box (Right)",				{ 0.368f, 0.581f, 0.730f } );
            Colour& sketch_cursor_images			        = Add("Cursor - Images",			        { 1.0f, 1.0f, 1.0f } );
            Colour& sketch_cursor_texts			            = Add("Cursor - Texts",				        { 0.9f, 0.9f, 0.9f } );
                                                           
            // Grid                                                 
            Colour& grid_coarse				                = Add("Grid - Coarse",				        { 0.95f, 0.95f, 0.95f } );	// Requires new static buffer on change
            Colour& grid_fine				                = Add("Grid - Fine", 				        { 0.8f, 0.8f, 0.8f } );	// Requires new static buffer on change
            // Viewport                 	                	  	        
            Colour& background				                = Add("Background", 				        { 0.670f, 0.729f, 0.766f } );	// { 0.669f, 0.741f, 0.758f };
            
        } colours;


        struct Images
        {
            // sketch images
            ImageTexture sketch 	    = { File::ThisDir("img/img_sketch.png").c_str() };
            ImageTexture draw 		    = { File::ThisDir("img/img_sketch_draw.png").c_str() };
            ImageTexture measure 	    = { File::ThisDir("img/img_sketch_measure.png").c_str() };
            ImageTexture select 	    = { File::ThisDir("img/img_sketch_select.png").c_str() };
            ImageTexture selectLoop 	= { File::ThisDir("img/img_sketch_selectloop.png").c_str() };
            ImageTexture point 		    = { File::ThisDir("img/img_sketch_point.png").c_str() };
            ImageTexture line 		    = { File::ThisDir("img/img_sketch_line.png").c_str() };
            ImageTexture arc 		    = { File::ThisDir("img/img_sketch_arc.png").c_str() };
            ImageTexture circle 	    = { File::ThisDir("img/img_sketch_circle.png").c_str() };
            ImageTexture square 	    = { File::ThisDir("img/img_sketch_square.png").c_str() };
            
            struct Constraints
            {
            ImageTexture coincident 	= { File::ThisDir("img/img_constraint_coincident.png").c_str() };
            ImageTexture midpoint 		= { File::ThisDir("img/img_constraint_midpoint.png").c_str() };
            ImageTexture vertical 		= { File::ThisDir("img/img_constraint_vertical.png").c_str() };
            ImageTexture horizontal 	= { File::ThisDir("img/img_constraint_horizontal.png").c_str() };
            ImageTexture parallel 		= { File::ThisDir("img/img_constraint_parallel.png").c_str() };
            ImageTexture perpendicular 	= { File::ThisDir("img/img_constraint_perpendicular.png").c_str() };
            ImageTexture tangent 		= { File::ThisDir("img/img_constraint_tangent.png").c_str() };
            ImageTexture equal 		    = { File::ThisDir("img/img_constraint_equal.png").c_str() };
            ImageTexture distance 		= { File::ThisDir("img/img_constraint_distance.png").c_str() };
            ImageTexture radius 		= { File::ThisDir("img/img_constraint_radius.png").c_str() };
            ImageTexture angle 		    = { File::ThisDir("img/img_constraint_angle.png").c_str() };
            
            
            ImageTexture GetImageTextureByType(Sketch::ConstraintType type) {
                if(type == Sketch::ConstraintType::Coincident)   { return coincident; }
                if(type == Sketch::ConstraintType::Midpoint)     { return midpoint; }
                if(type == Sketch::ConstraintType::Vertical)     { return vertical; }
                if(type == Sketch::ConstraintType::Horizontal)   { return horizontal; }
                if(type == Sketch::ConstraintType::Parallel)     { return parallel; }
                if(type == Sketch::ConstraintType::Perpendicular){ return perpendicular; }
                if(type == Sketch::ConstraintType::Tangent)      { return tangent; }
                if(type == Sketch::ConstraintType::Equal)        { return equal; }
                if(type == Sketch::ConstraintType::Distance)     { return distance; }
                if(type == Sketch::ConstraintType::Radius)       { return radius; }
                if(type == Sketch::ConstraintType::Angle)        { return angle; }
                assert(0 && "Unknown constraint type");
            }
            } constraints;
        } images;
        
        struct Update {
            bool isRequired = true;
        } update;
        
    };

} // end namespace Sketch
