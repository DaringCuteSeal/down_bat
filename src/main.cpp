#include "raylib-cpp/include/raylib-cpp.hpp"
#include <algorithm>
#include <ctime>
#include <dbg.h>
#include <deque>
#include <format>
#include <functional>
#include <iostream>
#include <raylib.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 956
static const int SCREEN_WIDTH_MID = SCREEN_WIDTH / 2;
static const int SCREEN_HEIGHT_MID = SCREEN_HEIGHT / 2;
#define OBSTACLES_SPEED 9
#define OBSTACLE_GAP 200
#define OBSTACLE_SPACING 2 // in seconds
#define WINDOW_TITLE "down bat!"
#define FPS 30
#define TEXT_HINT_FONT_SIZE 30

#define GRAVITY 1.9
#define JUMP_FORCE 16
#define OBSTACLE_DURATION 2

using std::deque;
using std::function;
using std::pair;
using std::string;
using std::swap;
using std::vector;

enum GameState {
  START_MENU,
  PLAY,
};

class Trunks {
public:
  raylib::Texture2D texture;
  std::deque<pair<bool, raylib::Vector2>> trunks_top;
  std::deque<raylib::Vector2> trunks_bot;
  int speed;
  int prev_loc_y;

  Trunks() {
    prev_loc_y = SCREEN_HEIGHT_MID;
    texture.Load("assets/wood.png");
  }

  // Tambah satu obstacle ke ujung kanan layar (terdiri atas bagian atas dan
  // bagian bawah)
  void add() {
    raylib::Vector2 trunk_top;
    int y_loc = GetRandomValue(300, SCREEN_HEIGHT - 300);
    trunk_top.SetY(-texture.height + y_loc);
    prev_loc_y = y_loc;
    trunk_top.SetX(SCREEN_WIDTH + GetRandomValue(-30, 30));

    raylib::Vector2 trunk_bot;
    trunk_bot.SetY(trunk_top.y + texture.height + OBSTACLE_GAP);
    trunk_bot.SetX(SCREEN_WIDTH);

    trunks_top.push_back({false, trunk_top});
    trunks_bot.push_back(trunk_bot);
  }

  void update() {
    // hapus semua trunks yang udah gak keliatan
    // bisa dengan asumsi semua trunk disusun berdasarkan posisi x nya
    // (ascending).
    while (trunks_top.size() > 0 &&
           trunks_top.front().second.x <= -SCREEN_WIDTH) {
      trunks_top.pop_front();
    }

    while (trunks_bot.size() > 0 && trunks_bot.front().x <= -SCREEN_WIDTH) {
      trunks_bot.pop_front();
    }
  }

  void draw() {
    for (auto &t : trunks_top) {
      texture.Draw(t.second);
    }

    for (auto &t : trunks_bot) {
      texture.Draw(t);
    }
  }
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

struct GameData {
  GameState state;
  Bat bat;
  Trunks trunks;
  raylib::Vector2 vel; // bg moves left at vel speed
  raylib::Texture2D bg;
  raylib::Vector2 bg_pos;
  bool game_over;
  int score;

  GameData() {
    state = GameState::START_MENU;
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
    game_over = false;
    score = 0;
  }
};

// Timer class
class Timer {
public:
  double time_end;
  double duration;
  function<void(GameData *game_data)> callback;
  bool to_delete;
  bool repeat;
};

class TimerMgr {
  GameData *game_data;
  vector<Timer> timers;

public:
  TimerMgr(GameData *game_data) { this->game_data = game_data; }

