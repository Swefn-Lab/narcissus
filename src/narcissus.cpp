// Consider writing an OpenGL backend and platform layer (quite honestly 
// beyond the scope of this project but will be entertaining).
// I would like to be able to use Dear ImGUI in this, as OpenCV is proving quite
// annoying. 


// Late night ideas (22:47): Add SDL2, OpenGL, and DearImgui dependency. Will be
// annoying at first but will greatly improve my quality of life. The OpenCV
// highgui library is an absolute abomination that the world will be better
// without. 


// On that same note, it would probably be better to handroll a quick platform
// layer. I don't want a dependency on OpenCV's GUI interface.


#include <stdio.h>

#include "SDL3/SDL.h"

#include "opencv2/opencv.hpp"

// Bruh let's just unity build bc why not?
#include "imgui.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"
#include "imgui_draw.cpp"
//#include "imgui_demo.cpp"

#include "imgui_impl_sdl3.cpp"
#include "imgui_impl_sdlrenderer3.cpp"

#include "layer.h"

#if defined(PLATFORM_WIN32)
# define WEBCAM 0
#elif defined(PLATFORM_MAC)
# define WEBCAM 1
#else
# error "Unknown platform"
#endif

static i32 lower_threshold = 100, upper_threshold = 200;

// TODO(sir-w7): Write platform layer.
// Will implement SDLRenderer interface because that's probably all we need. Can
// switch to OpenGL later if necessary. 
int main()
{
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);
    
#ifdef WEBCAM
    cv::VideoCapture cap(WEBCAM, cv::CAP_DSHOW);
    
    if (cap.isOpened() == false) {
        fprintln(stderr, "Cannot open webcam.");
        return -1;
    }
    
    f64 w = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    f64 h = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    
    println("Webcam dimensions: %f x %f", w, h);
#else
    cv::Mat image;
    cv::imread("../res/sfgate.jpg", image, cv::IMREAD_GRAYSCALE);
    assert(!image.empty());
#endif
    
    cap.set(cv::CAP_PROP_FPS, 60);
	
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintln(stderr, "Failed to initialize video.");
        return -1;
    }
    
    SDL_Window *window = 0;
    SDL_Renderer *renderer = 0;
    
    f32 main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    
    if (!SDL_CreateWindowAndRenderer("Project Narcissus",
                                     (int)(2*w*main_scale), (int)(2*h*main_scale),
                                     SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY,
                                     &window, &renderer)) {
        fprintln(stderr, "Failed to create window."); //
        return -1;
    }

    SDL_RaiseWindow(window);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    
    
    bool running = true;
    
    i32 scale_interpolation = 0;

    cv::Mat tmp; 
    cap.read(tmp);
    SDL_Texture *tex_resized = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_BGR24,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         64, 64);
    SDL_Texture *tex_full = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_BGR24,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         tmp.cols, tmp.rows);

    SDL_SetTextureScaleMode(tex_resized, SDL_SCALEMODE_NEAREST);
    defer { SDL_DestroyTexture(tex_resized); };
    defer { SDL_DestroyTexture(tex_full); };
    
    bool greyscale = true, blur = true, canny = true, resize = true;
    u64 time_elapsed = 0; 
    auto prev = SDL_GetTicks(); 
    while (running) {
        SDL_Event event = {};
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                default: break;
            }
        }
        
#ifdef WEBCAM
        cv::Mat frame;
        if(!cap.read(frame)) {
            fprintln(stderr, "Video camera is disconnected.");
            break;
        }
#endif
        
        cv::Mat working_mat = frame.clone(), tmp_mat = frame;
        
#ifdef WEBCAM
        if (greyscale) {
            cv::cvtColor(tmp_mat, working_mat, cv::COLOR_BGR2GRAY);
            tmp_mat = working_mat.clone();
        }
#else
        //cv::cvtColor(image, greyscale, cv::COLOR_BGR2GRAY);
        working_mat = image;
#endif
        if (blur) {
            cv::GaussianBlur(tmp_mat, working_mat, cv::Size(5, 5), 0);
            tmp_mat = working_mat.clone();
        }

        if (resize) {
            cv::resize(tmp_mat, working_mat, cv::Size(64, 64), 0, 0, scale_interpolation);
            tmp_mat = working_mat.clone(); 
        }
        
        if (canny){
            cv::Canny(tmp_mat, working_mat, lower_threshold, upper_threshold);
            tmp_mat = working_mat.clone(); 
        }

        cv::Mat final;

        if (greyscale) {
            cv::cvtColor(tmp_mat, final, cv::COLOR_GRAY2BGR);
        } else {
            final = working_mat;
        }

        if (resize) {
            SDL_UpdateTexture(tex_resized, 0, (void *)final.data, final.step1());
        } else {
            SDL_UpdateTexture(tex_full, 0, (void *)final.data, final.step1());
        }
        
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        {
            ImGui::Begin("Debug Window");
            defer { ImGui::End(); };
            
            ImGui::Text("%d FPS", (int)(1.0f/((f32)time_elapsed/1000.f)));
            ImGui::Text("Time since last frame: %lld ms", time_elapsed);
            ImGui::SliderInt("Lower threshold", &lower_threshold, 0, 255);
            ImGui::SliderInt("Upper threshold", &upper_threshold, 0, 255);

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Scaling interpolation options")) {
                ImGui::RadioButton("INTER_NEAREST", &scale_interpolation, 0);
                ImGui::RadioButton("INTER_LINEAR", &scale_interpolation, 1);
                ImGui::RadioButton("INTER_CUBIC", &scale_interpolation, 2);
                ImGui::RadioButton("INTER_AREA", &scale_interpolation, 3);
                ImGui::RadioButton("INTER_LANCZOS4", &scale_interpolation, 4);
                ImGui::RadioButton("INTER_LINEAR_EXACT", &scale_interpolation, 5);
                ImGui::RadioButton("INTER_NEAREST_EXACT", &scale_interpolation, 6);
                // ImGui::RadioButton("INTER_MAX", &scale_interpolation, 7);
                // ImGui::RadioButton("INTER_FILL_OUTLIERS", &scale_interpolation, 8);
                // ImGui::RadioButton("INTER_INVERSE_MAP", &scale_interpolation, 16);
            }
            
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Processing options")) {
                ImGui::Checkbox("Greyscale", &greyscale);
                ImGui::Checkbox("Blur", &blur);
                ImGui::Checkbox("Resize", &resize);
                ImGui::Checkbox("Canny", &canny);
            }

        }
        
        ImGui::Render();
        SDL_SetRenderScale(renderer,
                           io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 0.0f);
        SDL_RenderClear(renderer);
        
        if (resize) {
            SDL_RenderTexture(renderer, tex_resized, 0, 0);  
        } else {
            SDL_RenderTexture(renderer, tex_full, 0, 0);  
        }

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        
        // Cap to 60 fps. 
        auto now = SDL_GetTicks();
        
        // For some reason, the time elapsed is almost constant. 
        // I don't know if it's my fault, or OpenCV internally caps FPS. 
        // In any case, the time_elapsed looks like it's capped at 30 FPS.
        time_elapsed = now-prev;
        if (time_elapsed <= 17) {
            SDL_Delay(17-time_elapsed);
        }
        
        prev = now;
    }
    
    println("Lower threshold: %d", lower_threshold);
    println("Upper threshold: %d", upper_threshold);
    
    return 0;
}
