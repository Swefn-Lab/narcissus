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

// Unity building this because the Makefile doesn't accommodate the module
// compilation of a single file too well (only the entire directory).
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

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

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintln(stderr, "Failed to initialize video.");
        return -1;
    }

    SDL_Window *window = 0;
    SDL_Renderer *renderer = 0;

    f32 main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    if (!SDL_CreateWindowAndRenderer("Narcissus",
                                     (int)(1280*main_scale), (int)(720*main_scale),
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

#ifdef WEBCAM
    cv::VideoCapture cap(WEBCAM);

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
    bool running = true;
   
    cv::Mat tmp; 
    cap.read(tmp);
    SDL_Texture* tex = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_BGR24,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         tmp.cols, tmp.rows);
    defer { SDL_DestroyTexture(tex); };

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

        cv::Mat greyscale, blurred, edge, resized;

#ifdef WEBCAM
        cv::cvtColor(frame, greyscale, cv::COLOR_BGR2GRAY);
#else
        //cv::cvtColor(image, greyscale, cv::COLOR_BGR2GRAY);
        greyscale = image;
#endif
        cv::GaussianBlur(greyscale, blurred, cv::Size(5, 5), 0);
        cv::Canny(blurred, edge, lower_threshold, upper_threshold);

        cv::Mat final;
        cv::cvtColor(edge, final, cv::COLOR_GRAY2BGR);

        // cv::resize(edge, resized, cv::Size(64, 64), cv::INTER_AREA);
        // cv::imshow(window, resized);
        // cv::imshow(window, edge);
        //cv::imshow(window, edge);
        //
        SDL_UpdateTexture(tex, 0, (void *)final.data, final.step1());

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
        }

        ImGui::Render();
        SDL_SetRenderScale(renderer,
                           io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer, 0.0f, 0.0f, 0.0f, 0.0f);
        SDL_RenderClear(renderer);

        SDL_RenderTexture(renderer, tex, 0, 0);
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
