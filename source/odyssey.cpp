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
	EXIT,
	DOWN,
	UP,
	LOAD,
	GET,
	CD,
	SET,
	HIDD,
	MKDIR,
	OPEN,
	MOVE,
	BMOVE,
	EMOVE,
	REMOVE,
	RREMOVE,
	TOUCH,
	SELECT,
	COPY,
	RCOPY,
	COPYDIR,
	PASTE,
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
	std::string command;
};

struct open {
	std::string extension;
	std::string command;
};

# include "config.h"

class user_interface;

class commands {
	public:

		static void exit_ncurses(std::vector<std::string> args);
		static void selected_up(user_interface *ui);
		static void selected_down(user_interface *ui);
		static void set_selected(std::vector<std::string> args, user_interface *ui);
		static void load_directory(std::vector<std::string> args, user_interface *ui);
		static void get_string(std::vector<std::string> args, user_interface *ui);
		static void toggle_hidden(user_interface *ui);
		static void cd(std::vector<std::string> args, user_interface *ui);
		static void mkdir(std::vector<std::string> args, user_interface *ui);
		static void open(std::vector<std::string> args, user_interface *ui);
		static void move_file(std::vector<std::string> args, user_interface *ui);
		static void begin_move(std::vector<std::string> args, user_interface *ui);
		static void end_move(std::vector<std::string> args, user_interface *ui);
		static void remove(std::vector<std::string> args, user_interface *ui);
		static void remove_all(std::vector<std::string> args, user_interface *ui);
		static void touch(std::vector<std::string> args, user_interface *ui);
		static void select(std::vector<std::string> args, user_interface *ui);
		static void copy(std::vector<std::string> args, user_interface *ui);
		static void copy_all(std::vector<std::string> args, user_interface *ui);
		static void copy_directory();
		static void paste(user_interface *ui);
		static void process_command(std::string command, user_interface *ui);
};

class user_interface {
	private:

		WINDOW *main_window;
		WINDOW *preview_window;

		std::vector<std::string> main_elements;
		std::vector<std::string> preview_elements;
		std::vector<std::string> main_sizes;
		std::vector<std::string> preview_sizes;

		std::vector<std::string> file_history;

		std::vector<int> selected = { 0 };

		std::string file_info = "";

		int scroll = 0;

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
					for(int i = 0; i < event_map.size(); i++) {
						if(event_map[i].key == key) {
							commands::process_command(event_map[i].command, this);
						}
					}
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

					if(selected[0] == i + scroll) {
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

	public:

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
			commands::load_directory({boost::filesystem::current_path().string(), "main"}, this);
			commands::load_directory({main_elements[selected[0]], "preview"}, this);

			while(true) {
				clear_screen();
				handle_frame();
			}
		}

