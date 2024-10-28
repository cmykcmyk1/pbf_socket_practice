## Тестовое задание по сокетам для PBF

Два простых консольных приложения: TCP-сервер и TCP-клиент.  
Программы написаны на C++.  
Система сборки: CMake.  

### Сборка

Выполните в терминале:
```
git clone https://github.com/cmykcmyk1/pbf_socket_practice
cd pbf_socket_practice
mkdir build
cd build
cmake ..
make

``` 

### Запуск

```
// Usage: server <port>
// Usage: client <client_name> <server_port> <timeout>

./server 16000

./client Name1 16000 1
./client Name2 16000 2
./client Name3 16000 3

```
