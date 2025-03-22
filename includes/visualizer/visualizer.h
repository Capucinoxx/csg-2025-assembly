#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <functional>
#include "consts.h"
#include <ncurses.h>

using lines_t = std::vector<std::string>;

namespace visualizer {
struct i_data_t {
  virtual ~i_data_t() = default;
  virtual lines_t to_data() const = 0;
  virtual uint64_t pc() const { return 0; }
};

using ptr_data_t = std::shared_ptr<i_data_t>;
using color_t = block_color_cur_t;

template<typename T>
struct panel_t {
  static_assert(std::is_base_of<i_data_t, T>::value, 
    "T must have a member func `to_data()` returning lines_t");

  panel_t(int x, int y, int w, int h, const std::string& title)
    : x(x), y(y), width(w), height(h), title(title), scroll_pos(0) {
    win = newwin(height, width, y, x);
  }

  void resize(int x, int y, int w, int h) {
    std::cout << "RESIZE !!!" << std::endl;
    this->x = x;
    this->y = y;
    this->width = w;
    this->height = h;
    wresize(win, h, w);
    mvwin(win, y, x);
  }

  virtual ~panel_t() { delwin(win); }

  void add_observer(std::shared_ptr<T> o) { observed = o; }

  void set_focus(bool focus) { focused = focus; }

  void scroll_up() {
    if (scroll_pos = 0) return;
    --scroll_pos;
  }

  void scroll_down() {
    if (scroll_pos == content.size() - height + 2) return;
    ++scroll_pos;
  }

  virtual void draw_content() {
    int max_lines = height - 2;
    int start = std::max(0, scroll_pos);
    int end = std::min(static_cast<int>(content.size()), start + max_lines);

    for (int i = start; i < end; ++i)
      mvwprintw(win, i - start + 1, 1, "%s", content[i].c_str());
  }

  void draw() {
    werase(win);
    if (focused) wattron(win, color_pair(color_t::GREEN));
    box(win, 0, 0);
    wattroff(win, color_pair(color_t::GREEN));

    if (!title.empty()) mvwprintw(win, 0, 2, "%s", title.c_str());
    
    if (observed) {
      content = observed->to_data();
      this->draw_content();
    }

    wrefresh(win);
  }

protected:
  int x, y, width, height;
  std::string title;
  WINDOW *win;
  lines_t content;
  std::shared_ptr<T> observed;
  bool focused = false;
  int scroll_pos = 0;
};


template<typename T>
struct panel_colored_map_t : public panel_t<T> {
  panel_colored_map_t(int x, int y, int w, int h, const std::string& title)
    : panel_t<T>(x, y, w, h, title) {}

  void draw_content() override {
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++j) {
        block_color_cur_t color = static_cast<block_color_cur_t>(
          this->content[i * 8 + j][0] - '0');
        wattron(this->win, color_pair(color));
        mvwaddch(this->win, i + 1, j * 2 + 1, 
                 ACS_CKBOARD | color_pair(color));
        mvwaddch(this->win, i + 1, j * 2 + 2, 
                 ACS_CKBOARD | color_pair(color));
        wattroff(this->win, color_pair(color));
      }
    }
  }
};

template<typename T>
struct panel_cursor_t : public panel_t<T> {
  panel_cursor_t(int x, int y, int w, int h, const std::string& title)
    : panel_t<T>(x, y, w, h, title) {}

  void draw_content() override {
    auto cursor = this->observed->pc() / 4;

    for (size_t i = 0; i <= cursor; ++i)
      cursor += (this->content[i][this->content[i].size() - 1] == ':');

    int max_lines = this->height - 2;
    int start = std::max(0, this->scroll_pos);
    int end = std::min(static_cast<int>(this->content.size() - 2), 
                       start + max_lines - 3);

    if (old_cursor != cursor && (cursor >= end || cursor <= start)) {
      start = std::max(0, int(cursor - 3));
      end = std::min(static_cast<int>(this->content.size() - 2), 
                     start + max_lines - 3);
      this->scroll_pos = start;
    }
    old_cursor = cursor;

    for (int i = start; i < end; ++i) {
      const auto& line = this->content[i];

      if (i < cursor) 
        wattron(this->win, color_pair(color_t::WHITE));
      else if (i == cursor)
        wattron(this->win, color_pair(color_t::GREEN));
      else 
        wattroff(this->win, color_pair(color_t::GREEN));

      mvwprintw(this->win, i - start + 1, 1, "%s", line.c_str());
      wattroff(this->win, color_pair(color_t::WHITE));
      wattroff(this->win, color_pair(color_t::GREEN));
    }

    mvwprintw(this->win, max_lines, 1, "%s | %s",
              this->content[this->content.size() - 2].c_str(), 
              this->content[this->content.size() - 1].c_str());
  }

private:
  int old_cursor = -1;
};


struct visualizer_t {
  explicit visualizer_t() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();

    for (size_t i = 0; i < 8; ++i)
      init_pair(i + 1, blocks_color_t[i], -1);

