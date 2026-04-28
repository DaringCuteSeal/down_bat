#include "raylib-cpp/include/raylib-cpp.hpp"
#include <ctime>
#include <raylib.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 956
#define WINDOW_TITLE "down bat!"
#define FPS 30

enum State {
  START_MENU,
  PLAY,
};

class Bat {
  raylib::Texture2D bat_normal;
  raylib::Texture2D bat_flap;
  enum BatState { NORMAL, FLAP };
  BatState state;
  float rot;
  raylib::Vector2 pos;

  Bat() {
    bat_normal.Load("assets/bat1.png");
    bat_flap.Load("assets/bat2.png");
  };

  void set_state(BatState state) { this->state = state; }
  void draw() {
    switch (state) {
    case BatState::FLAP: {
      bat_flap.Draw(this->pos, this->rot);
    }
    case BatState::NORMAL: {
      bat_normal.Draw(this->pos, this->rot);
    }
    }
  }
};

void update() {}

void draw() {}

/** Fungsi utama dari aplikasi. Fungsi ini menginisalisasi jendela dan
 * menjalankan game hingga jendela ditutup.
 */
int main() {
  raylib::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  InitAudioDevice();
  SetMasterVolume(1.0);

  // Setel FPS (frame per second)
  SetTargetFPS(FPS);

  // Beri seed untuk RNG
  SetRandomSeed(time(NULL));

  while (!raylib::Window::ShouldClose()) {
    update();
    draw();
  }

  return 0;
}
