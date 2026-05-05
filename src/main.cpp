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
#define OBSTACLE_GAP 100
#define OBSTACLE_SPACING 3 // in seconds
#define WINDOW_TITLE "down bat!"
#define FPS 30

#define GRAVITY 1.9
#define JUMP_FORCE 20
#define OBSTACLE_DURATION 2

using std::deque;
using std::function;
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
  std::deque<raylib::Vector2> trunks;
  int speed;

  Trunks(int speed) {
    this->speed = speed;
    texture.Load("assets/wood.png");
  }

  // Tambah satu obstacle ke ujung kanan layar (terdiri atas bagian atas dan
  // bagian bawah)
  void add() {
    raylib::Vector2 trunk_top;
    trunk_top.SetY(-texture.height + GetRandomValue(30, SCREEN_HEIGHT - 30));
    trunk_top.SetX(SCREEN_WIDTH);

    raylib::Vector2 trunk_bot;
    trunk_bot.SetY(trunk_top.y + OBSTACLE_GAP);
    trunk_top.SetX(SCREEN_WIDTH);

    trunks.push_back(trunk_top);
    trunks.push_back(trunk_bot);
  }

  void update() {
    for (auto &t : trunks) {
      t.x -= speed;
    }

    // hapus semua trunks yang udah gak keliatan
    // bisa dengan asumsi semua trunk disusun berdasarkan posisi x nya
    // (ascending).
    while (trunks.size() > 0 && trunks.front().x <= -SCREEN_WIDTH) {
      trunks.pop_front();
    }
  }

  void draw() {
    for (auto &t : trunks) {
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

// Timer class
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

  // Tambah timer. Durasi dalam detik.
  void add_timer(double duration, function<void()> callback) {
    timers.push_back(Timer{GetTime() + duration, callback, false});
  }
  // Panggil callback ke timer-timer yang sudah expired dan hapus mereka.
  void update() {
    size_t del_count = 0;
    for (Timer &t : timers) {
      if (GetTime() >= t.time_end) {
        t.callback();
        t.to_delete = true;
        del_count += 1;
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

struct GameData {
  GameState state;
  TimerMgr timer;
  Bat bat;
  Trunks trunks;
  raylib::Vector2 vel; // bg moves left at vel speed
  raylib::Texture2D bg;
  raylib::Vector2 bg_pos;
  bool game_over;
  int score;

  GameData() : trunks(OBSTACLES_SPEED) {
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

  void add_trunk() { trunks.add(); }
};

// timer ini menambah trunk, lalu memanggil diri sendiri (selamanya sampai dia
// dihapus)
// Fungsi untuk dijalankan ketika permainan mulai (bukan aplikasi, hanya sesi
// permainan).
void init_gameplay(GameData *game_data) {
  game_data->vel.SetX(OBSTACLES_SPEED);
  game_data->state = GameState::PLAY;
  game_data->timer.clear_all();
  game_data->add_trunk();
  //  game_data->timer.add_timer();
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
  game_data->trunks.update();

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
  game_data->trunks.draw();
  game_data->bat.draw();
}

void draw_start(GameData *game_data) {
  draw_game(game_data);
  DrawText("Tekan spasi untuk mulai..", 10, 10, 30, WHITE);
  if (game_data->game_over) {
    DrawText("KALAH!", SCREEN_WIDTH_MID - 75, SCREEN_HEIGHT_MID, 50, WHITE);
    string fmt = &"Skor: "[game_data->score];
    DrawText(fmt.c_str(), SCREEN_WIDTH_MID - 100, SCREEN_HEIGHT_MID, 30, WHITE);
  }
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