  // Tambah timer. Durasi dalam detik.
  void add_timer(double duration, bool repeat,
                 function<void(GameData *game_data)> callback) {
    timers.push_back(
        Timer{GetTime() + duration, duration, callback, false, repeat});
  }
  // Panggil callback ke timer-timer yang sudah expired dan hapus mereka.
  void update() {
    size_t del_count = 0;
    for (Timer &t : timers) {
      if (GetTime() >= t.time_end) {
        t.callback(this->game_data);
        if (t.repeat) {
          // kalau mau ngulang kita tambah tambah aja terus
          t.time_end = GetTime() + t.duration;
        } else {
          // otherwise kita hapus
          t.to_delete = true;
          del_count += 1;
        }
      }
    }

    size_t new_size = timers.size() - del_count;

    size_t sep = 0;
    for (size_t i = 0; i < timers.size(); i++) {
      if (!timers[i].to_delete) {
        swap(timers[i], timers[sep]);
        sep += 1;
      }
    }

    timers.resize(new_size);
  }
  // Hapus semua timer
  void clear_all() { timers.clear(); }
};

struct Game {
  GameData data;
  TimerMgr timer;
  Game() : timer(&this->data) {}
};

// timer ini menambah trunk, lalu memanggil diri sendiri (selamanya sampai dia
// dihapus)
// Fungsi untuk dijalankan ketika permainan mulai (bukan aplikasi, hanya sesi
// permainan).
void init_gameplay(Game *game) {
  game->data.vel.SetX(OBSTACLES_SPEED);
  game->data.state = GameState::PLAY;
  game->timer.clear_all();
  game->timer.add_timer(
      OBSTACLE_SPACING, true,
      [](GameData *game_data) -> void { game_data->trunks.add(); });
}

void update_bg(Game *game) {

  game->data.bg_pos.x -= game->data.vel.x;
  if (game->data.bg_pos.x <= -game->data.bg.width) {
    game->data.bg_pos.x =
        0; // snap back ke posisi awal. gak bakal keliatan kenapa-kenapa karena
           // posisinya sama persis dengan posisi bg ke-2
  }
}

void update_trunks(Game *game) {
  for (auto &t : game->data.trunks.trunks_top) {
    t.second.x -= OBSTACLES_SPEED;
    if (!t.first &&
        t.second.x + game->data.trunks.texture.width <= game->data.bat.pos.x) {
      t.first = true;
      game->data.score += 1;
    }
  }

  for (auto &t : game->data.trunks.trunks_bot) {
    t.x -= OBSTACLES_SPEED;
  }
}

void update_game(Game *game) {
  update_bg(game);
  update_trunks(game);
  game->data.vel.y += GRAVITY;
  game->data.bat.pos.y += game->data.vel.y;
  game->data.trunks.update();

  if (raylib::Keyboard::IsKeyPressed(KEY_SPACE)) {
    game->data.vel.y = -JUMP_FORCE;
    game->data.bat.state = game->data.bat.state == Bat::BatState::NORMAL
                               ? Bat::BatState::FLAP
                               : Bat::BatState::NORMAL;
  }
}

void update_start(Game *game_data) {
  if (raylib::Keyboard::IsKeyPressed(KEY_SPACE)) {
    init_gameplay(game_data);
  }
}

void update(Game *game) {
  game->timer.update();
  switch (game->data.state) {
  case GameState::PLAY: {
    update_game(game);
    break;
  }
  case GameState::START_MENU: {
    update_start(game);
    break;
  }
  }
}

void draw_bg(Game *game) {
  game->data.bg.Draw(game->data.bg_pos);
  game->data.bg.Draw(
      game->data.bg_pos.Add(raylib::Vector2(game->data.bg.width, 0)));
}

void draw_game(Game *game) {
  draw_bg(game);
  game->data.trunks.draw();
  game->data.bat.draw();
  string s = "Skor: " + std::to_string(game->data.score);
  DrawText(s.c_str(), 10, 10, 30, WHITE);
}

void draw_start(Game *game) {
  draw_game(game);
  DrawText("Tekan spasi untuk mulai..", 10,
           SCREEN_HEIGHT - 10 - TEXT_HINT_FONT_SIZE, TEXT_HINT_FONT_SIZE,
           WHITE);
  if (game->data.game_over) {
    DrawText("KALAH!", SCREEN_WIDTH_MID - 75, SCREEN_HEIGHT_MID, 50, WHITE);
  }
}

void draw(Game *game) {
  BeginDrawing();

  switch (game->data.state) {
  case GameState::PLAY: {
    draw_game(game);
    break;
  }
  case GameState::START_MENU: {
    draw_start(game);
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

  Game game_data;

  while (!raylib::Window::ShouldClose()) {
    update(&game_data);
    draw(&game_data);
  }

  return 0;
}
