# ifndef COMMANDS_H
# define COMMANDS_H

class commands {
	private:

		/* private helper functions */

		static bool is_digit(std::string number);
		static std::string combine_vector(std::vector<std::string> vector);
		static void wipe_elements(user_interface *ui);
		
	public:

		/* public helper functions */

		static std::string file_last_mod_time(std::string directory);
		static std::string format_file_size(unsigned long file_size, int precision);
		static unsigned long file_sizes(std::string directory);
		static unsigned long file_size(std::string directory);
		static int directory_items(std::string directory);
		static std::string file_permissions(std::string directory);
		static std::string file_owner(std::string directory);
		static unsigned long free_space(std::string directory);
		static std::string find_and_replace(std::string str, std::string search, std::string replace);

		/* main functions */

		static void quit(std::vector<std::string> args);
		static void up(user_interface *ui);
		static void down(user_interface *ui);
		static void set(std::vector<std::string> args, user_interface *ui);
		static void load(std::vector<std::string> args, user_interface *ui);
		static std::string get(std::vector<std::string> args, int drawx, bool locked, user_interface *ui);
		static void hidden(user_interface *ui);
		static void cd(std::vector<std::string> args, user_interface *ui);
		static void mkdir(std::vector<std::string> args, user_interface *ui);
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
		static void copy_directory(user_interface *ui);
		static void paste(user_interface *ui);
		static void top(user_interface *ui);
		static void bottom(user_interface *ui);
		static void shell(std::vector<std::string> args);
		static void extract(std::vector<std::string> args, user_interface *ui);
		static void compress(std::vector<std::string> args, user_interface *ui);
		static void process_command(std::string command, user_interface *ui);
};

# endif