    setup();
  }

  ~visualizer_t() { endwin(); }

  void run() {
    int focused_panel = 0;

    panels[focused_panel]->set_focus(true);
    for (auto& panel : panels) panel->draw();

    nodelay(stdscr, TRUE);
    while (running) {
      setup();
      char ch = std::tolower(std::cin.get());

      switch (ch) {
        case 'q':
          running = false;
          break;

        case 'i':
          panels[focused_panel]->set_focus(false);
          focused_panel = (focused_panel + 1) % (panels.size() - 2);
          panels[focused_panel]->set_focus(true);
          break;

        case 'w':
          if (panels.size() > focused_panel)
            panels[focused_panel]->scroll_up();
          break;

        case 'x':
          if (panels.size() > focused_panel)
            panels[focused_panel]->scroll_down();
          break;
      }

      if (hooks.find(ch) != hooks.end()) hooks[ch]();

      for (auto& panel : panels) panel->draw();
    }
  }

  void display_error(const std::string& error) {
    int width = 80 + 6;
    int height = error.size() / 80 + 6;
    int start_x = (cols - width) / 2;
    int start_y = (rows - height) / 2;

    WINDOW* win = newwin(height, width, start_y, start_x);
    wattron(win, color_pair(color_t::RED));
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "[ Error ]");

    for (size_t i = 0; i < error.size(); i += 80)
      mvwprintw(win, i / 80 + 1, 1, "%s", error.substr(i, 80).c_str());
    mvwprintw(win, height - 2, 1, "Press any key to continue...");
    wattroff(win, color_pair(color_t::RED));
    wrefresh(win);

    int ch = std::cin.get();
    delwin(win);
    endwin();
    for (auto& panel : panels) panel->draw();
    running = false;
    ch = std::cin.get();
  }

  void hook(char key, std::function<void()> cb) {
    hooks[key] = cb;
  }

  void setup_panels(ptr_data_t ins, ptr_data_t proc, ptr_data_t mem,
                    ptr_data_t map, ptr_data_t pip, ptr_data_t help) {
    append_panel<panel_cursor_t<i_data_t>>(ins,
      0, 
      0,
      instruction_width, 
      rows - 6,
      "Instructions");

    append_panel<panel_t<i_data_t>>(proc,
      instruction_width, 
      0,
      right_section_width / 2, 
      rows - map_register_height - 6,
      "Processor");

    append_panel<panel_t<i_data_t>>(mem,
      instruction_width + right_section_width / 2,
      0,
      right_section_width / 2,
      rows - map_register_height - 6,
      "Memory");

    append_panel<panel_colored_map_t<i_data_t>>(map,
      instruction_width,
      rows - map_register_height - 6,
      map_register_width,
      map_register_height,
      "Map");

    append_panel<panel_t<i_data_t>>(pip,
      instruction_width + map_register_width,
      rows - map_register_height - 6,
      right_section_width - map_register_width,
      map_register_height,
      "Pipeline");

    append_panel<panel_t<i_data_t>>(help,
      0, 
      rows - 6,
      cols, 
      5,
      "");
  }

private:
  template<typename T>
  void append_panel(ptr_data_t data, int x, int y, int w, int h, 
                    const std::string& title) {
    auto panel = std::make_unique<T>(x, y, w, h, title);
    panel->add_observer(data);
    panels.emplace_back(std::move(panel));
  }

  void setup() {
    getmaxyx(stdscr, rows, cols);
    if (resize_term(rows, cols) == ERR && panels.size() != 0) {
      resize_panels();
    }

    instruction_width = std::max(min_instruction_width, cols / 3);
    right_section_width = cols - instruction_width;

    horizontal_panel_height = rows / 4;
    vertical_panel_height = rows - horizontal_panel_height;

    map_register_height = 10;
    map_register_width = 18;
  }

  void resize_panels() {
    panels[0]->resize(
      0, 
      0, 
      instruction_width, 
      rows - 6);

    panels[1]->resize(
      instruction_width, 
      0,
      right_section_width / 2, 
      rows - map_register_height - 6);

    panels[2]->resize(
      instruction_width + right_section_width / 2,
      0,
      right_section_width / 2,
      rows - map_register_height - 6);

    panels[3]->resize(
      instruction_width,
      rows - map_register_height - 6,
      map_register_width,
      map_register_height);

    panels[4]->resize(
      instruction_width,
      rows - map_register_height - 6,
      map_register_width,
      map_register_height);

    panels[5]->resize(
      0,
      rows - 6,
      cols,
      5);
  }

  bool running = true;
  std::unordered_map<char, std::function<void()>> hooks;
  std::vector<std::unique_ptr<panel_t<i_data_t>>> panels;

  int rows, cols;

  // Definitions panel dimensions
  static constexpr int min_instruction_width = 80;
  int instruction_width;
  int right_section_width;

  // height of horizontal panels 
  int horizontal_panel_height;
  int vertical_panel_height;

  // dimensions of the map and register panel
  int map_register_height;
  int map_register_width;
};

};
