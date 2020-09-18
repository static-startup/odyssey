# ifndef COMMANDS_H
# define COMMANDS_H

class commands {
	public:

		static void quit(std::vector<std::string> args);
		static void up(user_interface *ui);
		static void down(user_interface *ui);
		static void set(std::vector<std::string> args, user_interface *ui);
		static void load(std::vector<std::string> args, user_interface *ui);
		static void get(std::vector<std::string> args, user_interface *ui);
		static void hidden(user_interface *ui);
		static void cd(std::vector<std::string> args, user_interface *ui);
		static std::string mkdir(std::vector<std::string> args, user_interface *ui);
		static void open(std::vector<std::string> args, user_interface *ui);
		static void move_file(std::vector<std::string> args, user_interface *ui);
		static void begin_move(std::vector<std::string> args, user_interface *ui);
		static void end_move(std::vector<std::string> args, user_interface *ui);
		static void rename(std::vector<std::string> args, user_interface *ui);
		static void remove(std::vector<std::string> args, user_interface *ui);
		static void remove_all(std::vector<std::string> args, user_interface *ui);
		static void touch(std::vector<std::string> args, user_interface *ui);
		static void select(std::vector<std::string> args, user_interface *ui);
		static void copy(std::vector<std::string> args, user_interface *ui);
		static void copy_all(std::vector<std::string> args, user_interface *ui);
		static void copy_directory();
		static void paste(user_interface *ui);
		static void top(user_interface *ui);
		static void bottom(user_interface *ui);
		static void shell(std::vector<std::string> args);
		static void extract(std::vector<std::string> args, user_interface *ui);
		static void process_command(std::string command, user_interface *ui);
};

# endif
