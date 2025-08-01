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
# define SOLOMON
# if defined(SOLOMON)
#  include "serial.h"
# else 

# include <Windows.h>
HANDLE init_serial7() { return (HANDLE)NULL; }
BOOL serial_write(HANDLE h, BYTE *input, u32 size) { return NULL; }
void free_serial(HANDLE h) {}
# endif 

// Really scuffed "cross-platform implementation"
#elif defined(PLATFORM_MAC)

void *init_serial7() { return NULL; }

# define serial_write(a, b, c)
# define free_serial(a) 

# define BYTE char
#endif

#define profile(_time_elapsed) for (f64 _i##__LINE__ = get_ms(), _first##__LINE__ = true; \
                                    _first##__LINE__; \
                                    _first##__LINE__ = false, _time_elapsed = get_ms() - _i##__LINE__)

#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BINARY_ZERO '0'
#define BINARY_ONE  '1'
#define binary(byte)  \
    ((byte) & 0x80 ? BINARY_ONE : BINARY_ZERO), \
    ((byte) & 0x40 ? BINARY_ONE : BINARY_ZERO), \
    ((byte) & 0x20 ? BINARY_ONE : BINARY_ZERO), \
    ((byte) & 0x10 ? BINARY_ONE : BINARY_ZERO), \
    ((byte) & 0x08 ? BINARY_ONE : BINARY_ZERO), \
    ((byte) & 0x04 ? BINARY_ONE : BINARY_ZERO), \
    ((byte) & 0x02 ? BINARY_ONE : BINARY_ZERO), \
    ((byte) & 0x01 ? BINARY_ONE : BINARY_ZERO)

#if defined(PLATFORM_WIN32)
# define WEBCAM 0
#elif defined(PLATFORM_MAC)
# define WEBCAM 1
//# undef WEBCAM
#else
# error "Unknown platform"
#endif

static i32 lower_threshold = 100, upper_threshold = 200;

f64 get_ms()
{
    static u64 freq = SDL_GetPerformanceFrequency();
    u64 counter = SDL_GetPerformanceCounter();

    return ((f64)counter/(f64)freq)*1000.0f;
}
// TODO(sir-w7): Write platform layer.
// Will implement SDLRenderer interface because that's probably all we need. Can
// switch to OpenGL later if necessary. 
int main()
{
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);
    
#ifdef WEBCAM
#if defined(PLATFORM_WIN32)
    cv::VideoCapture cap(WEBCAM, cv::CAP_DSHOW);
#elif defined(PLATFORM_MAC)
    cv::VideoCapture cap(WEBCAM);
#endif
    if (cap.isOpened() == false) {
        fprintln(stderr, "Cannot open webcam.");
        return -1;
    }
    
    f64 w = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    f64 h = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    
    cap.set(cv::CAP_PROP_FPS, 60);

    println("Webcam dimensions: %f x %f", w, h);
#else
    cv::Mat image;
    cv::imread("../res/sfgate.jpg", image, cv::IMREAD_COLOR);
    assert(!image.empty());

    f64 w = image.size().width;
    f64 h = image.size().height;
#endif
    
	
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintln(stderr, "Failed to initialize video.");
        return -1;
    }
    
    SDL_Window *window = 0;
    SDL_Renderer *renderer = 0;
    
    f32 main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    
    if (!SDL_CreateWindowAndRenderer("Project Narcissus",
                                     (int)(h*main_scale), (int)(h*main_scale),
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

#ifdef WEBCAM
    cv::Mat tmp; 
    cap.read(tmp);
#else
    cv::Mat tmp = image;
#endif
    SDL_Texture *tex_resized = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_BGR24,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         64, 64);
    SDL_Texture *tex_full = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_BGR24,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         tmp.rows, tmp.rows);

    SDL_SetTextureScaleMode(tex_resized, SDL_SCALEMODE_NEAREST);
    defer { SDL_DestroyTexture(tex_resized); };
    defer { SDL_DestroyTexture(tex_full); };
#if defined(PLATFORM_WIN32) 
    auto M4 = init_serial7();    
    defer { free_serial(M4); };
#endif 

    f64 image_processing = 0.0f, rendering = 0.0f, processing_array = 0.0f, writing_array = 0.0f, webcam = 0.0f;
    bool greyscale = true, blur = true, canny = true, resize = true;
    f64 time_elapsed = 0; 
    f64 prev = get_ms(); 

    //u8 final[64*64] = {};
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

        cv::Mat final_to_send, final_for_rendering;