		void init_colors() {
			if(!has_colors() && color_terminal) {
				commands::exit_ncurses({"your terminal does not support color."});
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
			commands::load_directory({main_elements[selected[0]], "preview"}, this);
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

			commands::load_directory({main_elements[selected[0]], "preview"}, this);
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

void commands::selected_up(user_interface *ui) {
	set_selected({std::to_string(ui->get_selected()[0])}, ui);
}

void commands::selected_down(user_interface *ui) {
	set_selected({std::to_string(ui->get_selected()[0] + 2)}, ui);
}

void commands::set_selected(std::vector<std::string> args, user_interface *ui) {
	int sum = 0;
	for(int i = 0; i < args.size(); i++) {
		bool is_digit = true;

		for(int j = 0; j < args[i].length(); j++) {
			if(!isdigit(args[i][j])) {
				is_digit = false;
				break;
			}
		}

		if(is_digit) {
			sum += std::stoi(args[i]);
		}
	}

	if(sum < ui->get_main_elements().size() + 1 && sum > 0) {
		std::vector<int> selected = ui->get_selected();
		selected[0] = sum - 1;
		ui->set_selected(selected);
	}
}

void commands::exit_ncurses(std::vector<std::string> args) {
	endwin();
	for(int i = 0; i < args.size(); i++) {
		std::cout << args[i] << std::endl;
	}
	exit(0);
}

void commands::get_string(std::vector<std::string> args, user_interface *ui) {
	curs_set(1);

	std::string placeholder = "";

	for(int i = 1; i < args.size(); i++) {
		placeholder += args[i];
	}

	int cursor = placeholder.length();

	if(args.size() != 0 && args[0] != "-1") {
		cursor = std::stoi(args[0]);
	}

	int x = 0;

	mvwprintw(stdscr, LINES - 1, x, ":");
	mvwprintw(stdscr, LINES - 1, x + 1, std::string(1000, ' ').c_str());
	mvwprintw(stdscr, LINES - 1, x + 1, placeholder.c_str());
	move(LINES - 1, cursor + x + 1);

	int key;
	while((key = getch()) != 10) {
		if(key != ERR) {
			if(key == KEY_BACKSPACE) {
				if(cursor > 0) {
					placeholder.erase(cursor - 1, 1);
					cursor--;
				} else if(placeholder.length() == 0) {
					break;
				}
			} else if(key == KEY_RIGHT) {
				if(cursor < placeholder.length()) {
					cursor++;
				}
			} else if(key == KEY_LEFT) {
				if(cursor > 0) {
					cursor--;
				}
			} else {
				placeholder.insert(cursor, 1, static_cast<char>(key));
				cursor++;
			}

			mvwprintw(stdscr, LINES - 1, x, ":");
			mvwprintw(stdscr, LINES - 1, x + 1, std::string(1000, ' ').c_str());
			mvwprintw(stdscr, LINES - 1, x + 1, placeholder.c_str());
			move(LINES - 1, cursor + x + 1);
		}
	}

	mvwprintw(stdscr, LINES - 1, x, std::string(1000, ' ').c_str());
	process_command(placeholder, ui);

	curs_set(0);
}

void commands::load_directory(std::vector<std::string> args, user_interface *ui) {
	ui->clear_windows();
	std::vector<std::string> elements = {};
	std::vector<std::string> sizes = {};

	if(args.size() != 2 || !boost::filesystem::exists(args[0])) {
		return;
	}

	if(boost::filesystem::is_directory(args[0])) {
		for(const auto &entry : boost::filesystem::directory_iterator(args[0])) {
			std::string filename = entry.path().string();
			filename = filename.substr(filename.find_last_of("/") + 1, filename.length());

			if((filename[0] != '.' && !show_hidden) || (show_hidden)) {
				if(boost::filesystem::is_directory(
							boost::filesystem::absolute(args[0] + "/" + filename))) {

					filename += "/";
					sizes.push_back(std::to_string(
								ui->get_filenames(entry.path().string()).size()));
				} else {
					sizes.push_back(ui->get_file_size(entry.path().string()));
				}

				elements.push_back(filename);
			}
		}
	} else if(args[1] == "preview") {
		std::string selected_filename = ui->get_main_elements()[ui->get_selected()[0]];
		if(boost::filesystem::exists(selected_filename)) {
			std::ifstream read(selected_filename);
			std::string line;
			while(std::getline(read, line)) {
				elements.push_back(line);
			}
			read.close();
		}
	}

	if(args[1] == "main") {
		ui->set_main_elements(elements);
		ui->set_main_sizes(sizes);
	} else if(args[1] == "preview") {
		ui->set_preview_elements(elements);
		ui->set_preview_sizes(sizes);
	}

	ui->load_file_info();
}

void commands::cd(std::vector<std::string> args, user_interface *ui) {
	std::string current_directory;
	if(!ui->get_main_elements().empty()) {
		current_directory = boost::filesystem::canonical(ui->get_main_elements()[ui->get_selected()[0]]).string();
	}

	ui->set_selected(std::vector<int>{ 0 });

	if(args.size() == 0) {
		if(!ui->get_main_elements().empty()) {
			cd({ui->get_main_elements()[ui->get_selected()[0]]}, ui);
		}
	} else {
		if(boost::filesystem::exists(args[0])
		&& boost::filesystem::is_directory(args[0])) {

			std::string oldpath = boost::filesystem::current_path().string();
			std::string newpath = boost::filesystem::canonical(args[0]).string();

			if(boost::filesystem::exists(newpath)) {
				boost::filesystem::current_path(newpath);
				load_directory({boost::filesystem::current_path().string(), "main"}, ui);

				if(!ui->get_main_elements().empty()) {
					std::vector<std::string> file_history = ui->get_file_history();
					for(const auto &entry : boost::filesystem::directory_iterator(newpath)) {
						std::vector<std::string>::iterator iterator =
							std::find(file_history.begin(), file_history.end(), entry.path().string());

						if(iterator != file_history.end()) {
							std::string filename = file_history[std::distance(file_history.begin(), iterator)];
							filename = filename.substr(filename.find_last_of('/') + 1, filename.length());
							ui->set_selected(filename);
						}
					}
				}

				if(std::count(oldpath.begin(), oldpath.end(), '/')
				> std::count(newpath.begin(), newpath.end(), '/')
				&& oldpath.substr(0, newpath.length()) == newpath) {

					if(!ui->get_main_elements().empty()) {
						std::vector<std::string> file_history = ui->get_file_history();

						for(int i = 0; i < file_history.size(); i++) {
							if(file_history[i].substr(0, file_history[i].find_last_of('/'))
							== current_directory.substr(0, current_directory.find_last_of('/'))) {

								file_history.erase(file_history.begin() + i);
							}
						}

						file_history.push_back(current_directory);
						ui->set_file_history(file_history);
					}

					std::string filename = oldpath.substr(newpath.length() + 1, oldpath.length());
					if(std::count(filename.begin(), filename.end(), '/') == 0) {
						ui->set_selected(filename + "/");
					} else {
						ui->set_selected(filename.substr(0, filename.find_first_of('/')) + "/");
					}
				}
			}
		}
	}
}

void commands::toggle_hidden(user_interface *ui) {
	ui->set_selected(std::vector<int>{ 0 });
	show_hidden = !show_hidden;
}

void commands::mkdir(std::vector<std::string> args, user_interface *ui) {
	for(int i = 0; i < args.size(); i++) {
		if(!boost::filesystem::exists(args[i])) {
			boost::filesystem::create_directory(
					boost::filesystem::weakly_canonical(args[i]));
		}
	}
	ui->set_selected(std::vector<int>{ ui->get_selected()[0] });
}

void commands::open(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0 && !ui->get_main_elements().empty()) {
		open({ui->get_main_elements()[ui->get_selected()[0]]}, ui);
	} else if(!ui->get_main_elements().empty()) {
		if(boost::filesystem::exists(args[0])) {
			if(boost::filesystem::is_directory(args[0])) {
				cd({args[0]}, ui);
			} else {
				boost::filesystem::path current_arg(
						boost::filesystem::weakly_canonical(args[0]));

				for(int j = 0; j < open_map.size(); j++) {
					if(current_arg.extension().string() == open_map[j].extension) {
						std::string command = open_map[j].command;
						command = ui->find_and_replace(command, "{f}", current_arg.string());
						system(command.c_str());
						return;
					}
				}

				system(std::string("vim \"" + current_arg.string() + "\"").c_str());
			}
		}
	}
}

void commands::move_file(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		boost::filesystem::path selected_filename(ui->get_main_elements()[ui->get_selected()[0]]);
		get_string({"4", "mv \"", selected_filename.extension().string(), "\""}, ui);
	} if(args.size() == 1) {
		if(boost::filesystem::exists(args[0])
		&& boost::filesystem::is_directory(args[0])) {
			
			std::vector<int> selected = ui->get_selected();

			for(int i = 1; i < selected.size(); i++) {
				std::string selected_filename = ui->get_main_elements()[selected[i]];
				move_file({selected_filename, args[0]}, ui);
			}
		} else {
			std::string selected_filename = ui->get_main_elements()[ui->get_selected()[0]];
			move_file({selected_filename, args[0]}, ui);
		}
	} else if(args.size() == 2) {
		if(boost::filesystem::exists(args[0])
		&& (!boost::filesystem::exists(args[1])
		|| boost::filesystem::is_directory(args[1]))) {

			if(boost::filesystem::is_directory(args[1])) {
				boost::filesystem::path base_path(boost::filesystem::canonical(args[0]));
				std::string target_path = boost::filesystem::canonical(args[1]).string()
					+ "/" + base_path.filename().string();

				if(!boost::filesystem::exists(target_path)) {
					boost::filesystem::rename(base_path, target_path);
				}
			} else {
				boost::filesystem::rename(boost::filesystem::canonical(args[0]),
						boost::filesystem::weakly_canonical(args[1]));
			}
		}
	}

	ui->set_selected(std::vector<int>{ 0 });
}

void commands::begin_move(std::vector<std::string> args, user_interface *ui) {
	get_string({"4", args[0], "mv \"", ui->get_main_elements()[ui->get_selected()[0]], "\""}, ui);
}

void commands::end_move(std::vector<std::string> args, user_interface *ui) {
	boost::filesystem::path selected_filename(
				ui->get_main_elements()[ui->get_selected()[0]]);

	int begin_at = 4 + (selected_filename.string().length() -
			selected_filename.extension().string().length());

	get_string({std::to_string(begin_at + 1), "mv \"", selected_filename.string(), "\""}, ui);
}

void commands::remove(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::vector<int> selected = ui->get_selected();
		for(int i = 1; i < selected.size(); i++) {
			remove({ui->get_main_elements()[selected[i]]}, ui);
		}
	} else {
		ui->set_selected(std::vector<int>{0});

		for(int i = 0; i < args.size(); i++) {
			if(boost::filesystem::exists(args[i])
			|| !boost::filesystem::is_directory(args[i])) {

				boost::filesystem::remove(boost::filesystem::canonical(args[i]));
			}
		}
	}
}

