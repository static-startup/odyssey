# ifndef CONFIG_H
# define CONFIG_H

/* draw border on windows or not */
static constexpr bool draw_border = false;

/* sets the decimal precision for file sizes */
static constexpr int size_precision = 2;
static constexpr int free_size_precision = 1;

/* specify the directory on launch */
static const std::string starting_directory = "/home/static-startup";

/* show hidden files or not */
static bool show_hidden = false;

/* use terminal set colors (supports transparency) */
static constexpr bool use_terminal_colors = true;

/* color terminal or not */
static constexpr bool color_terminal = true;

/* map a name to a command */
static const std::vector<command> command_map = {
	{ "q",          EXIT },
	{ "x",          EXIT },
	{ "quit",       EXIT },
	{ "exit",       EXIT },

	{ "down",       DOWN },
	{ "up",         UP },
	{ "set",        SET },

	/* { "load",       LOAD }, */
	{ "get",        GET },
	{ "cd",         CD },

	{ "hidden",     HIDD },
	{ ".",          HIDD },

	{ "mkdir",      MKDIR },
	{ "open",       OPEN },
	{ "rename",     RENAME },
	{ "brename",    BRENAME },
	{ "erename",    ERENAME },

	{ "rm",         REMOVE },
	{ "rrm",        RREMOVE },

	{ "touch",      TOUCH },
	{ "select",     SELECT },

	{ "cp",         COPY },
	{ "rcp",        RCOPY },
	{ "cpdir",      COPYDIR },

	{ "paste",      PASTE },
};

/* map a key to a command */
std::vector<event> event_map = {
	{ 'q',      "q" },
	{ 'j',      "down" },
	{ 'k',      "up" },
	{ ':',      "get -1" },
	{ 'l',      "open" },
	{ 'h',      "cd .." },
	{ '.',      "hidden" },
	{ 'm',      "get -1 mkdir " },
	{ 'r',      "rename"},
	{ 'A',      "erename" },
	{ 'I',      "brename" },
	{ 'd',      "rm" },
	{ 'D',      "rrm" },
	{ ' ',      "select" },
};

/* map file type to color */
static const std::vector<colors> colors_map = {
	/* c++ */
	{ ".c++",      CYAN|BRIGHT },
	{ ".cpp",      CYAN|BRIGHT },
	{ ".cc",       CYAN|BRIGHT },
	{ ".h++",      CYAN|BRIGHT },
	{ ".hpp",      CYAN|BRIGHT },
	{ ".h",        CYAN|BRIGHT },

	/* python */
	{ ".py",       GREEN|BRIGHT },

	/* images */
	{ ".jpg",      YELLOW|BRIGHT },
	{ ".jpeg",     YELLOW|BRIGHT },
	{ ".png",      YELLOW|BRIGHT },
	{ ".gif",      YELLOW|BRIGHT },
	{ ".tiff",     YELLOW|BRIGHT },
	{ ".tif",      YELLOW|BRIGHT },
	{ ".raw",      YELLOW|BRIGHT },
	{ ".bmp",      YELLOW|BRIGHT },

	/* vector */
	{ ".svg",      YELLOW|BRIGHT },
	{ ".eps",      YELLOW|BRIGHT },
	{ ".ai",       YELLOW|BRIGHT },

	/* videos */
	{ ".mkv",      BLUE },
	{ ".flv",      BLUE },
	{ ".ogv",      BLUE },
	{ ".ogg",      BLUE },
	{ ".gif",      BLUE },
	{ ".avi",      BLUE },
	{ ".ts",       BLUE },
	{ ".mts",      BLUE },
	{ ".mov",      BLUE },
	{ ".wmv",      BLUE },
	{ ".mov",      BLUE },
	{ ".amv",      BLUE },
	{ ".mp4",      BLUE },
	{ ".m4p",      BLUE },
	{ ".m4v",      BLUE },
	{ ".mpg",      BLUE },
	{ ".mpeg",     BLUE },
	{ ".mpv",      BLUE },

	/* audios */
	{ ".wav",      CYAN },
	{ ".aiff",     CYAN },
	{ ".au",       CYAN },
	{ ".m4a",      CYAN },
	{ ".flac",     CYAN },
	{ ".mp3",      CYAN },
	{ ".aac",      CYAN },

	/* tar and zip */
	{ ".zip",      RED|BRIGHT },
	{ ".tar",      RED|BRIGHT },
	{ ".gz",       RED|BRIGHT },
	{ ".bz2",      RED|BRIGHT },

	/* web files */
	{ ".html",     MAGENTA|BRIGHT },
	{ ".css",      MAGENTA|BRIGHT },
	{ ".php",      MAGENTA|BRIGHT },
	{ ".rb",       MAGENTA|BRIGHT },

	/* storage files */
	{ ".epub",     YELLOW },
	{ ".pdf",      YELLOW },
	{ ".mobi",     YELLOW },
	{ ".aws",      YELLOW },

	/* directory */
	{ "dir",       BLUE|BRIGHT },

	/* default */
	{ "",          WHITE },
};

static const std::vector<open> open_map = {
	/* images */
	{ ".jpg",       "sxiv {f} > /dev/null 2>&1" },
	{ ".jpeg"       "sxiv {f} > /dev/null 2>&1" },
	{ ".png",       "sxiv {f} > /dev/null 2>&1" },
	{ ".gif",       "sxiv {f} > /dev/null 2>&1" },
	{ ".tiff",      "sxiv {f} > /dev/null 2>&1" },
	{ ".tif",       "sxiv {f} > /dev/null 2>&1" },
	{ ".raw",       "sxiv {f} > /dev/null 2>&1" },
	{ ".bmp",       "sxiv {f} > /dev/null 2>&1" },

	/* vectors */
	{ ".svg",       "sxiv {f}" },
	{ ".eps",       "sxiv {f}" },
	{ ".ai",        "sxiv {f}" },
	
	/* videos */
	{ ".mkv",       "mpv {f}" },
	{ ".flv",       "mpv {f}" },
	{ ".ogv",       "mpv {f}" },
	{ ".ogg",       "mpv {f}" },
	{ ".gif",       "mpv {f}" },
	{ ".avi",       "mpv {f}" },
	{ ".ts",        "mpv {f}" },
	{ ".mts",       "mpv {f}" },
	{ ".mov",       "mpv {f}" },
	{ ".wmv",       "mpv {f}" },
	{ ".mov",       "mpv {f}" },
	{ ".amv",       "mpv {f}" },
	{ ".mp4",       "mpv {f}" },
	{ ".m4p",       "mpv {f}" },
	{ ".m4v",       "mpv {f}" },
	{ ".mpg",       "mpv {f}" },
	{ ".mpeg",      "mpv {f}" },
	{ ".mpv",       "mpv {f}" },

	/* audios */
	{ ".wav",       "mpv {f}" },
	{ ".aiff",      "mpv {f}" },
	{ ".au",        "mpv {f}" },
	{ ".m4a",       "mpv {f}" },
	{ ".flac",      "mpv {f}" },
	{ ".mp3",       "mpv {f}" },
	{ ".aac",       "mpv {f}" },

	/* ebooks */
	{ ".epub",      "calibre {f}" },
	{ ".pdf",       "calibre {f}" },
	{ ".mobi",      "calibre {f}" },
	{ ".aws",       "calibre {f}" },
};

# endif
