Задание 03

Скорректировать решение задачи о книге контактов из модуля 2 так, чтобы список контактов хранился в файле.
Использовать небуферизованный ввод-вывод. При запуске программы список считывается из файла,
при завершении программы список записывается в файл. Учесть, что при запуске программы может не быть
сохраненных данных.

Задание из 2го модуля: Сделать контактную книгу, для хранения данных использовать двухсвязный список. Функции поместить в динамическую библиотеку.

В программе реализовно хранение контактной книги в файле, с использованием небуфферизованного ввода-вывода
При запуске программы открывается(создается) файл для чтения данных контактной книги contact_book, и соответственно записывается контактная книга в программу.
При выходе из программы происходит очистка файла и перезаписыввание данных контакнтой книги.

Запись в файл реализвоана с помощью отдельной функции write_book(), которая находится в файле реализации /library/source/list_func.c
Чтение из файла реализорвано алгоритмом в функции main()

Для сборки необходимо собрать динамическую библиотеку (в ./library прописать make), затем скомипилировать основную программу (в ./project make)
Для удаления книги (./project make clean_book)


#Комментарии к заданию 2го модуля:

Задание 6.2 2 модуль


Задание 4.1 реализовано с созданием динамической библиотеки /6.2/library/libmy.so , для ее создания создан /6.2/library/Makefile,
в каталоге /6.2/library/source находятся исходники исполняемых файлов для функций.
В каталоге /6.2/project находится главный файл с основным кодом task4_1.c и Makefile для создания исполняемого файла task6_2, 
который в при компиляции использует статитескую библиотеку.

Объектные файлы для сборки библиотеки удаляются автоматически после сборки библиотеки.

Контактная книга реализована в двухсвязном нециклическом списке.
Контакты сортируются по алфавиту (по именам)
В файле list_func описаны функции работы с эелемнетами списка

Пусть каждое имя (без фамилия) уникально, то есть не существует двух контактов с одинаковыми firstname.