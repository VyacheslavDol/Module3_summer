# Указываем компилятор
CC = gcc

# Исходные и объектные файлы
SER = server
CLI = client
SRC_DIR = ./source
OBJ_DIR = ./object_f
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Правило по умолчанию
all: $(SER) $(CLI)

$(SER): $(OBJS)
	$(CC) $(filter-out $(OBJ_DIR)/main_$(CLI).o, $(OBJS)) -o $(SER)

$(CLI): $(OBJS)
	$(CC) $(filter-out $(OBJ_DIR)/main_$(SER).o, $(OBJS)) -o $(CLI)

# Правило компиляции
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@

# Чистка объектных файлов
clean:
	rm -f $(OBJ_DIR)/*.o $(SER) $(CLI) file_client.txt file_client