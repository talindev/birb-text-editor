/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

/*** data ***/

// initializes the default terminal struct, empty
struct termios orig_termios;

/*** terminal ***/

// detects the errno global variable and prints also some custom text with it, also exits
// the program with an failure code
void die(const char *s) {
	perror(s);
	exit(1);
}

// disables raw mode, sets the terminal to default config
void disableRawMode(void) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

// enables raw mode through flags
// copies the original terminal config, turns on/off some specific flags
//
// INPUT FLAGS (_iflag)
// BRKINT sends a SIGINT signal when reaching break condition
// ICRNL disables Ctrl + M (carriage return, new line + reset cursor)
// INPCK enables parity checking
// ISTRIP causes the 8th bit of each input byte to be 0 (stripped)
// IXON disables data transmission flow control through Ctrl + S and Ctrl + Q
//
// OUTPUT FLAGS (_oflag)
// OPOST turns off all output processing features
//
// LOCAL FLAGS (_lflag)
// ECHO causes the key you type to be printed into the terminal
// ICANON allows byte-by-byte reading, instead of line-by-line. turns off canonical mode
// IEXTEN disables Ctrl + V and Ctrl + O (macOS)
// ISIG disables Ctrl + C (SIGINT signal) and Ctrl + Z (SIGTSTP signal)
//
// also we set VMIN (minimum number of bytes of input before read() can return)
// and also VTIME (maximum amount of time to wait before read() returns)
// saving this into the raw struct we initialized through tcsetattr()
void enableRawMode(void) {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/*** init ***/

// in the main loop, we read what was typed by the user and print it out
int main(void) {
	enableRawMode();
	
	while (1) {
		char c = '\0';
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
		if (iscntrl(c)) {
			printf("%d\r\n", c);
		} else {
			printf("%d ('%c')\r\n", c, c);
		}
		if (c == 'q') break;
	}
	return 0;
}
