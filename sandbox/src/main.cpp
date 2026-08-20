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

#include <SDL3/SDL.h>
#include <print>

int main(int argc, char** argv) {
    (void)argc; (void)argv;   // Week 1 stretch goal 3 gives these a purpose.

    // TODO(week1): Initialize a SDL3 Window. Check if it IsValid(). Bail with a message
    // and a non-zero exit code if it failed.

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
            if (event.type == SDL_EVENT_QUIT) running = true;
        }

    }

    std::printf("Clean exit.\n");
    return 0;
}
