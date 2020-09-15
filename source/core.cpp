# include <experimental/filesystem>
# include <boost/filesystem.hpp>
# include <sys/stat.h>
# include <algorithm>
# include <ncurses.h>
# include <iostream>
# include <stdlib.h>
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

		int scroll = 0;

		unsigned long current_time() {
			return std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		void add_key(int key) {
			for(int i = 0; i < event_map.size(); i++) {
				if(event_map[i].key == key) {
					commands::process_command(event_map[i].command, this);
					return;
				}
			}

			std::vector<int>::iterator iterator = std::find(keys.begin(), keys.end(), key);
			if(iterator != keys.end()) {
				int element = std::distance(keys.begin(), iterator); 

				for(int i = 0; i < event_map.size(); i++) {
					if(event_map[i].double_key == key
					&& key_times[i] + 500 < current_time()) {

						commands::process_command(event_map[i].command, this);
					}
				}

				keys.erase(keys.begin() + element);
				return;
			}

			for(int i = 0; i < event_map.size(); i++) {
				if(event_map[i].double_key == key) {
					keys.push_back(key);
					key_times.push_back(current_time());
					return;
				}
			}
		}

		void handle_frame() {
			clear_windows();

			if(draw_border) {
				draw_window_borders();
			}

			mvwprintw(stdscr, LINES - 1, 0, file_info.c_str());

			draw_current_directory();
			draw_elements(main_elements, main_sizes, main_window, true);
			draw_elements(preview_elements, preview_sizes, preview_window, false);
			handle_empty_directory();

			refresh();
			refresh_windows();

			int key;
			while(key = getch()) {
				if(key != ERR) {
					add_key(key);
					break;
				}
			}
		}

		void draw_window_borders() {
			for(int i = 2; i < LINES - 2; i++) {
				mvwaddch(stdscr, i, COLS / 2, ACS_VLINE);
				mvwaddch(stdscr, i, COLS - 1, ACS_VLINE);
				mvwaddch(stdscr, i, 0, ACS_VLINE);
			}

			for(int i = 1; i < COLS - 1; i++) {
				mvwaddch(stdscr, 1, i, ACS_HLINE);
				mvwaddch(stdscr, LINES - 2, i, ACS_HLINE);
			}

			mvwaddch(stdscr, 1, COLS / 2, ACS_TTEE);
			mvwaddch(stdscr, 1, 0, ACS_ULCORNER);
			mvwaddch(stdscr, 1, COLS - 1, ACS_URCORNER);
			mvwaddch(stdscr, LINES - 2, COLS / 2, ACS_BTEE);
			mvwaddch(stdscr, LINES - 2, 0, ACS_LLCORNER);
			mvwaddch(stdscr, LINES - 2, COLS - 1, ACS_LRCORNER);
		}

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

				if(color_terminal) {
					wattron(empty_window, COLOR_PAIR(9));
				}
				mvwprintw(empty_window, 0, 0, "EMPTY");
				wattroff(empty_window, COLOR_PAIR(9));
			}
		}

		std::string get_file_owner(std::string directory, struct stat info) {
			struct passwd *pw = getpwuid(info.st_uid);
			struct group *gr = getgrgid(info.st_gid);
			return std::string(pw->pw_name) + ":" + std::string(gr->gr_name);
		}

		std::string format_file_size(unsigned long long file_size, int precision) {
			std::stringstream stream;
			stream << std::fixed << std::setprecision(precision);

			if(file_size < 1024) {
				stream << file_size << "B";
			} if(file_size >= 1024 && file_size < 1048576) {
				stream << file_size / 1024 << "K";
			} else if(file_size >= 1048576 && file_size < 1073741824) {
				stream << file_size / 1024 / 1024 << "M";
			} else if(file_size >= 1073741824 && file_size < 1099511627776) {
				stream << file_size / 1024 / 1024 / 1024 << "G";
			} else if(file_size >= 1099511627776) {
				stream << file_size / 1024 / 1024 / 1024 / 1024 << "T";
			}

			return stream.str();
		}

		std::string get_current_directory_size() {
			unsigned long long file_size = 0;
			for(int i = 0; i < main_elements.size(); i++) {
				if(!boost::filesystem::is_directory(main_elements[i])
				&& boost::filesystem::exists(main_elements[i])) {

					try {
						file_size += boost::filesystem::file_size(main_elements[i]);
					} catch(...) {
					}
				}
			}
			return format_file_size(file_size, size_precision);
		}

		std::string get_file_creation_time(std::string directory, struct stat info) {
			struct tm *tm = gmtime(&(info.st_mtime));

			std::string year = std::to_string(1900 + tm->tm_year);
			std::string month = std::to_string(tm->tm_mon);
			std::string day = std::to_string(tm->tm_mday);

			std::string hours = std::to_string(
					tm->tm_hour - 4 < 0 ? 24 - (tm->tm_hour - 4) * -1 : tm->tm_hour - 4);

			std::string minutes = std::to_string(tm->tm_min);
			
			if(std::stoi(month) < 10) {
				month.insert(0, "0");
			} if(std::stoi(day) < 10) {
				day.insert(0, "0");
			} if(std::stoi(hours) < 10) {
				hours.insert(0, "0");
			} if(std::stoi(minutes) < 10) {
				minutes.insert(0, "0");
			}

			return year + "-" + month + "-" + day + " " + hours + ":" + minutes;
		}

		std::string get_free_space(std::string directory) {
			boost::filesystem::space_info disk_space = boost::filesystem::space("/");

			return format_file_size(disk_space.available,
					free_size_precision) + " free";
		}

		std::string get_file_permissions(std::string directory) {
			boost::filesystem::perms p =
				boost::filesystem::status(directory).permissions();

			std::stringstream stream;

		   	stream << ((p & 0400) != 0 ? "r" : "-")
				   << ((p & 0200) != 0 ? "w" : "-")
				   << ((p & 0100) != 0 ? "x" : "-")
				   << ((p & 040)  != 0 ? "r" : "-")
				   << ((p & 020)  != 0 ? "w" : "-")
				   << ((p & 010)  != 0 ? "x" : "-")
				   << ((p & 04)   != 0 ? "r" : "-")
				   << ((p & 02)   != 0 ? "w" : "-")
				   << ((p & 01)   != 0 ? "x" : "-");

			return stream.str();
		}

		void refresh_windows() {
			int window_height = draw_border ? LINES - 4 : LINES - 3;;

			wresize(main_window, window_height, COLS / 2 - 1);
			wresize(preview_window, window_height, COLS / 2 - 2);
			mvwin(main_window, 2, 1);
			mvwin(preview_window, 2, (COLS / 2) + 1);

			wrefresh(main_window);
			wrefresh(preview_window);
		}

		void draw_current_directory() {
			std::string current_path = std::string(
					boost::filesystem::current_path().string());

			mvwprintw(stdscr, 0, 0, std::string(COLS, ' ').c_str());
			mvwprintw(stdscr, 0, 0, current_path.c_str());

			wattron(stdscr, A_BOLD);

			mvwprintw(stdscr, 0, current_path.length(),
					std::string("/" + main_elements[selected[0]]).c_str());

			wattroff(stdscr, A_BOLD);
		}

		void colors_on(int i) {
			wattron(main_window, colors_map[i].color);
			wattron(preview_window, colors_map[i].color);
		}

		void handle_colors(std::string selected_file) {
			for(int i = 0; i < colors_map.size(); i++) {
				boost::filesystem::path path_object(selected_file);

				if((colors_map[i].extension == path_object.extension().string())
				|| (colors_map[i].extension == "dir"
				&& boost::filesystem::is_directory(selected_file))
				|| (colors_map[i].extension == "")) {

					colors_on(i);
					return;
				}
			}
		}

		void colors_off() {
			wattroff(main_window, A_REVERSE
					|BRIGHT
					|BLINK
					|COLOR_PAIR(1)
					|COLOR_PAIR(2)
					|COLOR_PAIR(3)
					|COLOR_PAIR(4)
					|COLOR_PAIR(5)
					|COLOR_PAIR(6)
					|COLOR_PAIR(7)
					|COLOR_PAIR(8));

			wattroff(preview_window, A_REVERSE
					|BRIGHT
					|BLINK
					|COLOR_PAIR(1)
					|COLOR_PAIR(2)
					|COLOR_PAIR(3)
					|COLOR_PAIR(4)
					|COLOR_PAIR(5)
					|COLOR_PAIR(6)
					|COLOR_PAIR(7)
					|COLOR_PAIR(8));
		}

		void draw_elements(std::vector<std::string> elements,
						   std::vector<std::string> sizes,
						   WINDOW *window,
						   bool main_window) {

			int y, x;
			getmaxyx(window, y, x);
			y -= 3;

			for(int i = 0; main_window ? i + scroll < elements.size() : i < elements.size(); i++) {
				int index = main_window ? i + scroll : i;

				if(main_window
				|| (!main_window
				&& boost::filesystem::is_directory(main_elements[selected[0]]))) {

					int draw_x = 0;

					if(main_window && selected[0] == i + scroll) {
						wattron(window, A_REVERSE);
					}

					if(main_window
					&& std::find(selected.begin() + 1, selected.end(), i + scroll) != selected.end()) {

						draw_x = selected_space_size;
					}

					if(color_terminal) {
						if(main_window) {
							handle_colors(elements[i]);
						} else {
							handle_colors(main_elements[selected[0]] + "/" + elements[i]);
						}
					}

					std::string size = sizes[index];

					if(elements[index].length() + size.length() + draw_x < x) {
						mvwprintw(window, i, draw_x, std::string(elements[index] + std::string(
								x - elements[index].length() - size.length() - draw_x, ' ') + size).c_str());
					} else {
						mvwprintw(window, i, draw_x, std::string(
								elements[index].substr(0, x - size.length() - 4 - draw_x + 2) + "~ " + size).c_str());
					}

					colors_off();
				} else {
					std::string line = elements[index];
					line = find_and_replace(line, "%", "%%");
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
				&& selected[0] > scroll + y - 3 ? scroll + 1 : scroll;

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

		void load_file_info() {
			std::string selected_filename = main_elements[selected[0]];
			file_info = "";

			struct stat info;
			stat(selected_filename.c_str(), &info);

			if(boost::filesystem::exists(selected_filename)) {
				if(std::string(get_current_directory_size() + " sum, ").length() > COLS) {
					return;
				}

				std::string right_info = get_current_directory_size() + " sum, ";

				if(right_info.length()
				+ get_free_space(selected_filename).length() + 1 > COLS) {
					
					file_info = right_info;
					return;
				}

				right_info += get_free_space(selected_filename) + " ";

				if(right_info.length()
				+ std::string(std::to_string(selected[0] + 1) + "/"
				+ std::to_string(main_elements.size())).length() + 1 > COLS) {
					
					file_info = right_info;
					return;
				}

				right_info += std::to_string(selected[0] + 1) + "/"
					+ std::to_string(main_elements.size());

				if(file_info.length()
				+ right_info.length()
				+ get_file_permissions(selected_filename).length() + 1 > COLS) {

					file_info += std::string(COLS - file_info.length() - right_info.length(), ' ')
						+ right_info;
					return;
				}

				file_info += get_file_permissions(selected_filename) + " ";

				if(file_info.length()
				+ right_info.length()
				+ get_file_owner(selected_filename, info).length() + 1 > COLS) {

					file_info += std::string(COLS - file_info.length() - right_info.length(), ' ')
						+ right_info;
					return;
				}

				file_info += get_file_owner(selected_filename, info) + " ";

				if(!boost::filesystem::is_directory(selected_filename)) {
					if(file_info.length()
					+ right_info.length()
					+ get_file_size(selected_filename).length() + 1 > COLS) {

						file_info += std::string(COLS - file_info.length() - right_info.length(), ' ')
							+ right_info;
						return;
					}

					file_info += get_file_size(selected_filename) + " ";
				}

				if(file_info.length()
				+ right_info.length()
				+ get_file_creation_time(selected_filename, info).length() + 1 > COLS) {

					file_info += std::string(COLS - file_info.length() - right_info.length(), ' ')
						+ right_info;
					return;
				}
				
				file_info += get_file_creation_time(selected_filename, info);
				file_info += std::string(COLS - file_info.length() - right_info.length(), ' ')
					+ right_info;
			}
		}

		std::string find_and_replace(std::string str, std::string search, std::string replace) {
			int pos = str.find(search);
			while(pos != std::string::npos) {
				str.replace(pos, search.length(), replace);
				pos = str.find(search, pos + replace.length());
			}
			return str;
		}

		std::string get_file_size(std::string directory) {
			if(boost::filesystem::exists(directory)) {
				try {
					return format_file_size(boost::filesystem::file_size(directory),
							size_precision);
				} catch(...) {
				}
			}

			return "N/A";
		}

		void loop() {
			boost::filesystem::current_path(starting_directory);
			commands::load({"main"}, this);
			if(!main_elements.empty()) {
				commands::load({"preview"}, this);
			}

			while(true) {
				clear_screen();
				handle_frame();
			}
		}

		void init_colors() {
			if(!has_colors() && color_terminal) {
				commands::quit({"your terminal does not support color"});
			}

			start_color();

			if(use_terminal_colors) {
				use_default_colors();
			}

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

		std::vector<std::string> get_filenames(std::string directory) {
			std::vector<std::string> filenames;
			for(const auto &entry : boost::filesystem::directory_iterator(directory)) {
				filenames.push_back(directory);
			}
			return filenames;
		}

		void clear_screen() {
			for(int i = 0; i < LINES; i++) {
				mvwprintw(stdscr, i, 0, std::string(COLS, ' ').c_str());
			}
			clear_windows();
			refresh();
		}

		std::vector<std::string> split_into_args(std::string str) {
			std::vector<std::string> result(1);
			bool in_quotation = false;
			for(int i = 0; i < str.length(); i++) {
				if(i != 0) {
					if(str[i] == '"') {
						if(str[i - 1] == '\\') {
							result[result.size() - 1].pop_back();
						} else {
							in_quotation = !in_quotation;
							continue;
						}
					}

					if(i != str.length() - 1
					&& str[i + 1] != ' '
					&& str[i] == ' '
					&& !in_quotation){

						if(str[i - 1] == '\\') {
							result[result.size() - 1].pop_back();
						} else {
							result.push_back("");
							continue;
						}
					}
				}
				result[result.size() - 1] += str[i];
			}
			return result;
		}
};

# include "commands.cpp"

int main() {
	user_interface ui;
	ui.init_ncurses();
	ui.loop();
}
