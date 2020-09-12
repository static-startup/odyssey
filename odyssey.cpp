# include <experimental/filesystem>
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
# include <sys/stat.h>

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
	RENAME,
	BRENAME,
	ERENAME,
	REMOVE,
	RREMOVE,
	TOUCH,
	SELECT,
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
# include "utils.h"

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
		static void rename(std::vector<std::string> args, user_interface *ui);
		static void begin_rename(std::vector<std::string> args, user_interface *ui);
		static void end_rename(std::vector<std::string> args, user_interface *ui);
		static void remove(std::vector<std::string> args, user_interface *ui);
		static void remove_all(std::vector<std::string> args, user_interface *ui);
		static void touch(std::vector<std::string> args, user_interface *ui);
		static void select(std::vector<std::string> args, user_interface *ui);
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

		std::vector<int> selected = { 0 };

		std::string file_info = "";

		int scroll = 0;

		void handle_frame() {
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

				clear_windows();

				if(draw_border) {
					draw_window_borders();
				}

				load_file_info();
				mvwprintw(stdscr, LINES - 1, 0, file_info.c_str());

				draw_current_directory();
				draw_elements(main_elements, main_sizes, main_window, true);
				draw_elements(preview_elements, preview_sizes, preview_window, false);
				handle_empty_directory();

				refresh();
				refresh_windows();
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
			   && std::experimental::filesystem::is_directory(main_elements[selected[0]]))) {

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
				if(!std::experimental::filesystem::is_directory(main_elements[i])
						&& std::experimental::filesystem::exists(main_elements[i])) {

					try {
						file_size += std::experimental::filesystem::file_size(main_elements[i]);
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
			std::string hours = std::to_string(tm->tm_hour - 4);
			std::string minutes = std::to_string(tm->tm_min);
			
			if(tm->tm_mon / 10 < 1) {
				month.insert(0, "0");
			} if(tm->tm_mday / 10 < 1) {
				day.insert(0, "0");
			} if(tm->tm_hour / 10 < 1) {
				hours.insert(0, "0");
			} if(tm->tm_min / 10 < 1) {
				hours.insert(0, "0");
			}

			return year + "-" + month + "-" + day + " " + hours + ":" + minutes;
		}

		std::string get_free_space(std::string directory) {
			std::experimental::filesystem::space_info disk_space
				= std::experimental::filesystem::space("/");

			return format_file_size(disk_space.available, free_size_precision) + " free";
		}

		std::string get_file_permissions(std::string directory) {
			std::experimental::filesystem::perms p =
				std::experimental::filesystem::status(directory).permissions();

			std::stringstream stream;

		   	stream << ((p & std::experimental::filesystem::perms::owner_read)
						   != std::experimental::filesystem::perms::none ? "r" : "-")
				   << ((p & std::experimental::filesystem::perms::owner_write)
						   != std::experimental::filesystem::perms::none ? "w" : "-")
				   << ((p & std::experimental::filesystem::perms::owner_exec)
						   != std::experimental::filesystem::perms::none ? "x" : "-")
				   << ((p & std::experimental::filesystem::perms::group_read)
						   != std::experimental::filesystem::perms::none ? "r" : "-")
				   << ((p & std::experimental::filesystem::perms::group_write)
						   != std::experimental::filesystem::perms::none ? "w" : "-")
				   << ((p & std::experimental::filesystem::perms::group_exec)
						   != std::experimental::filesystem::perms::none ? "x" : "-")
				   << ((p & std::experimental::filesystem::perms::others_read)
						   != std::experimental::filesystem::perms::none ? "r" : "-")
				   << ((p & std::experimental::filesystem::perms::others_write)
						   != std::experimental::filesystem::perms::none ? "w" : "-")
				   << ((p & std::experimental::filesystem::perms::others_exec)
						   != std::experimental::filesystem::perms::none ? "x" : "-");

			return stream.str();
		}

		void load_file_info() {
			std::string selected_filename = main_elements[selected[0]];
			file_info = "";

			struct stat info;
			stat(selected_filename.c_str(), &info);

			if(std::experimental::filesystem::exists(selected_filename)) {
				file_info += get_file_permissions(selected_filename) + " ";
				file_info += get_file_owner(selected_filename, info) + " ";

				if(!std::experimental::filesystem::is_directory(selected_filename)) {
					file_info += get_file_size(selected_filename) + " ";
				}

				file_info += get_file_creation_time(selected_filename, info);

				std::string right_info = get_current_directory_size() + " sum, "
										 + get_free_space(selected_filename) + " "
										 + std::to_string(selected[0] + 1) + "/"
										 + std::to_string(main_elements.size());

				file_info += std::string(COLS - file_info.length() - right_info.length(), ' ');
				file_info += right_info;
			}
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
			mvwprintw(stdscr, 0, 0, std::string(COLS, ' ').c_str());
			mvwprintw(stdscr, 0, 0, std::experimental::filesystem::current_path().c_str());
		}

		void colors_on(int i) {
			wattron(main_window, colors_map[i].color);
			wattron(preview_window, colors_map[i].color);
		}

		void handle_colors(std::string selected_file) {
			for(int i = 0; i < colors_map.size(); i++) {
				std::experimental::filesystem::path path_object(selected_file);
				if((colors_map[i].extension == path_object.extension().string())
				   || (colors_map[i].extension == "dir" && std::experimental::filesystem::is_directory(selected_file))
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

				if(main_window || (!main_window && std::experimental::filesystem::is_directory(main_elements[selected[0]]))) {
					if(main_window && std::find(selected.begin(), selected.end(), i + scroll) != selected.end()) {
						wattron(window, A_REVERSE);
					}

					if(color_terminal) {
						if(main_window) {
							handle_colors(elements[i]);
						} else {
							handle_colors(main_elements[selected[0]] + "/" + elements[i]);
						}
					}

					std::string size = sizes[index];

					if(elements[index].length() + size.length() < x) {
						mvwprintw(window, i, 0, std::string(elements[index] + std::string(
									x - elements[index].length() - size.length(), ' ') + size).c_str());
					} else {
						mvwprintw(window, i, 0, std::string(
									elements[index].substr(0, x - size.length() - 4) + "~ " + size).c_str());
					}

					colors_off();
				} else {
					mvwprintw(window, i, 0, std::string(elements[index]).c_str());
				}
			}
		}

		void bound_selected() {
			int y, x;
			getmaxyx(main_window, y, x);
			clear_windows();

			scroll = selected[0] < scroll && scroll != 0 ? scroll - 1 :
					 selected[0] < main_elements.size() && selected[0] > scroll + y - 3 ? scroll + 1 : scroll;

			selected[0] = selected[0] < 0 ? 0 :
						  selected[0] > main_elements.size() - 1 ? main_elements.size() - 1 : selected[0];
		}

	public:

		std::string find_and_replace(std::string str, std::string search, std::string replace) {
			int pos = str.find(search);
			while(pos != std::string::npos) {
				str.replace(pos, search.length(), replace);
				pos = str.find(search, pos + replace.length());
			}
			return str;
		}

		std::string get_file_size(std::string directory) {
			if(std::experimental::filesystem::exists(directory)) {
				try {
					return format_file_size(std::experimental::filesystem::file_size(directory), size_precision);
				} catch(...) {
				}
			}

			return "N/A";
		}

		void loop() {
			std::experimental::filesystem::current_path(starting_directory);
			commands::load_directory({std::string(std::experimental::filesystem::current_path()), "main"}, this);
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
			for(const auto &entry : std::experimental::filesystem::directory_iterator(directory)) {
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
};

void commands::selected_up(user_interface *ui) {
	set_selected({std::to_string(ui->get_selected()[0] - 1)}, ui);
}

void commands::selected_down(user_interface *ui) {
	set_selected({std::to_string(ui->get_selected()[0] + 1)}, ui);
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

	std::vector<int> selected = ui->get_selected();
	selected[0] = sum;
	ui->set_selected(selected);
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

	if(!std::experimental::filesystem::exists(args[0])) {
		return;
	}

	if(std::experimental::filesystem::is_directory(args[0])) {
		for(const auto &entry : std::experimental::filesystem::directory_iterator(args[0])) {
			std::string filename = entry.path();
			filename = filename.substr(filename.find_last_of("/") + 1, filename.length());

			if((filename[0] != '.' && !show_hidden) || (show_hidden)) {
				if(std::experimental::filesystem::is_directory(
							std::experimental::filesystem::absolute(args[0] + "/" + filename))) {

					filename += "/";
					sizes.push_back(std::to_string(ui->get_filenames(entry.path()).size()));
				} else {
					sizes.push_back(ui->get_file_size(entry.path()));
				}

				elements.push_back(filename);
			}
		}
	} else if(args[1] == "preview") {
		std::string selected_filename = ui->get_main_elements()[ui->get_selected()[0]];
		if(std::experimental::filesystem::exists(selected_filename)) {
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
}

void commands::cd(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		cd({ui->get_main_elements()[ui->get_selected()[0]]}, ui);
	} else {
		for(int i = 0; i < args.size(); i++) {
			if(std::experimental::filesystem::exists(args[i])
	   		   && std::experimental::filesystem::is_directory(args[i])) {

				std::string oldpath = std::experimental::filesystem::current_path();
				std::string newpath = std::experimental::filesystem::canonical(args[i]);

				if(std::experimental::filesystem::exists(newpath)) {
					std::experimental::filesystem::current_path(newpath);

					if(std::count(oldpath.begin(), oldpath.end(), '/')
					   > std::count(newpath.begin(), newpath.end(), '/')
			   		   && oldpath.substr(0, newpath.length()) == newpath) {

						load_directory({std::string(std::experimental::filesystem::current_path()), "main"}, ui);
						
						std::string filename = oldpath.substr(newpath.length() + 1, oldpath.length());
						if(std::count(filename.begin(), filename.end(), '/') == 0) {
							ui->set_selected(filename + "/");
						} else {
							ui->set_selected(filename.substr(0, filename.find_first_of('/')) + "/");
						}
					} else {
						set_selected({"0"}, ui);
					}
				}
			}
		}
	}
}

void commands::toggle_hidden(user_interface *ui) {
	show_hidden = !show_hidden;
}

void commands::mkdir(std::vector<std::string> args, user_interface *ui) {
	for(int i = 0; i < args.size(); i++) {
		if(!std::experimental::filesystem::exists(args[i])) {
			std::experimental::filesystem::create_directory(args[i]);
		}
	}
}

void commands::open(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0 && !ui->get_main_elements().empty()) {
		open({ui->get_main_elements()[ui->get_selected()[0]]}, ui);
	} else {
		for(int i = 0; i < args.size(); i++) {
			if(std::experimental::filesystem::exists(args[i])) {
				if(std::experimental::filesystem::is_directory(args[i])) {
					cd({args[i]}, ui);
				} else {
					std::experimental::filesystem::path path_object(args[i]);

					for(int j = 0; j < open_map.size(); j++) {
						if(path_object.extension().string() == open_map[j].extension) {
							std::string command = open_map[j].command;
							command = ui->find_and_replace(command, "{f}", args[i]);
							system(command.c_str());
							return;
						}
					}

					system(std::string("vim \"" + args[i] + "\"").c_str());
				}
			}
		}
	}
}

void commands::rename(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::experimental::filesystem::path selected_filename(
				ui->get_main_elements()[ui->get_selected()[0]]);

		get_string({"7", "rename " + selected_filename.extension().string()}, ui);
	} if(args.size() == 1) {
		if(!std::experimental::filesystem::exists(args[0])) {
			rename({ui->get_main_elements()[ui->get_selected()[0]], args[0]}, ui);
		}
	} else if(args.size() == 2) {
		if(std::experimental::filesystem::exists(args[0])
		   && !std::experimental::filesystem::exists(args[1])) {

			std::experimental::filesystem::rename(args[0], args[1]);
		}
	}
}

void commands::begin_rename(std::vector<std::string> args, user_interface *ui) {
	get_string({"7", "rename " + ui->get_main_elements()[ui->get_selected()[0]]}, ui);
}

void commands::end_rename(std::vector<std::string> args, user_interface *ui) {
	std::experimental::filesystem::path selected_filename(
				ui->get_main_elements()[ui->get_selected()[0]]);

	int begin_at = 7 + (selected_filename.string().length() - selected_filename.extension().string().length());
	get_string({std::to_string(begin_at), "rename " + selected_filename.string()}, ui);
}

void commands::remove(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::vector<int> selected = ui->get_selected();
		for(int i = 0; i < selected.size(); i++) {
			std::string current_filename = ui->get_main_elements()[selected[i]];

			if(std::experimental::filesystem::exists(current_filename)
			   && !std::experimental::filesystem::is_directory(current_filename)) {

				std::experimental::filesystem::remove(current_filename);
			}
		}
		selected[0] = 0;
		ui->set_selected(std::vector<int>(selected.begin(), selected.begin() + 1));
	} else {
		for(int i = 0; i < args.size(); i++) {
			if(std::experimental::filesystem::exists(args[i])
			   && !std::experimental::filesystem::is_directory(args[i])) {

				std::experimental::filesystem::remove(args[i]);
			}
		}
	}
}

void commands::remove_all(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::vector<int> selected = ui->get_selected();
		for(int i = 0; i < selected.size(); i++) {
			std::string current_filename = ui->get_main_elements()[selected[i]];

			if(std::experimental::filesystem::exists(current_filename)) {
				std::experimental::filesystem::remove_all(current_filename);
			}
		}
		selected[0] = 0;
		ui->set_selected(std::vector<int>(selected.begin(), selected.begin() + 1));
	} else {
		for(int i = 0; i < args.size(); i++) {
			if(std::experimental::filesystem::exists(args[i])) {
				std::experimental::filesystem::remove_all(args[i]);
			}
		}
	}
}

void commands::touch(std::vector<std::string> args, user_interface *ui) {
	for(int i = 0; i < args.size(); i++) {
		if(!std::experimental::filesystem::exists(args[i])) {
			system(std::string("vim \"" + args[i] + "\"").c_str());
		}
	}
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

void commands::process_command(std::string command, user_interface *ui) {
	std::vector<std::string> args = split_into_args(command);
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
				case RENAME   : commands::rename(argsp, ui);              break;
				case BRENAME  : commands::begin_rename(argsp, ui);        break;
				case ERENAME  : commands::end_rename(argsp, ui);          break;
				case REMOVE   : commands::remove(argsp, ui);              break;
				case RREMOVE  : commands::remove_all(argsp, ui);          break;
				case TOUCH    : commands::touch(argsp, ui);               break;
				case SELECT   : commands::select(argsp, ui);              break;
			}
		}
	}

	load_directory({std::string(std::experimental::filesystem::current_path()), "main"}, ui);
	if(!ui->get_main_elements().empty()) {
		load_directory({ui->get_main_elements()[ui->get_selected()[0]], "preview"}, ui);
	}
}

int main() {
	user_interface ui;
	ui.init_ncurses();
	ui.loop();
}
