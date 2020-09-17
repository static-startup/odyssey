CC = g++
LIBS = -lncurses -lboost_system -lboost_filesystem -lstdc++fs

install core.cpp:
	${CC} core.cpp ${LIBS} -o odyssey
	sudo mv ./odyssey /usr/bin/

uninstall:
	sudo rm /usr/bin/odyssey > /dev/null 2>&1 || echo -e "\e[31modyssey not installed.\e[0m"; exit 0
