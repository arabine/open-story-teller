#ifndef OST_WRAPPER_H
#define OST_WRAPPER_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <SDL.h>

void draw_ost_device(SDL_Renderer *renderer, double deltaTime);

#ifdef __cplusplus
}
#endif

#endif // OST_WRAPPER_H
