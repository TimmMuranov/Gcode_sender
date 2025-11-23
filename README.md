# Gcode_sender
Отправляет gcode из файла в usb порт.
# Установка
0) Скачайте git и компилятор с++ (я использую g++):
```
sudo apt update
sudo apt install git g++
```
1) Скачайте репозиторий:
```
git clone https://github.com/TimmMuranov/Gcode_sender
cd Gcode_sender
```
2) Скомпилируйте код:
```
g++ Gcode_sender -o gcode_sender
```
# Использование
0) Подключите чпу станок к компьютеру и убедитесь, что в папке `/dev` появился файл типа `/dev/ttyUSBx`. Это виртуальный порт чпу станка, на который мы будем отправлять gcode.
1) Откройте виртуальный порт станка для записи:
```
sudo chmod +777 /dev/ttyUSBx
```
2) Запустите скомпилированный файл со следующим синтаксисом:
```
./gcode_sender path/to/file.gcode /dev/ttyUSBx
```
*3) Добавьте скомпилированный файл в папку с бинарными файлами.<p>
```
sudo cp gcode_sender /usr/bin
```
Этот шаг позволит вызывать утилиту из любой директории.<p>
