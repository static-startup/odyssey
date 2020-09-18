# ifndef CONFIG_H
# define CONFIG_H

/* draw window border */
static constexpr bool draw_border = false;

/* sets the decimal precision of double */
static constexpr int size_precision = 2;
static constexpr int free_size_precision = 1;

/* amount of spaces for selected element */
static constexpr int selected_space_size = 2;

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
	{ "q",          QUIT },
	{ "down",       DOWN },
	{ "up",         UP },
	{ "set",        SET },
	{ "load",       LOAD },
	{ "get",        GET },
	{ "cd",         CD },
	{ "hidden",     HIDDEN },
	{ "mkdir",      MKDIR },
	{ "open",       OPEN },
	{ "mv",         MOVE },
	{ "bmv",        BMOVE },
	{ "emv",        EMOVE },
	{ "rn",         RENAME },
	{ "rm",         REMOVE },
	{ "touch",      TOUCH },
	{ "select",     SELECT },
	{ "cp",         COPY },
	{ "cpdir",      COPYDIR },
	{ "paste",      PASTE },
	{ "top",        TOP },
	{ "bottom",     BOTTOM },
	{ "sh",         SHELL },
	{ "extract",    EXTRACT },
	{ "compress",   COMPRESS },
};

/* map a key to a command */
std::vector<event> event_map = {
	{ 'q',   -1,      "q" },
	{ 'j',   -1,      "down" },
	{ 'k',   -1,      "up" },
	{ ':',   -1,      "get -1" },
	{ 'l',   -1,      "open" },
	{ 'h',   -1,      "cd .." },
	{ '.',   -1,      "hidden" },
	{ 'd',   -1,      "get 6 mkdir " },
	{ 'm',   -1,      "mv"},
	{ 'A',   -1,      "emv" },
	{ 'I',   -1,      "bmv" },
	{ 'r',   -1,      "rn" },
	{ 'd',   -1,      "rm" },
	{ ' ',   -1,      "select" },
	{ 'c',   -1,      "cp" },
	{ 'D',   -1,      "cpdir" },
	{ 'p',   -1,      "paste" },
	{ 't',   -1,      "get 6 touch " },
	{ 'G',   -1,      "bottom" },
	{ 'g',   'g',     "top" },
	{ 'g',   'h',     "cd /home/static-startup" },
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
	{ ".svg",       "sxiv {f} > /dev/null 2>&1" },
	{ ".eps",       "sxiv {f} > /dev/null 2>&1" },
	{ ".ai",        "sxiv {f} > /dev/null 2>&1" },
	
	/* videos */
	{ ".mkv",       "mpv {f} > /dev/null 2>&1" },
	{ ".flv",       "mpv {f} > /dev/null 2>&1" },
	{ ".ogv",       "mpv {f} > /dev/null 2>&1" },
	{ ".ogg",       "mpv {f} > /dev/null 2>&1" },
	{ ".gif",       "mpv {f} > /dev/null 2>&1" },
	{ ".avi",       "mpv {f} > /dev/null 2>&1" },
	{ ".ts",        "mpv {f} > /dev/null 2>&1" },
	{ ".mts",       "mpv {f} > /dev/null 2>&1" },
	{ ".mov",       "mpv {f} > /dev/null 2>&1" },
	{ ".wmv",       "mpv {f} > /dev/null 2>&1" },
	{ ".mov",       "mpv {f} > /dev/null 2>&1" },
	{ ".amv",       "mpv {f} > /dev/null 2>&1" },
	{ ".mp4",       "mpv {f} > /dev/null 2>&1" },
	{ ".m4p",       "mpv {f} > /dev/null 2>&1" },
	{ ".m4v",       "mpv {f} > /dev/null 2>&1" },
	{ ".mpg",       "mpv {f} > /dev/null 2>&1" },
	{ ".mpeg",      "mpv {f} > /dev/null 2>&1" },
	{ ".mpv",       "mpv {f} > /dev/null 2>&1" },

	/* audios */
	{ ".wav",       "mpv {f} > /dev/null 2>&1" },
	{ ".aiff",      "mpv {f} > /dev/null 2>&1" },
	{ ".au",        "mpv {f} > /dev/null 2>&1" },
	{ ".m4a",       "mpv {f} > /dev/null 2>&1" },
	{ ".flac",      "mpv {f} > /dev/null 2>&1" },
	{ ".mp3",       "mpv {f} > /dev/null 2>&1" },
	{ ".aac",       "mpv {f} > /dev/null 2>&1" },

	/* ebooks */
	{ ".epub",      "ebook-viewer {f} > /dev/null 2>&1" },
	{ ".pdf",       "ebook-viewer {f} > /dev/null 2>&1" },
	{ ".mobi",      "ebook-viewer {f} > /dev/null 2>&1" },
	{ ".aws",       "ebook-viewer {f} > /dev/null 2>&1" },
};

# endif
