#include "raylib-cpp/include/raylib-cpp.hpp"
#include <algorithm>
#include <ctime>
#include <functional>
#include <iostream>
#include <raylib.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 956
#define WINDOW_TITLE "down bat!"
#define FPS 30

#define GRAVITY 10

using std::function;
using std::vector;

enum GameState {
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

public:
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

class Timer {
public:
  double time_end;
  function<void()> callback;
  bool to_delete;
};

class TimerMgr {
  vector<Timer> timers;

public:
  TimerMgr() {}

  void add_timer(double duration, function<void()> callback) {
    timers.push_back(Timer{GetTime() + duration, callback, false});
  }
  void update() {
    for (Timer &t : timers) {
      if (GetTime() >= t.time_end) {
        t.callback();
        t.to_delete = true;
      }
    }
    // TODO:
    // challenge (medium): hapus semua timer yang ditandai to_delete = true
    // dalam waktu O(N)
  }
};

struct GameData {
  GameState state;
  TimerMgr timer;
  Bat bat;
  raylib::Vector2 vel; // bg moves left at vel speed
  raylib::Texture2D bg;
  raylib::Vector2 bg_pos;

  GameData() { bg.Load("assets/bg.png"); }
};

void update_start(GameData *game_data) {}

void update_game(GameData *game_data) {}

void update(GameData *game_data) {
  game_data->timer.update();
  switch (game_data->state) {
  case GameState::PLAY: {
    update_game(game_data);
  }
  case GameState::START_MENU: {
    update_start(game_data);
  }
  }
}

void draw_game(GameData *game_data) { game_data->bg.Draw(game_data->bg_pos); }

void draw_start(GameData *game_data) { draw_game(game_data); }

void draw(GameData *game_data) {
  BeginDrawing();

  switch (game_data->state) {
  case GameState::PLAY: {
    draw_game(game_data);
  }
  case GameState::START_MENU: {
    draw_start(game_data);
  }
  }
  EndDrawing();
}

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

  GameData game_data;
  game_data.state = GameState::START_MENU;

  while (!raylib::Window::ShouldClose()) {
    update(&game_data);
    draw(&game_data);
  }

  return 0;
}
