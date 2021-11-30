#include <SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_SDL(X)                                                           \
  if (X) {                                                                     \
    printf("SDL Error: %s\n", SDL_GetError());                                 \
    return -1;                                                                 \
  }

#define min(a, b) ((a) < (b) ? (a) : (b))

int SDL_RenderDrawCircle(SDL_Renderer *renderer, int x, int y, int radius) {
  int offsetx, offsety, d;
  int status;

  offsetx = 0;
  offsety = radius;
  d = radius - 1;
  status = 0;

  while (offsety >= offsetx) {
    status += SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
    status += SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
    status += SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
    status += SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
    status += SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
    status += SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
    status += SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
    status += SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

    if (status < 0) {
      status = -1;
      break;
    }

    if (d >= 2 * offsetx) {
      d -= 2 * offsetx + 1;
      offsetx += 1;
    } else if (d < 2 * (radius - offsety)) {
      d += 2 * offsety - 1;
      offsety -= 1;
    } else {
      d += 2 * (offsety - offsetx - 1);
      offsety -= 1;
      offsetx += 1;
    }
  }

  return status;
}

int SDL_RenderFillCircle(SDL_Renderer *renderer, int x, int y, int radius) {
  int offsetx, offsety, d;
  int status;

  offsetx = 0;
  offsety = radius;
  d = radius - 1;
  status = 0;

  while (offsety >= offsetx) {

    status += SDL_RenderDrawLine(renderer, x - offsety, y + offsetx,
                                 x + offsety, y + offsetx);
    status += SDL_RenderDrawLine(renderer, x - offsetx, y + offsety,
                                 x + offsetx, y + offsety);
    status += SDL_RenderDrawLine(renderer, x - offsetx, y - offsety,
                                 x + offsetx, y - offsety);
    status += SDL_RenderDrawLine(renderer, x - offsety, y - offsetx,
                                 x + offsety, y - offsetx);

    if (status < 0) {
      status = -1;
      break;
    }

    if (d >= 2 * offsetx) {
      d -= 2 * offsetx + 1;
      offsetx += 1;
    } else if (d < 2 * (radius - offsety)) {
      d += 2 * offsety - 1;
      offsety -= 1;
    } else {
      d += 2 * (offsety - offsetx - 1);
      offsety -= 1;
      offsetx += 1;
    }
  }

  return status;
}

typedef struct Point {
  int x;
  int y;
  struct Point *next;
} Point;

typedef struct State {
  Point *points;
} State;

int init_points(SDL_Renderer *renderer, State *state) {
  int output_width, output_height;
  CHECK_SDL(SDL_GetRendererOutputSize(renderer, &output_width, &output_height));

  state->points = NULL;
  for (size_t i = 0; i < 256 * 27 - 1; ++i) {
    Point *point = malloc(sizeof(Point));
    point->x = rand() % output_width;
    point->y = rand() % output_height;
    point->next = state->points;
    state->points = point;
  }
  return 0;
}

int destroy_points(State *state) {
  Point *point = state->points;
  while (point) {
    Point *next = point->next;
    free(point);
    point = next;
  }
  return 0;
}

int update_points(SDL_Renderer *renderer, State *state) {
  int output_width, output_height;
  CHECK_SDL(SDL_GetRendererOutputSize(renderer, &output_width, &output_height));
  const double speed = 60;
  const int jitter = 55;
  double average_x, average_y;
  int points;
  for (Point *point = state->points; point; point = point->next) {
    average_x += point->x;
    average_y += point->y;
    ++points;
  }
  average_x /= points;
  average_y /= points;
  for (Point *point = state->points; point; point = point->next) {
    Point *target = point->next;
    if (!target) target = state->points;
    // double da = sqrt(pow(point->x - average_x, 2) + pow(point->y - average_y, 2));
    int dx = (double)(target->x - point->x) + (double)(average_x - point->x) * -0.003 + (double)(rand() % jitter - jitter / 2);
    int dy = (double)(target->y - point->y) + (double)(average_y - point->y) * -0.003 + (double)(rand() % jitter - jitter / 2);
    if (dx == 0 && dy == 0) continue;
    double magnitude = sqrt(dx * dx + dy * dy);
    dx = dx / magnitude * min(magnitude, speed);
    dy = dy / magnitude * min(magnitude, speed);
    point->x += dx;
    point->y += dy;
    if (point->x < 0) {
      point->x = 0;
    } else if (point->x > output_width) {
      point->x = output_width;
    }
    if (point->y < 0) {
      point->y = 0;
    } else if (point->y > output_height) {
      point->y = output_height;
    }
  }
  return 0;
}

#define COLOR(hex)                                                             \
  ((hex & 0xff0000) >> 16), ((hex & 0x00ff00) >> 8), (hex & 0x0000ff), 255

int draw(SDL_Renderer *renderer, State *state) {
  CHECK_SDL(SDL_SetRenderDrawColor(renderer, COLOR(0x3b3b3b)));
  CHECK_SDL(SDL_RenderClear(renderer));
  const int radius = 4;

  int red = 255;
  int green = 0;
  int blue = 255;
  for (Point *p = state->points; p; p = p->next) {
    if (blue == 255) {
      --red;
      ++green;
    } else if (green == 255) {
      --blue;
      ++red;
    } else if (red == 255) {
      --green;
      ++blue;
    }
    CHECK_SDL(SDL_SetRenderDrawColor(renderer, red, green, blue, 255));
    CHECK_SDL(SDL_RenderFillCircle(renderer, p->x, p->y, radius));
    // CHECK_SDL(SDL_SetRenderDrawColor(renderer, COLOR(0x111111)));
    // CHECK_SDL(SDL_RenderDrawCircle(renderer, p->x, p->y, radius));
  }

  SDL_RenderPresent(renderer);

  return 0;
}

int main(int argc, char *argv[]) {
  srand((unsigned)time(NULL));

  CHECK_SDL(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));
  SDL_Window *window = SDL_CreateWindow(
      "Graph", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
      SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  State state = {0};

  init_points(renderer, &state);

  while (true) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        goto cleanup;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          destroy_points(&state);
          init_points(renderer, &state);
        }
        break;
      }
    }
    update_points(renderer, &state);
    draw(renderer, &state);
    SDL_Delay(1);
  }
cleanup:
  destroy_points(&state);
  return 0;
}
