void commands::up(user_interface *ui) {
	set({std::to_string(ui->get_selected()[0])}, ui);
}

void commands::down(user_interface *ui) {
	set({std::to_string(ui->get_selected()[0] + 2)}, ui);
}

void commands::set(std::vector<std::string> args, user_interface *ui) {
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

void commands::quit(std::vector<std::string> args) {
	endwin();
	for(int i = 0; i < args.size(); i++) {
		std::cout << args[i] << std::endl;
	}
	exit(0);
}

void commands::get(std::vector<std::string> args, user_interface *ui) {
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

void commands::load(std::vector<std::string> args, user_interface *ui) {
	ui->clear_windows();
	std::vector<std::string> elements = {};
	std::vector<std::string> sizes = {};

	if(args.size() != 1) {
		return;
	}

	if(args[0] == "main") {
		std::string current_directory = boost::filesystem::current_path().string();

		if(boost::filesystem::exists(current_directory)) {
			for(const auto &entry : boost::filesystem::directory_iterator(current_directory)) {
				std::string filename = entry.path().string();
				filename = filename.substr(filename.find_last_of("/") + 1, filename.length());
				if((filename[0] != '.' && !show_hidden) || (show_hidden)) {
					if(boost::filesystem::is_directory(
							boost::filesystem::absolute(current_directory + "/" + filename))) {

						filename += "/";
						sizes.push_back(std::to_string(
									ui->get_filenames(entry.path().string()).size()));
					} else {
						sizes.push_back(ui->get_file_size(entry.path().string()));
					}

					elements.push_back(filename);
				}
			}
		}

		ui->set_main_elements(elements);
		ui->set_main_sizes(sizes);
	} else if(args[0] == "preview") {
		std::string selected_filename = ui->get_main_elements()[ui->get_selected()[0]];
		if(boost::filesystem::exists(selected_filename)) {
			if(boost::filesystem::is_directory(selected_filename)) {
				for(const auto &entry : boost::filesystem::directory_iterator(selected_filename)) {
					std::string filename = entry.path().string();
					filename = filename.substr(filename.find_last_of("/") + 1, filename.length());
					if((filename[0] != '.' && !show_hidden) || (show_hidden)) {
						if(boost::filesystem::is_directory(
									boost::filesystem::absolute(selected_filename + "/" + filename))) {

							filename += "/";
							sizes.push_back(std::to_string(
										ui->get_filenames(entry.path().string()).size()));
						} else {
							sizes.push_back(ui->get_file_size(entry.path().string()));
						}

						elements.push_back(filename);
					}
				}
			} else {
				std::ifstream read(selected_filename);
				std::string line;
				while(std::getline(read, line)) {
					elements.push_back(line);
				}
				read.close();
			}
		}

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
				load({"main"}, ui);

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

	if(!ui->get_main_elements().empty()) {
		load({"preview"}, ui);
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::hidden(user_interface *ui) {
	show_hidden = !show_hidden;
	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::mkdir(std::vector<std::string> args, user_interface *ui) {
	for(int i = 0; i < args.size(); i++) {
		if(!boost::filesystem::exists(args[i])) {
			boost::filesystem::create_directory(
					boost::filesystem::weakly_canonical(args[i]));
		}
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
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

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::move_file(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		if(ui->get_selected().size() == 1) {
			boost::filesystem::path selected_filename(ui->get_main_elements()[ui->get_selected()[0]]);
			get({"4", "mv \"", selected_filename.extension().string(), "\""}, ui);
		} else {
			std::vector<int> selected = ui->get_selected();
			std::string selected_filename = ui->get_main_elements()[selected[0]];
			if(boost::filesystem::is_directory(selected_filename)) {
				for(int i = 1; i < selected.size(); i++) {
					move_file({ui->get_main_elements()[selected[i]], selected_filename}, ui);
				}
			}
		}
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

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::begin_move(std::vector<std::string> args, user_interface *ui) {
	get({"4", args[0], "mv \"", ui->get_main_elements()[ui->get_selected()[0]], "\""}, ui);
}

void commands::end_move(std::vector<std::string> args, user_interface *ui) {
	boost::filesystem::path selected_filename(
				ui->get_main_elements()[ui->get_selected()[0]]);

	int begin_at = 4 + (selected_filename.string().length() -
			selected_filename.extension().string().length());

	get({std::to_string(begin_at + 1), "mv \"", selected_filename.string(), "\""}, ui);
}

void commands::remove(std::vector<std::string> args, user_interface *ui) {
	if(args.size() == 0) {
		std::vector<int> selected = ui->get_selected();
		for(int i = 1; i < selected.size(); i++) {
			remove({ui->get_main_elements()[selected[i]]}, ui);
		}
	} else {
		for(int i = 0; i < args.size(); i++) {
			if(boost::filesystem::exists(args[i])) {
				boost::filesystem::remove_all(boost::filesystem::canonical(args[i]));
			}
		}
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::touch(std::vector<std::string> args, user_interface *ui) {
	for(int i = 0; i < args.size(); i++) {
		if(!boost::filesystem::exists(args[i])) {
			system(std::string("vim \""
						+ boost::filesystem::weakly_canonical(args[i]).string() + "\"").c_str());
		}
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
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

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
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

	while(std::getline(stream, line, '\n')) {
		copy({line, boost::filesystem::current_path().string()
				+ "/" + line.substr(line.find_last_of("/") + 1, line.length())}, ui);
	}

	ui->set_selected(std::vector<int>{ui->get_selected()[0]});
}

void commands::top(user_interface *ui) {
	if(!ui->get_main_elements().empty()) {
		set({std::to_string(ui->get_main_elements().size())}, ui);
	}
}

void commands::bottom(user_interface *ui) {
	if(!ui->get_main_elements().empty()) {
		set({"1"}, ui);
	}
}

void commands::shell(std::vector<std::string> args) {
	std::string command;

	for(std::string element : args) {
		command += element + " ";
	}

	system(command.c_str());
}

void commands::process_command(std::string command, user_interface *ui) {
	std::vector<std::string> args = ui->split_into_args(command);
	std::vector<std::string> argsp = std::vector<std::string>(args.begin() + 1, args.end());

	for(int i = 0; i < command_map.size(); i++) {
		if(command_map[i].name == args[0]) {
			switch(command_map[i].command) {
				case QUIT     : quit(argsp); break;
				case DOWN     : down(ui); break;
				case UP       : up(ui); break;
				case LOAD     : load(argsp, ui); break;
				case GET      : get(argsp, ui); break;
				case CD       : cd(argsp, ui); break;
				case SET      : set(argsp, ui); break;
				case HIDDEN   : hidden(ui); break;
				case MKDIR    : mkdir(argsp, ui); break;
				case OPEN     : open(argsp, ui); break;
				case MOVE     : move_file(argsp, ui); break;
				case BMOVE    : begin_move(argsp, ui); break;
				case EMOVE    : end_move(argsp, ui); break;
				case REMOVE   : remove(argsp, ui); break;
				case TOUCH    : touch(argsp, ui); break;
				case SELECT   : select(argsp, ui); break;
				case COPY     : copy(argsp, ui); break;
				case COPYDIR  : copy_directory(); break;
				case PASTE    : paste(ui); break;
				case TOP      : top(ui); break;
				case BOTTOM   : bottom(ui); break;
				case SHELL    : shell(argsp); break;
			}
		}
	}

	load({"main"}, ui);
	ui->bound_selected();
}