void commands::remove_all(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::vector<int> selected = ui->get_selected();
		for(int i = 1; i < selected.size(); i++) {
			remove_all({ui->get_main_elements()[selected[i]]}, ui);
		}
	} else {
		ui->set_selected(std::vector<int>{0});

		for(int i = 0; i < args.size(); i++) {
			if(boost::filesystem::exists(args[i])) {
				boost::filesystem::remove_all(boost::filesystem::canonical(args[i]));
			}
		}
	}
}

void commands::touch(std::vector<std::string> args, user_interface *ui) {
	for(int i = 0; i < args.size(); i++) {
		if(!boost::filesystem::exists(args[i])) {
			system(std::string("vim \""
						+ boost::filesystem::weakly_canonical(args[i]).string() + "\"").c_str());
		}
	}
	ui->set_selected(std::vector<int>{ ui->get_selected()[0] });
}

void commands::select(std::vector<std::string> args, user_interface *ui) {
	std::vector<int> selected = ui->get_selected();
	std::vector<int>::iterator iterator = std::find(selected.begin() + 1, selected.end(), selected[0]);

	if(iterator != selected.end()) {
		selected.erase(selected.begin() + std::distance(selected.begin(), iterator));
	} else {
		selected.push_back(selected[0]);
	}

	if(selected[0] != ui->get_main_elements().size()) {
		selected[0]++;
	}

	ui->set_selected(selected);
}

