
#include "sketch_params.h"

using namespace std;

namespace Sketch {
    
using namespace MaxLib::Geom;

    SketchParameters::Colours::Colour& SketchParameters::Colours::Add(std::string name, glm::vec3 value) { 
        return data.Add(SketchParameters::Colours::Colour(name, value)); 
    }


    SketchParameters::GUI::GUI() {
        // Get IO
        ImGuiIO& io = ImGui::GetIO();
           
        // Load Fonts (primary first)
        font_medium = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 17.0f);
        if(!font_medium)
            cout << "Error: Could not find font: Geomanist 17" << endl;
        font_small = io.Fonts->AddFontFromMemoryCompressedTTF(geomanist_compressed_data, geomanist_compressed_size, 13.0f);
        if(!font_small)
            cout << "Error: Could not find font: Geomanist 13" << endl;
            
        imageButton_SketchPrimary 	= ImGuiModules::ImageButtonStyle("Toolbar SketchPrimary",	ImVec2(70.0f, 70.0f),	imageSize, 	font_small, 	imageButtonOffset_Vertical);
        imageButton_Sketch 	        = ImGuiModules::ImageButtonStyle("Toolbar Sketch",         	ImVec2(46.0f, 60.0f),	imageSize, 	font_small, 	imageButtonOffset_Vertical);
        imageButton_Constraint 	    = ImGuiModules::ImageButtonStyle("Toolbar Constraints",  	ImVec2(70.0f, 50.0f),	imageSize, 	font_small, 	imageButtonOffset_Constraint);
    
    }
} // end namespac Sketch
