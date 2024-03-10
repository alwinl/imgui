/*
 * glfw_glew.cc Copyright 2024 Alwin Leerling dna.leerling@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <stdexcept>
#include <array>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "load_shaders.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class ImGUIDirector
{
public:
	ImGUIDirector( GLFWwindow *window );
	~ImGUIDirector();

	void render_gui();

	std::tuple<float, float, float, float> get_background_colour() { return std::make_tuple(clear_color.x, clear_color.y,clear_color.z, clear_color.w); }

private:
    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	void make_hello_window();
	void make_other_window();
};

ImGUIDirector::ImGUIDirector( GLFWwindow *window )
{
    const char* glsl_version = "#version 130";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( glsl_version );
}

ImGUIDirector::~ImGUIDirector()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
}

void ImGUIDirector::make_hello_window()
{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	ImGui::Checkbox("Another Window", &show_another_window);

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

	ImGui::End();
}

void ImGUIDirector::make_other_window()
{
	ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

	ImGui::Text("Hello from another window!");
	if (ImGui::Button("Close Me"))
		show_another_window = false;

	ImGui::End();
}

void ImGUIDirector::render_gui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	make_hello_window();
	if( show_another_window )
		make_other_window();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

class DemoApp
{
public:
	DemoApp( int argc, char **argv );
	~DemoApp();

	DemoApp( DemoApp & ) = delete;
	DemoApp( DemoApp && ) = delete;
	DemoApp &operator=( DemoApp & ) = delete;
	DemoApp &operator=( DemoApp && ) = delete;

	int run();

private:
	void scene_setup();
	void loop();
	void scene_render() const;

	std::unique_ptr<ImGUIDirector> gui;

	unsigned int vao = -1;
	GLFWwindow *window = nullptr;
};

DemoApp::DemoApp( int /*argc*/, char ** /*argv*/ )
{
	if( glfwInit() == 0 )
		throw std::runtime_error( "Cannot initialise GLFW" );

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow( 640, 480, "ImGUI demo", nullptr, nullptr );
	if( window == nullptr ) {
		glfwTerminate();
		throw std::runtime_error( "Cannot create GLFW Window" );
	}

	glfwMakeContextCurrent( window );
    glfwSwapInterval(1);

	if( glewInit() != GLEW_OK )
		throw std::runtime_error( "Cannot load GLEW" );

	gui = std::make_unique<ImGUIDirector>( window );
}

DemoApp::~DemoApp()
{
	gui.reset();

    glfwDestroyWindow(window);
	glfwTerminate();
}

void DemoApp::scene_setup()
{
	constexpr unsigned int position_index = 0;
	constexpr unsigned int colour_index = 1;

	struct vertex {
		std::array<float, 3> position;
		std::array<float, 3> colour;
	};

	struct attribute_description {
		unsigned int index;
		int component_count;
		unsigned int component_type;
		unsigned char is_normalized;
		size_t offset;
	};

	const std::vector<attribute_description> vertex_description = {
		{ position_index, 3, GL_FLOAT, GL_FALSE, offsetof( vertex, position ) }, // position attribute
		{ colour_index, 3, GL_FLOAT, GL_FALSE, offsetof( vertex, colour ) }		 // colour attribute
	};

	std::vector<vertex> vertices = { { { -1.0F, -1.0F, 0.0F }, { 1.0F, 0.0F, 0.0F } },
									 { { 0.0F, 1.0F, 0.0F }, { 0.0F, 1.0F, 0.0F } },
									 { { 1.0F, -1.0F, 0.0F }, { 0.0F, 0.0F, 1.0F } } };

	const unsigned int program_id = load_program( "../res/shaders/simple.glsl" );
	unsigned int vertex_buffer = -1;

	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	glGenBuffers( 1, &vertex_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, static_cast<int64_t>( vertices.size() * sizeof( vertex ) ), vertices.data(),
				  GL_STATIC_DRAW );

	for( const auto &attribute : vertex_description ) {
		glEnableVertexAttribArray( attribute.index );
		glVertexAttribPointer( attribute.index, attribute.component_count, attribute.component_type,
							   attribute.is_normalized, sizeof( vertex ),
							   /* trunk-ignore(clang-tidy/cppcoreguidelines-pro-type-reinterpret-cast) */
							   /* trunk-ignore(clang-tidy/performance-no-int-to-ptr) */
							   reinterpret_cast<const void *>( attribute.offset ) );
	}

	glUseProgram( program_id );

	glBindVertexArray( 0 );
}

void DemoApp::scene_render() const
{
	glBindVertexArray( vao );

	glDrawArrays( GL_TRIANGLES, 0, 3 );
}

void DemoApp::loop()
{
	/* Loop until the user closes the window */
	while( glfwWindowShouldClose( window ) == 0 ) {

		/* Get input */
		glfwPollEvents();

		/* Update state */

		/* Render graphics */
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

		std::tuple<float, float, float, float> bg_colour = gui->get_background_colour();
        glClearColor(
			std::get<0>(bg_colour) * std::get<3>(bg_colour),
			std::get<1>(bg_colour) * std::get<3>(bg_colour),
			std::get<2>(bg_colour) * std::get<3>(bg_colour),
			std::get<3>(bg_colour)
		);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		scene_render();

		gui->render_gui();

		glfwSwapBuffers( window );
	}
}

int DemoApp::run()
{
	scene_setup();

	loop();

	return 0;
}

int main( int argc, char **argv )
{
	return DemoApp( argc, argv ).run();
}

