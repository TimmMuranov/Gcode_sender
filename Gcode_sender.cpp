
#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

bool configureSerial(int fd, speed_t baudrate = B115200) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return false;
    }

    cfsetospeed(&tty, baudrate);
    cfsetispeed(&tty, baudrate);

    tty.c_cflag &= ~PARENB;     // No parity
    tty.c_cflag &= ~CSTOPB;     // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         // 8 bits
    tty.c_cflag &= ~CRTSCTS;    // No flow control
    tty.c_cflag |= CREAD | CLOCAL; // enable receiver, local mode

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw input
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
    tty.c_oflag &= ~OPOST; // raw output

    // Set read timeout
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 20; // 2 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <gcode_file> <serial_device>" << endl;
        return 1;
    }

    const char* filename = argv[1];
    const char* serialPort = argv[2];

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 2;
    }

    int fd = open(serialPort, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Error opening serial port");
        return 3;
    }

    if (!configureSerial(fd)) {
        close(fd);
        return 4;
    }

    // Даем Arduino время перезагрузиться после открытия порта (обычно)
    sleep(2);

    string line;
    char buf[256];
    ssize_t n;

    while (getline(file, line)) {
        // Игнорируем пустые строки и комментарии (начинаются с ';')
        if (line.empty() || line[0] == ';')
            continue;

        line += "\n";
        ssize_t to_write = line.size();
        const char* data = line.c_str();

        // Отправляем команду
        ssize_t written = write(fd, data, to_write);
        if (written != to_write) {
            cerr << "Error writing to serial port" << endl;
            break;
        }

        // Читаем ответ, ожидаем строку с "ok" или ошибку
        string response;
        while (true) {
            n = read(fd, buf, sizeof(buf) - 1);
            if (n < 0) {
                perror("read");
                break;
            } else if (n == 0) {
                // Таймаут, выходим из цикла чтения, возможно ошибка ответа
                break;
            } else {
                buf[n] = '\0';
                response += buf;
                // Проверяем, пришёл ли полный ответ с "ok" или "error"
                if (response.find("ok") != string::npos || response.find("error") != string::npos)
                    break;
            }
        }

        cout << "Sent: " << line;
        cout << "Response: " << response << endl;
    }

    close(fd);
    file.close();

    return 0;
}
