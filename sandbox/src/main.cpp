// =============================================================================
//  WEEK 1 TEMPLATE - you implement this.
//
//  The naive 'while' loop. Poll, clear, present, repeat. It is naive in a 
//  specific, nameable way: it runs the simulation as fast as the machine 
//  allows, so the game runs at a different speed on different hardware. We 
//  will implement a fixed timestep and an accumulator. Until then, naive is 
//  correct; do not try to fix it early.
// =============================================================================

#include <engine/platform/Window.h>
#include <engine/Error.h>
#include <engine/Log.h>
#include <engine/Exception.h>
#include <SDL3/SDL.h>
#include <print>

int main(int argc, char** argv) {
    (void)argc; (void)argv;   // Week 1 stretch goal 3 gives these a purpose.

    // TODO(week1): Initialize a SDL3 Window. Check if it IsValid(). Bail with a message
    // and a non-zero exit code if it failed.

    try {
        eng::Window win("game", 640, 480);
        fire::verify(win.IsValid(), "window is not valid");
        fire::log::trace("window is valid");


        bool running = false;
        while (!running) {
            // TODO(week1): drain the SDL event queue with SDL_PollEvent.
            //   Set running = false on SDL_EVENT_QUIT or SDL_EVENT_WINDOW_CLOSE_REQUESTED.
            //   Do nothing for any other event type.
            //
            // Note the shape of this loop: poll until the queue is EMPTY, once per
            // frame. Handling one event per frame is a bug that looks like input
            // lag.
            //
            // TODO(week1): clear to a colour of your choosing, then present.

            SDL_Event event{};
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = true;
            }

            // COPIED FROM SDL EXAMPLE
            const double now = ((double)SDL_GetTicks()) / 1000.0;  /* convert from milliseconds to seconds. */
            /* choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
            const float red = (float)(0.5 + 0.5 * SDL_sin(now));
            const float green = (float)(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 2 / 3));
            const float blue = (float)(0.5 + 0.5 * SDL_sin(now + SDL_PI_D * 4 / 3));
            SDL_SetRenderDrawColorFloat(win.GetRenderer(), red, green, blue, SDL_ALPHA_OPAQUE_FLOAT);  /* new color, full alpha. */

            /* clear the window to the draw color. */
            SDL_RenderClear(win.GetRenderer());

            /* put the newly-cleared rendering on the screen. */
            SDL_RenderPresent(win.GetRenderer());
            // COPIED FROM SDL EXAMPLE
        }

        std::printf("Clean exit.\n");
    }
    catch (const fire::Exception& e)
    {
        std::println("ERROR: {}",e.what());
    }
    return 0;
}
