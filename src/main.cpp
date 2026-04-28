#include "raylib-cpp/include/raylib-cpp.hpp"
#include <algorithm>
#include <ctime>
#include <functional>
#include <iostream>
#include <raylib.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 956
static const int SCREEN_WIDTH_MID = SCREEN_WIDTH / 2;
static const int SCREEN_HEIGHT_MID = SCREEN_HEIGHT / 2;
#define WINDOW_TITLE "down bat!"
#define FPS 30

#define GRAVITY 1.9
#define JUMP_FORCE 20

using std::function;
using std::vector;

enum GameState {
  START_MENU,
  PLAY,
};

class Bat {
public:
  raylib::Texture2D bat_normal;
  raylib::Texture2D bat_flap;
  float rot;
  raylib::Vector2 pos;
  enum BatState { NORMAL, FLAP };
  BatState state;

  Bat() {
    bat_normal.Load("assets/bat1.png");
    bat_flap.Load("assets/bat2.png");
  };

  void draw() {
    switch (state) {
    case BatState::FLAP: {
      bat_flap.Draw(this->pos, this->rot);
      break;
    }
    case BatState::NORMAL: {
      bat_normal.Draw(this->pos, this->rot);
      break;
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

  GameData() {
    state = GameState::START_MENU; // ganti nanti
    bat.pos.SetX(SCREEN_WIDTH_MID -
                 bat.bat_flap.width /
                     2); // assume the dimensions of bat_flap = bat_normal
    bat.pos.SetY(SCREEN_HEIGHT_MID - bat.bat_flap.height / 2);
    bat.state = Bat::BatState::NORMAL;
    bg.Load("assets/bg.png");
    bg_pos.SetX(0);
    bg_pos.SetY(0);
    vel.SetX(0);
    vel.SetY(0);
  }
};

// Fungsi untuk dijalankan ketika permainan mulai (bukan aplikasi, hanya sesi
// permainan).
void init_gameplay(GameData *game_data) {
  game_data->vel.SetX(9);
  game_data->state = GameState::PLAY;
}

void update_bg(GameData *game_data) {

  game_data->bg_pos.x -= game_data->vel.x;
  if (game_data->bg_pos.x <= -game_data->bg.width) {
    game_data->bg_pos.x =
        0; // snap back ke posisi awal. gak bakal keliatan kenapa-kenapa karena
           // posisinya sama persis dengan posisi bg ke-2
  }
}

void update_game(GameData *game_data) {
  update_bg(game_data);
  game_data->vel.y += GRAVITY;
  game_data->bat.pos.y += game_data->vel.y;

  if (raylib::Keyboard::IsKeyPressed(KEY_SPACE)) {
    game_data->vel.y = -JUMP_FORCE;
    game_data->bat.state = game_data->bat.state == Bat::BatState::NORMAL
                               ? Bat::BatState::FLAP
                               : Bat::BatState::NORMAL;
  }
}

void update_start(GameData *game_data) {
  if (raylib::Keyboard::IsKeyPressed(KEY_SPACE)) {
    init_gameplay(game_data);
  }
}

void update(GameData *game_data) {
  game_data->timer.update();
  switch (game_data->state) {
  case GameState::PLAY: {
    update_game(game_data);
    break;
  }
  case GameState::START_MENU: {
    update_start(game_data);
    break;
  }
  }
}

void draw_bg(GameData *game_data) {
  game_data->bg.Draw(game_data->bg_pos);
  game_data->bg.Draw(
      game_data->bg_pos.Add(raylib::Vector2(game_data->bg.width, 0)));
}

void draw_game(GameData *game_data) {
  draw_bg(game_data);
  game_data->bat.draw();
}

void draw_start(GameData *game_data) {
  draw_game(game_data);
  DrawText("Tekan spasi untuk mulai..", 10, 10, 30, WHITE);
}

void draw(GameData *game_data) {
  BeginDrawing();

  switch (game_data->state) {
  case GameState::PLAY: {
    draw_game(game_data);
    break;
  }
  case GameState::START_MENU: {
    draw_start(game_data);
    break;
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

  while (!raylib::Window::ShouldClose()) {
    update(&game_data);
    draw(&game_data);
  }

  return 0;
}
