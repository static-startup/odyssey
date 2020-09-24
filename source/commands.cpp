/* private helper functions */

bool commands::is_digit(std::string number) {
	for(char character : number) {
		if(!isdigit(character)) {
			return false;
		}
	}
	return true;
}

std::string commands::combine_vector(std::vector<std::string> vector) {
	// copys vector with a delimiter of ' '
	std::ostringstream stream;
	if(!vector.empty()) {
		std::copy(vector.begin(), vector.end() - 1, std::ostream_iterator<std::string>(stream, " "));
		stream << vector.back();
	}
	return stream.str();
}

void commands::wipe_elements(user_interface *ui) {
	ui->set_main_elements({});
	ui->set_main_sizes({});
	ui->set_preview_elements({});
	ui->set_preview_sizes({});
}

/* public helper functions */

// turns int file size to string with character indicating file size type
std::string commands::format_file_size(unsigned long file_size, int precision) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision);

	if(file_size == -1) {
		return "N/A";
	}

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

unsigned long commands::file_size(std::string directory) {
	if(boost::filesystem::exists(directory)) {
		try {
			return boost::filesystem::file_size(directory);
		} catch(...) {
		}
	}
	return -1;
}

// gets the sum of all file sizes of directory
unsigned long commands::file_sizes(std::string directory) {
	unsigned long sum = 0;
	for(auto const &entry : boost::filesystem::directory_iterator(directory)) {
		std::string filename = entry.path().string();
		filename = filename.substr(filename.find_last_of('/') + 1, filename.size());

		if(((filename[0] != '.' && !show_hidden) || show_hidden)
		&& boost::filesystem::exists(entry.path().string())
		&& !boost::filesystem::is_directory(entry.path().string())) {

			sum += boost::filesystem::file_size(entry.path().string());
		}
	}
	return sum;
}

std::string commands::file_owner(std::string directory) {
	struct stat info;
	stat(directory.c_str(), &info);
	struct passwd *pw = getpwuid(info.st_uid);
	struct group *gr = getgrgid(info.st_gid);
	return std::string(pw->pw_name) + ":" + std::string(gr->gr_name);
}

// gets how many items is in a directory
int commands::directory_items(std::string directory) {
	int sum = 0;
	if(boost::filesystem::exists(directory)
	&& boost::filesystem::is_directory(directory)) {

		for(auto const &entry : boost::filesystem::directory_iterator(directory)) {
			std::string filename = entry.path().string();
			filename = filename.substr(filename.find_last_of('/') + 1, filename.size());
			if((filename[0] != '.' && !show_hidden) || show_hidden) {
				sum++;
			}
		}
	}
	return sum;
}

