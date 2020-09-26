# include <boost/algorithm/string.hpp>
# include <experimental/filesystem>
# include <boost/filesystem.hpp>
# include <sys/stat.h>
# include <algorithm>
# include <ncurses.h>
# include <iostream>
# include <stdlib.h>
# include <locale.h>
# include <iomanip>
# include <sstream>
# include <fstream>
# include <chrono>
# include <vector>
# include <string>
# include <pwd.h>
# include <grp.h>
# include <array>

static constexpr int BLACK    = COLOR_PAIR(1);
static constexpr int RED      = COLOR_PAIR(2);
static constexpr int GREEN    = COLOR_PAIR(3);
static constexpr int YELLOW   = COLOR_PAIR(4);
static constexpr int BLUE     = COLOR_PAIR(5);
static constexpr int MAGENTA  = COLOR_PAIR(6);
static constexpr int CYAN     = COLOR_PAIR(7);
static constexpr int WHITE    = COLOR_PAIR(8);

static constexpr int BRIGHT   = 2097152;
static constexpr int BLINK    = 524288;

enum action {
	QUIT,
	DOWN,
	UP,
	LOAD,
	GET,
	CD,
	SET,
	HIDDEN,
	MKDIR,
	OPEN,
	MOVE,
	BMOVE,
	EMOVE,
	REMOVE,
	TOUCH,
	SELECT,
	COPY,
	COPYDIR,
	PASTE,
	TOP,
	BOTTOM,
	SHELL,
	RENAME,
	EXTRACT,
	COMPRESS,
};

struct colors {
	std::string extension;
	int color;
};

struct command {
	std::string name;
	action command;
};

struct event {
	int key;
	int double_key = -1;
	std::string command;
};

struct open {
	std::string extension;
	std::string command;
};

# include "config.h"

class user_interface;

# include "commands.h"

class user_interface {
	private:

		WINDOW *main_window;
		WINDOW *preview_window;

		std::vector<std::string> main_elements;
		std::vector<std::string> preview_elements;
		std::vector<std::string> main_sizes;
		std::vector<std::string> preview_sizes;

		std::vector<std::string> file_history;

		std::vector<int> keys;
		std::vector<unsigned long> key_times;

		std::vector<int> selected = { 0 };

		std::string file_info = "";
		bool error_message = false;

		int width, height;

		int scroll = 0;