void commands::copy_directory() {
	system(std::string("echo \"" + boost::filesystem::current_path().string()
				+ "\" | xclip -selection clipboard").c_str());
}

void commands::copy(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 2) {
		if(boost::filesystem::exists(args[0])
		&& !boost::filesystem::is_directory(args[0])
		&& !boost::filesystem::exists(args[1])
		|| (boost::filesystem::exists(args[1])
		&& boost::filesystem::is_directory(args[1]))) {

			if(boost::filesystem::is_directory(args[1])) {
				boost::filesystem::path base_path(boost::filesystem::canonical(args[0]));

				boost::filesystem::copy(base_path,
						boost::filesystem::canonical(args[1]).string() + "/" + base_path.filename().string());
			} else {
				boost::filesystem::copy(boost::filesystem::canonical(args[0]),
						boost::filesystem::weakly_canonical(args[1]));
			}

			ui->set_selected(std::vector<int>{0});
		}
	} else if(args.size() == 1) {
		std::vector<int> selected = ui->get_selected();
		for(int i = 1; i < selected.size(); i++) {
			copy({ui->get_main_elements()[selected[i]], args[0]}, ui);
		}
	} else if(args.size() == 0 && ui->get_selected().size() != 1) {
		std::string command = "";
		std::vector<int> selected = ui->get_selected();
		for(int i = 1; i < selected.size(); i++) {
			command += boost::filesystem::absolute(ui->get_main_elements()[selected[i]]).string();
			if(i != selected.size() - 1) {
				command += "\\n";
			}
		}
		command += "\" | xclip -selection clipboard";
		command.insert(0, "echo -e \"");
		system(command.c_str());
	}
}