#ifdef WEBCAM
        cv::Mat frame;
        profile(webcam) {
            if(!cap.read(frame)) {
                fprintln(stderr, "Video camera is disconnected.");
                break;
            }
        }

        profile(image_processing) {
            cv::Mat cropped = frame(cv::Range(0, frame.size().height),
                                    cv::Range(frame.size().width/2-frame.size().height/2,
                                              frame.size().width/2+frame.size().height/2));
            cv::Mat working_mat = cropped.clone(), tmp_mat = cropped;

            if (greyscale) {
                cv::cvtColor(tmp_mat, working_mat, cv::COLOR_BGR2GRAY);
                tmp_mat = working_mat.clone();
            }
#else
        profile(image_processing) {
            //cv::cvtColor(image, greyscale, cv::COLOR_BGR2GRAY);
            cv::Mat cropped = image(cv::Range(0, image.size().height),
                                    cv::Range(image.size().width/2-image.size().height/2,
                                              image.size().width/2+image.size().height/2));

            cv::Mat working_mat = cropped.clone(), tmp_mat = cropped;
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

            final_to_send = working_mat;

            if (greyscale) {
                cv::cvtColor(tmp_mat, final_for_rendering, cv::COLOR_GRAY2BGR);
            } else {
                final_for_rendering = working_mat;
            }
            if (resize) {
                SDL_UpdateTexture(tex_resized, 0, 
                                  (void *)final_for_rendering.data, 
                                  final_for_rendering.step1());
            } else {
                SDL_UpdateTexture(tex_full, 0, 
                                  (void *)final_for_rendering.data,
                                  final_for_rendering.step1());
            }
        }

        profile(rendering) {
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            {
                ImGui::Begin("Debug Window");
                defer { ImGui::End(); };

                ImGui::Text("%f FPS", (1.0f/((f32)time_elapsed/1000.f)));
                ImGui::Text("Time since last frame: %f ms", time_elapsed);

                ImGui::Spacing();

                if (ImGui::CollapsingHeader("Profiler")) {
                    ImGui::Text("Getting webcam data: %f ms", webcam);
                    ImGui::Text("Image processing: %f ms", image_processing);
                    ImGui::Text("Rendering: %f ms", rendering);
                    ImGui::Text("Processing array: %f ms", processing_array);
                    ImGui::Text("Writing to serial connection: %f ms", writing_array);
                }

                ImGui::Spacing();

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
        }
        // Encoding compression "algorithm" thing
        profile(processing_array) {
            u8 array[64*64/8] = {};
            u32 write_at = 0;
            for (i32 y = 0; y < 64; ++y) {
                for (i32 x = 0; x < 64; x += 8) {
                    u8 byte = 0;
                    for (i32 i = 0; i < 8; ++i) {
                        byte |= (final_to_send.at<u8>(y, x+i)?1:0)<<(7-i);
                    }

                    //printf(BINARY_PATTERN, binary(byte));
                    array[write_at] = byte;
                    write_at++;
                }
                //println("");
            }
        }

/*
        for (int i = 0; i < final_to_send.rows; ++i) {
            for (int j = 0; j < final_to_send.cols; ++j) {
                 final[i*64+j] = final_to_send.at<u8>(i, j)?1:0; 
            }
        } */

/*
        profile(processing_array) {
            for (int i = 0; i < final_to_send.rows; ++i) {
                for (int j = 0; j < final_to_send.cols; j += 8) {
                    u8 byte = 0x00;
                    
                    for (i32 x = 0; x < 8; ++x) {
                        u8 pixel = final_to_send.at<i32>(i, j+x) ? 1 : 0; 
                        u8 bit_position = 8-x;
                        byte |= pixel << bit_position;
                    }

                    println(BINARY_PATTERN, binary(byte));
                }
            } 
        }
*/
#if defined(PLATFORM_WIN32)
        profile(writing_array) {
            serial_write(M4, array, sizeof(BYTE)*64*64/8);  
        }
#endif
        
        // Cap to 60 fps. 
        auto now = get_ms();
        
        // For some reason, the time elapsed is almost constant. 
        // I don't know if it's my fault, or OpenCV internally caps FPS. 
        // In any case, the time_elapsed looks like it's capped at 30 FPS.
        time_elapsed = now-prev;
        if (time_elapsed <= 16.67f) {
            SDL_Delay(16.67f-time_elapsed);
        }
        
        prev = now;
    }
/*
    for (i32 i = 0; i < 8; ++i) {
        auto test = 1 << (7-i);
        println(BINARY_PATTERN, binary(test));
    }
    println("");
    for (i32 i = 0; i < 64; ++i) {
        for (i32 j = 0; j < 64; ++j) {
            printf("%c", final[i*64+j]?'X':' ');
        }
        println("");
    }
    println("");
    for (i32 i = 0; i < 64; ++i) {
        for (i32 j = 0; j < 64; j+=8) {
            u8 byte = 0;
            for (i32 x = 0; x < 8; ++x) {
                byte |= (final[i*64+(j+x)]?1:0)<<(7-x);
            }

            printf(BINARY_PATTERN, binary(byte));
        }
        println("");
    }*/
    println("Lower threshold: %d", lower_threshold);
    println("Upper threshold: %d", upper_threshold);
    
    return 0;
}
