Задание 07 (Очереди сообщений POSIX)
Решено аналогичное задание из весенней школы (Задание 3.5)

Персональный чат между двумя процессами с помощью очереди сообщений POSIX

Реализован персональный чат с помощью 2х очередей сообщений POSIX между двумя программами А и B, которые создают дочерние процессы, отвечающие за чтение сообщений из "чата",
а родительские процессы отвечают за отправку сообщений в чат.
Использование двух очредей обусловлено отсутствием возможности выбирать сообщение по приоритету (Особенность очереди сообщений POSIX), 
и в любом случае извлекается сообщение с наивысшим приоритетом.
Таким образом, род процесс А (отправка) и доч процесс В (чтение) подключены к очереди1, и противоположные процессы подключены к очереди2.
Закрытие чата происходит в случае, если один из собеседников отправляет слово "пока"

PS: Запуск производить в двух терминалах