std::string commands::file_permissions(std::string directory) {
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

// gets file last modified time
std::string commands::file_last_mod_time(std::string directory) {
	struct stat info;
	stat(directory.c_str(), &info);
	struct tm *tm = gmtime(&(info.st_mtime));

	std::string year = std::to_string(1900 + tm->tm_year);
	std::string month = std::to_string(tm->tm_mon);
	std::string day = std::to_string(tm->tm_mday);

	std::string hours = std::to_string(tm->tm_hour - 4 < 0 ?
			24 - (tm->tm_hour - 4) * -1 : tm->tm_hour - 4);

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

// get disk free space
unsigned long commands::free_space(std::string directory) {
	return boost::filesystem::space("/").available;
}

std::string commands::find_and_replace(std::string str, std::string search, std::string replace) {
	int pos = str.find(search);
	while(pos != std::string::npos) {
		str.replace(pos, search.length(), replace);
		pos = str.find(search, pos + replace.length());
	}
	return str;
}

/* main functions */

void commands::up(user_interface *ui) {
	if(ui->get_selected()[0] > 0) {
		set({std::to_string(ui->get_selected()[0])}, ui);
	}
}

void commands::down(user_interface *ui) {
	if(ui->get_selected()[0] + 2 <= ui->get_main_elements().size()) {
		set({std::to_string(ui->get_selected()[0] + 2)}, ui);
	}
}

void commands::set(std::vector<std::string> args, user_interface *ui) {
	int sum = 0;
	for(int i = 0; i < args.size(); i++) {
		if(is_digit(args[i])) {
			sum += std::stoi(args[i]);
		} else {
			ui->set_error_message("\"" + args[i] + "\" cannot be converted to integer");
			return;
		}
	}

	if(sum < ui->get_main_elements().size() + 1 && sum > 0) {
		std::vector<int> selected = ui->get_selected();
		selected[0] = sum - 1;
		ui->set_selected(selected);
	} else {
		ui->set_error_message("\"" + std::to_string(sum) + "\" is not in bounds");
	}
}

void commands::quit(std::vector<std::string> args) {
	endwin();
	if(combine_vector(args) != "") {
		std::cout << combine_vector(args) << std::endl;
	}
	exit(0);
}

std::string commands::get(std::vector<std::string> args, int drawx, bool locked, user_interface *ui) {
	curs_set(1);

	std::string placeholder = combine_vector(std::vector<std::string>(args.begin() + 1, args.end()));

	int cursor = placeholder.length();

	if(args.size() != 0 && args[0] != "-1") {
		cursor = std::stoi(args[0]) + drawx;
	}

	int x = 0;

	mvwprintw(stdscr, LINES - 1, x + drawx, std::string(1000, ' ').c_str());
	mvwprintw(stdscr, LINES - 1, x + drawx, placeholder.c_str());
	move(LINES - 1, cursor + x + drawx);

	int key;
	while((key = getch()) != 10) {
		if(key != ERR) {
			if(key == KEY_BACKSPACE) {
				if(cursor > 0) {
					placeholder.erase(cursor - 1, 1);
					cursor--;
				} else if(placeholder.length() == 0 && !locked) {
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

			mvwprintw(stdscr, LINES - 1, x + drawx, std::string(1000, ' ').c_str());
			mvwprintw(stdscr, LINES - 1, x + drawx, placeholder.c_str());
			move(LINES - 1, cursor + x + drawx);
		}
	}

	mvwprintw(stdscr, LINES - 1, x, std::string(1000, ' ').c_str());
	curs_set(0);
	return placeholder;
}

// loads the file of the current directory to vectors
void commands::load(std::vector<std::string> args, user_interface *ui) {
	ui->clear_windows();
	std::vector<std::string> elements = {};
	std::vector<std::string> sizes = {};

	if(args.size() != 1) {
		return;
	}

	std::string directory;

	if(args[0] == "main") {
		directory = boost::filesystem::current_path().string();
	} else if(args[0] == "preview") {
		// if main vector empty? exit
		if(ui->get_main_elements().empty()) {
			wipe_elements(ui);
			return;
		}

		std::string selected_filename = ui->get_main_elements()[ui->get_selected()[0]];
		directory = selected_filename;

		// if the selected filename does not exists? exit
		if(!boost::filesystem::exists(selected_filename)) {
			wipe_elements(ui);
			return;
		}

		// if selected filename is a file? read file & exit
		if(!boost::filesystem::is_directory(selected_filename)) {
			std::ifstream read(selected_filename);
			std::string line;
			for(int i = 0; i < LINES && std::getline(read, line); i++) {
				elements.push_back(line);
			}
			read.close();

			ui->set_preview_elements(elements);
			ui->set_preview_sizes(sizes);
			return;
		}
	}

	// loop though directory add append to vector
	for(const auto &entry : boost::filesystem::directory_iterator(directory)) {
		if(boost::filesystem::exists(entry.path().string())) {
			std::string filename = entry.path().string();
			filename = filename.substr(filename.find_last_of("/") + 1, filename.length());
			if((filename[0] != '.' && !show_hidden) || (show_hidden)) {
				if(boost::filesystem::is_directory(entry.path().string())) {
					filename += "/";
					sizes.push_back(std::to_string(directory_items(entry.path().string())));
				} else {
					sizes.push_back(format_file_size(file_size(entry.path().string()), size_precision));
				}
				elements.push_back(filename);
			}
		}
	}

	if(args[0] == "main") {
		ui->set_main_elements(elements);
		ui->set_main_sizes(sizes);
	} else if(args[0] == "preview") {
		ui->set_preview_elements(elements);
		ui->set_preview_sizes(sizes);
	}
}

void commands::cd(std::vector<std::string> args, user_interface *ui) {
	// get selected filename
	std::string current_directory;
	if(!ui->get_main_elements().empty()) {
		current_directory = boost::filesystem::canonical(
				ui->get_main_elements()[ui->get_selected()[0]]).string();
	}

	if(args.size() == 0) {
		// not empty? cd into selected filename
		if(!ui->get_main_elements().empty()) {
			cd({current_directory}, ui);
		} else {
			ui->set_error_message("Cannot change directory (In empty directory)");
		}
	} else {
		std::string directory = combine_vector(args);

		if(boost::filesystem::exists(directory)
		&& boost::filesystem::is_directory(directory)) {

			ui->set_selected(std::vector<int>{ 0 });

			std::string oldpath = boost::filesystem::current_path().string();
			std::string newpath = boost::filesystem::canonical(directory).string();

			if(boost::filesystem::exists(newpath)) {
				boost::filesystem::current_path(newpath);
				load({"main"}, ui);

				// if directory not empty? set selected to previous selected
				if(!ui->get_main_elements().empty()) {
					std::vector<std::string> file_history = ui->get_file_history();
					for(const auto &entry : std::experimental::filesystem::directory_iterator(newpath)) {
						std::vector<std::string>::iterator iterator =
							std::find(file_history.begin(), file_history.end(), entry.path().string());

						// if current path is found in file history
						if(iterator != file_history.end()) {
							std::string filename = file_history[std::distance(file_history.begin(), iterator)];
							filename = filename.substr(filename.find_last_of('/') + 1, filename.length());
							if(boost::filesystem::exists(filename)) {
								ui->set_selected(filename);
							} else {
								file_history.erase(file_history.begin()
										+ std::distance(file_history.begin(), iterator));
							}
						}
					}
				}

				// if oldpath contains newpath and oldpath is bigger then newpath
				if(std::count(oldpath.begin(), oldpath.end(), '/')
				> std::count(newpath.begin(), newpath.end(), '/')
				&& oldpath.substr(0, newpath.length()) == newpath) {

					if(!ui->get_main_elements().empty()) {

						std::vector<std::string> file_history = ui->get_file_history();

						// find paths in filehistory that contains current path & delete them
						for(int i = 0; i < file_history.size(); i++) {
							if(file_history[i].substr(0, file_history[i].find_last_of('/'))
							== current_directory.substr(0, current_directory.find_last_of('/'))) {

								file_history.erase(file_history.begin() + i);
							}
						}

						// push back the previous selected filename
						file_history.push_back(current_directory);
						ui->set_file_history(file_history);
					}

					// sets selected to folder we came from
					std::string filename = oldpath.substr(newpath.length() + 1, oldpath.length());
					if(std::count(filename.begin(), filename.end(), '/') == 0) {
						ui->set_selected(filename + "/");
					} else {
						ui->set_selected(filename.substr(0, filename.find_first_of('/')) + "/");
					}
				}
			}
		} else {
			if(!boost::filesystem::exists(directory)) {
				ui->set_error_message(
						"Cannot change directory \"" + directory + "\" (No such file or directory)");
			} else {
				ui->set_error_message(
						"Cannot change directory \"" + directory + "\" (Not a directory)");
			}
		}
	}
}

void commands::hidden(user_interface *ui) {
	show_hidden = !show_hidden;
	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::mkdir(std::vector<std::string> args, user_interface *ui) {
	std::string filename = combine_vector(args);

	if(!boost::filesystem::exists(filename)) {
		boost::filesystem::create_directory(filename);
		ui->set_selected(std::vector<int>{ui->get_selected()[0]});
	} else { 
		if(boost::filesystem::is_directory(filename)) {
			ui->set_error_message("Cannot create directory \"" + filename + "\" (Directory exists)");
		} else {
			ui->set_error_message("Cannot create directory \"" + filename + "\" (File exists)");
		}
	}
}

// cd if directory otherwise opens file
void commands::open(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		if(!ui->get_main_elements().empty()) {
			open({ui->get_main_elements()[ui->get_selected()[0]]}, ui);
		} else {
			ui->set_error_message("Cannot open (In empty directory)");
			return;
		}
	} else {
		std::string filename = combine_vector(args);

		if(boost::filesystem::exists(filename)) {
			if(boost::filesystem::is_directory(filename)) {
				cd({filename}, ui);
			} else {
				boost::filesystem::path filename_obj(
						boost::filesystem::canonical(filename));

				for(int j = 0; j < open_map.size(); j++) {
					if(filename_obj.extension().string() == open_map[j].extension) {
						std::string command = open_map[j].command;
						filename = find_and_replace(filename, "\"", "\\\"");
						command = find_and_replace(command, "{f}", "\"" + filename + "\"");
						system(command.c_str());
						return;
					}
				}

				// if file extension not found it will open file with vim
				filename = find_and_replace(filename, "\"", "\\\"");
				system(std::string("vim \"" + filename + "\"").c_str());
			}
		} else {
			ui->set_error_message("Cannot open \"" + filename + "\" (No such file or directory)");
		}
	}
}

void commands::move_file(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::vector<int> selected = ui->get_selected();
		std::string selected_filename = ui->get_main_elements()[selected[0]];

		// have selected elements
		if(ui->get_selected().size() != 1) {
			if(boost::filesystem::exists(selected_filename)
			&& boost::filesystem::is_directory(selected_filename)) {
				// check if user is trying to move something to a subdirectory of itself
				for(int i = 1; i < selected.size(); i++) {
					std::string filename_full = boost::filesystem::canonical(selected_filename).string();
					std::string target_full = boost::filesystem::canonical(ui->get_main_elements()[selected[i]]).string();

					if(filename_full.substr(0, target_full.length()) == target_full) {
						ui->set_error_message("Cannot move \""
								+ ui->get_main_elements()[selected[i]] + "\" to a subdirectory of itself");
						return;
					}
				}

				// loop though selected and move elements to selected filename
				for(int i = 1; i < selected.size(); i++) {
					boost::filesystem::rename(ui->get_main_elements()[selected[i]],
							selected_filename + ui->get_main_elements()[selected[i]]);
				}
				ui->set_selected(std::vector<int>{ui->get_selected()[0]});
			} else {
				ui->set_error_message("Cannot move to \"" + selected_filename + "\" (Not a directory)");
			}
		} else {
			ui->set_error_message("Cannot move (No selected elements)");
		}
	} else {
		std::string filename = combine_vector(args);
		
		if(filename == "") {
			ui->set_error_message("Cannot move (No filename)");
			return;
		}

		if(boost::filesystem::exists(filename)
		&& boost::filesystem::is_directory(filename)) {
			
			std::vector<int> selected = ui->get_selected();

			// check for impossible scenario
			for(int i = 1; i < selected.size(); i++) {
				boost::filesystem::path base_path(boost::filesystem::canonical(ui->get_main_elements()[ui->get_selected()[i]]));
				std::string target_path = boost::filesystem::canonical(filename).string()
					+ "/" + base_path.filename().string();
				boost::filesystem::path filename_full = boost::filesystem::canonical(filename).string();

				if(boost::filesystem::exists(target_path)) {
					if(boost::filesystem::is_directory(target_path)) {
						ui->set_error_message("Cannot move \"" + target_path + "\" (directory exists)");
					} else {
						ui->set_error_message("Cannot move \"" + target_path + "\" (file exists)");
					}
					return;
				}

				if(filename_full.string().substr(0, base_path.string().length()) == base_path.string()) {
					ui->set_error_message("Cannot move \""
							+ ui->get_main_elements()[selected[i]] + "\" to a subdirectory of itself");
					return;
				}
			}

			// loop through and move
			for(int i = 1; i < selected.size(); i++) {
				boost::filesystem::path base_path(boost::filesystem::canonical(ui->get_main_elements()[selected[i]]));
				boost::filesystem::rename(base_path, boost::filesystem::canonical(filename).string()
					+ "/" + base_path.filename().string());
			}

			ui->set_selected(std::vector<int>{ui->get_selected()[0]});
		} else if(!boost::filesystem::exists(filename)) {
			boost::filesystem::rename(boost::filesystem::canonical(ui->get_main_elements()[ui->get_selected()[0]]),
					boost::filesystem::weakly_canonical(filename));
		}
	}
}

void commands::rename(std::vector<std::string> args, user_interface *ui) {
	boost::filesystem::path selected_filename(
			ui->get_main_elements()[ui->get_selected()[0]]);
	mvprintw(LINES - 1, 0, ":");
	process_command(get({"3", "mv", selected_filename.extension().string()}, 1, false, ui), ui);
}

void commands::begin_move(std::vector<std::string> args, user_interface *ui) {
	mvprintw(LINES - 1, 0, ":");
	process_command(get({"3", "mv", ui->get_main_elements()[ui->get_selected()[0]]}, 1, false, ui), ui);
}

void commands::end_move(std::vector<std::string> args, user_interface *ui) {
	boost::filesystem::path selected_filename(
				ui->get_main_elements()[ui->get_selected()[0]]);

	int begin_at = 3 + (selected_filename.string().length() -
			selected_filename.extension().string().length());

	mvprintw(LINES - 1, 0, ":");
	process_command(get({std::to_string(begin_at), "mv", selected_filename.string()}, 1, false, ui), ui);
}

void commands::remove(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::vector<int> selected = ui->get_selected();
		if(selected.size() == 1) {
			ui->set_error_message("Cannot remove (No selected elements)");
		}

		// ask for comfirmation
		mvprintw(LINES - 1, 0, "are you sure > ");
		std::string choice = get({"-1", ""}, 15, true, ui);
		if(choice != "y") {
			ui->set_message("ignored.");
			return;
		}

		for(int i = 1; i < selected.size(); i++) {
			if(boost::filesystem::exists(ui->get_main_elements()[selected[i]])) {
				boost::filesystem::remove_all(ui->get_main_elements()[selected[i]]);
			}
		}
	} else {
		std::string filename = combine_vector(args);
		if(boost::filesystem::exists(filename)) {
			// ask for comfirmation
			mvprintw(LINES - 1, 0, "are you sure > ");
			std::string choice = get({"-1", ""}, 15, true, ui);
			if(choice != "y") {
				ui->set_message("ignored.");
				return;
			}
			boost::filesystem::remove_all(boost::filesystem::canonical(filename));
		} else {
			ui->set_error_message("Cannot remove \"" + filename + "\" (No such file or directory)");
			return;
		}
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::touch(std::vector<std::string> args, user_interface *ui) {
	std::string filename = combine_vector(args);
	if(!boost::filesystem::exists(filename)) {
		std::ofstream write(filename);
		write.close();
		ui->set_selected(std::vector<int>{ui->get_selected()[0]});
	} else {
		if(boost::filesystem::is_directory(filename)) {
			ui->set_error_message("Cannot create file \"" + filename + "\" (Directory exists)");
		} else {
			ui->set_error_message("Cannot create file \"" + filename + "\" (File exists)");
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

void commands::copy_directory(user_interface *ui) {
	std::string filename = boost::filesystem::current_path().string();
	filename = find_and_replace(filename, "\"", "\\\"");
	system(std::string("echo \"" + filename
				+ "\" | xclip -selection clipboard").c_str());
}

void commands::copy(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		if(ui->get_selected().size() != 1) {
			// copy file paths to xclip
			std::string command = "";
			std::vector<int> selected = ui->get_selected();
			for(int i = 1; i < selected.size(); i++) {
				command += boost::filesystem::absolute(ui->get_main_elements()[selected[i]]).string();
				if(command.back() == '/') {
					command.pop_back();
				}

				if(i != selected.size() - 1) {
					command += "\\n";
				}
			}
			ui->set_selected(std::vector<int>{ui->get_selected()[0]});
			command += "\" | xclip -selection clipboard";
			command.insert(0, "echo -e \"");
			system(command.c_str());
		} else {
			ui->set_error_message("Cannot copy (No selected elements)");
		}
	} else {
		std::string filename = combine_vector(args);

		if(filename == "") {
			ui->set_error_message("Cannot copy (No filename)");
			return;
		}

		if(boost::filesystem::exists(filename)
		&& boost::filesystem::is_directory(filename)) {

			std::vector<int> selected = ui->get_selected();

			// scans for errors
			for(int i = 1; i < selected.size(); i++) {
				std::string selected_filename = ui->get_main_elements()[selected[i]];

				boost::filesystem::path base_path(boost::filesystem::canonical(selected_filename));
				std::string target = boost::filesystem::canonical(filename).string()
					+ "/" + base_path.filename().string();
				std::string filename_full = boost::filesystem::canonical(filename).string();

				if(filename_full.substr(0, base_path.string().length()) == base_path
				|| boost::filesystem::exists(target)) {

					if(boost::filesystem::exists(target)) {
						ui->set_error_message("Cannot copy to \"" + target + "\" (File already exists)");
					} else {
						ui->set_error_message("Cannot copy \""
								+ ui->get_main_elements()[selected[i]] + "\" to a subdirectory of itself");
					}
					return;
				}
			}

			// does the copying
			for(int i = 1; i < selected.size(); i++) {
				std::string selected_filename = ui->get_main_elements()[selected[i]];
				boost::filesystem::path base_path(boost::filesystem::canonical(selected_filename));

				std::experimental::filesystem::copy(base_path.string(),
						boost::filesystem::canonical(filename).string() + "/" + base_path.filename().string(),
						std::experimental::filesystem::copy_options::recursive);
			}
			ui->set_selected(std::vector<int>{ui->get_selected()[0]});
		} else if(!boost::filesystem::exists(filename)) {
			std::experimental::filesystem::copy(ui->get_main_elements()[ui->get_selected()[0]],
					boost::filesystem::weakly_canonical(filename).string(),
					std::experimental::filesystem::copy_options::recursive);
		}
	}
}

void commands::paste(user_interface *ui) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("xclip -selection clipboard -o", "r"), pclose);
	
	if(!pipe) {
		quit({"popen() failed"});
	}

	while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	std::stringstream stream(result);
	std::string line;

	// check for errors
	while(std::getline(stream, line, '\n')) {
		std::string target = boost::filesystem::current_path().string()
				+ "/" + line.substr(line.find_last_of("/") + 1, line.length());

		if(boost::filesystem::exists(target)
		|| line == target) {

			if(boost::filesystem::exists(target)) {
				if(boost::filesystem::is_directory(target)) {
					ui->set_error_message("Cannot paste \"" + target + "\" (Directory exists)");
				} else {
					ui->set_error_message("Cannot paste \"" + target + "\" (File exists)");
				}
			} else {
				ui->set_error_message("Cannot paste \"" + target + "\" into a subdirectory if itself");
			}
			return;
		}
	}

	stream = std::stringstream(result);
	
	// do the copying
	while(std::getline(stream, line, '\n')) {
		std::experimental::filesystem::copy(line, boost::filesystem::current_path().string()
				+ "/" + line.substr(line.find_last_of("/") + 1, line.length()),
				std::experimental::filesystem::copy_options::recursive);
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::top(user_interface *ui) {
	if(!ui->get_main_elements().empty()) {
		set({"1"}, ui);
	}
}

void commands::bottom(user_interface *ui) {
	if(!ui->get_main_elements().empty()) {
		set({std::to_string(ui->get_main_elements().size())}, ui);
	}
}

void commands::shell(std::vector<std::string> args) {
	system(combine_vector(args).c_str());
}

void commands::extract(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		boost::filesystem::path selected_filename(ui->get_main_elements()[ui->get_selected()[0]]);

		if(selected_filename.extension().string() == ".bz2"
		|| selected_filename.extension().string() == ".gz"
		|| selected_filename.extension().string() == ".tar") {

			std::string filename = selected_filename.string();
			if(selected_filename.extension().string() == ".bz2") {
				filename = filename.substr(0, filename.length() - 8);
			} else if(selected_filename.extension().string() == ".gz") {
				filename = filename.substr(0, filename.length() - 7);
			} else if(selected_filename.extension().string() == ".tar") {
				filename = filename.substr(0, filename.length() - 4);
			}

			filename = find_and_replace(filename, "\"", "\\\"");

			if(!boost::filesystem::exists(filename)) {
				mkdir({filename}, ui);
				system(std::string("tar -xf \"" + selected_filename.string() + "\" -C " + filename).c_str());
				ui->set_selected(std::vector<int>{ui->get_selected()[0]});
			} else {
				if(boost::filesystem::is_directory(filename)) {
					ui->set_error_message("Cannot extract to \"" + filename + "\" (Directory exists)");
				} else {
					ui->set_error_message("Cannot extract to \"" + filename + "\" (File exists)");
				}
			}
		} else {
			ui->set_error_message("Cannot extract \""
					+ selected_filename.string() + "\" (Not a compressed file)");
		}
	}
}

// sf asd fasf sadf asdf sadf 

void commands::compress(std::vector<std::string> args, user_interface *ui) {
	std::vector<int> selected = ui->get_selected();
	std::string elements = "";
	for(int i = 1; i < selected.size(); i++) {
		elements += ui->get_main_elements()[selected[i]] + " ";
	}

	std::string filename = "";
	for(int i = 0; i < args.size(); i++) {
		filename += args[i];
		if(i != args.size() - 1) {
			filename += " ";
		}
	}

	if(elements == "") {
		ui->set_error_message("Cannot compress (No selected elements)");
		return;
	}

	if(filename == "") {
		ui->set_error_message("Cannot compress (No filename)");
		return;
	}

	if(!boost::filesystem::exists(filename)) {
		if(boost::filesystem::path(filename).extension().string() == ".gz") {
			filename = find_and_replace(filename, "\"", "\\\"");
			system(std::string("tar -czf \"" + filename + "\" " + elements).c_str());
		} else {
			ui->set_error_message("Cannot compress (Unrecognized compression type)");
			return;
		}
	} else {
		if(boost::filesystem::is_directory(filename)) {
			ui->set_error_message("Cannot compress \"" + filename + "\" (Directory exists)");
		} else {
			ui->set_error_message("Cannot compress \"" + filename + "\" (File exists)");
		}
		return;
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::process_command(std::string command, user_interface *ui) {
	std::vector<std::string> args = ui->split_into_args(command);
	std::vector<std::string> argsp = std::vector<std::string>(args.begin() + 1, args.end());
	bool executed = false;

	for(int i = 0; i < command_map.size(); i++) {
		if(command_map[i].name == args[0]) {
			switch(command_map[i].command) {
				case QUIT : quit(argsp); break;
				case DOWN : down(ui); break;
				case UP : up(ui); break;
				case LOAD : load(argsp, ui); break;
				case GET : mvprintw(LINES - 1, 0, ":"); process_command(get(argsp, 1, false, ui), ui); break;
				case CD : cd(argsp, ui); break;
				case SET : set(argsp, ui); break;
				case HIDDEN : hidden(ui); break;
				case MKDIR : mkdir(argsp, ui); break;
				case OPEN : open(argsp, ui); break;
				case MOVE : move_file(argsp, ui); break;
				case BMOVE : begin_move(argsp, ui); break;
				case EMOVE : end_move(argsp, ui); break;
				case REMOVE : remove(argsp, ui); break;
				case TOUCH : touch(argsp, ui); break;
				case SELECT : select(argsp, ui); break;
				case COPY : copy(argsp, ui); break;
				case COPYDIR : copy_directory(ui); break;
				case PASTE : paste(ui); break;
				case TOP : top(ui); break;
				case BOTTOM : bottom(ui); break;
				case SHELL : shell(argsp); break;
				case RENAME : rename(argsp, ui); break;
				case EXTRACT : extract(argsp, ui); break;
				case COMPRESS : compress(argsp, ui); break;
			}
			executed = true;
		}

		if(i == command_map.size() - 1 && !executed && args[0] != "") {
			ui->set_error_message("Command \"" + args[0] + "\"" + " not found.");
		}
	}

	load({"main"}, ui);
	ui->bound_selected();
	load({"preview"}, ui);
}