void commands::copy_all(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 2) {
		if(boost::filesystem::exists(args[0])
		&& !boost::filesystem::exists(args[1])
		|| (boost::filesystem::exists(args[1])
		&& boost::filesystem::is_directory(args[1]))) {

			if(boost::filesystem::is_directory(args[1])) {
				boost::filesystem::path base_path(boost::filesystem::canonical(args[0]));

				std::experimental::filesystem::copy(base_path.string(),
						boost::filesystem::canonical(args[1]).string() + "/" + base_path.filename().string(),
						std::experimental::filesystem::copy_options::recursive);
			} else {
				std::experimental::filesystem::copy(boost::filesystem::canonical(args[0]).string(),
						boost::filesystem::weakly_canonical(args[1]).string(),
						std::experimental::filesystem::copy_options::recursive);
			}

			ui->set_selected(std::vector<int>{0});
		}
	} else if(args.size() == 1) {
		std::vector<int> selected = ui->get_selected();
		for(int i = 1; i < selected.size(); i++) {
			copy({ui->get_main_elements()[selected[i]], args[0]}, ui);
		}
	} else if(args.size() == 0 && ui->get_selected().size() != 1) {
		std::string command = "";
		std::vector<int> selected = ui->get_selected();
		for(int i = 1; i < selected.size(); i++) {
			command += boost::filesystem::absolute(ui->get_main_elements()[selected[i]]).string();
			if(i != selected.size() - 1) {
				command += "\\n";
			}
		}
		command += "\" | xclip -selection clipboard";
		command.insert(0, "echo -e \"");
		system(command.c_str());
	}
}

void commands::paste(user_interface *ui) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("xclip -selection clipboard -o", "r"), pclose);
	
	if(!pipe) {
		exit_ncurses({"popen() failed"});
	}

	while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	std::stringstream stream(result);
	std::string line;

	while(std::getline(stream, line, '\n')) {
		copy_all({line, boost::filesystem::current_path().string()
				+ "/" + line.substr(line.find_last_of("/") + 1, line.length())}, ui);
	}

	ui->set_selected(std::vector<int>{ ui->get_selected()[0] });
}

void commands::process_command(std::string command, user_interface *ui) {
	std::vector<std::string> args = ui->split_into_args(command);
	std::vector<std::string> argsp = std::vector<std::string>(args.begin() + 1, args.end());

	for(int i = 0; i < command_map.size(); i++) {
		if(command_map[i].name == args[0]) {
			switch(command_map[i].command) {
				case EXIT     : commands::exit_ncurses(argsp);            break;
				case DOWN     : commands::selected_down(ui);              break;
				case UP       : commands::selected_up(ui);                break;
				case LOAD     : commands::load_directory({argsp}, ui);    break;
				case GET      : commands::get_string(argsp, ui);          break;
				case CD       : commands::cd(argsp, ui);                  break;
				case SET      : commands::set_selected(argsp, ui);        break;
				case HIDD     : commands::toggle_hidden(ui);              break;
				case MKDIR    : commands::mkdir(argsp, ui);               break;
				case OPEN     : commands::open(argsp, ui);                break;
				case MOVE     : commands::move_file(argsp, ui);           break;
				case BMOVE    : commands::begin_move(argsp, ui);          break;
				case EMOVE    : commands::end_move(argsp, ui);            break;
				case REMOVE   : commands::remove(argsp, ui);              break;
				case RREMOVE  : commands::remove_all(argsp, ui);          break;
				case TOUCH    : commands::touch(argsp, ui);               break;
				case SELECT   : commands::select(argsp, ui);              break;
				case COPY     : commands::copy(argsp, ui);                break;
				case RCOPY    : commands::copy_all(argsp, ui);            break;
				case COPYDIR  : commands::copy_directory();               break;
				case PASTE    : commands::paste(ui);                      break;
			}
		}
	}

	load_directory({boost::filesystem::current_path().string(), "main"}, ui);
}

int main() {
	user_interface ui;
	ui.init_ncurses();
	ui.loop();
}