		unsigned long current_time() {
			return std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		void add_key(int key) {
			// handles double keys
			for(int i = 0; i < keys.size(); i++) {
				for(int j = 0; j < event_map.size(); j++) {
					// if keys matches
					if(keys[i] == event_map[j].key
					&& key == event_map[j].double_key) {

						// check if keystroke are not too old
						bool flag = false;
						if(key_times[i] + 500 > current_time()) {
							flag = true;
						}

						keys.erase(keys.begin() + i);
						key_times.erase(key_times.begin() + i);

						if(flag) {
							commands::process_command(event_map[j].command, this);
							return;
						}
					}
				}
			}

			// handles regular keys
			for(int i = 0; i < event_map.size(); i++) {
				if(event_map[i].key == key && event_map[i].double_key == -1) {
					commands::process_command(event_map[i].command, this);
					return;
				}
			}
			
			// if no match, try to store keys
			for(int i = 0; i < event_map.size(); i++) {
				if(event_map[i].key == key) {
					keys.push_back(key);
					key_times.push_back(current_time());
					return;
				}
			}
		}

		// update graphics
		void update() {
			commands::load({"preview"}, this);

			// draw bottom message
			if(error_message) {
				wattron(stdscr, COLOR_PAIR(9));
			}

			mvwprintw(stdscr, LINES - 1, 0, file_info.c_str());
			wattroff(stdscr, COLOR_PAIR(9));

			error_message = false;

			draw_current_directory();
			draw_elements(main_elements, main_sizes, main_window, true);
			draw_elements(preview_elements, preview_sizes, preview_window, false);

			handle_empty_directory();
			
			refresh_windows();
		}

		void handle_frame() {
			clear_windows();

			width = COLS;
			height = LINES;

			update();

			int key;
			while(key = getch()) {
				if(key != ERR) {
					add_key(key);
					break;
				}

				if(width != COLS || height != LINES) {
					width = COLS;
					height = LINES;
					update();
				}
			}
		}

		// draws EMPTY if directory is empty. also permission checks
		void handle_empty_directory() {
			if((main_elements.empty())
			|| (preview_elements.empty()
			&& boost::filesystem::is_directory(main_elements[selected[0]]))) {

				WINDOW *empty_window;

				if(main_elements.empty()) {
					empty_window = main_window;
				} else {
					empty_window = preview_window;
				}

				wattron(empty_window, COLOR_PAIR(9));
				mvwprintw(empty_window, 0, 0, "EMPTY");
				wattroff(empty_window, COLOR_PAIR(9));
			}

			if(!main_elements.empty() && preview_elements.empty()) {
				boost::system::error_code error;
				for(const auto &entry : boost::filesystem::directory_iterator(main_elements[selected[0]], error));

				if(error.value() == 13) {
					wattron(preview_window, COLOR_PAIR(9));
					mvwprintw(preview_window, 0, 0, "NO PERIMISSIONS TO FOLDER");
					wattroff(preview_window, COLOR_PAIR(9));
				}
			}
		}

		void refresh_windows() {
			wresize(main_window, LINES - 3, COLS / 2 - 1);
			wresize(preview_window, LINES - 3, COLS / 2 - 2);
			mvwin(main_window, 2, 1);
			mvwin(preview_window, 2, (COLS / 2) + 1);

			wrefresh(main_window);
			wrefresh(preview_window);
		}

		// draw the current directory at top
		void draw_current_directory() {
			std::string current_path = boost::filesystem::current_path().string();

			mvwprintw(stdscr, 0, 0, std::string(COLS, ' ').c_str());
			mvwprintw(stdscr, 0, 0, current_path.c_str());

			wattron(stdscr, A_BOLD);

			if(!main_elements.empty()) {
				mvwprintw(stdscr, 0, current_path.length(),
						std::string((current_path != "/" ? "/" : "") + main_elements[selected[0]]).c_str());
			}

			wattroff(stdscr, A_BOLD);
		}

		// chooses color for a filename
		void handle_colors(std::string selected_file) {
			for(int i = 0; i < colors_map.size(); i++) {
				boost::filesystem::path path_object(selected_file);

				if((colors_map[i].extension == path_object.extension().string())
				|| (colors_map[i].extension == "dir"
				&& boost::filesystem::is_directory(selected_file))
				|| (colors_map[i].extension == "")) {

					wattron(main_window, colors_map[i].color);
					wattron(preview_window, colors_map[i].color);
					return;
				}
			}
		}

		// turns off every color
		void colors_off() {
			wattroff(main_window, A_REVERSE|BRIGHT|BLINK);
			wattroff(preview_window, A_REVERSE|BRIGHT|BLINK);
			for(int i = 1; i < 8; i++) {
				wattroff(main_window, COLOR_PAIR(i));
				wattroff(preview_window, COLOR_PAIR(i));
			}
		}

		// thicc chunker
		void draw_elements(std::vector<std::string> elements,
						   std::vector<std::string> sizes,
						   WINDOW *window,
						   bool main_window) {

			int y, x;
			getmaxyx(window, y, x);
			y -= 3;

			// if main window then account for scroll
			for(int i = 0; main_window ? i + scroll < elements.size() : i < elements.size(); i++) {

				int index = main_window ? i + scroll : i;

				if(main_window
				|| (!main_window
				&& boost::filesystem::is_directory(main_elements[selected[0]]))) {

					int draw_x = 0;

					// highlight selected
					if(main_window && selected[0] == i + scroll) {
						wattron(window, A_REVERSE);
					}

					// tab other selected
					if(main_window
					&& std::find(selected.begin() + 1, selected.end(), i + scroll)
					!= selected.end()) {

						draw_x = selected_space_size;
					}

					// draw colors
					if(main_window) {
						handle_colors(elements[i]);
					} else {
						handle_colors(main_elements[selected[0]] + "/" + elements[i]);
					}

					std::string size = sizes[index];

					// meat
					if(elements[index].length() + size.length() + draw_x < x) {
						mvwprintw(window, i, draw_x, std::string(
								elements[index] + std::string(
								x - elements[index].length() - size.length() - draw_x, ' ')
							   	+ size).c_str());
					} else {
						mvwprintw(window, i, draw_x, std::string(elements[index].substr(
							0, x - size.length() - 4 - draw_x + 2) + "~ " + size).c_str());
					}

					colors_off();
				} else {
					std::string line = elements[index];
					line = commands::find_and_replace(line, "%", "%%");
					mvwprintw(window, i, 0, line.c_str());
				}
			}
		}

	public:

		void bound_selected() {
			int y, x;
			getmaxyx(main_window, y, x);
			clear_windows();

			scroll = selected[0] < scroll && scroll != 0 ? scroll - 1 :
				selected[0] < main_elements.size()
				&& selected[0] > scroll + y - 1 ? scroll + 1 : scroll;

			selected[0] = selected[0] < 0 ? 0 :
				selected[0] > main_elements.size() - 1 ?
				main_elements.size() - 1 : selected[0];
		}

		std::vector<std::string> get_file_history() {
			return file_history;
		}

		void set_file_history(std::vector<std::string> file_history_) {
			file_history = file_history_;
		}

		void set_error_message(std::string error_message_) {
			file_info = error_message_;
			error_message = true;
		}

		// thicc chunker
		void load_file_info() {
			std::string selected_filename;
			if(!main_elements.empty()) {
				selected_filename = main_elements[selected[0]];
			}

			file_info = "";

			if(!main_elements.empty() && boost::filesystem::exists(selected_filename)) {
				std::string file_sizes = commands::format_file_size(commands::file_sizes(
							boost::filesystem::current_path().string()), size_precision) + " sum, ";

				if(file_sizes.length() > COLS) {
					return;
				}

				std::string right_info = file_sizes;
				std::string free_space = commands::format_file_size(
						commands::free_space(selected_filename), size_precision) + " free, ";

				if(right_info.length() + free_space.length() > COLS) {
					file_info = right_info;
					return;
				}

				right_info += free_space;
				std::string position = std::to_string(selected[0] + 1) + "/" + std::to_string(main_elements.size());

				if(right_info.length() + position.length() > COLS) {
					file_info = right_info;
					return;
				}

				right_info += position;
				std::string permissions = commands::file_permissions(selected_filename) + " ";

				if(right_info.length() + permissions.length() > COLS) {
					file_info += std::string(COLS - file_info.length() - right_info.length(), ' ') + right_info;
					return;
				}

				file_info += permissions;
				std::string owner = commands::file_owner(selected_filename) + " ";

				if(file_info.length() + right_info.length() + owner.length() > COLS) {
					file_info += std::string(COLS - file_info.length() - right_info.length(), ' ') + right_info;
					return;
				}

				file_info += owner;

				if(!boost::filesystem::is_directory(selected_filename)) {
					std::string file_size = commands::format_file_size(commands::file_size(selected_filename), size_precision) + " ";

					if(file_info.length() + right_info.length() + file_size.length() > COLS) {
						file_info += std::string(COLS - file_info.length() - right_info.length(), ' ') + right_info;
						return;
					}

					file_info += file_size;
				}

				std::string mod_time = commands::file_last_mod_time(selected_filename) + " ";

				if(file_info.length() + right_info.length() + mod_time.length() > COLS) {
					file_info += std::string(COLS - file_info.length() - right_info.length(), ' ') + right_info;
					return;
				}
				
				file_info += mod_time;
				file_info += std::string(COLS - file_info.length() - right_info.length(), ' ') + right_info;
			}
		}


		void set_message(std::string message_) {
			file_info = message_;
		}

		// main loop
		void loop() {
			boost::filesystem::current_path(starting_directory);
			commands::load({"main"}, this);
			commands::load({"preview"}, this);
			load_file_info();

			while(true) {
				clear_screen();
				handle_frame();
			}
		}

		// init all colors
		void init_colors() {
			if(!has_colors()) {
				commands::quit({"your terminal does not support color"});
			}

			start_color();
			use_default_colors();

			init_pair(1, 0, -1);
			init_pair(2, 1, -1);
			init_pair(3, 2, -1);
			init_pair(4, 3, -1);
			init_pair(5, 4, -1);
			init_pair(6, 5, -1);
			init_pair(7, 6, -1);
			init_pair(8, 7, -1);
			init_pair(9, 7,  1);
		}

		void init_ncurses() {
			if(!setlocale(LC_ALL, "")) {
				std::cout << "warning: no locale\n";
			}
			initscr();
			cbreak();
			noecho();
			refresh();
			raw();
			keypad(stdscr, true);
			nodelay(stdscr, true);
			init_colors();
			curs_set(0);

			main_window = newwin(0, 0, 0, 0);
			preview_window = newwin(0, 0, 0, 0);

			refresh_windows();
		}

		std::vector<int> get_selected() {
			return selected;
		}

		std::vector<std::string> get_main_elements() {
			return main_elements;
		}

		void set_selected(std::vector<int> selected_) {
			selected = selected_;
			bound_selected();
			commands::load({"preview"}, this);
			load_file_info();
		}
		
		void set_main_elements(std::vector<std::string> main_elements_) {
			main_elements = main_elements_;
		}

		void set_main_sizes(std::vector<std::string> main_sizes_) {
			main_sizes = main_sizes_;
		}

		void set_preview_sizes(std::vector<std::string> preview_sizes_) {
			preview_sizes = preview_sizes_;
		}

		void set_preview_elements(std::vector<std::string> preview_elements_) {
			preview_elements = preview_elements_;
		}

		void set_selected(std::string selected_) {
			std::vector<std::string>::iterator iterator = std::find(
					main_elements.begin(), main_elements.end(), selected_);

			if(iterator != main_elements.end()) {
				selected[0] = std::distance(main_elements.begin(), iterator);
			}

			commands::load({"preview"}, this);
			load_file_info();
		}

		void clear_windows() {
			int main_x, main_y, preview_x, preview_y;

			getmaxyx(main_window, main_y, main_x);
			getmaxyx(preview_window, preview_y, preview_x);

			for(int i = 0; i < main_y; i++) {
				mvwprintw(main_window, i, 0, std::string(main_x, ' ').c_str());
			}

			for(int i = 0; i < preview_y; i++) {
				mvwprintw(preview_window, i, 0, std::string(preview_x, ' ').c_str());
			}
		}

		void clear_screen() {
			for(int i = 0; i < LINES; i++) {
				mvwprintw(stdscr, i, 0, std::string(COLS, ' ').c_str());
			}
			clear_windows();
			refresh();
		}

		std::vector<std::string> split_into_args(std::string str) {
			std::vector<std::string> result;
			boost::split(result, str, boost::is_any_of(" "), boost::token_compress_on);
			return result;
		}
};

# include "commands.cpp"

int main() {
	user_interface ui;
	ui.init_ncurses();
	ui.loop();
}
